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

#include <catch2/catch_test_macros.hpp>

#include "istringview.hpp"

TEST_CASE("String compares")
{
    SECTION("Identical match")
    {
        CLTestbench::IStringView string("abcde");

        // Implicit std::string_view construction.
        CHECK(string == "abcde");
        CHECK(string.startsWith("a"));
        CHECK(string.startsWith("ab"));
        CHECK(string.startsWith("abc"));
        CHECK(string.startsWith("abcd"));
        CHECK(string.startsWith("abcde"));

        // Explicit std::string_view
        std::string_view other("abcde");
        CHECK(string == other);
    }

    SECTION("Case-insensitive match")
    {
        CLTestbench::IStringView string("ABCDE");

        // Implicit std::string_view construction.
        CHECK(string == "abcde");
        CHECK(string.startsWith("a"));
        CHECK(string.startsWith("ab"));
        CHECK(string.startsWith("abc"));
        CHECK(string.startsWith("abcd"));
        CHECK(string.startsWith("abcde"));

        // Explicit std::string_view
        std::string_view other("abcde");
        CHECK(string == other);
    }

    SECTION("Mismatch")
    {
        CLTestbench::IStringView string("abcde");

        CHECK(string != "mismatch");
        CHECK(string != "");
        CHECK(!string.startsWith("mis"));
        CHECK(!string.startsWith("mismatch"));
    }
}

TEST_CASE("String matches")
{
    SECTION("Valid matches")
    {
        CLTestbench::IStringView string("mat");

        CHECK(string.autocomplete({"match", "nomatch", "mismatch"}) == 0);
        CHECK(string.autocomplete({"", "match", "mismatch"}) == 1);
        CHECK(string.autocomplete({"", "ma", "match"}) == 2);
    }

    SECTION("Valid insensitive matches")
    {
        CLTestbench::IStringView string("MAT");

        CHECK(string.autocomplete({"match", "nomatch", "mismatch"}) == 0);
        CHECK(string.autocomplete({"", "match", "mismatch"}) == 1);
        CHECK(string.autocomplete({"", "ma", "match"}) == 2);
    }

    SECTION("Ambiguous matches")
    {
        CLTestbench::IStringView string("mat");

        CHECK(string.autocomplete({"match", "match2", "match3"}) == CLTestbench::IStringView::ambiguous);
        CHECK(string.autocomplete({"", "match", "matc"}) == CLTestbench::IStringView::ambiguous);
    }

    SECTION("No matches")
    {
        CLTestbench::IStringView string("mat");

        CHECK(string.autocomplete({"miss", "miss2", "miss3"}) == std::string_view::npos);
        CHECK(string.autocomplete({"ma", "m", ""}) == std::string_view::npos);
    }
}
