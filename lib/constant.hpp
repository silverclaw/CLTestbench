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
#include <cstdlib>
#include <cstring>
#include <cctype> // tolower
#include <limits>
#include <type_traits>

#include "error.hpp"
#include "istringview.hpp"
#include "token.hpp"

namespace CLTestbench
{
template<typename T>
T TokenStream::parseConstant(Token token) const
{
    IStringView str = getTokenText(token);

    if constexpr (std::is_same<T, bool>::value) {
        if (str.size() == 1) {
            switch (tolower(str[0])) {
            case '0':
            case 'n':
            case 'f': // short for false
                return false;
            case '1':
            case 'y':
            case 't': // short for true
                return true;
            }
        }

        if (str == "false" || str == "no") return false;
        if (str == "true" || str == "yes") return true;

        throw CommandError("Cannot parse as boolean", token);
    }

    std::array<char, 64> buffer;
    if (str.size() >= buffer.size())
        throw CommandError("Constant string is too large.", token);

    memcpy(buffer.data(), str.data(), str.size());
    buffer[str.size()] = '\0';

    // std::from_chars does not handle base prefixes, at least not on GCC,
    // so use stroll and friends instead.
    // libc++ (from llvm) does not implement std::from_chars<float> either.

    if constexpr (std::is_integral<T>::value && std::is_signed<T>::value) {
        char* endptr = nullptr;
        const auto val = strtoll(buffer.data(), &endptr, 0);
        // The entire string must be consumed by the parser
        if (*endptr != '\0')
            throw CommandError("Could not parse numeric token.", token);
        if (val > std::numeric_limits<T>::max())
            throw CommandError("Value too large to fit specified type.", token);
        if (val < std::numeric_limits<T>::lowest())
            throw CommandError("Value too negative to fit specified type.", token);
        return static_cast<T>(val);
    }

    if constexpr (std::is_integral<T>::value && std::is_unsigned<T>::value) {
        char* endptr = nullptr;
        const auto val = strtoull(buffer.data(), &endptr, 0);
        // The entire string must be consumed by the parser
        if (*endptr != '\0')
            throw CommandError("Could not parse numeric token.", token);
        if (val > std::numeric_limits<T>::max())
            throw CommandError("Value too large to fit specified type.", token);
        return static_cast<T>(val);
    }

    if constexpr (std::is_floating_point<T>::value) {
        char* endptr = nullptr;
        const auto val = strtod(buffer.data(), &endptr);
        // The entire string must be consumed by the parser
        if (*endptr != '\0')
            throw CommandError("Could not parse numeric token.", token);
        // This will add appropriate rounding and clamping.
        return static_cast<T>(val);
    }

    throw CommandError("Internal error - unhandled constant type.");
}
} // namespace CLTestbench
