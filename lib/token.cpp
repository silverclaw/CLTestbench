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
#include <cassert>
#include <cctype>
#include <iostream>

#include "error.hpp"
#include "token.hpp"

using namespace CLTestbench;

std::ostream& CLTestbench::operator<<(std::ostream& out, const Token& token)
{
    out << '{';

#define MatchType(type) \
    case Token::type: out << #type; break
    switch (token.mType) {
    MatchType(Constant);
    MatchType(String);
    MatchType(Text);
    MatchType(Equal);
    MatchType(Comma);
    MatchType(OpenParen);
    MatchType(CloseParen);
    MatchType(End);
    default: out << "(invalid)";
    }
#undef MatchType

    out << '}';
    return out;
}

namespace
{
bool IsTokenSeparator(char c)
{
    if (std::isalnum(c)) return false;

    switch (c) {
    case '_':
    case '-':
    case '+':
    case '~':  // Home-directory character
    case '.':  // File extension separator
    case '/':  // Unix path separator
    case '\'': // Windows path separator
        return false;
    default: break;
    }

    return true;
}

bool LooksLikeNumber(std::string_view input) noexcept
{
    if (input[0] == '-' || input[0] == '+')
        input = input.substr(1);
    if (input[0] >= '0' && input[0] <= '9')
        return true;
    return false;
}

Token ParseToken(std::string_view input, std::size_t index) noexcept
{
    auto start = input.find_first_not_of(" \t\n\r", index);
    if (start == std::string_view::npos) return Token{Token::End, index, index};

#define MatchChar(c, tok) \
    case c: \
        return Token \
        { \
            Token::tok, start, start + 1 \
        }
    switch (input[start]) {
        MatchChar('=', Equal);
        MatchChar(',', Comma);
        MatchChar('(', OpenParen);
        MatchChar(')', CloseParen);
    }
#undef MatchChar

    auto startIt = std::begin(input) + start;
    decltype(startIt) endIt;

    Token::Type type = Token::String;

    if (input[start] == '"') {
        bool escape = false;
        for (endIt = startIt + 1; endIt != std::end(input); ++endIt) {
            // Search for the matching ", skipping escaped ones.
            if (*endIt == '"' && !escape) {
                break;
            } else if (*endIt == '\\' && !escape) {
                escape = true;
            } else {
                escape = false;
            }
        }
        if (endIt != std::end(input)) {
            type = Token::Text;
            // Include terminating " on the text.
            ++endIt;
        } else
            type = Token::Invalid;
    } else {
        endIt = std::find_if(startIt, std::end(input), IsTokenSeparator);
        if (LooksLikeNumber(input.substr(start))) type = Token::Constant;
    }

    const std::size_t end = std::distance(std::begin(input), endIt);

    return Token{type, start, end};
}
} // namespace

std::string_view CLTestbench::TrimWhitespace(std::string_view in) noexcept
{
    auto start = in.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    auto end = in.find_last_not_of(" \t\n\r");
    assert(end != std::string_view::npos);
    return in.substr(start, end - start + 1);
}

TokenStream::TokenStream(std::string_view line) noexcept
    : mLine(line), mToken(ParseToken(mLine, 0))
{
}

Token TokenStream::next() noexcept
{
    assert(mToken && "Need the current token parsed for line offset");
    // Return an Invalid token.
    if (!(*this)) return {};
    if (!mNext) mNext = ParseToken(mLine, mToken.mEnd);
    return mNext;
}

void TokenStream::advance() noexcept
{
    // Preserve the end-of-stream token.
    if (mToken.mType == Token::End)
        return;
    mToken = next();
    mNext = Token{};
    mIndex = mToken.mEnd;
}

std::string TokenStream::getUnquotedText(const Token& token) const
{
    assert(token.mType == Token::Text && "This is only valid for text tokens.");

    std::string_view source = getTokenText(token);
    assert(source.size() >= 2 && "At least the opening/closing quotes must be present.");
    // Strip leading and trailing quotation marks
    source = source.substr(1, source.size() - 2);

    // Scan for escape sequences.
    std::string ret;
    ret.reserve(source.length());

    bool escape = false;
    for (auto c : source) {
        if (escape) {
            switch (c) {
            case 'n': ret += '\n'; break;
            case 't': ret += '\t'; break;
            case '\\': ret += '\\'; break;
            case '"': ret += '"'; break;
            // An exception should be thrown here - invalid/unrecognised escape sequence.
            default: break;
            }
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }

        ret += c;
    }

    return ret;
}
