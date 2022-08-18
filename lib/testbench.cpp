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

#include <cassert>
#include <iostream>

#include "testbench.hpp"

#include "driver.hpp"
#include "error.hpp"
#include "istringview.hpp"
#include "object.hpp"
#include "object_cl.hpp"
#include "table.hpp"
#include "token.hpp"

using namespace CLTestbench;

Testbench::Testbench() : mOut(&std::cout), mErr(&std::cerr) {}

Testbench::Result Testbench::run(std::string_view line)
{
    TokenStream tokens(line);

    try {
        return run(tokens);
    } catch (const Driver::Error& e) {
        *mErr << "OpenCL API error: " << e.what() << '\n';
    } catch (const Library::Error& e) {
        *mErr << "OpenCL library error: " << e.what() << '\n';
    } catch (const CommandError& e) {
        // 120 is an arbitrary limit when there are copy-pasted lines onto
        // the prompt where the caret indices would not be helpful, as they'd
        // span multiple lines.  TODO: Replace with terminal query.
        if (mOptions.caretPrint && e.hasLocationInfo() && line.length() < 120) {
            if (mScriptLevel == 0) {
                // Offset the prompt size.
                *mErr << "        ";
            } else if (mOptions.scriptEcho == 0) {
                // There won't be an echo from the prompt when running
                // a script line with echo off, so echo that now.
                *mErr << line << '\n';
            }
            for (std::size_t space = 0; space < e.mBegin; ++space) *mErr << ' ';
            for (std::size_t caret = e.mBegin; caret < e.mEnd; ++caret) *mErr << '^';
            *mErr << '\n';
        }
        if (e.mPrinter)
            e.mPrinter(*mErr);
        else
            *mErr << "Syntax error on command: " << e.what() << '\n';
    } catch (const std::bad_alloc&) {
        *mErr << "Memory allocation failed. (out of memory?)\n";
    } // Allow any other exception to propagate out.

    return Result::Fail;
}

Testbench::Result Testbench::run(TokenStream& tokens)
{
    if (!tokens) return Result::Good;

    Token first = tokens.consume();
    assert(first.mType != Token::End);

    if (tokens.current().mType == Token::Equal) {
        // Variable assignment.
        tokens.advance();

        std::string_view variableName = tokens.getTokenText(first);

        auto match = std::find_if(std::begin(mObjects), std::end(mObjects),
                                  [&](const auto& it) { return it.first == variableName; });
        if (match != std::end(mObjects)) {
            throw CommandError([=](std::ostream& out) {
                out << "An object named '" << variableName << "' already exists.  Use 'release' to clear it.";
            });
        }

        if (!tokens) {
            throw CommandError("Missing expression to evaluate for assignment.  Use 'release' to clear an object.");
        }

        // Remaining tokens form an expression tree that evaluates to an Object instance.
        std::shared_ptr<Object> evaluated(evaluate(tokens));
        assert(evaluated && "Evaluation returned a nullptr");
        if (tokens.current().mType == Token::Invalid) {
            throw CommandError("Internal error: token stream is invalid but no diagnostic presented.");
        }
        if (tokens.current().mType != Token::End) {
            throw CommandError("Trailing tokens in assignment command not allowed.", tokens.current());
        }
        mObjects.insert(std::make_pair(variableName, std::move(evaluated)));
        return Result::Good;
    }

    const std::initializer_list<std::string_view> commands {
        "load", "select", "info", "list", "set",
        // Add some unmatchable filler commands here to push
        // the index of "quit" and "help" down, so we can
        // add more commands without having to update the indicies.
        "release", "save", "run", "script",
        "wait", " filler", " filler", " filler",
        " filler", " filler", " filler", " filler",
        "help", "quit"
    };

    IStringView command = tokens.getTokenText(first);
    auto match = command.autocomplete(commands);
    switch (match) {
    case 0: executeLoad(tokens); break;
    case 1: executeSelect(tokens); break;
    case 2: executeInfo(tokens); break;
    case 3: executeList(tokens); break;
    case 4: executeSet(tokens); break;
    case 5: executeRelease(tokens); break;
    case 6: executeSave(tokens); break;
    case 7: executeRun(tokens); break;
    case 8: executeFlush(tokens); break;
    case 9: executeWait(tokens); break;
    case 10: executeScript(tokens); break;
    case 17: executeHelp(tokens); break;
    case 18:
        if (tokens) *mErr << "Trailing tokens after 'quit' command ignored.\n";
        return Result::Quit;
    case IStringView::ambiguous:
        *mErr << "Ambiguous command autocomplete for '" << command << "'.\n";
        return Result::Fail;
    case std::string_view::npos:
        *mErr << "Unknown command '" << command << "'.\n";
        return Result::Fail;
    default:
        *mErr << "Internal error - unexpected command match return.\n";
        return Result::Fail;
    }

    // Quit command was not invoked.
    return Result::Good;
}

