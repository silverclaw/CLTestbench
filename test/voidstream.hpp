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
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <sstream>

/// Discards all output.
class VoidStream final : public std::ostream
{
private:
    class VoidOutBuffer final : public std::streambuf
    {
    public:
        int overflow(int c) { return c; }
    } mVoidBuffer;

public:
    VoidStream() : std::ostream(&mVoidBuffer) {}
};
