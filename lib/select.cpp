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

#include <charconv>
#include <iostream>

#include "constant.hpp"
#include "driver.hpp"
#include "error.hpp"
#include "istringview.hpp"
#include "object.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

void Testbench::executeSelect(TokenStream& tokens)
{
    if (!mDriver) {
        *mErr << "No OpenCL implementation loaded.\n";
        return;
    }

    Token commandToken = tokens.current();
    IStringView command = tokens.getTokenText(commandToken);

    if (command.empty() || !tokens) {
        *mOut << "Selected platform: " << mDriver->getPlatformInfo(mDriver->mPlatform)
              << "\nSelected device: " << mDriver->getDeviceInfo(mDriver->mDevice) << '\n';
        return;
    }

    tokens.advance();
    Token argumentToken = tokens.consume();

    if (command == "platform") {
        if (argumentToken.mType == Token::End) {
            if (mOptions.verbose)
                *mOut << "Selected platform: " << mDriver->getPlatformInfo(mDriver->mPlatform) << '\n';
            return;
        }
        if (argumentToken.mType != Token::Constant) {
            throw CommandError("Expected number argument for 'select platform'.", argumentToken);
        }
        uint8_t n = tokens.parseConstant<uint8_t>(argumentToken);
        auto platforms = mDriver->getPlatformIDs();
        if (n >= platforms.size()) {
            throw CommandError(
                "Argument for 'select platform' out of range.  Use 'info platform' for available options.",
                argumentToken);
        }
        auto platformInfo = mDriver->getPlatformInfo(platforms[n]);

        // Selecting a platform will reset the device.
        auto devices = mDriver->getDeviceIDs(platforms[n]);
        if (devices.size() == 0) {
            *mErr << "Cannot change platform: target platform has no devices.\n";
            return;
        }
        auto deviceInfo = mDriver->getDeviceInfo(devices[0]);

        // Finally commit the changes.
        if (auto count = clearDriverObjects()) {
            if (mOptions.verbose) *mOut << "Switching platform cleared " << count << " objects.\n";
        }
        if (mOptions.verbose) {
            *mOut << "Selected platform '" << platformInfo.mName << ' ' << platformInfo.mVersion
                  << "\nResetting device to '" << deviceInfo.mName << "'\n";
        }
        mDriver->mPlatform = platforms[n];
        mDriver->mDevice = devices[0];
    } else if (command == "device") {
        if (argumentToken.mType == Token::End) {
            if (mOptions.verbose) *mOut << "Selected device: " << mDriver->getDeviceInfo(mDriver->mDevice) << '\n';
            return;
        }
        if (argumentToken.mType != Token::Constant) {
            throw CommandError("expected number argument for 'select device'.", argumentToken);
        }
        uint8_t n = tokens.parseConstant<uint8_t>(argumentToken);
        auto devices = mDriver->getDeviceIDs(mDriver->mPlatform);
        if (n >= devices.size()) {
            throw CommandError("Argument for 'select device' out of range.  Use 'info device' for available options.",
                               argumentToken);
        }
        auto deviceInfo = mDriver->getDeviceInfo(devices[n]);
        if (auto count = clearDriverObjects()) {
            if (mOptions.verbose) *mOut << "Switching device cleared " << count << " objects.\n";
        }
        if (mOptions.verbose) *mOut << "Selected device '" << deviceInfo.mName << "'\n";
        mDriver->mDevice = devices[n];
    } else {
        *mErr << "Invalid selection '" << command << "'.  Use 'help select' for info.\n";
        return;
    }

    if (tokens) {
        *mErr << "Trailing tokens on 'select' command ignored.\n";
    }
}
