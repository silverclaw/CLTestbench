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

#include "istringview.hpp"

using namespace CLTestbench;

namespace
{
// Does a IStringView(match).startsWith(str)
// but we know "match" is lowercase, and "str" might not be.
bool PossibleMatch(IStringView str, std::string_view match) noexcept
{
    // "this" string is larger than the match entry - no match possible.
    if (match.size() < str.size()) return false;
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (tolower(str[i]) != match[i]) return false;
    }
    return true;
}
} // namespace

std::size_t IStringView::autocomplete(std::initializer_list<std::string_view> matches) const noexcept
{
    std::size_t match = std::string_view::npos;
    std::size_t matchIndex = 0;

    for (std::string_view entry : matches) {
        auto thisMatch = matchIndex;
        ++matchIndex;
        if (!PossibleMatch(*this, entry)) continue;

        // This is a possible match.  If there's one already,
        // then it's an ambiguity case.
        if (match != std::string_view::npos) return IStringView::ambiguous;

        match = thisMatch;
    }

    return match;
}
