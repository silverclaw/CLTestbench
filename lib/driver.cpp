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

#include <cassert>
#include <iostream>

#include "driver.hpp"
#include "object_cl.hpp"

using namespace CLTestbench;

/// Binds a function into the dispatch table.
#define BindFn(fnname) \
    if (!mCLFns.fnname) mCLFns.fnname = mLib.bind<cl_api_##fnname>(#fnname)

/// Verifies if the CL call X returns an error, and throws it.
#define Checked(X) \
    if (cl_int errCode = X; errCode != CL_SUCCESS) throw Error(errCode)

Driver::Driver(std::string_view filename) : mLib(filename), mCLFns{}
{
    // Attempt to load the basic CL functions that must
    // be available for any meaningful work to be done.
    // Any of these could throw, and that's what we want.
    BindFn(clGetPlatformIDs);
    BindFn(clGetDeviceIDs);
    BindFn(clCreateContext);
    BindFn(clReleaseContext);

    cl_uint numPlatforms = 0;
    Checked(mCLFns.clGetPlatformIDs(1, &mPlatform, &numPlatforms));

    // This is a fake error, but the rest of the code assumes that there will be a platform available.
    if (numPlatforms == 0) throw Error(CL_INVALID_PLATFORM);

    cl_uint numDevices = 0;
    Checked(mCLFns.clGetDeviceIDs(mPlatform, CL_DEVICE_TYPE_ALL, 1, &mDevice, &numDevices));
    // This is a fake error, but the rest of the code assumes that there will be a device available.
    if (numDevices == 0) throw Error(CL_INVALID_DEVICE);
}

Driver::~Driver()
{
    clearContext();
}

const char* Driver::Error::what() const noexcept
{
    switch (mError) {
    case CL_DEVICE_NOT_FOUND: return "Device not found";
    case CL_DEVICE_NOT_AVAILABLE: return "Device not available";
    case CL_COMPILER_NOT_AVAILABLE: return "Compiler not available";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "Memory object allocation failure";
    case CL_OUT_OF_RESOURCES: return "Out of resources";
    case CL_OUT_OF_HOST_MEMORY: return "Out of host memory";
    case CL_IMAGE_FORMAT_MISMATCH: return "Image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "Image format not supported";
    case CL_BUILD_PROGRAM_FAILURE: return "Program build failure";
    case CL_INVALID_VALUE: return "Invalid value";
    case CL_INVALID_DEVICE: return "Invalid device";
    case CL_INVALID_MEM_OBJECT: return "Invalid memory object";
    case CL_INVALID_BUILD_OPTIONS: return "Invalid build options";
    case CL_INVALID_KERNEL_NAME: return "Invalid kernel name";
    case CL_INVALID_ARG_INDEX: return "Invalid kernel argument index";
    case CL_INVALID_KERNEL_ARGS: return "Invalid kernel argument";
    case CL_INVALID_BUFFER_SIZE: return "Invalid buffer size";
    case CL_INVALID_GLOBAL_WORK_SIZE: return "Invalid global size";
    case CL_INVALID_IMAGE_DESCRIPTOR: return "Invalid image descriptor";
    default: break;
    }

    return "unknown error";
}

std::ostream& CLTestbench::operator<<(std::ostream& out, const Driver::PlatformInfo& info)
{
    out << "{\n\tName: " << info.mName << "\n\tVendor: " << info.mVendor << "\n\tVersion: " << info.mVersion
        << "\n\tProfile: " << info.mProfile << "\n\tExtensions: '" << info.mExtensions << "'\n}";
    return out;
}

std::ostream& CLTestbench::operator<<(std::ostream& out, const Driver::DeviceInfo& info)
{
    out << "{\n\tName: " << info.mName << "\n\tVersion: " << info.mVersion << "\n}";
    return out;
}

void Driver::clearContext()
{
    if (!mContext)
        return;
    if (mQueue) {
        mCLFns.clFinish(mQueue);
        mCLFns.clReleaseCommandQueue(mQueue);
        mQueue = nullptr;
    }
    mCLFns.clReleaseContext(mContext);
    mContext = nullptr;
}

Driver::operator cl_command_queue()
{
    if (mQueue) return mQueue;

    BindFn(clFinish);
    BindFn(clCreateCommandQueue);
    BindFn(clReleaseCommandQueue);

    cl_int err = CL_SUCCESS;
    cl_command_queue_properties properties = 0;
    mQueue = mCLFns.clCreateCommandQueue(*this, mDevice, properties, &err);
    Checked(err);
    return mQueue;
}

Driver::operator cl_context()
{
    if (mContext) return mContext;

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(mPlatform),
        0, 0
    };

    void(CL_CALLBACK * pfnNotify)(const char*, const void*, size_t, void*) = nullptr;
    void* userData = nullptr;
    cl_int err = CL_SUCCESS;
    mContext = mCLFns.clCreateContext(properties, 1, &mDevice, pfnNotify, userData, &err);
    Checked(err);
    return mContext;
}

void Driver::flush()
{
    BindFn(clFlush);
    if (!mQueue) return;
    mCLFns.clFlush(*this);
}

