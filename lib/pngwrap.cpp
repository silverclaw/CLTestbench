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

#include "cltb_config.h"

#if USE_LIBPNG

#include <linux/limits.h> // PATH_MAX
#include <png.h>
#include <setjmp.h>
#include <cassert>
#include <cstring> // memcpy, memset
#include <vector>

#endif // USE_LIBPNG

#include "error.hpp"
#include "object_image.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

#if USE_LIBPNG

namespace
{
struct PNGImage final : public ImageObject
{
    cl_image_format mFormat;
    cl_image_desc mDescriptor;
    std::string mFilename;
    std::vector<png_byte> mData;

    cl_image_format format() const noexcept override { return mFormat; }

    cl_image_desc descriptor() const noexcept override { return mDescriptor; }

    std::string_view type() const noexcept override { return mFilename; }

    const void* data() const noexcept override { return mData.data(); }

    std::size_t size() const noexcept override { return mData.size(); }

    ~PNGImage() = default;
};

class PNGDecoder
{
public:
    PNGDecoder(const Token& token) : mToken(token)
    {
        mPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!mPNG) {
            throw std::bad_alloc();
        }

        mInfo = png_create_info_struct(mPNG);
        if (!mInfo) {
            png_destroy_read_struct(&mPNG, nullptr, nullptr);
            throw std::bad_alloc();
        }

        mEnd = png_create_info_struct(mPNG);
        if (!mEnd) {
            png_destroy_read_struct(&mPNG, &mInfo, nullptr);
            throw std::bad_alloc();
        }

        png_set_read_fn(mPNG, reinterpret_cast<void*>(&mReadState), &ReadFn);
    }

    std::unique_ptr<PNGImage> read(const png_byte* const data, const size_t size)
    {
        mReadState.data = data;
        mReadState.offset = 0;
        mReadState.dataSize = size;

        readInfo();

        const auto height = png_get_image_height(mPNG, mInfo);
        assert(height > 0);

        const auto rowSize = png_get_rowbytes(mPNG, mInfo);
        assert(rowSize > 0);
        std::vector<png_byte> imageBuffer;
        imageBuffer.resize(rowSize * height);
        std::vector<png_byte*> imageRows;
        imageRows.resize(height);
        for (unsigned i = 0; i < height; ++i) {
            imageRows[i] = imageBuffer.data() + (i * rowSize);
        }

        readImage(imageRows.data());
        readEnd();

        auto ret = std::make_unique<PNGImage>();
        ret->mData = std::move(imageBuffer);
        ret->mDescriptor = genDescriptor();
        ret->mFormat = genFormat();

        return ret;
    }

    ~PNGDecoder() { png_destroy_read_struct(&mPNG, &mInfo, &mEnd); }

private:
    const Token& mToken;
    png_struct* mPNG;
    png_info* mInfo;
    png_info* mEnd;

    struct PtrOffsetPair
    {
        const png_byte* data = nullptr;
        size_t offset = 0;
        size_t dataSize = 0;
    } mReadState;

    void readInfo()
    {
        if (setjmp(png_jmpbuf(mPNG))) {
            throw CommandError("Error reading PNG information.", mToken);
        }
        png_read_info(mPNG, mInfo);
    }

    void readImage(png_byte** rows)
    {
        if (setjmp(png_jmpbuf(mPNG))) {
            throw CommandError("Invalid PNG data.", mToken);
        }
        png_read_image(mPNG, rows);
    }

    void readEnd()
    {
        if (setjmp(png_jmpbuf(mPNG))) {
            throw CommandError("Error reading PNG tail.", mToken);
        }
        png_read_end(mPNG, mEnd);
    }

    cl_image_format genFormat()
    {
        cl_image_format ret;
        switch (png_get_bit_depth(mPNG, mInfo)) {
        case 8: ret.image_channel_data_type = CL_UNSIGNED_INT8; break;
        case 16: ret.image_channel_data_type = CL_UNSIGNED_INT16; break;
        default: throw CommandError("Unsupported PNG bit depth.", mToken);
        }

        switch (png_get_color_type(mPNG, mInfo)) {
        case PNG_COLOR_TYPE_GRAY: ret.image_channel_order = CL_R; break;
        case PNG_COLOR_TYPE_GA: ret.image_channel_order = CL_RA; break;
        case PNG_COLOR_TYPE_RGB: ret.image_channel_order = CL_RGB; break;
        case PNG_COLOR_TYPE_RGBA: ret.image_channel_order = CL_RGBA; break;
        default: throw CommandError("Unsupported PNG colour type", mToken);
        }

        return ret;
    }

    cl_image_desc genDescriptor()
    {
        cl_image_desc ret{};
        ret.image_height = png_get_image_height(mPNG, mInfo);
        ret.image_width = png_get_image_width(mPNG, mInfo);
        ret.image_type = ret.image_height == 1 ? CL_MEM_OBJECT_IMAGE1D : CL_MEM_OBJECT_IMAGE2D;

        return ret;
    }

