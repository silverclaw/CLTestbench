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
#include <cassert>
#include <cctype> // tolower
#include <initializer_list>
#include <string_view>

namespace CLTestbench
{
/// Wrapper to provide case-insensitive compares.
class IStringView : public std::string_view
{
public:
    IStringView(std::string_view other) : std::string_view(other) {}

    bool startsWith(const std::string_view& other) const noexcept
    {
        if (size() < other.size()) return false;
        for (std::size_t i = 0; i < other.size(); ++i) {
            assert(other[i] == tolower(other[i]) && "We're using manifest strings here - make it lowercase");
            if (tolower((*this)[i]) != other[i]) return false;
        }
        return true;
    }

    static const auto ambiguous = std::string_view::npos - 1;

    /// Check if this autocompletes to any entry in matches.  Returns the index of the
    /// matched entry, or std::string_view::npos if there was no match, or IStringView::ambiguous
    /// if there were no clear matches.
    std::size_t autocomplete(std::initializer_list<std::string_view> matches) const noexcept;
};

inline bool operator==(const IStringView& a, const IStringView& b)
{
    if (a.length() != b.length()) return false;

    for (std::size_t i = 0; i < a.length(); ++i) {
        assert(b[i] == tolower(b[i]) && "We're using manifest strings here - make it lowercase");
        if (tolower(a[i]) != b[i]) return false;
    }

    return true;
}

inline bool operator==(const IStringView& a, const std::string_view& b)
{
    return (a == IStringView(b));
}
} // namespace CLTestbench
