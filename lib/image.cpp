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

#include "constant.hpp"
#include "driver.hpp"
#include "error.hpp"
#include "istringview.hpp"
#include "object_image.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
uint8_t NumberOfChannels(cl_channel_order order) noexcept
{
    switch (order) {
    case CL_R: return 1;
    case CL_RA: return 2;
    case CL_RGB: return 3;
    case CL_RGBA: return 4;
    }
    assert(false && "Unrecognised channel order");
    // This is a user-assistance function, and thus if the
    // channel order isn't recognised, we can just return 0.
    // This will cause the computed image size to be 0 too,
    // skipping the image buffer size checks.
    return 0;
}

uint8_t ChannelSize(cl_channel_type type) noexcept
{
    switch (type) {
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
        return 1;
    case CL_SIGNED_INT16:
    case CL_UNSIGNED_INT16:
    case CL_HALF_FLOAT:
        return 2;
    case CL_SIGNED_INT32:
    case CL_UNSIGNED_INT32:
    case CL_FLOAT:
        return 4;
    }
    assert(false && "Unrecognised channel order");
    // This is a user-assistance function, and thus if the
    // channel size isn't recognised, we can just return 0.
    // This will cause the computed image size to be 0 too,
    // skipping the image buffer size checks.
    return 0;
}
} // namespace

std::shared_ptr<Object> Testbench::evaluateImage(TokenStream& tokens)
{
    if (!tokens.expect(Token::OpenParen)) throw CommandError("Expected '(' for 'image' function.", tokens.current());

    // First argument will be a data object.
    auto objectToken = tokens.current();
    auto evaluated = evaluate(tokens);
    const DataObject* data = dynamic_cast<DataObject*>(evaluated.get());
    if (!data) { throw CommandError("Expected data object", objectToken); }

    bool dataIsImage = false;
    cl_image_format format{};
    cl_image_desc desc{};

    // If this Data object is an image, use that.
    if (auto image = dynamic_cast<const ImageObject*>(data)) {
        format = image->format();
        desc = image->descriptor();
        dataIsImage = true;
    }

    if (tokens.expect(Token::Comma)) {
        auto token = tokens.expect(Token::Constant);
        if (!token) throw CommandError("Expected constant for image width value.", tokens.current());
        desc.image_width = tokens.parseConstant<size_t>(token);

        if (!tokens.expect(Token::Comma)) { throw CommandError("Expected ','.", tokens.current()); }

        token = tokens.expect(Token::Constant);
        if (!token) throw CommandError("Expected constant for image height value.", tokens.current());
        desc.image_height = tokens.parseConstant<size_t>(token);

        desc.image_type = desc.image_height > 1 ? CL_MEM_OBJECT_IMAGE2D : CL_MEM_OBJECT_IMAGE1D;

        if (!tokens.expect(Token::Comma)) { throw CommandError("Expected ','.", tokens.current()); }

        token = tokens.expect(Token::String);
        if (!token)
            throw CommandError("Expected image format.  See 'help expression image' for info.", tokens.current());

        IStringView formatStr = tokens.getTokenText(token);
        if (formatStr == "cl_r") {
            format.image_channel_order = CL_R;
        } else if (formatStr == "cl_ra") {
            format.image_channel_order = CL_RA;
        } else if (formatStr == "cl_rgb") {
            format.image_channel_order = CL_RGB;
        } else if (formatStr == "cl_rgba") {
            format.image_channel_order = CL_RGBA;
        } else {
            throw CommandError("Unrecognised image format.  See 'help expression image' for info.", token);
        }

        if (!tokens.expect(Token::Comma)) { throw CommandError("Expected ','.", tokens.current()); }

        token = tokens.expect(Token::String);
        if (!token)
            throw CommandError("Expected image type.  See 'help expression image' for info.", tokens.current());
        IStringView typeStr = tokens.getTokenText(token);

        const std::initializer_list<std::string_view> types{"char", "uchar", "short", "ushort",
                                                            "int",  "uint",  "half",  "float"};
        switch (typeStr.autocomplete(types)) {
        case 0: format.image_channel_data_type = CL_SIGNED_INT8; break;
        case 1: format.image_channel_data_type = CL_UNSIGNED_INT8; break;
        case 2: format.image_channel_data_type = CL_SIGNED_INT16; break;
        case 3: format.image_channel_data_type = CL_UNSIGNED_INT16; break;
        case 4: format.image_channel_data_type = CL_SIGNED_INT32; break;
        case 5: format.image_channel_data_type = CL_UNSIGNED_INT32; break;
        case 6: format.image_channel_data_type = CL_HALF_FLOAT; break;
        case 7: format.image_channel_data_type = CL_FLOAT; break;
        case IStringView::ambiguous: throw CommandError("Ambiguous image type.", token);
        case std::string_view::npos: throw CommandError("Unrecognised image type.", token);
        }
    } else if (!dataIsImage) {
        throw CommandError("Expected image properties for 'image' argument.  "
                           "See 'help expression image' for info.", tokens.current());
    }

    if (!tokens.expect(Token::CloseParen)) {
        throw CommandError("Expected ')' for image expression.", tokens.current());
    }

    // The OpenCL implementation will copy the data object into the image buffer
    // in the creation call.  However, because the input for the image dimensions
    // and data source might be human-provided, it's very much possible that a typo
    // is present and out-of-bounds memory is going to get accessed.  Protect the
    // Testbench users against themselves, and verify that the data size is sufficient.
    size_t imageSize = desc.image_width;
    if (desc.image_height != 0)
        imageSize *= desc.image_height;
    imageSize *= NumberOfChannels(format.image_channel_order);
    imageSize *= ChannelSize(format.image_channel_data_type);

    if (imageSize > data->size()) {
        throw CommandError([=](std::ostream& out) {
            out << "Computed image size of " << imageSize << " bytes is larger than the given data object.";
        }, objectToken);
    }

    return mDriver->createImage(format, desc, data->data());
}
