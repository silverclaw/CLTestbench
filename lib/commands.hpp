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

#include <initializer_list>
#include <string_view>

namespace CLTestbench
{
/// List of commands recognised by CLTB.
const std::initializer_list<std::string_view> CommandList {
    "load", "select", "info", "list", "set",
    "release", "save", "run", "script",
    "wait", "flush", "bind",
    "help", "quit"
};
namespace Command
{
constexpr std::size_t Load = 0;
constexpr std::size_t Select = 1;
constexpr std::size_t Info = 2;
constexpr std::size_t List = 3;
constexpr std::size_t Set = 4;
constexpr std::size_t Release = 5;
constexpr std::size_t Save = 6;
constexpr std::size_t Run = 7;
constexpr std::size_t Script = 8;
constexpr std::size_t Wait = 9;
constexpr std::size_t Flush = 10;
constexpr std::size_t Bind = 11;
constexpr std::size_t Help = 12;
constexpr std::size_t Quit = 13;

} // namespace command
} // namespace CLTestbench

