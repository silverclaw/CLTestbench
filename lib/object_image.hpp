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

#include <cstdint>
#include <memory>
#include <string_view>

#include <CL/cl.h>

#include "object_data.hpp"

namespace CLTestbench
{
class ImageObject : public DataObject
{
public:
    virtual cl_image_format format() const noexcept = 0;
    virtual cl_image_desc descriptor() const noexcept = 0;
    virtual ~ImageObject() = default;
};

struct Token;

/// Attempt to decode this buffer as a PNG input.
std::unique_ptr<ImageObject> LoadPNG(const void*, size_t, const Token&, std::string_view filename = "");

/// Attmpt tow rite this image object onto the given file name.
/// The token for the image name and file name are given for diagnostics.
void WritePNG(const ImageObject&, const Token&, const Token&, std::string_view);
} // namespace CLTestbench
