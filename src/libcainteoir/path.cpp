/* Path API.
 *
 * Copyright (C) 2013-2014 Reece H. Dunn
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

#include "config.h"
#include "compatibility.hpp"

#include <cainteoir/path.hpp>

#if HAVE_MKSTEMP
#include <string.h>
#include <unistd.h>
#endif

cainteoir::path &cainteoir::path::operator/=(const char *aPath)
{
	if (aPath == nullptr || *aPath == '\0') return *this;
	if (empty())
	{
		mPath = aPath;
		return *this;
	}

	while (aPath[0] == '.' && aPath[1] == '.' && aPath[2] == '/')
	{
		aPath += 3;
		*this = parent();
	}

	auto end = mPath.end();
	--end;
	if (*end != '/' && *aPath != '/')
		mPath.push_back('/');
	else if (*end == '/' && *aPath == '/')
		mPath.resize(mPath.size() - 1);
	mPath += aPath;
	return *this;
}

cainteoir::path &cainteoir::path::operator/=(const std::string &aPath)
{
	if (aPath.empty()) return *this;
	if (empty())
	{
		mPath = aPath;
		return *this;
	}

	std::string::size_type offset = 0;
	while (aPath.find("../", offset) == offset)
	{
		offset += 3;
		*this = parent();
	}

	auto end = mPath.end();
	--end;
	if (*end != '/' && (*aPath.begin()) != '/')
		mPath.push_back('/');
	else if (*end == '/' && (*aPath.begin()) == '/')
		mPath.resize(mPath.size() - 1);
	if (offset == 0)
		mPath += aPath;
	else
		mPath += aPath.substr(offset);
	return *this;
}

cainteoir::path cainteoir::path::parent() const
{
	std::string::size_type pos = mPath.rfind('/');
	if (pos == std::string::npos) return cainteoir::path(std::string());
	return path(mPath.substr(0, pos));
}

cainteoir::path cainteoir::path::zip_file() const
{
	std::string::size_type pos = mPath.rfind('!');
	if (pos == std::string::npos) return *this;
	return path(mPath.substr(0, pos));
}

cainteoir::path cainteoir::path::zip_path() const
{
	std::string::size_type pos = mPath.rfind('!');
	if (pos == std::string::npos) return cainteoir::path(std::string());
	return path(mPath.substr(pos + 2));
}

cainteoir::path cainteoir::get_data_path()
{
	const char *datadir = getenv("CAINTEOIR_DATA_DIR");
	if (!datadir)
		datadir = DATADIR "/" PACKAGE;
	return path(datadir);
}

FILE *cainteoir::create_temp_file(const char *mode)
{
#ifdef ANDROID
	// Android has buggy support for tempnam in that when it works, the files cannot
	// be created (`fopen` returns nullptr with a Permission Denied error).

	static int nextid = 0;
	int tmpid = nextid++ % 1000;

	char tmpfile[100];
	snprintf(tmpfile, 100, "tmp-%04d", tmpid);

	return fopen(cainteoir::get_data_path() / ".." / "cache" / tmpfile, mode);
#elif HAVE_TMPFILE
	return tmpfile();
#elif HAVE_MKSTEMP
	char name[20] = "";
	strcpy(name, "/tmp/ctts.XXXXXX");

	int fd = mkstemp(name);
	if (fd == -1) return nullptr;

	FILE *ret = fdopen(fd, mode);
	if (ret == nullptr)
	{
		unlink(name);
		close(fd);
	}
	return ret;
#else
	return fopen(tempnam("/tmp", nullptr), mode);
#endif
}
