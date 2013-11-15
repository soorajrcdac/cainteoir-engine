/* Test for the path API.
 *
 * Copyright (C) 2013 Reece H. Dunn
 *
 * This file is part of cainteoir-engine.
 *
 * cainteoir-engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cainteoir-engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cainteoir-engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cainteoir/path.hpp>
#include <cstring>

#include "tester.hpp"

REGISTER_TESTSUITE("path");

struct path_join_t
{
	const char *base;
	const char *join;
	const char *result;
};

static const path_join_t path_join_testcases[] =
{
	{ "ab/cd", "ef", "ab/cd/ef" },
};

TEST_CASE("C-style string construction")
{
	cainteoir::path a{""};
	assert(a.empty());
	assert(a.size() == 0);
	assert(a.str() == std::string());
	assert(!strcmp(a, ""));

	cainteoir::path b{"test/case"};
	assert(!b.empty());
	assert(b.size() == 9);
	assert(b.str() == "test/case");
	assert(!strcmp(b, "test/case"));

	cainteoir::path c{nullptr};
	assert(c.empty());
	assert(c.size() == 0);
	assert(c.str() == std::string());
	assert(!strcmp(c, ""));
}

TEST_CASE("std::string construction")
{
	cainteoir::path a{std::string()};
	assert(a.empty());
	assert(a.size() == 0);
	assert(a.str() == std::string());
	assert(!strcmp(a, ""));

	cainteoir::path b{std::string("case/test")};
	assert(!b.empty());
	assert(b.size() == 9);
	assert(b.str() == "case/test");
	assert(!strcmp(b, "case/test"));
}

TEST_CASE("parent")
{
	assert(cainteoir::path("a/b/c").parent().str() == "a/b");
	assert(cainteoir::path("a/b").parent().str() == "a");
	assert(cainteoir::path("a").parent().str() == std::string());
	assert(cainteoir::path("abc/de/fgh-ijk").parent().str() == "abc/de");
}

TEST_CASE("operator/=")
{
	for (const auto &test : path_join_testcases)
	{
		cainteoir::path a{test.base};
		a /= test.join;
		assert(a.str() == test.result);

		cainteoir::path b{test.base};
		b /= std::string(test.join);
		assert(b.str() == test.result);
	}
}

TEST_CASE("operator/")
{
	for (const auto &test : path_join_testcases)
	{
		assert((cainteoir::path(test.base) / test.join).str() == test.result);
		assert((cainteoir::path(test.base) / std::string(test.join)).str() == test.result);
	}
}