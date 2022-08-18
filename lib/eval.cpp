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
#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#include "constant.hpp"
#include "error.hpp"
#include "object.hpp"
#include "object_data.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
class HalfFP final
{
    uint16_t mData;

    struct HalfData
    {
        uint16_t significand : 10;
        uint16_t exponent : 5;
        uint16_t sign : 1;
    };

    struct DoubleData
    {
        uint64_t significand : 52;
        uint64_t exponent : 11;
        uint64_t sign : 1;
    };

public:
    HalfFP(double in) noexcept
    {
        HalfData hd;
        if (std::abs(in) > HalfFP::max()) {
            mData = in > 0 ? 0x7c00 : 0xfc00;
            return;
        }

        if (std::abs(in) < HalfFP::min()) {
            mData = 0;
            return;
        }

        DoubleData dd;
        static_assert(sizeof(double) == sizeof(DoubleData));
        memcpy(&dd, &in, sizeof(dd));
        hd.sign = dd.sign;

        hd.exponent = (dd.exponent - 1023 + 15);
        // This will truncate the fraction.
        hd.significand = (dd.significand >> (52 - 10));

        static_assert(sizeof(uint16_t) == sizeof(HalfData));
        memcpy(&mData, &hd, sizeof(HalfData));
    }

    operator uint16_t() const noexcept { return mData; }

    static constexpr double max() noexcept { return 65504.0; }

    static constexpr double min() noexcept { return 0x1.0p-14; }
};

template<typename T>
class ParsedDataObject final : public DataObject
{
    std::vector<T> mData;

public:
    ParsedDataObject(std::vector<T>&& data) : mData(std::move(data)) {}

    std::size_t size() const noexcept override { return mData.size() * sizeof(T); }

    const void* data() const noexcept override { return mData.data(); }

    std::string_view type() const noexcept override
    {
        if constexpr (std::is_same_v<T, HalfFP>) {
            return "half data";
        }
        if constexpr (std::is_same_v<T, float>) {
            return "float data";
        }
        if constexpr (std::is_same_v<T, double>) {
            return "double data";
        }
        if constexpr (std::is_same_v<T, int8_t>) {
            return "char data";
        }
        if constexpr (std::is_same_v<T, uint8_t>) {
            return "uchar data";
        }
        if constexpr (std::is_same_v<T, int16_t>) {
            return "short data";
        }
        if constexpr (std::is_same_v<T, uint16_t>) {
            return "ushort data";
        }
        if constexpr (std::is_same_v<T, int32_t>) {
            return "int data";
        }
        if constexpr (std::is_same_v<T, uint32_t>) {
            return "uint data";
        }
        if constexpr (std::is_same_v<T, int64_t>) {
            return "long data";
        }
        if constexpr (std::is_same_v<T, uint64_t>) {
            return "ulong data";
        }
        return "CLType data";
    }
};

template<typename T>
std::shared_ptr<ParsedDataObject<T>> ParseBuffer(TokenStream& tokens)
{
    std::vector<T> values;

    using ParseTy = typename std::conditional<std::is_same_v<T, HalfFP>, double, T>::type;

    // Expect '('.
    auto token = tokens.current();
    if (token.mType != Token::OpenParen) throw CommandError("Expected '(' for data object creation.", token);

    do {
        tokens.advance();
        token = tokens.expect(Token::Constant);
        if (!token) throw CommandError("Expected a constant token.", token);

        T value = tokens.parseConstant<ParseTy>(token);
        values.push_back(value);
    } while (tokens.current().mType == Token::Comma);

    // Expect ')'.
    token = tokens.expect(Token::CloseParen);
    if (!token) throw CommandError("Expected ')' for data object creation.", token);

    return std::make_shared<ParsedDataObject<T>>(std::move(values));
}
} // namespace

std::shared_ptr<Object> Testbench::evaluate(TokenStream& tokens)
{
    if (!tokens) throw CommandError("Unexpected end-of-input.");

    auto token = tokens.consume();
    if (token.mType == Token::OpenParen) {
        // Tolerate parenthesis expressions.
        auto object = evaluate(tokens);
        // Expect a closing parenthesis.
        if (!tokens.expect(Token::CloseParen)) {
            throw CommandError("Unmatched open parenthesis.", token);
        }
        return object;
    }

    if (token.mType != Token::String) {
        throw CommandError("Expected identifier or command.", token);
    }

    auto tokenText = tokens.getTokenText(token);

    // This allows object names to shadow (hide) commands.
    // Perhaps not a good idea.
    auto objMatch = std::find_if(std::begin(mObjects), std::end(mObjects),
                                 [&](const auto& pair) { return pair.first == tokenText; });
    if (objMatch != mObjects.end()) {
        return objMatch->second;
    }

    if (tokenText == "file") {
        return evaluateFile(tokens);
    }

    if (tokenText == "clone") {
        return evaluateClone(tokens);
    }

    if (tokenText == "program") {
        if (!mDriver) {
            throw CommandError("A driver is required for a 'program' expression.", token);
        }
        return evaluateProgram(tokens);
    }

    if (tokenText == "kernel") {
        if (!mDriver) {
            throw CommandError("A driver is required for a 'kernel' expression.", token);
        }
        return evaluateKernel(tokens);
    }

    if (tokenText == "buffer") {
        if (!mDriver) {
            throw CommandError("A driver is required for a 'buffer' expression.", token);
        }
        return evaluateBuffer(tokens);
    }

    if (tokenText == "image") {
        if (!mDriver) {
            throw CommandError("A driver is required for a 'image' expression.", token);
        }
        return evaluateImage(tokens);
    }

#define CLTypeParseBuffer(TYPE, CTYPE) \
    if (tokenText == #TYPE) return ParseBuffer<CTYPE>(tokens)

    CLTypeParseBuffer(half, HalfFP);
    CLTypeParseBuffer(float, float);
    CLTypeParseBuffer(double, double);
    CLTypeParseBuffer(char, int8_t);
    CLTypeParseBuffer(uchar, uint8_t);
    CLTypeParseBuffer(short, int16_t);
    CLTypeParseBuffer(ushort, uint16_t);
    CLTypeParseBuffer(int, int32_t);
    CLTypeParseBuffer(uint, uint32_t);
    CLTypeParseBuffer(long, int64_t);
    CLTypeParseBuffer(ulong, uint64_t);

#undef CLTypeParseBuffer

    throw CommandError("Unexpected token.", token);
}
