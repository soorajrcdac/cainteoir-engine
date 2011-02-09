/* Test for parsing XML/HTML documents.
 *
 * Copyright (C) 2010 Reece H. Dunn
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

#include <cainteoir/xmlreader.hpp>
#include <stdexcept>

namespace xml = cainteoir::xml;

int main(int argc, char ** argv)
{
	try
	{
		argc -= 1;
		argv += 1;

		if (argc != 1)
			throw std::runtime_error("no document specified");

		xml::reader reader(std::auto_ptr<cainteoir::buffer>(new cainteoir::mmap_buffer(argv[0])));
		while (reader.read())
		{
			switch (reader.nodeType())
			{
			case xml::reader::beginTagNode:
				fprintf(stdout, "|begin-tag| %s\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::endTagNode:
				fprintf(stdout, "|end-tag| %s\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::tagNode:
				fprintf(stdout, "|tag| %s\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::processingInstructionNode:
				fprintf(stdout, "|processing-instruction| %s\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::commentNode:
				fprintf(stdout, "|comment| \"\"\"%s\"\"\"\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::cdataNode:
				fprintf(stdout, "|cdata| \"\"\"%s\"\"\"\n", reader.nodeValue().str().c_str());
				break;
			case xml::reader::textNode:
				fprintf(stdout, "|text| \"\"\"%s\"\"\"\n", reader.nodeValue().str().c_str());
				break;
			}
		}
	}
	catch (std::runtime_error &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}

	return 0;
}