void Driver::finish()
{
    if (!mQueue) return;
    mCLFns.clFinish(*this);
}

std::vector<cl_platform_id> Driver::getPlatformIDs()
{
    cl_uint numPlatforms;
    Checked(mCLFns.clGetPlatformIDs(0, nullptr, &numPlatforms));
    std::vector<cl_platform_id> platforms;
    platforms.resize(numPlatforms);
    Checked(mCLFns.clGetPlatformIDs(numPlatforms, platforms.data(), &numPlatforms));
    return platforms;
}

Driver::PlatformInfo Driver::getPlatformInfo(cl_platform_id platform)
{
    BindFn(clGetPlatformInfo);
    PlatformInfo info;
#define BuildString(entry, ID) \
    { \
        size_t size = 0; \
        Checked(mCLFns.clGetPlatformInfo(platform, ID, 0, nullptr, &size)); \
        info.entry.resize(size + 1); \
        Checked(mCLFns.clGetPlatformInfo(platform, ID, size, info.entry.data(), &size)); \
    }

    BuildString(mProfile, CL_PLATFORM_PROFILE)
    BuildString(mVersion, CL_PLATFORM_VERSION)
    BuildString(mName, CL_PLATFORM_NAME)
    BuildString(mVendor, CL_PLATFORM_VENDOR)
    BuildString(mExtensions, CL_PLATFORM_EXTENSIONS)

#undef BuildString
    return info;
}

std::vector<cl_device_id> Driver::getDeviceIDs(cl_platform_id platform)
{
    cl_uint numDevices;
    Checked(mCLFns.clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices));
    std::vector<cl_device_id> devices;
    devices.resize(numDevices);
    Checked(mCLFns.clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), &numDevices));
    return devices;
}

Driver::DeviceInfo Driver::getDeviceInfo(cl_device_id device)
{
    BindFn(clGetDeviceInfo);
    DeviceInfo info;

#define BuildString(entry, ID) \
    { \
        size_t size = 0; \
        Checked(mCLFns.clGetDeviceInfo(device, ID, 0, nullptr, &size)); \
        info.entry.resize(size + 1); \
        Checked(mCLFns.clGetDeviceInfo(device, ID, size, info.entry.data(), &size)); \
    }

    BuildString(mVersion, CL_DEVICE_VERSION)
    BuildString(mName, CL_DEVICE_NAME)

#undef BuildString

    return info;
}

std::unique_ptr<ProgramObject> Driver::createProgram(std::string_view source)
{
    BindFn(clCreateProgramWithSource);
    BindFn(clBuildProgram);
    BindFn(clReleaseProgram);

    const char* srcData = source.data();
    size_t srcLen = source.length();
    cl_int err = CL_SUCCESS;
    cl_program program = mCLFns.clCreateProgramWithSource(*this, 1, &srcData, &srcLen, &err);
    Checked(err);

    return std::make_unique<ProgramObject>(program, mCLFns.clReleaseProgram);
}

void Driver::buildProgram(cl_program program, const char* opts)
{
    BindFn(clBuildProgram);

    void (*pfnNotify)(cl_program, void*) = nullptr;
    void* userData = nullptr;

    Checked(mCLFns.clBuildProgram(program, 1, &mDevice, opts, pfnNotify, userData));
}

std::string Driver::programBuildLog(cl_program program)
{
    BindFn(clGetProgramBuildInfo);

    size_t buildLogSize = 0;
    Checked(mCLFns.clGetProgramBuildInfo(program, mDevice, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buildLogSize));
    std::string log;
    log.resize(buildLogSize + 1);
    Checked(mCLFns.clGetProgramBuildInfo(program, mDevice, CL_PROGRAM_BUILD_LOG, buildLogSize, log.data(), &buildLogSize));
    return log;
}

std::vector<char> Driver::programBinary(cl_program program)
{
    BindFn(clGetProgramInfo);

    size_t binarySize = 0;
    size_t* sizeArray = &binarySize;
    size_t outputSize = 0;
    Checked(mCLFns.clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(sizeArray), &sizeArray, &outputSize));
    std::vector<char> binary;
    binary.resize(binarySize);
    char* binaryData = binary.data();
    Checked(mCLFns.clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(binaryData), &binaryData, &outputSize));
    return binary;
}

std::unique_ptr<KernelObject> Driver::createKernel(cl_program program, const char* name)
{
    BindFn(clCreateKernel);
    BindFn(clReleaseKernel);

    cl_int err = CL_SUCCESS;
    cl_kernel kernel = mCLFns.clCreateKernel(program, name, &err);
    Checked(err);
    return std::make_unique<KernelObject>(kernel, mCLFns.clReleaseKernel);
}

std::unique_ptr<KernelObject> Driver::cloneKernel(cl_kernel kernel)
{
    BindFn(clCloneKernel);
    assert(mCLFns.clReleaseKernel && "How was the original kernel obj generated?");

    cl_int err = CL_SUCCESS;
    cl_kernel clone = mCLFns.clCloneKernel(kernel, &err);
    Checked(err);
    return std::make_unique<KernelObject>(clone, mCLFns.clReleaseKernel);
}

