// This file is part of CLTestbench.

// CLTestbench is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CLTestbench is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.

#include <CL/cl.h>

typedef struct _cl_platform_id
{
    // No body required.
} _cl_platform_id;

typedef struct _cl_device_id
{
    // No body required.
} _cl_device_id;

static _cl_platform_id DummyPlatform;
static _cl_device_id DummyDevice;

__attribute__((visibility("default"))) cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms)
{
    if (num_platforms) *num_platforms = 1;
    if (platforms && num_entries > 0) {
        platforms[0] = &DummyPlatform;
    }
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetPlatformInfo(cl_platform_id, cl_platform_info, size_t, void*,
                                                                size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetDeviceIDs(cl_platform_id platform,
                                                             cl_device_type device_type, cl_uint num_entries,
                                                             cl_device_id* devices, cl_uint* num_devices)
{
    if (platform != &DummyPlatform) return CL_INVALID_PLATFORM;
    if ((device_type & CL_DEVICE_TYPE_DEFAULT) == 0) {
        if (num_devices) *num_devices = 0;
        return CL_SUCCESS;
    }
    if (num_devices) *num_devices = 1;
    if (devices && num_entries > 0) {
        devices[0] = &DummyDevice;
    }
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetDeviceInfo(cl_device_id, cl_device_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clCreateSubDevices(cl_device_id, const cl_device_partition_property*,
                                                                 cl_uint, cl_device_id*, cl_uint*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clRetainDevice(cl_device_id)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseDevice(cl_device_id)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clSetDefaultDeviceCommandQueue(cl_context, cl_device_id, cl_command_queue)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetDeviceAndHostTimer(cl_device_id, cl_ulong*, cl_ulong*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetHostTimer(cl_device_id, cl_ulong*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_1

__attribute__((visibility("default"))) cl_context
clCreateContext(const cl_context_properties*, cl_uint, const cl_device_id*,
                void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_context
clCreateContextFromType(const cl_context_properties*, cl_device_type,
                        void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_int clRetainContext(cl_context)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseContext(cl_context)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetContextInfo(cl_context, cl_context_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_3_0

__attribute__((visibility("default"))) cl_int
clSetContextDestructorCallback(cl_context, void(CL_CALLBACK*)(cl_context, void*), void*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_3_0

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_command_queue
clCreateCommandQueueWithProperties(cl_context, cl_device_id, const cl_queue_properties*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}
#endif

__attribute__((visibility("default"))) cl_int clRetainCommandQueue(cl_command_queue)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseCommandQueue(cl_command_queue)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetCommandQueueInfo(cl_command_queue, cl_command_queue_info, size_t,
                                                                    void*, size_t*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_mem clCreateBuffer(cl_context, cl_mem_flags, size_t, void*,
                                                             cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_mem clCreateSubBuffer(cl_mem, cl_mem_flags, cl_buffer_create_type,
                                                                const void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_1_1

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_mem clCreateImage(cl_context, cl_mem_flags, const cl_image_format*,
                                                            const cl_image_desc*, void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_mem clCreatePipe(cl_context, cl_mem_flags, cl_uint, cl_uint,
                                                           const cl_pipe_properties*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_2_0

#ifdef CL_VERSION_3_0

__attribute__((visibility("default"))) cl_mem
clCreateBufferWithProperties(cl_context, const cl_mem_properties*, cl_mem_flags, size_t, void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_mem clCreateImageWithProperties(cl_context, const cl_mem_properties*,
                                                                          cl_mem_flags, const cl_image_format*,
                                                                          const cl_image_desc*, void*,
                                                                          cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_3_0

__attribute__((visibility("default"))) cl_int clRetainMemObject(cl_mem)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseMemObject(cl_mem)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetSupportedImageFormats(cl_context, cl_mem_flags, cl_mem_object_type,
                                                                         cl_uint, cl_image_format*, cl_uint*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetMemObjectInfo(cl_mem, cl_mem_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetImageInfo(cl_mem, cl_image_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_int clGetPipeInfo(cl_mem, cl_pipe_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_0

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clSetMemObjectDestructorCallback(cl_mem,
                                                                               void(CL_CALLBACK*)(cl_mem, void*), void*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_1

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) void* clSVMAlloc(cl_context, cl_svm_mem_flags, size_t,
                                                        cl_uint) CL_API_SUFFIX__VERSION_2_0
{
    return NULL;
}

__attribute__((visibility("default"))) void clSVMFree(cl_context, void*)
{
    return;
}

#endif // CL_VERSION_2_0

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_sampler
clCreateSamplerWithProperties(cl_context, const cl_sampler_properties*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_2_0

__attribute__((visibility("default"))) cl_int clRetainSampler(cl_sampler)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseSampler(cl_sampler)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetSamplerInfo(cl_sampler, cl_sampler_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

/* Program Object APIs */
__attribute__((visibility("default"))) cl_program clCreateProgramWithSource(cl_context, cl_uint, const char**,
                                                                            const size_t*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_program clCreateProgramWithBinary(cl_context, cl_uint, const cl_device_id*,
                                                                            const size_t*, const unsigned char**,
                                                                            cl_int*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_program
clCreateProgramWithBuiltInKernels(cl_context, cl_uint, const cl_device_id*, const char*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_1

__attribute__((visibility("default"))) cl_program clCreateProgramWithIL(cl_context, const void*, size_t,
                                                                        cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clRetainProgram(cl_program)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseProgram(cl_program)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clBuildProgram(cl_program, cl_uint, const cl_device_id*, const char*,
                                                             void(CL_CALLBACK*)(cl_program, void*), void*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clCompileProgram(cl_program, cl_uint, const cl_device_id*, const char*,
                                                               cl_uint, const cl_program*, const char**,
                                                               void(CL_CALLBACK*)(cl_program, void*), void*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_program clLinkProgram(cl_context, cl_uint, const cl_device_id*, const char*,
                                                                cl_uint, const cl_program*,
                                                                void(CL_CALLBACK*)(cl_program, void*), void*,
                                                                cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}
#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_2

__attribute__((visibility("default"))) cl_int clSetProgramReleaseCallback(cl_program,
                                                                          void(CL_CALLBACK*)(cl_program, void*), void*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clSetProgramSpecializationConstant(cl_program, cl_uint, size_t,
                                                                                 const void*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_2

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clUnloadPlatformCompiler(cl_platform_id)
{
    return CL_SUCCESS;
}
#endif // CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clGetProgramInfo(cl_program, cl_program_info, size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetProgramBuildInfo(cl_program, cl_device_id, cl_program_build_info,
                                                                    size_t, void*, size_t*)
{
    return CL_SUCCESS;
}

/* Kernel Object APIs */
__attribute__((visibility("default"))) cl_kernel clCreateKernel(cl_program, const char*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}
__attribute__((visibility("default"))) cl_int clCreateKernelsInProgram(cl_program, cl_uint, cl_kernel*, cl_uint*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_2_1

__attribute__((visibility("default"))) cl_kernel clCloneKernel(cl_kernel, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

#endif // CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clRetainKernel(cl_kernel)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clReleaseKernel(cl_kernel)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clSetKernelArg(cl_kernel, cl_uint, size_t, const void*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_int clSetKernelArgSVMPointer(cl_kernel, cl_uint, const void*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clSetKernelExecInfo(cl_kernel, cl_kernel_exec_info, size_t, const void*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_0

__attribute__((visibility("default"))) cl_int clGetKernelInfo(cl_kernel, cl_kernel_info, size_t, void*,
                                                              size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clGetKernelArgInfo(cl_kernel, cl_uint, cl_kernel_arg_info, size_t, void*,
                                                                 size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clGetKernelWorkGroupInfo(cl_kernel, cl_device_id,
                                                                       cl_kernel_work_group_info, size_t, void*,
                                                                       size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

#ifdef CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clGetKernelSubGroupInfo(cl_kernel, cl_device_id, cl_kernel_sub_group_info,
                                                                      size_t, const void*, size_t, void*,
                                                                      size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clWaitForEvents(cl_uint, const cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clGetEventInfo(cl_event, cl_event_info, size_t, void*,
                                                             size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_event clCreateUserEvent(cl_context context, cl_int* errcode_ret);

#endif // CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0;

__attribute__((visibility("default"))) cl_int clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0;

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clSetUserEventStatus(cl_event event, cl_int execution_status);

__attribute__((visibility("default"))) cl_int
clSetEventCallback(cl_event event, cl_int command_exec_callback_type,
                   void(CL_CALLBACK* pfn_notify)(cl_event event, cl_int event_command_status, void* user_data),
                   void* user_data);

#endif // CL_VERSION_1_1

/* Profiling APIs */
__attribute__((visibility("default"))) cl_int clGetEventProfilingInfo(cl_event, cl_profiling_info, size_t, void*,
                                                                      size_t* param_value_size_ret)
{
    if (param_value_size_ret) *param_value_size_ret = 0;
    return CL_SUCCESS;
}

/* Flush and Finish APIs */
__attribute__((visibility("default"))) cl_int clFlush(cl_command_queue)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clFinish(cl_command_queue)
{
    return CL_SUCCESS;
}

/* Enqueued Commands APIs */
__attribute__((visibility("default"))) cl_int clEnqueueReadBuffer(cl_command_queue, cl_mem, cl_bool, size_t, size_t,
                                                                  void*, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clEnqueueReadBufferRect(cl_command_queue, cl_mem, cl_bool, const size_t*,
                                                                      const size_t*, const size_t*, size_t, size_t,
                                                                      size_t, size_t, void*, cl_uint, const cl_event*,
                                                                      cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clEnqueueWriteBuffer(cl_command_queue, cl_mem, cl_bool, size_t, size_t,
                                                                   const void*, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clEnqueueWriteBufferRect(cl_command_queue, cl_mem, cl_bool, const size_t*,
                                                                       const size_t*, const size_t*, size_t, size_t,
                                                                       size_t, size_t, const void*, cl_uint,
                                                                       const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_1

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueFillBuffer(cl_command_queue, cl_mem, const void*, size_t, size_t,
                                                                  size_t, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueCopyBuffer(cl_command_queue, cl_mem, cl_mem, size_t, size_t,
                                                                  size_t, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clEnqueueCopyBufferRect(cl_command_queue, cl_mem, cl_mem, const size_t*,
                                                                      const size_t*, const size_t*, size_t, size_t,
                                                                      size_t, size_t, cl_uint, const cl_event*,
                                                                      cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_1

__attribute__((visibility("default"))) cl_int clEnqueueReadImage(cl_command_queue, cl_mem, cl_bool, const size_t*,
                                                                 const size_t*, size_t, size_t, void*, cl_uint,
                                                                 const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueWriteImage(cl_command_queue, cl_mem, cl_bool, const size_t*,
                                                                  const size_t*, size_t, size_t, const void*, cl_uint,
                                                                  const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueFillImage(cl_command_queue, cl_mem, const void*, const size_t*,
                                                                 const size_t*, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueCopyImage(cl_command_queue, cl_mem, cl_mem, const size_t*,
                                                                 const size_t*, const size_t*, cl_uint, const cl_event*,
                                                                 cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueCopyImageToBuffer(cl_command_queue, cl_mem, cl_mem,
                                                                         const size_t*, const size_t*, size_t, cl_uint,
                                                                         const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueCopyBufferToImage(cl_command_queue, cl_mem, cl_mem, size_t,
                                                                         const size_t*, const size_t*, cl_uint,
                                                                         const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) void* clEnqueueMapBuffer(cl_command_queue, cl_mem, cl_bool, cl_map_flags, size_t,
                                                                size_t, cl_uint, const cl_event*, cl_event*,
                                                                cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) void* clEnqueueMapImage(cl_command_queue, cl_mem, cl_bool, cl_map_flags,
                                                               const size_t*, const size_t*, size_t*, size_t*, cl_uint,
                                                               const cl_event*, cl_event*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueUnmapMemObject(cl_command_queue, cl_mem, void*, cl_uint,
                                                                      const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueMigrateMemObjects(cl_command_queue, cl_uint, const cl_mem*,
                                                                         cl_mem_migration_flags, cl_uint,
                                                                         const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueNDRangeKernel(cl_command_queue, cl_kernel, cl_uint,
                                                                     const size_t*, const size_t*, const size_t*,
                                                                     cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueNativeKernel(cl_command_queue, void(CL_CALLBACK*)(void*), void*,
                                                                    size_t, cl_uint, const cl_mem*, const void**,
                                                                    cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) cl_int clEnqueueMarkerWithWaitList(cl_command_queue, cl_uint, const cl_event*,
                                                                          cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueBarrierWithWaitList(cl_command_queue, cl_uint, const cl_event*,
                                                                           cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_0

__attribute__((visibility("default"))) cl_int
clEnqueueSVMFree(cl_command_queue, cl_uint, void**, void(CL_CALLBACK*)(cl_command_queue queue, cl_uint, void**, void*),
                 void*, cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueSVMMemcpy(cl_command_queue, cl_bool, void*, const void*, size_t,
                                                                 cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueSVMMemFill(cl_command_queue, void*, const void*, size_t, size_t,
                                                                  cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueSVMMap(cl_command_queue, cl_bool, cl_map_flags, void*, size_t,
                                                              cl_uint, const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueSVMUnmap(cl_command_queue, void*, cl_uint, const cl_event*,
                                                                cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_0

#ifdef CL_VERSION_2_1

__attribute__((visibility("default"))) cl_int clEnqueueSVMMigrateMem(cl_command_queue, cl_uint, const void**,
                                                                     const size_t*, cl_mem_migration_flags, cl_uint,
                                                                     const cl_event*, cl_event*)
{
    return CL_SUCCESS;
}

#endif // CL_VERSION_2_1

#ifdef CL_VERSION_1_2

__attribute__((visibility("default"))) void* clGetExtensionFunctionAddressForPlatform(cl_platform_id, const char*)
{
    return NULL;
}

#endif // CL_VERSION_1_2

#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
__attribute__((visibility("default"))) cl_int clSetCommandQueueProperty(cl_command_queue, cl_command_queue_properties,
                                                                        cl_bool, cl_command_queue_properties*)
{
    return CL_SUCCESS;
}
#endif // CL_USE_DEPRECATED_OPENCL_1_0_APIS

/* Deprecated OpenCL 1.1 APIs */
__attribute__((visibility("default"))) cl_mem clCreateImage2D(cl_context, cl_mem_flags, const cl_image_format*, size_t,
                                                              size_t, size_t, void*, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_mem clCreateImage3D(cl_context, cl_mem_flags, const cl_image_format*, size_t,
                                                              size_t, size_t, size_t, size_t, void*,
                                                              cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_int clEnqueueMarker(cl_command_queue, cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueWaitForEvents(cl_command_queue, cl_uint, const cl_event*)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clEnqueueBarrier(cl_command_queue)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) cl_int clUnloadCompiler(void)
{
    return CL_SUCCESS;
}

__attribute__((visibility("default"))) void* clGetExtensionFunctionAddress(const char*)
{
    return NULL;
}

__attribute__((visibility("default"))) cl_command_queue
clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_sampler clCreateSampler(cl_context, cl_bool, cl_addressing_mode,
                                                                  cl_filter_mode, cl_int* errcode_ret)
{
    if (errcode_ret) *errcode_ret = CL_SUCCESS;
    return NULL;
}

__attribute__((visibility("default"))) cl_int clEnqueueTask(cl_command_queue, cl_kernel, cl_uint, const cl_event*,
                                                            cl_event*)
{
    return CL_SUCCESS;
}
