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

#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>

namespace Util
{
class Table
{
public:
	Table(Table&&) = default;
	Table(const Table&) = default;
	explicit Table(std::size_t columns, std::size_t preallocRows = 0) :
		Columns(columns), header(columns)
	{
		assert(columns != 0);
		if (preallocRows != 0)
			rows.reserve(preallocRows);
	}

	Table& setHeader(std::size_t i, std::string_view name) noexcept
	{
		headerSet = true;
		header[i] = name;
		return *this;
	}

	Table& setHeader(std::initializer_list<std::string_view> headers, std::size_t offset = 0) noexcept
	{
		assert((headers.size() + offset) <= header.size());
		for (const auto& name : headers) {
			setHeader(offset, name);
			++offset;
		}
		return *this;
	}

	using Row_t = std::vector<std::string_view>;

	Row_t& operator[](std::size_t i)
	{
		if (rows.size() <= i) {
			rows.resize(i+1);
		}
		rows[i].resize(Columns);
		return rows[i];
	}

	friend std::ostream& operator<<(std::ostream& O, const Table& table);

	void clearHeader() noexcept
	{
		header.clear();
		header.resize(Columns); // reset to empty strings
		headerSet = true;
	}

	void clearData() noexcept
	{
		rows.clear();
	}

	void clear() noexcept
	{
		clearHeader();
		clearData();
	}

private:
	const std::size_t Columns;
	bool headerSet = false;
	std::vector<std::string_view> header;
	std::vector<Row_t> rows;
};

inline std::ostream& operator<<(std::ostream& O, const Table& table)
{
	// Figure out the number of characters in each column.
	std::vector<std::size_t> colSizes(table.Columns);
	for (unsigned col = 0; col < table.Columns; ++col) {
		colSizes[col] = table.header[col].size();
	}
	for (unsigned col = 0; col < table.Columns; ++col) {
		for (const auto& row : table.rows)
		{
			if (row.empty()) continue;
			colSizes[col] = std::max(colSizes[col], row[col].size());
		}
	}

	// Add a whitespace before and after each row.
	for (auto& s : colSizes) s += 2;

	// Print the headers.
	if (table.headerSet) {
		O << "  ";
		for (unsigned col = 0; col < table.Columns - 1; ++col) {
			O << table.header[col];
			// Insert blank padding.
			for (auto pad = table.header[col].size() + 2; pad < colSizes[col]; ++pad) {
				O << ' ';
			}
			O << " | ";
		}
		O << table.header[table.Columns-1] << '\n';

		// A nice separating line.
		for (unsigned col = 0 ; col < colSizes.size(); ++col) {
			auto size = colSizes[col];
			O << '+';
			for (unsigned i = 0; i < size; ++i) O << '-';
		}
		O << "+\n";
	}

	// And the data.
	for (const auto& row : table.rows) {
		for (unsigned col = 0; col < table.Columns; ++col) {
			const std::string_view& data = row.empty() ? "" : row[col];
			if (col != 0) {
				O << '|';
			} else {
				O << ' ';
			}
			const bool isLastCol = (col == table.Columns-1);
			// Avoid printing trailing whitespace.
			if (isLastCol && data.empty()) break;
			O << ' ' << data;
			// Don't bother adding padding on the last column.
			if (isLastCol) break;
			// Insert blank padding.
			for (auto pad = data.size() + 1; pad < colSizes[col]; ++pad) {
				O << ' ';
			}
		}
		O << '\n';
	}

	return O;
}
}
