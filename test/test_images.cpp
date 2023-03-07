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

#include "cltb_config.h"
#include "object_image.hpp"
#include "testbench.hpp"
#include "token.hpp"
#include "voidstream.hpp"

TEST_CASE("Images")
{
    CLTestbench::Testbench bench;
    VoidStream output;
    // For debugging purposes, these can be commented out
    // to allow testbench print outs.
    bench.resetOutput(output);
    bench.resetErrorOutput(output);

    try {
        CLTestbench::TokenStream stream("load libOpenCL.so");
        bench.run(stream);
    } catch (...) {
        INFO("No CL implementation available.  Skipping Image tests.");
        return;
    }

    using Result = CLTestbench::Testbench::Result;
    std::string_view source = "img = image(file(" PROJECT_SOURCE_DIR "/test/pngtest8rgba.png))";

    SECTION("Create")
    {
        REQUIRE(bench.run(source) == Result::Good);
        CHECK(bench.run("release img") == Result::Good);
    }

    SECTION("Clone")
    {
        REQUIRE(bench.run(source) == Result::Good);
        CHECK(bench.run("img2 = clone(img)") == Result::Good);
        CHECK(bench.run("release img img2") == Result::Good);
    }

    SECTION("Save")
    {
        REQUIRE(bench.run(source) == Result::Good);
        std::string_view save = "save img " CMAKE_BINARY_DIR "/test/pngtest16rgba_OUTPUT.png";
        CHECK(bench.run(save) == Result::Good);
        CHECK(bench.run("release img") == Result::Good);
    }
}

