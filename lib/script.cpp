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

#include <cstring>
#include <filesystem>
#include <fstream>

#include "error.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
/// Ensures the script level is decrememnted, even on exceptions.
struct ScriptLevelGuard final
{
	uint32_t& mLevel;
	explicit ScriptLevelGuard(uint32_t& level) : mLevel(level)
	{
		mLevel += 1;
	}

	~ScriptLevelGuard()
	{
		mLevel -= 1;
	}
};
} // namespace

void Testbench::executeScript(TokenStream& tokens)
{
	if (!tokens)
		throw CommandError("Expected file name argument to 'script' command.\n");

	auto filename = tokens.currentText();
	std::filesystem::path path(filename);
	std::ifstream file(path);

	ScriptLevelGuard guard(mScriptLevel);

	for (std::string line; std::getline(file, line); ) {
		auto trimmedLine = TrimWhitespace(line);
		if (trimmedLine.empty()) continue;
		if (trimmedLine[0] == '#') continue;
		TokenStream stream(trimmedLine);
		if (mOptions.scriptEcho && trimmedLine[0] != '@')
			*mOut << trimmedLine;
		auto result = run(stream);
		if (result != Result::Good)
			return;
	}
}
