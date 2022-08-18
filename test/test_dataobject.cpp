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

#include "object_data.hpp"
#include "testbench.hpp"
#include "token.hpp"

TEST_CASE("Parse integer data objects")
{
    SECTION("char")
    {
        CLTestbench::TokenStream tokens("char(0, 1, 2, -3, 4, 127)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 6);

        const auto* numbers = static_cast<const int8_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 1);
        CHECK(numbers[2] == 2);
        CHECK(numbers[3] == -3);
        CHECK(numbers[4] == 4);
        CHECK(numbers[5] == 127);
    }

    SECTION("uchar")
    {
        CLTestbench::TokenStream tokens("uchar(0, 5, 6, 7, 10, 255)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 6);

        const auto* numbers = static_cast<const uint8_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 5);
        CHECK(numbers[2] == 6);
        CHECK(numbers[3] == 7);
        CHECK(numbers[4] == 10);
        CHECK(numbers[5] == 255);
    }

    SECTION("short")
    {
        CLTestbench::TokenStream tokens("short(0, 256, 512, 1024, -2048, 32767)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 12);

        const auto* numbers = static_cast<const int16_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 256);
        CHECK(numbers[2] == 512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == -2048);
        CHECK(numbers[5] == 32767);
    }

    SECTION("ushort")
    {
        CLTestbench::TokenStream tokens("ushort(0, 256, 512, 1024, 32767, 65535)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 12);

        const auto* numbers = static_cast<const uint16_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 256);
        CHECK(numbers[2] == 512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == 32767);
        CHECK(numbers[5] == 65535);
    }

    SECTION("int")
    {
        CLTestbench::TokenStream tokens("int(0, 256, -512, 1024, 65535, 2147483647)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 24);

        const auto* numbers = static_cast<const int32_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 256);
        CHECK(numbers[2] == -512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == 0xffff);
        CHECK(numbers[5] == 0x7fffffff);
    }

    SECTION("uint")
    {
        CLTestbench::TokenStream tokens("uint(0, 256, 512, 1024, 2147483647, 4294967295)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 24);

        const auto* numbers = static_cast<const uint32_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 256);
        CHECK(numbers[2] == 512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == 0x7fffffff);
        CHECK(numbers[5] == 0xffffffff);
    }

    SECTION("long")
    {
        CLTestbench::TokenStream tokens("long(0, 4294967295, -512, 1024, 65535, 9223372036854775807)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 48);

        const auto* numbers = static_cast<const int64_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 4294967295);
        CHECK(numbers[2] == -512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == 0xffff);
        CHECK(numbers[5] == 0x7fffffffffffffffULL);
    }

    SECTION("ulong")
    {
        CLTestbench::TokenStream tokens("ulong(0, 4294967295, 512, 1024, 9223372036854775807, 18446744073709551615)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 48);

        const auto* numbers = static_cast<const uint64_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 4294967295);
        CHECK(numbers[2] == 512);
        CHECK(numbers[3] == 1024);
        CHECK(numbers[4] == 0x7fffffffffffffffULL);
        CHECK(numbers[5] == 0xffffffffffffffffULL);
    }
}

TEST_CASE("Parse float data objects")
{
    SECTION("half")
    {
        CLTestbench::TokenStream tokens("half(0, 0.5, 1.0, -2.0, 65504, 0.00006103515625)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 12);

        const auto* numbers = static_cast<const uint16_t*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 0b0011100000000000);
        CHECK(numbers[2] == 0x3c00);
        CHECK(numbers[3] == 0b1100000000000000);
        CHECK(numbers[4] == 0x7bff);
        CHECK(numbers[5] == 0x0400);
    }

    SECTION("float")
    {
        CLTestbench::TokenStream tokens("float(0, 0.5, 1.0, -2.0, 65504, 0.00006103515625)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 24);

        const auto* numbers = static_cast<const float*>(data->data());
        CHECK(numbers[0] == 0.0f);
        CHECK(numbers[1] == 0.5f);
        CHECK(numbers[2] == 1.0f);
        CHECK(numbers[3] == -2.0f);
        CHECK(numbers[4] == 65504.0f);
        CHECK(numbers[5] == 0.00006103515625f);
    }

    SECTION("double")
    {
        CLTestbench::TokenStream tokens("double(0, 0.5, 1.0, -2.0, 65504, 0.00006103515625)");
        CLTestbench::Testbench bench;

        auto object = bench.evaluate(tokens);
        REQUIRE(object);
        const CLTestbench::DataObject* data = dynamic_cast<CLTestbench::DataObject*>(object.get());
        REQUIRE(data);
        REQUIRE(data->size() == 48);

        const auto* numbers = static_cast<const double*>(data->data());
        CHECK(numbers[0] == 0);
        CHECK(numbers[1] == 0.5);
        CHECK(numbers[2] == 1.0);
        CHECK(numbers[3] == -2.0);
        CHECK(numbers[4] == 65504.0);
        CHECK(numbers[5] == 0.00006103515625);
    }
}
