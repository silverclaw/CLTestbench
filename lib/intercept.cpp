// This file is part of CLTestbench.

// CLTestbench is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CLTestbench is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include <CL/cl.h>
#include <CL/cl_icd.h>

#include "driver.hpp"

using namespace CLTestbench;

namespace
{
const char* getLibraryName()
{
    // TODO: check getenv() for a library to load.
    return "libOpenCL.so";
}

class DriverLoader
{
    Library mLibrary;
    cl_icd_dispatch mICDTable;

public:
    DriverLoader() : mLibrary(getLibraryName())
    {
        #define BindFn(fnname) \
            mICDTable.fnname = mLibrary.bind<decltype(mICDTable.fnname)>(#fnname)
        BindFn(clBuildProgram);
        BindFn(clCompileProgram);
        BindFn(clCreateBuffer);
        BindFn(clCreateCommandQueue);
        BindFn(clCreateContext);
        BindFn(clCreateProgramWithSource);
        BindFn(clCreateKernel);
        BindFn(clEnqueueNDRangeKernel);
        BindFn(clFinish);
        BindFn(clGetDeviceIDs);
        BindFn(clGetDeviceInfo);
        BindFn(clGetKernelArgInfo);
        BindFn(clGetPlatformIDs);
        BindFn(clGetPlatformInfo);
        BindFn(clGetProgramBuildInfo);
        BindFn(clReleaseCommandQueue);
        BindFn(clReleaseContext);
        BindFn(clReleaseKernel);
        BindFn(clReleaseMemObject);
        BindFn(clReleaseProgram);
        BindFn(clRetainCommandQueue);
        BindFn(clRetainContext);
        BindFn(clRetainKernel);
        BindFn(clRetainMemObject);
        BindFn(clRetainProgram);
        BindFn(clSetKernelArg);

        #undef BindFn
    }

    const cl_icd_dispatch& getICD() const noexcept
    {
        return mICDTable;
    }
};

/// Retrieves the list of functions for the loaded driver.
const cl_icd_dispatch& getDriverICD()
{

    static DriverLoader icd;
    return icd.getICD();
}
} // end anon namespace

struct _cl_context
{
    const cl_icd_dispatch* const mDispatchTable;
    const cl_context object;
    const uint8_t platformIndex;
    const uint8_t deviceIndex;
    std::unique_ptr<std::ostream> mOut;
    /// Number of programs compiled.
    std::atomic<uint32_t> mProgramCount{0};
    /// Number of kernels created.
    std::atomic<uint32_t> mKernelCount{0};
    /// Number of memory objects created.
    std::atomic<uint32_t> mBufferCount{0};
    std::atomic<uint32_t> mRefCount{1};
    std::vector<std::unique_ptr<_cl_command_queue>> mQueues;
    std::vector<std::unique_ptr<_cl_mem>> mBuffers;
    std::vector<std::unique_ptr<_cl_program>> mPrograms;
    std::vector<std::unique_ptr<_cl_kernel>> mKernels;

    std::mutex mLock;

    explicit _cl_context(cl_context data, uint8_t platform, uint8_t device) :
        mDispatchTable(&getDriverICD()), object(data),
        platformIndex(platform), deviceIndex(device),
        // TODO: generate a filename.
        mOut(new std::ofstream("CLIntercept.txt"))
    {
    }

    cl_command_queue wrap(cl_command_queue queue);
    cl_program wrap(std::string& name, cl_program program);
    cl_kernel wrap(std::string& name, cl_kernel kernel);
    cl_mem wrap(std::string&& name, cl_mem buffer);

    std::unique_lock<std::mutex> lock()
    {
        return std::unique_lock<std::mutex>(mLock);
    }
};

#define DeclareCLObject(X) \
    struct _##X { \
        const cl_icd_dispatch* const mDispatchTable; \
        const std::string name; \
        const cl_context parent; \
        const X object; \
        explicit _##X(std::string& objName, cl_context context, X data) : \
            mDispatchTable(context->mDispatchTable), name(std::move(objName)), \
            parent(context), object(data) \
        { } \
    }

DeclareCLObject(cl_event);
DeclareCLObject(cl_kernel);
DeclareCLObject(cl_mem);
DeclareCLObject(cl_program);
DeclareCLObject(cl_sampler);

#undef DeclareCLObject

struct _cl_command_queue
{
    const cl_icd_dispatch* const mDispatchTable;
    const cl_command_queue object;
    const cl_context parent;

