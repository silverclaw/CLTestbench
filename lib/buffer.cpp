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
#include <limits>
#include <memory>

#include "constant.hpp"
#include "driver.hpp"
#include "error.hpp"
#include "object.hpp"
#include "object_cl.hpp"
#include "object_data.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

std::shared_ptr<Object> Testbench::evaluateBuffer(TokenStream& tokens)
{
    // Expect a '('.
    if (!tokens.expect(Token::OpenParen)) throw CommandError("Expected '(' for 'buffer' function.", tokens.current());

    std::unique_ptr<MemoryObject> buffer;

    if (tokens.current().mType == Token::Constant) {
        // Expect the single-argument buffer command variant.
        auto size = tokens.parseConstant<std::size_t>(tokens.current());
        buffer = mDriver->createBuffer(size);
        tokens.advance();
    } else {
        Token objectToken = tokens.current();
        // Expect a data object as the first argument.
        auto evaluated = evaluate(tokens);
        DataObject* data = dynamic_cast<DataObject*>(evaluated.get());
        if (!data) {
            throw CommandError("Expected data object", objectToken);
        }

        std::size_t start = 0;
        std::size_t len = data->size();
        // Check for start offset.
        if (tokens.current().mType == Token::Comma) {
            tokens.advance();
            Token sizeToken = tokens.consume();
            if (sizeToken.mType != Token::Constant)
                throw CommandError("Expected start offset constant.", sizeToken);
            start = tokens.parseConstant<std::size_t>(sizeToken);
            if (start > data->size())
                throw CommandError("Size argument exceeds data size.", sizeToken);

            // Check for a length value.
            if (tokens.current().mType == Token::Comma) {
                tokens.advance();
                Token lenToken = tokens.consume();
                if (lenToken.mType != Token::Constant)
                    throw CommandError("Expected length constant.", lenToken);
                len = tokens.parseConstant<std::size_t>(lenToken);

                if ((start + len) > data->size())
                    throw CommandError("Length argument exceeds data size.", lenToken);
            }
        }

        buffer = mDriver->createBuffer(len);
        mDriver->writeBuffer(*buffer, data->data(), start, len);
    }

    // Expect a ')'.  Failing at this point might be excessive because we've already created
    // the buffer and set up all the data.  However, in a nested expression, we need to fail.
    if (!tokens.expect(Token::CloseParen)) throw CommandError("Expected ')' for 'buffer' function.", tokens.current());
    tokens.advance();

    return buffer;
}
