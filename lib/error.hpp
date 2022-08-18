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

#include <exception>
#include <functional>
#include <iosfwd>

namespace CLTestbench
{
/// Generic execution error.
class CommandError final : public std::exception
{
public:
    /// A custom error message printer.
    const std::function<void(std::ostream&)> mPrinter;

    /// The error message for this exception.
    const char* const mMessage = "";

    /// If this error was generated during token parsing, or evaluation,
    /// these indices represent where in the input line the error appears.
    const std::size_t mBegin = 0;
    const std::size_t mEnd = 0;

    explicit CommandError(const char* msg) noexcept : mMessage(msg) {}
    explicit CommandError(const char* msg, std::size_t begin, std::size_t end) noexcept :
        mMessage(msg), mBegin(begin), mEnd(end)
    {}
    explicit CommandError(const char* msg, std::pair<std::size_t, std::size_t> index) noexcept :
        mMessage(msg), mBegin(index.first), mEnd(index.second)
    {}

    explicit CommandError(std::function<void(std::ostream&)> printer) : mPrinter(printer) {}
    explicit CommandError(std::function<void(std::ostream&)> printer, std::size_t begin, std::size_t end) :
        mPrinter(printer), mBegin(begin), mEnd(end)
    {}
    explicit CommandError(std::function<void(std::ostream&)> printer, std::pair<std::size_t, std::size_t> index) :
        mPrinter(printer), mBegin(index.first), mEnd(index.second)
    {}

    ~CommandError() = default;

    const char* what() const noexcept override { return mMessage; }

    bool hasLocationInfo() const noexcept { return mEnd != 0 && mEnd > mBegin; }
};
} // namespace CLTestbench