    explicit _cl_command_queue(cl_command_queue data, cl_context context) :
        mDispatchTable(context->mDispatchTable), object(data), parent(context)
    {}
};

cl_command_queue _cl_context::wrap(cl_command_queue queue)
{
    auto l = lock();
    mQueues.emplace_back(std::make_unique<_cl_command_queue>(queue, this));
    return mQueues.back().get();
}

cl_program _cl_context::wrap(std::string& name, cl_program program)
{
    auto l = lock();
    mPrograms.emplace_back(std::make_unique<_cl_program>(name, this, program));
    return mPrograms.back().get();
}

cl_kernel _cl_context::wrap(std::string& name, cl_kernel kernel)
{
    auto l = lock();
    mKernels.emplace_back(std::make_unique<_cl_kernel>(name, this, kernel));
    return mKernels.back().get();
}

cl_mem _cl_context::wrap(std::string&& name, cl_mem buffer)
{
    auto l = lock();
    mBuffers.emplace_back(std::make_unique<_cl_mem>(name, this, buffer));
    return mBuffers.back().get();
}

namespace
{
/// Issues a script comment
struct Comment
{
    std::string_view comment;
    // Use any constructor valid for string_view.
    template<typename T> Comment(const T& C) : comment(C) {}
};

void operator<<(_cl_context& C, const Comment& comment)
{
    // We don't want to return the output stream because
    // when this function exits, the mutex is released.
    auto lock = C.lock();
    (*C.mOut) << "# " << comment.comment << '\n';
}

/// Bind a kernel argument.
struct BindArg
{
    cl_kernel kernel = nullptr;
    cl_uint argIndex = 0;
    cl_mem buffer = nullptr;
    const void* data = nullptr;
    size_t dataSize = 0;
};

template<typename T>
struct CLVector
{
    const T* data;
    const cl_uint size;
    CLVector() = delete;
    CLVector(const void* ptr, cl_uint s) : data(static_cast<const T*>(ptr)), size(s) {}
    CLVector(const T* ptr, cl_uint s) : data(ptr), size(s) {}
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const CLVector<T>& V)
{
    out << V.data[0];
    for (cl_uint i = 1; i < V.size; ++i) {
        // uint8_t is an alias for char, and we don't want characters
        // printed out instead of numbers.  Upcast it to another integer size.
        if (sizeof(T) == 1) out << "," << static_cast<uint16_t>(V.data[i]);
        else out << ',' << V.data[i];
    }
    return out;
}

struct CLData
{
    const void* const ptr;
    const cl_uint size;
    CLData() = delete;
    CLData(const void* data, cl_uint s) : ptr(data), size(s) {}
};

std::ostream& operator<<(std::ostream& out, const CLData& data)
{
    if ((data.size % 8) == 0) {
        out << "ulong(" << CLVector<uint64_t>(data.ptr, data.size / 8);
    } else if ((data.size % 4) == 0) {
        out << "uint(" << CLVector<uint32_t>(data.ptr, data.size / 4);
    } else if ((data.size % 2) == 0) {
        out << "ushort(" << CLVector<uint16_t>(data.ptr, data.size / 2);
    } else {
        out << "uchar(" << CLVector<uint8_t>(data.ptr, data.size);
    }
    return out << ')';
}

void operator<<(_cl_context& C, const BindArg& bind)
{
    // We don't want to return the output stream because
    // when this function exits, the mutex is released.
    auto lock = C.lock();

    (*C.mOut) << "bind " << bind.kernel->name << ' ' << bind.argIndex << ' ';
    if (bind.buffer) {
        (*C.mOut) << bind.buffer->name << '\n';
        return;
    } // TODO other argument types.

    assert(bind.dataSize > 0);

    (*C.mOut) << CLData(bind.data, bind.dataSize) << '\n';
}

struct EnqueueKernel
{
    cl_kernel kernel = nullptr;
    cl_uint dim = 0;
    std::array<size_t, 3> globalSize{};
    std::array<size_t, 3> localSize{};
};

void operator<<(_cl_context& C, const EnqueueKernel& E)
{
    auto lock = C.lock();
    (*C.mOut) << "run " << E.kernel->name << "((" << CLVector<size_t>(E.globalSize.data(), E.dim);
    if (E.localSize[0] != 0) {
        (*C.mOut) << "),(" << CLVector<size_t>(E.localSize.data(), E.dim);
    }
    (*C.mOut) << "))\n";
}

struct CreateBuffer
{
    cl_mem buffer = nullptr;
    size_t size = 0;
    const void* data = nullptr;
    std::string filename;
};

void operator<<(_cl_context& C, const CreateBuffer& create)
{
    auto lock = C.lock();
    (*C.mOut) << create.buffer->name << " = buffer(";
    if (create.data) {
        (*C.mOut) << CLData(create.data, create.size);
    } else if (!create.filename.empty()) {
        (*C.mOut) << "file(" << create.filename << ')';
    } else {
        (*C.mOut) << create.size;
    }
    (*C.mOut) << ")\n";
}

template<typename T>
std::ostream& operator<<(_cl_context& C, const T& data)
{
    return (*C.mOut) << data;
}
} // anon namespace

