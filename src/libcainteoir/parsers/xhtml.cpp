/* XHTML Document Parser.
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

#include "parsers.hpp"
#include <cainteoir/xmlreader.hpp>

namespace xml = cainteoir::xml;
namespace rdf = cainteoir::rdf;

void skipNode(xml::reader & reader, const cainteoir::buffer name)
{
	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::endTagNode:
		if (!reader.nodeName().compare(name))
			return;
		break;
	}
}

void parseTitleNode(xml::reader & reader, const cainteoir::buffer name, const rdf::uri &aSubject, cainteoir::document_events &events)
{
	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::endTagNode:
		if (!reader.nodeName().compare(name))
			return;
		break;
	case xml::reader::textNode:
		{
			std::string title = reader.nodeValue().buffer()->str();
			std::string::iterator begin = title.begin();
			while (begin != title.end() && (*begin == ' ' || *begin == '\t' || *begin == '\r' || *begin == '\n'))
				++begin;

			if (begin != title.end())
				events.metadata(rdf::statement(aSubject, rdf::dc("title"), rdf::literal(title)));
		}
		break;
	}
}

void parseHeadNode(xml::reader & reader, const cainteoir::buffer name, const rdf::uri &aSubject, cainteoir::document_events &events)
{
	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::beginTagNode:
		if (!reader.nodeName().comparei("title"))
			parseTitleNode(reader, reader.nodeName(), aSubject, events);
		else if (!reader.nodeName().comparei("meta") ||
		         !reader.nodeName().comparei("link"))
			;
		else
			skipNode(reader, reader.nodeName());
	case xml::reader::endTagNode:
		if (!reader.nodeName().compare(name))
			return;
		break;
	}
}

void parseBodyNode(xml::reader & reader, const cainteoir::buffer name, const rdf::uri &aSubject, cainteoir::document_events &events)
{
	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::beginTagNode:
		if (!reader.nodeName().comparei("script") ||
		    !reader.nodeName().comparei("style"))
			skipNode(reader, reader.nodeName());
		else if (!reader.nodeName().comparei("p"))
			events.begin_context(cainteoir::document_events::paragraph);
		else if (!reader.nodeName().comparei("pre"))
			events.begin_context(cainteoir::document_events::paragraph, cainteoir::document_events::monospace);
		else if (!reader.nodeName().comparei("h1"))
			events.begin_context(cainteoir::document_events::heading, 1);
		else if (!reader.nodeName().comparei("h2"))
			events.begin_context(cainteoir::document_events::heading, 2);
		else if (!reader.nodeName().comparei("h3"))
			events.begin_context(cainteoir::document_events::heading, 3);
		else if (!reader.nodeName().comparei("h4"))
			events.begin_context(cainteoir::document_events::heading, 4);
		else if (!reader.nodeName().comparei("h5"))
			events.begin_context(cainteoir::document_events::heading, 5);
		else if (!reader.nodeName().comparei("h6"))
			events.begin_context(cainteoir::document_events::heading, 6);
		break;
	case xml::reader::textNode:
		{
			std::tr1::shared_ptr<cainteoir::buffer> text = reader.nodeValue().buffer();

			const char * str = text->begin();
			const char * end = text->end();
			while (str != end && (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n'))
				++str;

			if (str != end)
				events.text(text);
		}
		break;
	case xml::reader::endTagNode:
		if (!reader.nodeName().compare(name))
			return;
		if (!reader.nodeName().comparei("p") ||
		    !reader.nodeName().comparei("pre") ||
		    !reader.nodeName().comparei("h1") ||
		    !reader.nodeName().comparei("h2") ||
		    !reader.nodeName().comparei("h3") ||
		    !reader.nodeName().comparei("h4") ||
		    !reader.nodeName().comparei("h5") ||
		    !reader.nodeName().comparei("h6"))
			events.end_context();
		break;
	}
}

void parseHtmlNode(xml::reader & reader, const cainteoir::buffer name, const rdf::uri &aSubject, cainteoir::document_events &events)
{
	std::string lang;
	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::beginTagNode:
		if (!reader.nodeName().comparei("head"))
			parseHeadNode(reader, reader.nodeName(), aSubject, events);
		else
			parseBodyNode(reader, reader.nodeName(), aSubject, events);
		break;
	case xml::reader::attribute:
		if ((!reader.nodeName().comparei("lang") || !reader.nodeName().comparei("xml:lang")) && lang.empty())
		{
			lang = reader.nodeValue().buffer()->str();
			events.metadata(rdf::statement(aSubject, rdf::dc("language"), rdf::literal(lang)));
		}
		break;
	case xml::reader::endTagNode:
		if (!reader.nodeName().compare(name))
			return;
		break;
	}
}

void cainteoir::parseXHtmlDocument(std::tr1::shared_ptr<cainteoir::buffer> data, const rdf::uri &aSubject, cainteoir::document_events &events)
{
	xml::reader reader(data);

	if (reader.isPlainText())
	{
		events.text(data);
		return;
	}

	while (reader.read()) switch (reader.nodeType())
	{
	case xml::reader::beginTagNode:
		if (!reader.nodeName().comparei("html"))
			parseHtmlNode(reader, reader.nodeName(), aSubject, events);
		break;
	}
}
