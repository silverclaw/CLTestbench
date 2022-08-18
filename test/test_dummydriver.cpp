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

#include <sstream>

#include "cltb_config.h"
#include "testbench.hpp"
#include "token.hpp"
#include "voidstream.hpp"

TEST_CASE("Dummy driver")
{
    SECTION("load command")
    {
        CLTestbench::TokenStream tokens("load " CMAKE_BINARY_DIR "/test/libdummycl.so");

        CLTestbench::Testbench bench;
        std::ostringstream out;
        VoidStream err;
        bench.resetOutput(out);
        bench.resetErrorOutput(err);
        CHECK_NOTHROW(bench.run(tokens) == CLTestbench::Testbench::Result::Good);
        std::string verbose = out.str();
        CHECK(verbose == "Loaded " CMAKE_BINARY_DIR "/test/libdummycl.so\n");
    }

    SECTION("quit command")
    {
        CLTestbench::TokenStream tokens("quit");

        CLTestbench::Testbench bench;
        CHECK_NOTHROW(bench.run(tokens) == CLTestbench::Testbench::Result::Quit);
    }
}