#define EXPORT __attribute__((visibility("default")))

EXPORT CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties* properties, cl_uint num_devices,
                const cl_device_id* devices,
                void (CL_CALLBACK* pfn_notify)(const char* errinfo,
                                               const void* private_info,
                                               size_t cb, void* user_data),
                void* user_data, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    const auto& driver = getDriverICD();

    if (num_devices != 1) {
        // Multi-device context isn't supported by CL Testbench.
        std::cerr << "CLIntercept: Multi-device context is not supported\n";
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }

    cl_int error;
    cl_context created = driver.clCreateContext(properties, num_devices, devices, pfn_notify, user_data, &error);
    if (!created) {
        if (errcode_ret) *errcode_ret = error;
        return nullptr;
    }

    // The device being selected.
    cl_device_id device = devices[0];
    // The platform associated with the device.
    cl_platform_id platform = nullptr;
    size_t platformSize = sizeof(cl_platform_id);
    if (driver.clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform, &platformSize)
        != CL_SUCCESS) {
        // Couldn't get the owning platform.
        std::cerr << "CLIntercept: Cannot figure out the owning platform of the selected device\n.";
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }

    // Figure out the index of the selected platform;
    std::array<cl_platform_id, 16> platforms;
    cl_uint platformCount = 0;
    if (driver.clGetPlatformIDs(platforms.size(), platforms.data(), &platformCount) != CL_SUCCESS) {
        std::cerr << "CLIntercept: Cannot figure out the owning platform of the selected device\n.";
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }
    unsigned platformIndex = 0;
    for (; platformIndex < platformCount; ++platformIndex)
        if (platforms[platformIndex] == platform)
            break;
    if (platformIndex == platformCount) {
        std::cerr << "CLIntercept: Cannot figure out the owning platform of the selected device\n.";
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }

    // Figure out the index of the selected device.
    std::array<cl_device_id, 16> deviceList;
    cl_uint deviceCount = 0;
    if (driver.clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, deviceList.size(), deviceList.data(), &deviceCount)
        != CL_SUCCESS) {
        std::cerr << "CLIntercept: Cannot figure the ID of the selected device\n.";
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }
    unsigned deviceIndex = 0;
    for (; deviceIndex < deviceCount; ++deviceIndex)
        if (deviceList[deviceIndex] == device)
            break;
    if (deviceIndex == deviceCount) {
        std::cerr << "CLIntercept: Cannot figure the ID of the selected device\n.";
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return nullptr;
    }

    try {
        cl_context wrapped = new _cl_context(created, platformIndex, deviceIndex);
        char name[255] = {};
        size_t nameSize = 0;
        if (driver.clGetPlatformInfo(platform, CL_PLATFORM_NAME, 255, name, &nameSize) == CL_SUCCESS) {
            (*wrapped) << "# Selecting " << std::string_view(name, nameSize) << '\n';
        }
        (*wrapped) << "select platform " << platformIndex << '\n';
        if (driver.clGetDeviceInfo(device, CL_DEVICE_NAME, 255, name, &nameSize) == CL_SUCCESS) {
            (*wrapped) << "# Selecting " << std::string_view(name, nameSize) << '\n';
        }
        (*wrapped) << "select device " << deviceIndex << '\n';

        return wrapped;
    } catch (...) {
        driver.clReleaseContext(created);
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    }

    return nullptr;
}

EXPORT CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context context, cl_device_id device,
                     cl_command_queue_properties properties,
                     cl_int* errcode_ret)
{
    // CLTestbench assumes a single, in-order command queue.
    // Ignoring queues should be fine, we just run everything serially.
    // However, for command enqueueing, we need to get the context,
    // so store it through the queue object.
    cl_int error = CL_SUCCESS;
    cl_command_queue queue = getDriverICD().clCreateCommandQueue(context->object, device, properties, &error);
    if (error != CL_SUCCESS) {
        if (errcode_ret) *errcode_ret = error;
        return nullptr;
    }

    try {
        cl_command_queue wrapped = context->wrap(queue);
        if (errcode_ret) *errcode_ret = CL_SUCCESS;
        return wrapped;
    } catch (...) {
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    }

    return nullptr;
}

