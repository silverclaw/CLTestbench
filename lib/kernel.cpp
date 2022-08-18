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

#include <algorithm>
#include <iostream>
#include <memory>

#include "driver.hpp"
#include "error.hpp"
#include "object.hpp"
#include "object_cl.hpp"
#include "object_data.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

std::shared_ptr<Object> Testbench::evaluateKernel(TokenStream& tokens)
{
    // Expect a '('.
    if (tokens.current().mType != Token::OpenParen)
        throw CommandError("Expected '(' for 'kernel' function.", tokens.current());
    tokens.advance();

    // Expect the first argument to be a program object.
    Token programToken = tokens.current();
    auto programObj = evaluate(tokens);
    auto* program = dynamic_cast<ProgramObject*>(programObj.get());

    if (!program) {
        throw CommandError("Expected a program object for 'kernel' command.", programToken);
    }

    if (tokens.current().mType != Token::Comma)
        throw CommandError("Expected ',' for 'kernel' function.", tokens.current());
    tokens.advance();

    // Now the kernel name.
    Token kernelNameToken = tokens.consume();
    std::string kernelName;
    if (kernelNameToken.mType == Token::Text) {
        kernelName = tokens.getUnquotedText(kernelNameToken);
    } else if (kernelNameToken.mType == Token::String) {
        kernelName = tokens.getTokenText(kernelNameToken);
    } else {
        throw CommandError("Expected kernel name.", kernelNameToken);
    }

    // Expect a ')'.
    if (tokens.current().mType != Token::CloseParen)
        throw CommandError("Expected ')' for 'kernel' function.", tokens.current());
    tokens.advance();

    return mDriver->createKernel(*program, kernelName.c_str());
}
