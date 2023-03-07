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

#include "testbench.hpp"

#include <iostream>

#include "constant.hpp"
#include "driver.hpp"
#include "object_cl.hpp"
#include "object_data.hpp"
#include "token.hpp"

using namespace CLTestbench;

void Testbench::executeBind(TokenStream& tokens)
{
    if (!mDriver) throw CommandError("A driver is required for a 'bind' command.");

    // Three arguments are required: a kernel object, a constant (arg index), and an object.
    Token kernelToken = tokens.current();
    auto kernelObj = evaluate(tokens);
    auto kernel = dynamic_cast<KernelObject*>(kernelObj.get());
    if (!kernel) {
        throw CommandError("Expected kernel object.", kernelToken);
    }

    // Argument index.
    Token indexToken = tokens.expect(Token::Constant);
    unsigned argIndex = tokens.parseConstant<unsigned>(indexToken);

    if (!tokens) {
        throw CommandError("Object expected for bind command.", tokens.current());
    }

    do {
        Token token = tokens.current();
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
        auto evaluated = evaluate(tokens);
        if (auto* memObj = dynamic_cast<MemoryObject*>(evaluated.get())) {
            if (!memObj) throw CommandError("Expected memory object.", token);
            mDriver->setKernelArg(*kernel, argIndex, *memObj);
        } else if (auto* dataObj = dynamic_cast<DataObject*>(evaluated.get())) {
            mDriver->setKernelArg(*kernel, argIndex, dataObj->data(), dataObj->size());
        } else {
            throw CommandError("Unsupported kernel argument type.", token);
        }
    } while(tokens);
}
