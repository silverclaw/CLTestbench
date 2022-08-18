// This file is part of CLTestbench.

// CLTestbench is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CLTestbench is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.#include <catch2/catch_test_macros.hpp>

#include <catch2/catch_test_macros.hpp>

#include "constant.hpp"
#include "error.hpp"
#include "token.hpp"

TEST_CASE("Tokens")
{
    SECTION("Empty line")
    {
        const char* empty = "";
        CLTestbench::TokenStream stream(empty);

        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(!stream); // stream is empty
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Whitespace")
    {
        const char* line = " \t\n\r";
        CLTestbench::TokenStream stream(line);

        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(!stream); // stream is empty
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Single command")
    {
        const char* line = "load";
        CLTestbench::TokenStream stream(line);

        auto token = stream.current();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.next().mType == CLTestbench::Token::End);
    }

    SECTION("Command with arg")
    {
        const char* line = "command argument";
        CLTestbench::TokenStream stream(line);

        auto token = stream.current();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "command");
        stream.advance();
        token = stream.current();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "argument");
        CHECK(stream.next().mType == CLTestbench::Token::End);
    }

    SECTION("File paths")
    {
        const char* line = "/some/path/to/file.ext";
        CLTestbench::TokenStream stream(line);

        auto token = stream.current();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.next().mType == CLTestbench::Token::End);
    }

    SECTION("Non-alphanum path")
    {
        const char* line = "/some/path/to/file_non-alpha.ext";
        CLTestbench::TokenStream stream(line);

        auto token = stream.current();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.next().mType == CLTestbench::Token::End);
    }

    SECTION("Token types")
    {
        const char* line = "\t( ), text \"quoted text\" 123.5\n-99";
        CLTestbench::TokenStream stream(line);

        REQUIRE(stream);
        CHECK(!stream.expect(CLTestbench::Token::Text));
        CHECK(stream.current().mType == CLTestbench::Token::OpenParen);
        CHECK(stream.next().mType == CLTestbench::Token::CloseParen);
        CHECK(stream.expect(CLTestbench::Token::OpenParen));
        CHECK(!stream.expect(CLTestbench::Token::OpenParen));
        CHECK(stream.expect(CLTestbench::Token::CloseParen));
        CHECK(stream.expect(CLTestbench::Token::Comma));
        CHECK(stream.getTokenText(stream.current()) == "text");
        CHECK(stream.expect(CLTestbench::Token::String));
        CHECK(stream.getTokenText(stream.current()) == "\"quoted text\"");
        CHECK(stream.getUnquotedText(stream.current()) == "quoted text");
        CHECK(stream.expect(CLTestbench::Token::Text));
        CHECK(stream.parseConstant<float>(stream.current()) == 123.5);
        CHECK(stream.expect(CLTestbench::Token::Constant));
        CHECK(stream.parseConstant<float>(stream.current()) == -99);
        CHECK(stream.parseConstant<int>(stream.current()) == -99);
        CHECK(stream.expect(CLTestbench::Token::Constant));
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(!stream);
    }

    SECTION("Simple text")
    {
        const char* line = "\"Normal text contents\"";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Text);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.getUnquotedText(token) == "Normal text contents");
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Invalid text")
    {
        const char* line = "\"Unterminated text";
        CLTestbench::TokenStream stream(line);

        auto token = stream.current();
        CHECK(token.mType == CLTestbench::Token::Invalid);
        CHECK(stream.getTokenText(token) == line);
    }

    SECTION("Escape characters text")
    {
        const char* line = "\"\\tEscaped \\\"char\\\": \\\\\"";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Text);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.getUnquotedText(token) == "\tEscaped \"char\": \\");
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Parse slash-paragraph")
    {
        const char* line = "\"\\\\\\n\"";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Text);
        CHECK(stream.getTokenText(token) == line);
        CHECK(stream.getUnquotedText(token) == "\\\n");
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Parse two texts")
    {
        const char* line = "\"text 1\"\"text 2\"";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Text);
        CHECK(stream.getTokenText(token) == "\"text 1\"");
        CHECK(stream.getUnquotedText(token) == "text 1");
        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Text);
        CHECK(stream.getTokenText(token) == "\"text 2\"");
        CHECK(stream.getUnquotedText(token) == "text 2");
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Whitespace")
    {
        CHECK(CLTestbench::TrimWhitespace("") == "");
        CHECK(CLTestbench::TrimWhitespace("\t\n\r ") == "");
        CHECK(CLTestbench::TrimWhitespace("   a  ") == "a");
        CHECK(CLTestbench::TrimWhitespace("   a  b") == "a  b");
        CHECK(CLTestbench::TrimWhitespace("\n   a  ") == "a");
        CHECK(CLTestbench::TrimWhitespace("\t   a\t b") == "a\t b");
    }

    SECTION("Specials")
    {
        const char* line = "=(),";
        CLTestbench::TokenStream stream(line);

        // Nothing parsed yet.
        CHECK(stream.text() == stream.currentText());

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Equal);
        CHECK(stream.getTokenText(token) == "=");
        CHECK(stream.currentText() == "(),");
        CHECK(stream.remainingText() == "),");
        token = stream.currentTextAsToken();
        CHECK(stream.getTokenText(token) == stream.currentText());
        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::OpenParen);
        CHECK(stream.getTokenText(token) == "(");
        CHECK(stream.currentText() == "),");
        CHECK(stream.remainingText() == ",");
        token = stream.currentTextAsToken();
        CHECK(stream.getTokenText(token) == stream.currentText());
        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::CloseParen);
        CHECK(stream.getTokenText(token) == ")");
        CHECK(stream.currentText() == ",");
        CHECK(stream.remainingText() == "");
        token = stream.currentTextAsToken();
        CHECK(stream.getTokenText(token) == stream.currentText());
        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Comma);
        CHECK(stream.getTokenText(token) == ",");
        CHECK(stream.currentText() == "");
        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }
}

