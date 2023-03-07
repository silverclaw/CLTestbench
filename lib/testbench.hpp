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
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace CLTestbench
{
class Driver;
class TokenStream;
class Object;

class Testbench final
{
    /// Loaded OpenCL implementation.
    std::unique_ptr<Driver> mDriver;
    /// List of created objects.
    // This must be declared after the driver variable to ensure
    // that the objects are released before the driver is unloaded.
    // C++ mandates destruction order is the inverse of construction,
    // which follows declaration order.
    std::map<std::string, std::shared_ptr<Object>> mObjects;
    std::ostream* mOut;
    std::ostream* mErr;

public:
    Testbench();

    /// The status code a TokenStream execution.
    enum class Result
    {
        /// The command was successful.
        Good,
        /// The command failed to execute.
        Fail,
        /// A quit command was run.
        Quit
    };

    /// Run this command string.  This will catch command exceptions.
    Result run(std::string_view);

    /// Run with pre-constructed tokenstream.
    Result run(TokenStream&);

    /// Attempts to evaluate this token stream expression.
    std::shared_ptr<Object> evaluate(TokenStream&);

    /// Resets the output stream.
    void resetOutput(std::ostream& stream) noexcept { mOut = &stream; }

    /// Reset the error output stream.
    void resetErrorOutput(std::ostream& stream) noexcept { mErr = &stream; }

    /// Unloads the current driver (if any) and replaces with this new one.
    /// Any driver-specific objects will be released.
    void resetDriver(std::unique_ptr<Driver>) noexcept;

    ~Testbench();

private:
    /// Release any objects attached to the driver.  Returns number of objects released.
    unsigned clearDriverObjects() noexcept;

    void executeLoad(TokenStream&);
    void executeSelect(TokenStream&);
    void executeInfo(TokenStream&);
    void executeList(TokenStream&);
    void executeSet(TokenStream&);
    void executeRelease(TokenStream&);
    void executeSave(TokenStream&);
    void executeRun(TokenStream&);
    void executeFlush(TokenStream&);
    void executeBind(TokenStream&);
    void executeWait(TokenStream&);
    void executeScript(TokenStream&);
    void executeHelp(TokenStream&);

    /// Evaluate a "buffer" directive.
    std::shared_ptr<Object> evaluateBuffer(TokenStream&);
    /// Evaluate a "program" directive.
    std::shared_ptr<Object> evaluateProgram(TokenStream&);
    /// Evaluate a "binary" directive.
    std::shared_ptr<Object> evaluateBinary(TokenStream&);
    /// Evaluate a "kernel" directive.
    std::shared_ptr<Object> evaluateKernel(TokenStream&);
    /// Evaluate a "file" directive.
    std::shared_ptr<Object> evaluateFile(TokenStream&);
    /// Evaluate a "image" directive.
    std::shared_ptr<Object> evaluateImage(TokenStream&);
    /// Evaluate a "clone" directive.
    std::shared_ptr<Object> evaluateClone(TokenStream&);

    struct Options
    {
        bool verbose : 1;
        bool caretPrint : 1;
        bool scriptEcho : 1;

        Options() : verbose(true), caretPrint(true), scriptEcho(false) {}
    } mOptions;

    /// Nesting level for scripts.
    uint32_t mScriptLevel = 0;
};
} // namespace CLTestbench