EXPORT CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context context, cl_uint count,
                          const char** strings, const size_t* lengths,
                          cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_int error = CL_SUCCESS;
    cl_program program = getDriverICD().clCreateProgramWithSource(context->object, count, strings, lengths, &error);

    if (error != CL_SUCCESS) {
        if (errcode_ret) *errcode_ret = error;
        return nullptr;
    }

    try {
        const uint32_t sourceNo = context->mProgramCount++;
        std::stringstream sstream;
        sstream << "source_" << sourceNo << ".cl";
        std::string filename = sstream.str();
        std::ofstream source(filename);
        for (cl_uint i = 0; i < count; ++i) {
            const size_t len = lengths ? lengths[i] : std::strlen(strings[i]);
            source << std::string_view(strings[i], len);
        }
        source.close();
        sstream.str("");
        sstream << "Dumping source " << filename;
        (*context) << Comment(sstream.str());
        // Trim the string to remove ".cl".
        filename.resize(filename.size() - 3);
        // And use that for the object identifier.
        cl_program wrapped = context->wrap(filename, program);
        if (errcode_ret) *errcode_ret = CL_SUCCESS;
        return wrapped;
    } catch (...) {
        getDriverICD().clReleaseProgram(program);
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    }
    return nullptr;
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program program, cl_uint num_devices,
               const cl_device_id* device_list, const char* options,
               void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
               void* user_data) CL_API_SUFFIX__VERSION_1_0
{
    cl_int err = getDriverICD().clBuildProgram(program->object, num_devices, device_list, options, pfn_notify, user_data);
    if (err != CL_SUCCESS)
        return err;

    cl_context context = program->parent;
    auto lock = context->lock();
    (*context) << program->name << " = program(file(\"" << program->name << ".cl\")";
    if (options && options[0] != '\0') {
        (*context) << ", \"" << options << '"';
    }
    (*context) << ")\n";
    return CL_SUCCESS;
}

EXPORT CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program program, const char* kernel_name, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_int error = CL_SUCCESS;
    cl_kernel kernel = getDriverICD().clCreateKernel(program->object, kernel_name, &error);
    if (error != CL_SUCCESS) {
        if (errcode_ret) *errcode_ret = error;
        return kernel;
    }

    try {
        const uint32_t kernelNo = program->parent->mKernelCount++;
        std::string kernelName("kern_");
        kernelName += std::to_string(kernelNo) + "_" + kernel_name;
        cl_kernel wrapped = program->parent->wrap(kernelName, kernel);
        auto lock = program->parent->lock();
        (*program->parent) << wrapped->name << " = kernel(" << program->name << ", " << kernel_name << ")\n";
        return wrapped;
    } catch (...) {
        getDriverICD().clReleaseKernel(kernel);
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    }

    return nullptr;
}

/* Memory Object APIs */
EXPORT CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context context, cl_mem_flags flags,
               size_t size, void* host_ptr,
               cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    // Memory flags will be lost when scripting this.
    // In particular, host-accessible pointers.
    // This should be fine because CLTestbench isn't meant
    // to emulate the host application, only the CL commands.
    cl_int error = CL_SUCCESS;
    cl_mem buffer = getDriverICD().clCreateBuffer(context->object, flags, size, host_ptr, &error);
    if (error != CL_SUCCESS) {
        if (errcode_ret) *errcode_ret = error;
        return nullptr;
    }

    try {
        const uint32_t bufferNo = context->mBufferCount++;
        CreateBuffer create;
        std::string bufferName("buff_");
        bufferName += std::to_string(bufferNo);
        create.buffer = context->wrap(std::string(bufferName), buffer);
        create.size = size;
        if ((flags & CL_MEM_COPY_HOST_PTR) != 0) {
            // If we got to copy host data, we can inline define the contents,
            // but not too much as to make the script file unreadable (by humans).
            if (size <= (16 * sizeof(cl_ulong))) {
                create.data = host_ptr;
            } else {
                // Too large - put the contents into a file.
                create.filename = bufferName;
                create.filename += ".bin";
                std::ofstream outFile(create.filename);
                outFile.write(static_cast<const char*>(host_ptr), size);
                outFile.close();
            }
        }
        (*context) << create;
        return create.buffer;
    } catch (...) {
        getDriverICD().clReleaseMemObject(buffer);
        if (errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    }

    return nullptr;
}

