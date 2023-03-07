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

#include <iostream>
#include <getopt.h>

#include "driver.hpp"
#include "editlinewrap.hpp"
#include "testbench.hpp"

namespace
{
const struct option LongOptions[] = {
    {"no-auto-load", no_argument, 0, 0 },
    {"help",         no_argument, 0, 'h'},
    {0,              0,           0, 0 }
};

void PrintHelp(std::string_view binName)
{
    std::cout <<
        "OpenCL Testbench  Usage:\n\n"
        "    " << binName << " [options] [opencl library]\n\n"
        "Options available:\n"
        "  --no-auto-load      Do not load the system default libOpenCL.\n"
        "  -h,--help           Print this help text.\n"
        "\n"
        "The Testbench will attempt to load the specified OpenCL implementation\n"
        "given by the 'opencl library' argument, which must be a shared library.\n"
        "By default, this is 'libOpenCL.so', found on the system configured\n"
        "library search paths.\n";
}
}

int main(int argc, char* const argv[])
{
    std::string_view driverLib("libOpenCL.so");
    std::string_view binaryName(argv[0]);
    bool customLib = false;

    while (true) {
        int index = 0;
        int c = getopt_long(argc, argv, "h", LongOptions, &index);

        switch (c) {
        case 0:
            if (index == 0)
                driverLib = "";
            break;
        case '?':
        case 'h': {
            PrintHelp(binaryName);
            return 0;
        }
        }
        if (c == -1) break;
    }

    if (optind < argc) {
        if ((optind + 1) != argc) {
            std::cerr << "Only one driver library may be specified.\n";
            return -1;
        }
        driverLib = argv[optind];
        customLib = true;
    }

    std::unique_ptr<CLTestbench::Driver> driver;
    if (!driverLib.empty())
    {
        try {
            driver.reset(new CLTestbench::Driver(driverLib));
        } catch (...) {
            std::cerr << driverLib << " did not load.\n";
            if (customLib) return -1;
        }
    }

    try {
        CLTestbench::Testbench testbench{};
        if (driver) testbench.resetDriver(std::move(driver));

        CLTestbench::EditLine input;

        if (!input.isValid()) {
            std::cerr << "ERROR: Could not initialise input.\n";
            return -1;
        }

        auto result = CLTestbench::Testbench::Result::Good;
        do {
            auto line = input.getLine();
            if (line.empty()) continue;
            result = testbench.run(line);
        } while (result != CLTestbench::Testbench::Result::Quit);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Unhandled error during testbench execution: " << e.what() << std::endl;
    }

    return -1;
}