std::unique_ptr<MemoryObject> Driver::createBuffer(std::size_t size)
{
    BindFn(clCreateBuffer);
    BindFn(clReleaseMemObject);

    cl_int err = CL_SUCCESS;
    cl_mem buffer = mCLFns.clCreateBuffer(*this, CL_MEM_READ_WRITE, size, nullptr, &err);
    Checked(err);
    auto bufferObj = std::make_unique<MemoryObject>(buffer, mCLFns.clReleaseMemObject);
    bufferObj->data.mBufferSize = size;
    return bufferObj;
}

void Driver::writeBuffer(cl_mem buffer, const void* data, std::size_t offset, std::size_t size)
{
    BindFn(clEnqueueWriteBuffer);

    cl_bool blocking = mBlock ? CL_TRUE : CL_FALSE;
    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    Checked(mCLFns.clEnqueueWriteBuffer(*this, buffer, blocking, offset, size, data, 0, wait, event));
}

void Driver::readBuffer(cl_mem buffer, void* data, std::size_t offset, std::size_t size)
{
    BindFn(clEnqueueReadBuffer);

    cl_bool blocking = mBlock ? CL_TRUE : CL_FALSE;
    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    Checked(mCLFns.clEnqueueReadBuffer(*this, buffer, blocking, offset, size, data, 0, wait, event));
}

void Driver::copyBuffer(cl_mem src, cl_mem dst, size_t srcOffset, size_t dstOffset, size_t size)
{
    BindFn(clEnqueueCopyBuffer);

    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    Checked(mCLFns.clEnqueueCopyBuffer(*this, src, dst, srcOffset, dstOffset, size, 0, wait, event));
}

std::size_t Driver::getBufferSize(cl_mem buffer)
{
    BindFn(clGetMemObjectInfo);

    std::size_t size = 0;
    std::size_t sizeStored = 0;
    Checked(mCLFns.clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size), &size, &sizeStored));
    return size;
}

void Driver::setKernelArg(cl_kernel kernel, uint32_t index, cl_mem memObj)
{
    BindFn(clSetKernelArg);
    Checked(mCLFns.clSetKernelArg(kernel, index, sizeof(cl_mem), &memObj));
}

void Driver::setKernelArg(cl_kernel kernel, uint32_t index, const void* data, std::size_t size)
{
    BindFn(clSetKernelArg);
    Checked(mCLFns.clSetKernelArg(kernel, index, size, data));
}

void Driver::enqueueKernel(cl_kernel kernel, EnqueueSize global,
                           std::optional<EnqueueSize> local)
{
    BindFn(clEnqueueNDRangeKernel);

    cl_uint dim = 1;
	if (global[2] != 0) dim = 3;
	else if (global[1] != 0) dim = 2;
    const std::size_t* localSize = local ? local->data() : nullptr;

    Checked(mCLFns.clEnqueueNDRangeKernel(*this, kernel, dim, nullptr, global.data(),
                                          localSize, 0, nullptr, nullptr));
}

std::unique_ptr<MemoryObject> Driver::createImage(const cl_image_format& format, const cl_image_desc& desc,
                                                  const void* data)
{
    BindFn(clCreateImage);
    BindFn(clReleaseMemObject);
    cl_int err;
    cl_int flags = CL_MEM_READ_WRITE;
    if (data != nullptr) flags |= CL_MEM_COPY_HOST_PTR;
    cl_mem image = mCLFns.clCreateImage(*this, flags, &format, &desc, const_cast<void*>(data), &err);
    Checked(err);
    auto imageObj = std::make_unique<MemoryObject>(image, mCLFns.clReleaseMemObject);
    imageObj->data.mDescriptor = desc;
    imageObj->data.mFormat = format;
    // We can probably calculate this value ourselves.
    imageObj->data.mBufferSize = getBufferSize(image);
    return imageObj;
}

void Driver::writeImage(cl_mem img, const void* data, ImageCoords origin, ImageCoords region)
{
    BindFn(clEnqueueWriteImage);

    cl_bool blocking = mBlock ? CL_TRUE : CL_FALSE;
    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    size_t pitch = 0;
    Checked(mCLFns.clEnqueueWriteImage(*this, img, blocking, origin.data(), region.data(), pitch, pitch, data, 0, wait, event));
}

void Driver::readImage(cl_mem img, void* data, ImageCoords origin, ImageCoords region)
{
    BindFn(clEnqueueReadImage);

    cl_bool blocking = mBlock ? CL_TRUE : CL_FALSE;
    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    size_t pitch = 0;
    Checked(mCLFns.clEnqueueReadImage(*this, img, blocking, origin.data(), region.data(), pitch, pitch, data, 0, wait, event));
}

void Driver::copyImage(cl_mem src, cl_mem dst, ImageCoords srcOrigin, ImageCoords dstOrigin, ImageCoords region)
{
    BindFn(clEnqueueCopyImage);

    cl_event* event = nullptr;
    const cl_event* wait = nullptr;
    Checked(mCLFns.clEnqueueCopyImage(*this, src, dst, srcOrigin.data(), dstOrigin.data(), region.data(), 0, wait, event));
}
