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

#include "driver.hpp"
#include "istringview.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

void Testbench::executeInfo(TokenStream& tokens)
{
    IStringView command = tokens.getTokenText(tokens.current());
    if (command.empty()) {
        *mOut << "Missing argument to 'info'.  Use 'help info' for available commands.\n";
        return;
    }

    auto match = command.autocomplete({"library", "platforms", "devices"});
    switch (match) {
    case 0: {
        if (mDriver) {
            *mOut << "Loaded OpenCL implementation: " << mDriver->getLibraryName() << '\n';
        } else {
            *mOut << "No OpenCL library loaded.\n";
        }
        break;
    }
    case 1: {
        if (!mDriver) {
            *mOut << "No OpenCL library loaded.\n";
            return;
        }
        auto platforms = mDriver->getPlatformIDs();
        if (platforms.empty()) {
            *mOut << "No platforms enumerated.\n";
            return;
        }

        for (unsigned i = 0; i < platforms.size(); ++i) {
            *mOut << '[' << i << ']';
            if (platforms[i] == mDriver->mPlatform) *mOut << " (selected)";
            *mOut << " =\n" << mDriver->getPlatformInfo(platforms[i]) << '\n';
        }
        break;
    }
    case 2: {
        if (!mDriver) {
            *mOut << "No OpenCL library loaded.\n";
            return;
        }
        auto devices = mDriver->getDeviceIDs(mDriver->mPlatform);
        for (unsigned i = 0; i < devices.size(); ++i) {
            *mOut << '[' << i << ']';
            if (devices[i] == mDriver->mDevice) *mOut << " (selected)";
            *mOut << " =\n" << mDriver->getDeviceInfo(devices[i]) << '\n';
        }
        break;
    }
    default:
        *mErr << "Unknown info option '" << command << "'.  Use 'help info' for available commands.\n";
        break;
    }
}
