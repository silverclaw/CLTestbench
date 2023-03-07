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

#pragma once
#include <array>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <CL/cl_icd.h>

#include "library.hpp"
#include "object_cl.hpp"

namespace CLTestbench
{
class Driver final
{
    /// The loaded OpenCL shared library.
    Library mLib;
    /// The bound functions for the CL API.
    cl_icd_dispatch mCLFns;

    /// The CL Context for this driver.
    cl_context mContext = nullptr;

    /// The default CL queue for commands.
    cl_command_queue mQueue = nullptr;

    void clearContext();
    operator cl_context();
    operator cl_command_queue();

public:
    Driver(std::string_view filename);
    ~Driver();

    std::string_view getLibraryName() const { return mLib.getName(); }

    /// The selected platform.
    cl_platform_id mPlatform = nullptr;

    /// The selected device.
    cl_device_id mDevice = nullptr;

    /// Whether the commands are submitted to the queue as blocking commands.
    bool mBlock = true;

    /// Encodes a CL error.
    class Error final : public std::exception
    {
    public:
        const cl_int mError;

        Error(cl_int error) : mError(error) {}

        const char* what() const noexcept override;
    };

    struct PlatformInfo
    {
        std::string mProfile;
        std::string mVersion;
        std::string mName;
        std::string mVendor;
        std::string mExtensions;
    };

    struct DeviceInfo
    {
        // There's a lot of device information,
        // so we query only what's required for identification.
        std::string mName;
        std::string mVersion;
    };

    /// Flushes the command queue.
    void flush();
    /// Waits for the command queue to finish.
    void finish();

    std::vector<cl_platform_id> getPlatformIDs();
    PlatformInfo getPlatformInfo(cl_platform_id);
    std::vector<cl_device_id> getDeviceIDs(cl_platform_id);
    DeviceInfo getDeviceInfo(cl_device_id);

    std::unique_ptr<ProgramObject> createProgram(std::string_view source);
    std::unique_ptr<ProgramObject> createProgramBinary(const void* binary, std::size_t size);
    void buildProgram(cl_program, const char* opts);
    std::string programBuildLog(cl_program);
    std::vector<char> programBinary(cl_program);

    std::unique_ptr<KernelObject> createKernel(cl_program, const char* name);
    std::unique_ptr<KernelObject> cloneKernel(cl_kernel);

    std::unique_ptr<MemoryObject> createBuffer(std::size_t);
    void writeBuffer(cl_mem, const void* data, std::size_t offset, std::size_t size);
    void readBuffer(cl_mem, void* data, size_t offset, size_t size);
    void copyBuffer(cl_mem src, cl_mem dst, size_t srcOffset, size_t dstOffet, size_t size);
    size_t getBufferSize(cl_mem);

    void setKernelArg(cl_kernel kernel, uint32_t index, cl_mem memObj);
    void setKernelArg(cl_kernel kernel, uint32_t index, const void* data, std::size_t size);

    using EnqueueSize = std::array<size_t, 3>;
    void enqueueKernel(cl_kernel, EnqueueSize global, std::optional<EnqueueSize> local);

    using ImageCoords = std::array<size_t, 3>;
    std::unique_ptr<MemoryObject> createImage(const cl_image_format&, const cl_image_desc&, const void* = nullptr);
    void writeImage(cl_mem, const void* data, ImageCoords origin, ImageCoords region);
    void readImage(cl_mem, void* data, ImageCoords origin, ImageCoords region);
    void copyImage(cl_mem src, cl_mem dst, ImageCoords srcOrigin, ImageCoords dstOrigin, ImageCoords region);
};

std::ostream& operator<<(std::ostream&, const Driver::PlatformInfo&);
std::ostream& operator<<(std::ostream&, const Driver::DeviceInfo&);
} // namespace CLTestbench
