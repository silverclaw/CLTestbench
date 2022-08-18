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

#include <dlfcn.h>
#include <link.h>         // link_map
#include <linux/limits.h> // PATH_MAX
#include <cassert>
#include <cstring>

#include "library.hpp"

using namespace CLTestbench;

Library::Library(std::string_view filename)
{
    char file[PATH_MAX] = {};
    strncpy(file, filename.data(), filename.size());
    mLib = dlopen(file, RTLD_LAZY | RTLD_LOCAL);
    if (!mLib) throw Error(dlerror());
}

void* Library::getSymbol(const char* name)
{
    void* symbol = dlsym(mLib, name);
    if (!symbol) {
        // dlsym docs state that a nullptr might be a valid address for a symbol,
        // but we won't support that until there's a known case for it.
        throw Error(dlerror());
    }
    return symbol;
}

std::string_view Library::getName() const
{
    assert(mLib && "How did we get here?");
    link_map* map;
    dlinfo(mLib, RTLD_DI_LINKMAP, &map);
    return map->l_name;
}

Library::~Library()
{
    if (mLib) dlclose(mLib);
}