Testbench::~Testbench() {}

void Testbench::resetDriver(std::unique_ptr<Driver> newDriver) noexcept
{
    if (mDriver) {
        clearDriverObjects();
        *mOut << "Unloading " << mDriver->getLibraryName() << '\n';
    }
    std::swap(mDriver, newDriver);
}

void Testbench::executeLoad(TokenStream& tokens)
{
    try {
        Token libNameToken = tokens.current();
        if (libNameToken.mType == Token::End) {
            throw CommandError("Missing library name for 'load' command.");
        }

        std::string_view libName = tokens.getTokenText(libNameToken);
        if (libName.empty()) {
            throw CommandError("Missing library name for 'load' command.");
        }
        auto newDriver = std::make_unique<Driver>(
            libNameToken.mType == Token::Text ? tokens.getUnquotedText(libNameToken) : libName);

        assert(newDriver && "Load failure is signaled through exceptions!");
        if (mOptions.verbose) *mOut << "Loaded " << newDriver->getLibraryName() << '\n';
        resetDriver(std::move(newDriver));
    } catch (const Library::Error& e) {
        *mErr << "Error while loading library: " << e.what() << '\n';
    }
}

unsigned Testbench::clearDriverObjects() noexcept
{
    unsigned count = 0;
    for (auto it = mObjects.begin(); it != mObjects.end();) {
        Object* obj = it->second.get();
        ++it;
        if (dynamic_cast<CLObject*>(obj)) {
            mObjects.erase(it);
            ++count;
        }
    }
    return count;
}

void Testbench::executeList(TokenStream&)
{
    if (mObjects.empty()) {
        if (mOptions.verbose) *mOut << "No created objects.\n";
        return;
    }

    if (mOptions.verbose) *mOut << "List of objects currently available:\n";
    Util::Table table(2, mObjects.size());
    unsigned row = 0;
    table.setHeader({"Identifier", "Type"});
    for (const auto& pair : mObjects) {
        table[row][0] = pair.first;
        table[row][1] = pair.second->type();
        ++row;
    }
    // The table will always be printed, even on non-verbose.  Otherwise this command would be meaningless.
    *mOut << table << '\n';
}

void Testbench::executeRelease(TokenStream& tokens)
{
    if (!tokens) {
        throw CommandError("Expected identifier for 'release' command.");
    }

    do {
        auto token = tokens.consume();
        if (token.mType != token.String) {
            throw CommandError("Invalid identifier.", token);
        }

        std::string_view identifier = tokens.getTokenText(token);
        auto match = std::find_if(std::begin(mObjects), std::end(mObjects),
                                  [&](const auto& it) { return it.first == identifier; });
        if (match == std::end(mObjects)) throw CommandError("Object not found.", token);

        mObjects.erase(match);
    } while (tokens);
}

void Testbench::executeWait(TokenStream& tokens)
{
    if (!mDriver) throw CommandError("A driver is required for a 'wait' command.");

    mDriver->finish();
    if (tokens && mOptions.verbose) *mOut << "Trailing tokens on 'wait' command ignored.\n";
}

void Testbench::executeFlush(TokenStream& tokens)
{
    if (!mDriver) throw CommandError("A driver is required for a 'flush' command.");

    mDriver->flush();
    if (tokens && mOptions.verbose) *mOut << "Trailing tokens on 'flush' command ignored.\n";
}
