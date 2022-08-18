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

#pragma once
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace CLTestbench
{
struct [[nodiscard]] Token final
{
    enum Type : uint8_t
    {
        Invalid,

        Constant,
        String,
        Text,

        Comma,
        Equal,
        OpenParen,
        CloseParen,

        End
    };

    Type mType = Invalid;

    /// Index where the token was defined on the input line.
    std::size_t mBegin = 0, mEnd = 0;

    /// Checks validity of this token.
    explicit operator bool() const noexcept { return mType != Invalid; }

    /// Construct the text of this token from the original line.
    std::string_view text(std::string_view line) const noexcept
    {
        assert(mEnd >= mBegin);
        return line.substr(mBegin, mEnd - mBegin);
    }

    operator std::pair<std::size_t, std::size_t>() const noexcept { return {mBegin, mEnd}; }
};

std::ostream& operator<<(std::ostream& out, const Token& token);

class TokenStream final
{
    /// Line being parsed.
    std::string_view mLine;
    /// Current parse index into mLine.
    std::size_t mIndex = 0;
    /// The current processed token.
    Token mToken;
    /// The upcoming token.
    Token mNext;

    TokenStream() = delete;

public:
    explicit TokenStream(std::string_view line) noexcept;

    /// Returns the text from this token.
    std::string_view getTokenText(const Token& token) const noexcept { return token.text(mLine); }
    /// If a token is text based, remove quotation marks and un-escape characters.
    std::string getUnquotedText(const Token& token) const;

    /// Peek at the next token.
    Token next() noexcept;

    /// Retrieves current token.
    Token current() const noexcept { return mToken; }

    /// Consumes the current token, returning it.
    Token consume() noexcept
    {
        Token copy = current();
        advance();
        return copy;
    }

    /// Advances the stream.
    void advance() noexcept;

    /// If the current token is of the given type, consume it and return the token.
    /// Otherwise, return an invalid token.
    Token expect(Token::Type type) noexcept
    {
        if (current().mType != type) return {};
        return consume();
    }

    /// Checks if this stream has more tokens.
    explicit operator bool() noexcept
    {
        Token t = current();
        return t.mType != Token::End && t.mType != Token::Invalid;
    }

    /// Returns the remaining text to be parsed, excluding current token.
    /// This does not strip token separators.  Run TrimWhitespace.
    std::string_view remainingText() const noexcept { return mLine.substr(mIndex); }

    /// Returns a text token with all remaining text.
    /// This is useful for diagnostics on commands where the entire remaining output is used.
    Token remainingTextAsToken() const noexcept {
        // This will return the end token if the stream is at the end.
        if (!mToken || mToken.mType == Token::End) return mToken;
        Token ret;
        ret.mType = Token::String;
        ret.mBegin = mIndex;
        ret.mEnd = mLine.size();
        return ret;
    }

    /// Returns the current parsing line, including the current token.
    std::string_view currentText() const noexcept { return mLine.substr(mToken.mBegin); }

    /// Returns a text token with all remaining text.
    /// This is useful for diagnostics on commands where the entire remaining output is used.
    Token currentTextAsToken() const noexcept {
        // This will return the end token if the stream is at the end.
        if (!mToken || mToken.mType == Token::End) return mToken;
        Token ret;
        ret.mType = Token::String;
        ret.mBegin = mToken.mBegin;
        ret.mEnd = mLine.size();
        return ret;
    }

    /// Returns the text that generated this stream.
    std::string_view text() const noexcept { return mLine; }

    /// Parse this token as a constant of type T.
    // This is defined in constant.h and that must be included.
    template<typename T>
    T parseConstant(Token token) const;
};

/// Trims whitespace from the start and end of this string view.
std::string_view TrimWhitespace(std::string_view) noexcept;
} // namespace CLTestbench