namespace
{
template<typename T>
struct UniquePtrCompare
{
    const T* const RHS;
    UniquePtrCompare(const T* val) : RHS(val) {}

    bool operator()(const std::unique_ptr<T>& LHS)
    {
        return LHS.get() == RHS;
    }
};
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
               const void* arg_value) CL_API_SUFFIX__VERSION_1_0
{
    cl_context context = kernel->parent;
    BindArg binding;
    binding.kernel = kernel;
    binding.argIndex = arg_index;
    // If the argument is a known context object, it needs to be unwrapped.
    if (arg_size == sizeof(cl_mem)) {
        cl_mem buffer = nullptr;
        std::memcpy(&buffer, arg_value, sizeof(cl_mem));
        if (std::any_of(context->mBuffers.begin(), context->mBuffers.end(), UniquePtrCompare(buffer))) {
            // Setting a buffer object as argument.
            const cl_int error = getDriverICD().clSetKernelArg(kernel->object, arg_index, arg_size, &buffer->object);
            if (error != CL_SUCCESS) return error;
            binding.buffer = buffer;
        }
    } else {
        // TODO: setting samplers, and local memory.
        if (arg_size == 0) return CL_INVALID_VALUE;
        const cl_int error = getDriverICD().clSetKernelArg(kernel->object, arg_index, arg_size, arg_value);
        if (error != CL_SUCCESS) return error;
        // As we have left is setting constant arguments
        binding.data = arg_value;
        binding.dataSize = arg_size;
    }

    (*context) << binding;
    return CL_SUCCESS;
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel,
                       cl_uint work_dim, const size_t* global_work_offset,
                       const size_t* global_work_size, const size_t* local_work_size,
                       cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
                       cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
    cl_context context = command_queue->parent;
    // CLTestbench does not support offsets on the queue (but it could be added)
    // so we have to bail out.
    if (global_work_offset) {
        for (cl_uint i = 0; i < work_dim; ++i) {
            if (global_work_offset[i] != 0) {
                (*context) << Comment("non-zero global work offsets not supported.");
                return CL_INVALID_VALUE;
            }
        }
    }

    const cl_int error = getDriverICD().clEnqueueNDRangeKernel(
        command_queue->object, kernel->object, work_dim, global_work_offset,
        global_work_size, local_work_size, num_events_in_wait_list,
        event_wait_list, event);

    if (error != CL_SUCCESS) return error;
    EnqueueKernel enqueue;
    enqueue.kernel = kernel;
    enqueue.dim = work_dim;
    for (cl_uint i = 0; i < work_dim; ++i) {
        enqueue.globalSize[i] = global_work_size[i];
        if (local_work_size)
            enqueue.localSize[i] = local_work_size[i];
    }

    (*context) << enqueue;

    return CL_SUCCESS;
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    const cl_int error = getDriverICD().clFinish(command_queue->object);
    if (error != CL_SUCCESS) return error;
    auto lock = command_queue->parent->lock();
    (*command_queue->parent->mOut) << "wait\n";
    return CL_SUCCESS;
}

// All API entry-points below do not require interception,
// and are forwarded to the implementation (with unwrapped objects).

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint num_entries,
                 cl_platform_id* platforms,
                 cl_uint* num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clGetPlatformIDs(num_entries, platforms, num_platforms);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type,
               cl_uint num_entries, cl_device_id* devices,
               cl_uint* num_devices) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program program, cl_device_id device,
                      cl_program_build_info param_name, size_t param_value_size,
                      void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clGetProgramBuildInfo(program->object, device, param_name, param_value_size, param_value, param_value_size_ret);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clRetainKernel(kernel->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clReleaseKernel(kernel->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clRetainProgram(program->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clReleaseProgram(program->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clRetainMemObject(memobj->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clReleaseMemObject(memobj->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clRetainCommandQueue(command_queue->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    return getDriverICD().clReleaseCommandQueue(command_queue->object);
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    const cl_int error = getDriverICD().clRetainContext(context->object);
    if (error == CL_SUCCESS)
        context->mRefCount++;
    return error;
}

EXPORT CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    const cl_int error = getDriverICD().clReleaseContext(context->object);
    if (error == CL_SUCCESS) {
        if ((--context->mRefCount) == 0) {
            delete context;
        }
    }
    return error;
}
