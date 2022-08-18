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

#include <memory>

#include "driver.hpp"
#include "error.hpp"
#include "object.hpp"
#include "object_cl.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

std::shared_ptr<Object> Testbench::evaluateClone(TokenStream& tokens)
{
    if (!tokens.expect(Token::OpenParen))
        throw CommandError("Expected '(' for clone expression.", tokens.current());

    // Expect an object reference.
    Token objToken = tokens.current();
    if (objToken.mType != Token::String)
        throw CommandError("Expected object identifier.", objToken);

    auto objectToken = tokens.current();
    auto object = evaluate(tokens);
    assert(object && "nullptr objects not expected");

    if (!tokens.expect(Token::CloseParen))
        throw CommandError("Expected ')' for clone expression.", tokens.current());

    // How the object is cloned depends on the object itself.
    if (auto* memObj = dynamic_cast<MemoryObject*>(object.get())) {
        assert(mDriver && "How do we have a CL object without a driver?");
        // This may be an image or memory buffer.
        std::unique_ptr<MemoryObject> clone;
        if (memObj->data.mDescriptor.image_width != 0) {
            clone = mDriver->createImage(memObj->data.mFormat, memObj->data.mDescriptor);
            Driver::ImageCoords region;
            region[0] = memObj->data.mDescriptor.image_width;
            region[1] = memObj->data.mDescriptor.image_height;
            region[2] = memObj->data.mDescriptor.image_depth;
            if (region[2] == 0) {
                region[2] = 1;
                if (region[1] == 0)
                    region[1] = 1;
            }
            mDriver->copyImage(*memObj, *clone, {}, {}, region);
        } else {
            clone = mDriver->createBuffer(memObj->data.mBufferSize);
            mDriver->copyBuffer(*memObj, *clone, 0, 0, memObj->data.mBufferSize);
        }
        return clone;
    } else if (auto* kernelObj = dynamic_cast<KernelObject*>(object.get())) {
        assert(mDriver && "How do we have a CL object without a driver?");
        return mDriver->cloneKernel(*kernelObj);
    }

    throw CommandError("Cannot clone given object", objectToken);
}
