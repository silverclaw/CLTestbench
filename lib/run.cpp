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

#include <array>
#include <iostream>
#include <optional>

#include "constant.hpp"
#include "driver.hpp"
#include "error.hpp"
#include "object_cl.hpp"
#include "object_data.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
std::array<std::size_t, 3> ParseDim(TokenStream& tokens)
{
    std::array<std::size_t, 3> ret{0, 0, 0};

    Token token = tokens.consume();
    if (token.mType != Token::Constant)
        throw CommandError("Expected constant for size operand.", token);
    ret[0] = tokens.parseConstant<std::size_t>(token);

    token = tokens.consume();
    if (token.mType == Token::CloseParen)
        return ret;
    // Comma is optional
    if (token.mType == Token::Comma)
        token = tokens.consume();
    if (token.mType != Token::Constant)
        throw CommandError("Expected constant for size operand.", token);
    ret[1] = tokens.parseConstant<std::size_t>(token);

    token = tokens.consume();
    if (token.mType == Token::CloseParen)
        return ret;
    // Comma is optional
    if (token.mType == Token::Comma)
        token = tokens.consume();
    if (token.mType != Token::Constant)
        throw CommandError("Expected constant for size operand.", token);
    ret[2] = tokens.parseConstant<std::size_t>(token);

    token = tokens.consume();
    if (token.mType != Token::CloseParen)
        throw CommandError("Expected ')'.", token);
    return ret;
}
}

void Testbench::executeRun(TokenStream& tokens)
{
    if (!mDriver)
        throw CommandError("An OpenCL implementation must be loaded for a 'run' command.");

    auto kernelToken = tokens.current();
    auto kernel = evaluate(tokens);
    auto* kernelObj = dynamic_cast<KernelObject*>(kernel.get());
    if (!kernelObj)
        throw CommandError("Expected kernel object.", kernelToken);

    // Consume two ( tokens to parse the size argument.
    auto token = tokens.consume();
    if (token.mType != Token::OpenParen)
        throw CommandError("Expected '('.", token);
    token = tokens.consume();
    if (token.mType != Token::OpenParen)
        throw CommandError("Expected '('.", token);

    std::optional<std::array<std::size_t, 3>> localSize;
    auto globalSize = ParseDim(tokens);

    if (!tokens.expect(Token::Comma))
        throw CommandError("Expected ','.", token);

    if (tokens.expect(Token::OpenParen))
        localSize = ParseDim(tokens);

    // Any subsequent tokens enumerate kernel arguments.
    token = tokens.consume();
    if (token.mType != Token::Comma && token.mType != Token::CloseParen)
        throw CommandError("Expected ',' or ')'.", token);

    uint32_t argIndex = 0;
    if (token.mType != Token::CloseParen) {
        do {
            if (token.mType == Token::Constant) {
                // This is a tricky one because it could be any type.
                // The only way of knowing is to query the argument type of the
                // kernel, which isn't implemented at the moment.
                // The type is required to set the appropriate "size" argument
                // to clSetKernelArg.
                throw CommandError([&](std::ostream& out){
                    out << "A type is required for a constant argument.  Example:\n"
                           "  ulong(" << tokens.getTokenText(token) << ')';
                }, token);
            }
            // Anything else we evaluate to a memory object.
            Token objectTokenStart = tokens.current();
            auto evaluated = evaluate(tokens);
            if (auto* memObj = dynamic_cast<MemoryObject*>(evaluated.get())) {
                if (!memObj) throw CommandError("Expected memory object.", objectTokenStart);
                mDriver->setKernelArg(*kernelObj, argIndex, *memObj);
            } else if (auto* dataObj = dynamic_cast<DataObject*>(evaluated.get())) {
                mDriver->setKernelArg(*kernelObj, argIndex, dataObj->data(), dataObj->size());
            } else {
                throw CommandError("Unsupported kernel argument type.", token);
            }
            token = tokens.consume();
            argIndex++;

            if (token.mType != Token::Comma && token.mType != Token::CloseParen)
                throw CommandError("Expected ',' or ')'.", token);
        } while(token.mType != Token::CloseParen);
    }

    mDriver->enqueueKernel(*kernelObj, globalSize, localSize);
}