    static void ReadFn(png_struct* png, png_byte* data, const size_t size)
    {
        void* rawPtr = png_get_io_ptr(png);
        PtrOffsetPair* pngReadState = reinterpret_cast<PtrOffsetPair*>(rawPtr);
        // Bounds-check the input data.
        const auto readSize = std::min<size_t>(size, pngReadState->dataSize - pngReadState->offset);
        auto readPtr = pngReadState->data + pngReadState->offset;
        memcpy(data, readPtr, readSize);
        pngReadState->offset += readSize;

        // Zero-out any out-of-bounds read.
        // Alternative: call png_longjmp to exit out of png reading.
        if (size != readSize) {
            memset(data + readSize, 0, size - readSize);
        }
    }
};

class PNGEncoder
{
public:
    PNGEncoder(std::string_view filename, const Token& token)
    {
        mPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!mPNG) {
            throw std::bad_alloc();
        }

        mInfo = png_create_info_struct(mPNG);
        if (!mInfo) {
            png_destroy_read_struct(&mPNG, nullptr, nullptr);
            throw std::bad_alloc();
        }

        char file[PATH_MAX];
        memcpy(file, filename.data(), filename.size());
        file[filename.size()] = '\0';
        mFP = fopen(file, "wb");
        if (!mFP) {
            throw CommandError("Cannot open file for writing.", token);
        }
    }

    void write(const ImageObject& img, const Token& token)
    {
        if (setjmp(png_jmpbuf(mPNG))) {
            throw CommandError("Invalid PNG data.", token);
        }
        writePNG(img, token);
    }

    ~PNGEncoder()
    {
        if (mFP) fclose(mFP);
        png_destroy_write_struct(&mPNG, &mInfo);
    }

private:
    png_uint_32 genPNGColour(cl_image_format format, const Token& token)
    {
        switch (format.image_channel_order) {
        case CL_R: return PNG_COLOR_TYPE_GRAY;
        case CL_RA: return PNG_COLOR_TYPE_GA;
        case CL_RGB: return PNG_COLOR_TYPE_RGB;
        case CL_RGBA: return PNG_COLOR_TYPE_RGBA;
        default: throw CommandError("Unsupported PNG channel format", token);
        }
    }

    png_uint_32 getPNGBits(cl_image_format format, const Token& token)
    {
        switch (format.image_channel_data_type) {
        case CL_UNSIGNED_INT8: return 8;
        case CL_UNSIGNED_INT16: return 16;
        default: throw CommandError("Unsupported PNG image bit depth", token);
        }
    }

    void writePNG(const ImageObject& img, const Token& token)
    {
        png_init_io(mPNG, mFP);

        auto format = img.format();

        png_uint_32 colour = genPNGColour(format, token);
        png_uint_32 bits = getPNGBits(format, token);

        auto desc = img.descriptor();
        png_set_IHDR(mPNG, mInfo, desc.image_width, desc.image_height, bits, colour, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(mPNG, mInfo);
        const png_byte* data = static_cast<const png_byte*>(img.data());
        const unsigned stride = png_get_rowbytes(mPNG, mInfo);
        if ((stride * desc.image_height) > img.size())
            throw CommandError("Bounds check error: insufficient data for PNG write.", token);
        for (unsigned row = 0; row < desc.image_height; ++row) {
            png_write_row(mPNG, data);
            data += stride;
        }
        png_write_end(mPNG, nullptr);
    }

    FILE* mFP = nullptr;
    png_struct* mPNG = nullptr;
    png_info* mInfo = nullptr;
};
} // namespace

#endif // USE_LIBPNG

std::unique_ptr<ImageObject> CLTestbench::LoadPNG(const void* data, std::size_t size, const Token& token,
                                                  std::string_view filename)
{
#if USE_LIBPNG
    const png_byte* pngData = reinterpret_cast<const png_byte*>(data);
    if (png_sig_cmp(pngData, 0, 8) != 0) {
        throw CommandError("PNG signature check failed.", token);
    }

    PNGDecoder decoder(token);
    auto image = decoder.read(reinterpret_cast<const png_byte*>(data), size);
    assert(image);
    if (!filename.empty()) image->mFilename = filename;
    return image;
#else // USE_LIBPNG
    throw CommandError("PNG support not enabled", token);
#endif // USE_LIBPNG
}

void CLTestbench::WritePNG(const ImageObject& img, const Token& imgToken, const Token& token,
                           std::string_view filename)
{
#if USE_LIBPNG
    PNGEncoder encoder(filename, token);
    encoder.write(img, imgToken);
#else // USE_LIBPNG
    throw CommandError("PNG support not enabled", token);
#endif // USE_LIBPNG
}
