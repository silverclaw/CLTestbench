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
#include <charconv>
#include <filesystem>
#include <fstream>
#include <istream>
#include <limits>
#include <streambuf>
#include <vector>

#include <CL/cl.h>

#include "cltb_config.h"
#include "error.hpp"
#include "object_data.hpp"
#include "object_image.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
template<typename T>
T parse(Token token, std::string_view arg)
{
    if (token.mType != Token::Constant) {
        throw CommandError("Expected numeric constant for offset for 'file' function.", token);
    }

    T val;
    auto [ptr, ec] = std::from_chars(arg.begin(), arg.end(), val);
    if (ptr != arg.end() || ec != std::errc()) {
        throw CommandError("could not parse numeric argument for 'file' function.", token);
    }
    return val;
}

class FileDataObject final : public DataObject
{
    const std::string mFilename;
    std::vector<char> mData;

public:
    FileDataObject(std::string filename, std::vector<char> data) : mFilename(filename), mData(std::move(data)) {}

    std::size_t size() const noexcept override { return mData.size(); }

    const void* data() const noexcept override { return static_cast<const void*>(mData.data()); }

    std::string_view type() const noexcept override { return mFilename; }

    ~FileDataObject() = default;
};
} // namespace

std::shared_ptr<Object> Testbench::evaluateFile(TokenStream& tokens)
{
    // Expect a '('.
    auto paren = tokens.consume();
    if (paren.mType != Token::OpenParen)
        throw CommandError("Expected '(' for 'file' function.", paren);

    auto filenameToken = tokens.consume();
    std::string unquotedFilename;
    std::string_view filename;

    if (filenameToken.mType == Token::String) {
        filename = tokens.getTokenText(filenameToken);
    } else if (filenameToken.mType == Token::Text) {
        unquotedFilename = tokens.getUnquotedText(filenameToken);
        filename = unquotedFilename;
    } else {
        throw CommandError("Expected filename for 'file' function.", filenameToken);
    }

    std::streamsize start = 0;
    std::streamsize parsedLen = std::numeric_limits<std::streamsize>::max();
    // This might be needed later for diagnostics in case of a bad start offset.
    Token startToken;
    Token lenToken;
    // Optional parameters: start and end offsets.
    if (tokens.current().mType == Token::Comma) {
        tokens.advance(); // skip comma

        startToken = tokens.consume();
        start = parse<std::streamsize>(startToken, tokens.getTokenText(startToken));

        if (tokens.current().mType == Token::Comma) {
            tokens.advance(); // skip comma
            lenToken = tokens.consume();
            parsedLen = parse<std::streamsize>(lenToken, tokens.getTokenText(lenToken));
        }
    }

    // Expect a ')'.
    if (tokens.current().mType != Token::CloseParen)
        throw CommandError("Expected ')' for 'file' function.", tokens.current());
    tokens.advance();

    std::filesystem::path filepath(filename);
    if (!std::filesystem::exists(filepath)) {
        throw CommandError("File does not exist.", filenameToken);
    }

    std::ifstream file(filepath, std::ios::ate);
    if (!file.is_open()) {
        throw CommandError("Could not open file.", filenameToken);
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize == 0)
        throw CommandError("Empty file.", filenameToken);

    if (start >= fileSize) {
        throw CommandError([=](std::ostream& out) {
                out << "Start offset " << start << " exceeds file size of " << fileSize;
            }, startToken);
    }

    assert(fileSize > start);
    auto length = std::min<std::streamsize>(parsedLen, fileSize - start);
    if (length <= 0) {
        throw CommandError("Internal error on 'file' command: calculated load size is 0.", filenameToken);
    }

    if (lenToken && parsedLen != length && mOptions.verbose) {
        *mOut << "Length of " << parsedLen << " given to 'file' command clamped to " << length << '\n';
    }

    std::vector<char> data;
    data.resize(static_cast<decltype(data)::size_type>(length));
    file.seekg(start);
    file.read(data.data(), length);

#if CLTB_USE_LIBPNG
    if (filepath.extension() == ".png") {
        if (startToken || lenToken) {
            if (mOptions.verbose) {
                *mOut << "Will not auto-create image from PNG: "
                         "start and/or size parameters to 'file' were specified\n";
            }
        } else {
            return LoadPNG(data.data(), data.size(), filenameToken, filename);
        }
    }
#endif // CLTB_USE_LIBPNG

    return std::make_unique<FileDataObject>(std::string(filename), std::move(data));
}
