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

TEST_CASE("PNG")
{
    SECTION("read8rgba")
    {
        CLTestbench::TokenStream tokens("file(" PROJECT_SOURCE_DIR "/test/pngtest8rgba.png)");
        CLTestbench::Testbench bench;
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        REQUIRE(object != nullptr);
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 91);
        CHECK(image->descriptor().image_height == 69);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT8);
        CHECK(image->format().image_channel_order == CL_RGBA);
    }

    SECTION("Write8rgba")
    {
        auto saveCmd =
            "save file(" PROJECT_SOURCE_DIR "/test/pngtest8rgba.png) " CMAKE_BINARY_DIR "/test/pngtest8rgba_OUTPUT.png";
        CLTestbench::Testbench bench;
        REQUIRE_NOTHROW(bench.run(saveCmd) == CLTestbench::Testbench::Result::Good);

        CLTestbench::TokenStream tokens("file(" CMAKE_BINARY_DIR "/test/pngtest8rgba_OUTPUT.png)");
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 91);
        CHECK(image->descriptor().image_height == 69);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT8);
        CHECK(image->format().image_channel_order == CL_RGBA);
    }

    SECTION("pngtest16rgba")
    {
        CLTestbench::TokenStream tokens("file(" PROJECT_SOURCE_DIR "/test/pngtest16rgba.png)");
        CLTestbench::Testbench bench;
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        REQUIRE(object != nullptr);
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 32);
        CHECK(image->descriptor().image_height == 32);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT16);
        CHECK(image->format().image_channel_order == CL_RGBA);
    }

    SECTION("Write16rgba")
    {
        auto saveCmd = "save file(" PROJECT_SOURCE_DIR "/test/pngtest16rgba.png) " CMAKE_BINARY_DIR
                       "/test/pngtest16rgba_OUTPUT.png";
        CLTestbench::Testbench bench;
        REQUIRE(bench.run(saveCmd) == CLTestbench::Testbench::Result::Good);

        CLTestbench::TokenStream tokens("file(" CMAKE_BINARY_DIR "/test/pngtest16rgba_OUTPUT.png)");
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        REQUIRE(object != nullptr);
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 32);
        CHECK(image->descriptor().image_height == 32);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT16);
        CHECK(image->format().image_channel_order == CL_RGBA);
    }

    SECTION("saturation map")
    {
        CLTestbench::TokenStream tokens("file(" PROJECT_SOURCE_DIR "/test/sat.png)");
        CLTestbench::Testbench bench;
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 256);
        CHECK(image->descriptor().image_height == 256);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT8);
        CHECK(image->format().image_channel_order == CL_RGB);
    }

    SECTION("Write saturation map")
    {
        auto saveCmd = "save file(" PROJECT_SOURCE_DIR "/test/sat.png) " CMAKE_BINARY_DIR "/test/sat_OUTPUT.png";
        CLTestbench::Testbench bench;
        REQUIRE_NOTHROW(bench.run(saveCmd) == CLTestbench::Testbench::Result::Good);

        CLTestbench::TokenStream tokens("file(" CMAKE_BINARY_DIR "/test/sat_OUTPUT.png)");
        std::shared_ptr<CLTestbench::Object> object;
        REQUIRE_NOTHROW(object = bench.evaluate(tokens));
        auto image = dynamic_cast<CLTestbench::ImageObject*>(object.get());
        REQUIRE(image != nullptr);
        CHECK(image->descriptor().image_width == 256);
        CHECK(image->descriptor().image_height == 256);
        CHECK(image->descriptor().image_type == CL_MEM_OBJECT_IMAGE2D);
        CHECK(image->format().image_channel_data_type == CL_UNSIGNED_INT8);
        CHECK(image->format().image_channel_order == CL_RGB);
    }
}
