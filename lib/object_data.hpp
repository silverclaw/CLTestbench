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

#include "object.hpp"

namespace CLTestbench
{
class DataObject : public Object
{
public:
    virtual std::size_t size() const noexcept = 0;
    virtual const void* data() const noexcept = 0;
    virtual ~DataObject() = default;
};
} // namespace CLTestbench
