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
#include "constant.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

template<typename T>
std::string_view YesNo(T v)
{
    if (static_cast<bool>(v)) return "yes";
    return "no";
}

void Testbench::executeSet(TokenStream& tokens)
{
    if (!tokens) {
        *mOut << "Options:"
                 "\n  verbose:  " << YesNo(mOptions.verbose) <<
                 "\n  caret:    " << YesNo(mOptions.caretPrint) <<
                 "\n  echo:     " << YesNo(mOptions.scriptEcho) << '\n';
        if (mDriver)
            *mOut << "Driver blocking commands: " << YesNo(mDriver->mBlock) << '\n';
        return;
    }

    Token optionToken = tokens.expect(Token::String);
    if (!optionToken) throw CommandError("Unknown option.", optionToken);

    IStringView optionStr = tokens.getTokenText(optionToken);
    const std::initializer_list<std::string_view> options{"verbose", "caret", "echo", "block"};

    Token valueToken = tokens.consume();
    if (!valueToken) throw CommandError("Missing argument for 'set' command.", optionToken);

    switch (optionStr.autocomplete(options)) {
    case 0: mOptions.verbose = tokens.parseConstant<bool>(valueToken); break;
    case 1: mOptions.caretPrint = tokens.parseConstant<bool>(valueToken); break;
    case 2: mOptions.scriptEcho = tokens.parseConstant<bool>(valueToken); break;
    case 3:
        if (!mDriver) throw CommandError("No driver loaded.", optionToken);
        mDriver->mBlock = tokens.parseConstant<bool>(valueToken); break;
    default: throw CommandError("Unknown option.", optionToken);
    }
}
