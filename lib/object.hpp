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

#include <string_view>

namespace CLTestbench
{
/// Wraps any constructible object.
class Object
{
public:
    /// A string representation of this object's type, for the 'list' command.
    virtual std::string_view type() const noexcept = 0;
    virtual ~Object() = default;
};

} // namespace CLTestbench