TEST_CASE("Constants")
{
    SECTION("Integer")
    {
        const char* line = "0 1 123 -456 987 9 0xFFFF";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "0");
        CHECK(stream.parseConstant<int>(token) == 0);
        CHECK(stream.remainingText() == " 123 -456 987 9 0xFFFF");
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "1");
        CHECK(stream.parseConstant<int>(token) == 1);
        CHECK(stream.remainingText() == " -456 987 9 0xFFFF");
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "123");
        CHECK(stream.parseConstant<int>(token) == 123);
        CHECK(stream.remainingText() == " 987 9 0xFFFF");
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "-456");
        CHECK(stream.parseConstant<int>(token) == -456);
        CHECK(stream.remainingText() == " 9 0xFFFF");
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "987");
        CHECK(stream.parseConstant<int>(token) == 987);
        CHECK(stream.remainingText() == " 0xFFFF");
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "9");
        CHECK(stream.parseConstant<int>(token) == 9);
        token = stream.remainingTextAsToken();
        CHECK(stream.getTokenText(token) == stream.remainingText());

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "0xFFFF");
        CHECK(stream.parseConstant<uint16_t>(token) == 0xFFFF);

        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
        CHECK(stream.remainingText().empty());
        CHECK(stream.remainingTextAsToken().mType == CLTestbench::Token::End);
    }

    SECTION("Boolean")
    {
        const char* booleans = "1 y yes t true 0 n no f false";
        CLTestbench::TokenStream stream(booleans);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "1");
        CHECK(stream.parseConstant<bool>(token) == true);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "y");
        CHECK(stream.parseConstant<bool>(token) == true);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "yes");
        CHECK(stream.parseConstant<bool>(token) == true);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "t");
        CHECK(stream.parseConstant<bool>(token) == true);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "true");
        CHECK(stream.parseConstant<bool>(token) == true);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "0");
        CHECK(stream.parseConstant<bool>(token) == false);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "n");
        CHECK(stream.parseConstant<bool>(token) == false);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "no");
        CHECK(stream.parseConstant<bool>(token) == false);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "f");
        CHECK(stream.parseConstant<bool>(token) == false);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "false");
        CHECK(stream.parseConstant<bool>(token) == false);
    }

    SECTION("Float")
    {
        const char* line = "0.0 1 -1.5 1024 -23.0";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "0.0");
        CHECK(stream.parseConstant<float>(token) == 0.0f);
        CHECK(stream.remainingText() == " -1.5 1024 -23.0");

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "1");
        CHECK(stream.parseConstant<double>(token) == 1.0);
        CHECK(stream.remainingText() == " 1024 -23.0");

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "-1.5");
        CHECK(stream.parseConstant<float>(token) == -1.5f);
        CHECK(stream.remainingText() == " -23.0");

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "1024");
        CHECK(stream.parseConstant<double>(token) == 1024.0);
        CHECK(stream.remainingText() == "");

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "-23.0");
        CHECK(stream.parseConstant<float>(token) == -23.0f);
        CHECK(stream.remainingText() == "");

        CHECK(stream.current().mType == CLTestbench::Token::End);
        CHECK(stream.next().mType == CLTestbench::Token::Invalid);
    }

    SECTION("Invalid")
    {
        const char* line = "b0.0. 1b 32a3 -32";
        CLTestbench::TokenStream stream(line);

        auto token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::String);
        CHECK(stream.getTokenText(token) == "b0.0.");
        CHECK_THROWS_AS(stream.parseConstant<float>(token), CLTestbench::CommandError);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "1b");
        CHECK_THROWS_AS(stream.parseConstant<int>(token), CLTestbench::CommandError);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "32a3");
        CHECK_THROWS_AS(stream.parseConstant<short>(token), CLTestbench::CommandError);

        token = stream.consume();
        CHECK(token.mType == CLTestbench::Token::Constant);
        CHECK(stream.getTokenText(token) == "-32");
        CHECK_THROWS_AS(stream.parseConstant<unsigned>(token), CLTestbench::CommandError);
    }
}
