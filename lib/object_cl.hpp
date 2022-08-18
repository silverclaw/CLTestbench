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

#include <type_traits>

#include <CL/cl.h>

#include "object.hpp"

namespace CLTestbench
{

class CLObject : public Object {};

template<typename T, typename ExtraData>
class CLWrapper final : public CLObject
{
    using ReleaseFnTy = cl_int (*)(T);
    T __restrict const mObject;
    const ReleaseFnTy mReleaseFn;

public:
    explicit CLWrapper(T __restrict object, ReleaseFnTy releaseFn) noexcept :
        mObject(object), mReleaseFn(releaseFn) {}

    operator T() noexcept { return mObject; }

    std::string_view type() const noexcept
    {
        if (std::is_same_v<T, cl_mem>) return "CL memory object";
        if (std::is_same_v<T, cl_kernel>) return "CL kernel object";
        if (std::is_same_v<T, cl_program>) return "CL program object";
        return "Unknown CL object";
    }

    ExtraData data;

    ~CLWrapper()
    {
        mReleaseFn(mObject);
    }
};

struct CLImageData
{
    cl_image_format mFormat{};
    cl_image_desc mDescriptor{};
    size_t mBufferSize = 0;
};

struct EmptyStruct {};

using MemoryObject = CLWrapper<cl_mem, CLImageData>;
using KernelObject = CLWrapper<cl_kernel, EmptyStruct>;
using ProgramObject = CLWrapper<cl_program, EmptyStruct>;

} // namespace CLTestbench
