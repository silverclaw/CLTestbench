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

#include <filesystem>
#include <fstream>
#include <vector>

#include "cltb_config.h"
#include "driver.hpp"
#include "error.hpp"
#include "object_cl.hpp"
#include "object_data.hpp"
#include "object_image.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
/// Used to provide image-like properties from a MemoryObject instance.
class ImageDataWrapper final : public ImageObject
{
public:
    CLImageData mImageData;
    const void* mData;

    cl_image_format format() const noexcept override { return mImageData.mFormat; }
    cl_image_desc descriptor() const noexcept override { return mImageData.mDescriptor; }
    std::size_t size() const noexcept override { return mImageData.mBufferSize; }
    const void* data() const noexcept override { return mData; }

    // This is a short-lived object, so this will never be called.
    std::string_view type() const noexcept override { return ""; }
};
}

void Testbench::executeSave(TokenStream& tokens)
{
    // Expect an object reference.
    Token objToken = tokens.current();
    if (objToken.mType != Token::String)
        throw CommandError("Expected object identifier.", objToken);

    auto object = evaluate(tokens);
    assert(object && "nullptr objects not expected");

    std::vector<char> memObjData;
    const char* dataPtr = nullptr;
    std::size_t dataSize = 0;

    auto filename = TrimWhitespace(tokens.currentText());
    std::filesystem::path filepath(filename);

    // Must be a data object.
    if (auto* data = dynamic_cast<DataObject*>(object.get())) {
        dataPtr = static_cast<const char*>(data->data());
        dataSize = data->size();
#if USE_LIBPNG
        if (filepath.extension() == ".png") {
            if (ImageObject* imgObj = dynamic_cast<ImageObject*>(data)) {
                WritePNG(*imgObj, objToken, tokens.currentTextAsToken(), filename);
                return;
            } else if (mOptions.verbose) {
                *mOut << "A PNG filename was given, but object is not an image.  Writing raw data.\n";
            }
        }
#endif // USE_LIBPNG
    } else if (auto* memObj = dynamic_cast<MemoryObject*>(object.get())) {
        assert(mDriver && "How do we have a memory object without a driver?");
        dataSize = memObj->data.mBufferSize;
        memObjData.resize(dataSize);
        if (memObj->data.mDescriptor.image_width != 0) {
            Driver::ImageCoords region;
            region[0] = memObj->data.mDescriptor.image_width;
            region[1] = memObj->data.mDescriptor.image_height;
            region[2] = memObj->data.mDescriptor.image_depth;
            if (region[2] == 0) {
                region[2] = 1;
                if (region[1] == 0)
                    region[1] = 1;
            }
            mDriver->readImage(*memObj, memObjData.data(), {}, region);
        } else {
            mDriver->readBuffer(*memObj, memObjData.data(), 0, dataSize);
        }
        dataPtr = memObjData.data();
#if USE_LIBPNG
        if (filepath.extension() == ".png") {
            if (memObj->data.mDescriptor.image_width != 0) {
                ImageDataWrapper image;
                image.mImageData = memObj->data;
                image.mData = dataPtr;
                WritePNG(image, objToken, tokens.currentTextAsToken(), filename);
                return;
            } else if (mOptions.verbose) {
                *mOut << "A PNG filename was given, but object is not an image.  Writing raw data.\n";
            }
        }
#endif // USE_LIBPNG
    } else if (auto* progObj = dynamic_cast<ProgramObject*>(object.get())) {
        assert(mDriver && "How do we have a program object without a driver?");
        memObjData = mDriver->programBinary(*progObj);
    } else {
        throw CommandError("Cannot get object data.", objToken);
    }

    std::ofstream file(filepath);

    if (!file.is_open()) {
        throw CommandError([=](std::ostream& out) { out << "Could not open " << filepath << ".\n"; });
    }

    file.write(dataPtr, static_cast<std::streamsize>(dataSize));
    if (file.fail()) {
        *mErr << "Output error when writing to " << filepath << '\n';
    }
}
