/* Test for parsing XML/HTML documents.
 *
 * Copyright (C) 2010-2011 Reece H. Dunn
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
#include <getopt.h>

namespace xml = cainteoir::xml;

enum args
{
	ARG_REPEAT = 'n',
	ARG_TIME = 't',
	ARG_SILENT = 's',
};

const char *options_short = "n:st";

static struct option options[] =
{
	{ "repeat", required_argument, 0, ARG_REPEAT },
	{ "time",   no_argument,       0, ARG_TIME },
	{ "silent", no_argument,       0, ARG_SILENT },
	{ 0, 0, 0, 0 }
};

const char * node_type_name(xml::reader::node_type type)
{
	switch (type)
	{
	case xml::reader::beginTagNode:
		return "begin-tag";
	case xml::reader::endTagNode:
		return "end-tag";
	case xml::reader::beginProcessingInstructionNode:
		return "begin-processing-instruction";
	case xml::reader::endProcessingInstructionNode:
		return "end-processing-instruction";
	case xml::reader::commentNode:
		return "comment";
	case xml::reader::cdataNode:
		return "cdata";
	case xml::reader::textNode:
		return "text";
	case xml::reader::doctypeNode:
		return "doctype";
	case xml::reader::attribute:
		return "attribute";
	case xml::reader::endOfData:
		return "end-of-data";
	default:
		return "unknown";
	}
}

void parse_xml(xml::reader &reader, bool silent)
{
	while (reader.read())
	{
		std::string ns = reader.namespaceUri();
		if (!silent) switch (reader.nodeType())
		{
		default:
			if (reader.nodePrefix().empty() && ns.empty())
			{
				fprintf(stdout, "|%s| %s\n",
				        node_type_name(reader.nodeType()),
				        reader.nodeName().str().c_str());
			}
			else
			{
				fprintf(stdout, "|%s| [%s|%s]%s\n",
				        node_type_name(reader.nodeType()),
				        reader.nodePrefix().str().c_str(),
				        ns.c_str(),
				        reader.nodeName().str().c_str());
			}
			break;
		case xml::reader::commentNode:
		case xml::reader::cdataNode:
		case xml::reader::textNode:
			fprintf(stdout, "|%s| \"\"\"%s\"\"\"\n",
			        node_type_name(reader.nodeType()),
			        reader.nodeValue().str().c_str());
			break;
		case xml::reader::attribute:
			if (reader.nodePrefix().empty() && ns.empty())
			{
				fprintf(stdout, "|%s| %s=\"\"\"%s\"\"\"\n",
				        node_type_name(reader.nodeType()),
				        reader.nodeName().str().c_str(),
				        reader.nodeValue().str().c_str());
			}
			else
			{
				fprintf(stdout, "|%s| [%s|%s]%s=\"\"\"%s\"\"\"\n",
				        node_type_name(reader.nodeType()),
				        reader.nodePrefix().str().c_str(),
				        ns.c_str(),
				        reader.nodeName().str().c_str(),
				        reader.nodeValue().str().c_str());
			}
			break;
		case xml::reader::error:
			fprintf(stdout, "|error| internal parser error\n");
			break;
		}
	}
}

int main(int argc, char ** argv)
{
	try
	{
		int repeatCount = 1;
		bool time = false;
		bool silent = false;

		while (1)
		{
			int option_index = 0;
			int c = getopt_long(argc, argv, options_short, options, &option_index);
			if (c == -1)
				break;

			switch (c)
			{
			case ARG_REPEAT:
				repeatCount = atoi(optarg);
				break;
			case ARG_SILENT:
				silent = true;
				break;
			case ARG_TIME:
				time = true;
				break;
			}
		}

		argc -= optind;
		argv += optind;

		if (argc != 1)
			throw std::runtime_error("no document specified");

		clock_t start_time = ::clock();

		for (int i = 0; i != repeatCount; ++i)
		{
			xml::reader reader(cainteoir::make_file_buffer(argv[0]), "windows-1252");
			parse_xml(reader, silent);
		}

		clock_t end_time = ::clock();
		if (time)
			fprintf(stderr, "elapsed time: %G\n", (float(end_time - start_time) / CLOCKS_PER_SEC));
	}
	catch (std::runtime_error &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}

	return 0;
}
