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

std::shared_ptr<Object> Testbench::evaluateProgram(TokenStream& tokens)
{
    // Expect a '('.
    auto paren = tokens.current();
    if (paren.mType != Token::OpenParen)
        throw CommandError("Expected '(' for 'program' function.", paren);
    tokens.advance();

    // The next token might be a string with the program source.
    std::string programSource;
    std::shared_ptr<Object> sourceObject;
    if (tokens.current().mType == Token::Text) {
        programSource = tokens.getUnquotedText(tokens.current());
    } else {
        // Save the next token for diagnostics.
        auto nextToken = tokens.current();
        // This could be some expression which will evaluate to the source.
        sourceObject = evaluate(tokens);
        // Whatever source object was parsed needs to be usable as a string.
        auto* data = dynamic_cast<DataObject*>(sourceObject.get());
        if (!data) {
            throw CommandError("Provided argument cannot be used as string data", nextToken);
        }
        programSource = std::string(static_cast<const char*>(data->data()), data->size());
    }

    std::string buildOpts;
    if (tokens.current().mType == Token::Comma) {
        // Build options were specified.
        tokens.advance();
        if (tokens.current().mType != Token::Text) {
            throw CommandError("Expected build options string for 'program' command.", tokens.current());
        }
        buildOpts = tokens.getUnquotedText(tokens.consume());
    }

    paren = tokens.current();
    if (paren.mType != Token::CloseParen)
        throw CommandError("Expected ')' for 'program' function.", paren);
    tokens.advance();

    auto program = mDriver->createProgram(programSource);

    // Intercept build failures here
    try {
        mDriver->buildProgram(*program, buildOpts.c_str());
    } catch (const Driver::Error& e) {
        if (e.mError == CL_BUILD_PROGRAM_FAILURE) {
            *mErr << "Program build failure:\n" << mDriver->programBuildLog(*program) << '\n';
        }

        // We need to rethrow to break out of nested operations.
        throw;
    }

    return program;
}
