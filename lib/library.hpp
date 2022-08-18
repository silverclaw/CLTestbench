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
#include <string_view>
#include <type_traits>

namespace CLTestbench
{
class Library final
{
    void* __restrict mLib = nullptr;
    void* getSymbol(const char* name);

    Library() = delete;
    Library(const Library&) = delete;
    Library(Library&&) = delete;

public:
    explicit Library(std::string_view filename);

    template<typename T>
    T bind(const char* name)
    {
        static_assert(std::is_function<typename std::remove_pointer<T>::type>::value);
        return reinterpret_cast<T>(getSymbol(name));
    }

    std::string_view getName() const;

    ~Library();

    class Error final : public std::exception
    {
        const char* mMessage;

    public:
        explicit Error(const char* message) : mMessage(message) {}

        const char* what() const noexcept override { return mMessage; }
    };
};
} // namespace CLTestbench
