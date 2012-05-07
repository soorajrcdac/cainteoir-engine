/* Document Parser.
 *
 * Copyright (C) 2010-2012 Reece H. Dunn
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
#include "i18n.h"

#include <cainteoir/mimetype.hpp>
#include <cainteoir/document.hpp>
#include "parsers.hpp"
#include <stdexcept>

namespace rdf = cainteoir::rdf;
namespace mime = cainteoir::mime;

struct DecodeDocument
{
	const mime::mimetype *mimetype;
	cainteoir::decoder_ptr decoder;
};

static const DecodeDocument decode_handlers[] = {
	{ &mime::gzip, &cainteoir::inflate_gzip },
};

struct ParseZipDocument
{
	const mime::mimetype *mimetype;
	decltype(cainteoir::parseEpubDocument) *parser;
};

// magic = uncompressed mimetype file ...
static const ParseZipDocument zip_handlers[] = {
	{ &mime::epub, &cainteoir::parseEpubDocument },
};

struct ParseXmlDocument
{
	const mime::mimetype *mimetype;
	decltype(cainteoir::parseNcxDocument) *parser;
};

// magic = xml ; match namespaceUri and rootName on XML document ...
static const ParseXmlDocument xml_handlers[] = {
	{ &mime::ncx,    &cainteoir::parseNcxDocument },
	{ &mime::opf,    &cainteoir::parseOpfDocument },
	{ &mime::rdfxml, &cainteoir::parseRdfXmlDocument },
	{ &mime::smil,   &cainteoir::parseSmilDocument },
	{ &mime::ssml,   &cainteoir::parseSsmlDocument },
};

std::shared_ptr<cainteoir::buffer> buffer_from_stdin()
{
	cainteoir::rope data;
	char buffer[1024];

	size_t read;
	while ((read = fread(buffer, 1, sizeof(buffer), stdin)) != 0)
	{
		std::shared_ptr<cainteoir::buffer> fiber = std::make_shared<cainteoir::data_buffer>(read);
		memcpy((void *)fiber->begin(), buffer, read);
		data += fiber;
	}

	return data.buffer();
}

inline bool is_mime_header_char(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-';
}

struct mime_headers : public cainteoir::buffer
{
	std::shared_ptr<cainteoir::buffer> mOriginal;
	std::string encoding;
	std::string mimetype;
	parser_flags flags;

	bool parse_headers(const rdf::uri &subject, cainteoir::document_events &events, rdf::graph &aGraph, cainteoir::buffer &boundary)
	{
		while (first <= last)
		{
			cainteoir::buffer name(first, first);
			cainteoir::buffer value(first, first);

			while (first <= last && is_mime_header_char(*first))
				++first;

			name = cainteoir::buffer(name.begin(), first);
			if (name.empty())
			{
				if (*first == '\r' || *first == '\n')
				{
					++first;
					if (*first == '\n')
						++first;
					return true;
				}
				return false;
			}

			if (first[0] == ':' && first[1] == ' ')
			{
				const char * start = first;
				while (first <= last && !(first[0] == '\n' && first[1] != ' ' && first[1] != '\t'))
					++first;

				value = cainteoir::buffer(start + 2, *(first-1) == '\r' ? first-1 : first);
				++first;
			}
			else
				return false;

			if (!name.comparei("Content-Transfer-Encoding"))
			{
				const char * type = value.begin();
				while (type <= value.end() && !(*type == ';' || *type == '\n'))
					++type;
				encoding = std::string(value.begin(), *(type-1) == '\r' ? type-1 : type);
			}
			else if (!name.comparei("Content-Type"))
			{
				const char * type = value.begin();
				while (type <= value.end() && !(*type == ';' || *type == '\n'))
					++type;
				mimetype = std::string(value.begin(), type);

				if (mimetype == "multipart/mixed" || mimetype == "multipart/related")
				{
					++type;
					while (type <= value.end() && (*type == ' ' || *type == '\t'))
						++type;

					const char * name = type;
					while (type <= value.end() && *type != '=')
						++type;

					cainteoir::buffer arg(name, type);
					++type;

					if (*type != '"') continue;
					++type;

					const char * bounds = type;
					while (type <= value.end() && *type != '"')
						++type;
					boundary = cainteoir::buffer(bounds, type);
				}
			}
			else if (!name.comparei("Subject"))
			{
				std::string title = value.str();
				aGraph.statement(subject, rdf::dc("title"), rdf::literal(title));

				events.toc_entry(0, subject, title);
				events.anchor(subject, std::string());
				flags = include_document_mimetype;
			}
			else if (!name.comparei("From"))
			{
				// name ...

				const char * name_begin = value.begin();
				const char * name_end = value.begin();

				while (name_end <= value.end() && *name_end == ' ')
					++name_end;

				while (name_end <= value.end() && *name_end != '<')
					++name_end;

				if (name_end > value.end()) // name only (no email address)
					aGraph.statement(subject, rdf::dc("creator"), rdf::literal(std::string(name_begin, value.end())));
				else
				{
					// email address ...

					const char * mbox_begin = name_end + 1;
					const char * mbox_end = value.end();

					while (mbox_end > mbox_begin && *mbox_end != '>')
						--mbox_end;

					// clean-up name ...

					--name_end;
					while (name_end > value.begin() && *name_end == ' ')
						--name_end;
					++name_end;

					// metadata ...

					const rdf::uri from = aGraph.genid();
					aGraph.statement(subject, rdf::dc("creator"), from);
					aGraph.statement(from, rdf::rdf("type"), rdf::foaf("Person"));
					aGraph.statement(from, rdf::rdf("value"), rdf::literal(std::string(name_begin, name_end)));
					aGraph.statement(from, rdf::foaf("mbox"), rdf::literal("mailto:" + std::string(mbox_begin, mbox_end)));
				}
			}
			else if (!name.comparei("Newsgroups"))
				aGraph.statement(subject, rdf::dc("publisher"), rdf::literal(value.str()));
		}

		return false;
	}

	mime_headers(std::shared_ptr<cainteoir::buffer> &data, const rdf::uri &subject, cainteoir::document_events &events, rdf::graph &aGraph, parser_flags aFlags)
		: cainteoir::buffer(*data)
		, mOriginal(data)
		, encoding("8bit")
		, flags(aFlags)
	{
		while (first <= last && (*first == ' ' || *first == '\t' || *first == '\r' || *first == '\n'))
			++first;

		if (!strncmp(first, "HTTP/1.0 ", 9) || !strncmp(first, "HTTP/1.1 ", 9))
		{
			while (first <= last && *first != '\n')
				++first;
			++first;
		}

		cainteoir::buffer boundary(nullptr, nullptr);
		if (!parse_headers(subject, events, aGraph, boundary))
			first = mOriginal->begin();
		else if (!boundary.empty())
		{
			const char * begin = nullptr;

			while (first <= last)
			{
				if (first[0] == '-' && first[1] == '-' && !strncmp(first + 2, boundary.begin(), boundary.size()))
				{
					if (begin == nullptr)
					{
						first += 2;
						first += boundary.size();
						begin = first;
					}
					else
					{
						last = first;
						first = begin;
						return;
					}
				}
				++first;
			}
		}
	}
};

std::shared_ptr<cainteoir::document_reader>
cainteoir::createDocumentReader(std::shared_ptr<buffer> &aData,
                                const rdf::uri &aSubject,
                                rdf::graph &aPrimaryMetadata)
{
	if (!aData || aData->empty())
		return std::shared_ptr<document_reader>();

	if (mime::xml.match(aData))
	{
		cainteoir::xml::reader reader(aData);
		while (reader.read() && reader.nodeType() != cainteoir::xml::reader::beginTagNode)
			;

		std::string namespaceUri = reader.namespaceUri();
		std::string rootName     = reader.nodeName().str();

		if (mime::xhtml.match(namespaceUri, rootName) || mime::html.match(aData))
			return createXHtmlReader(reader, aSubject, aPrimaryMetadata);

		if (!mime::html.match(aData))
			return std::shared_ptr<document_reader>();
	}

	if (mime::html.match(aData))
		return createHtmlReader(aData, aSubject, aPrimaryMetadata);

	if (mime::rtf.match(aData))
		return createRtfReader(aData, aSubject, aPrimaryMetadata);

	return createPlainTextReader(aData, aSubject, aPrimaryMetadata);
}

std::shared_ptr<cainteoir::document_reader>
cainteoir::createDocumentReader(const char *aFilename,
                                rdf::graph &aPrimaryMetadata)
{
	const rdf::uri subject = rdf::uri(aFilename ? aFilename : "stdin", std::string());

	std::shared_ptr<cainteoir::buffer> data;
	if (aFilename)
		data = std::make_shared<cainteoir::mmap_buffer>(aFilename);
	else
		data = buffer_from_stdin();

	return createDocumentReader(data, subject, aPrimaryMetadata);
}

bool parseDocumentBuffer(std::shared_ptr<cainteoir::buffer> &data,
                         const rdf::uri &subject,
                         cainteoir::document_events &events,
                         rdf::graph &aGraph,
                         parser_flags flags)
{
	// Encoded documents ...

	for (const DecodeDocument *decode = std::begin(decode_handlers); decode != std::end(decode_handlers); ++decode)
	{
		if (decode->mimetype->match(data))
		{
			std::shared_ptr<cainteoir::buffer> decompressed = decode->decoder(*data, 0);
			return parseDocumentBuffer(decompressed, subject, events, aGraph, flags);
		}
	}

	// XML documents ...

	if (mime::xml.match(data))
	{
		cainteoir::xml::reader reader(data);
		while (reader.read() && reader.nodeType() != cainteoir::xml::reader::beginTagNode)
			;

		std::string namespaceUri = reader.namespaceUri();
		std::string rootName     = reader.nodeName().str();

		for (const ParseXmlDocument *xml = std::begin(xml_handlers); xml != std::end(xml_handlers); ++xml)
		{
			if (xml->mimetype->match(namespaceUri, rootName))
			{
				xml->parser(reader, subject, events, aGraph);
				if ((flags & include_document_mimetype) == include_document_mimetype)
					aGraph.statement(subject, rdf::tts("mimetype"), rdf::literal(xml->mimetype->mime_type));
				return true;
			}
		}

		if (mime::xhtml.match(namespaceUri, rootName) || mime::html.match(data))
			goto create_reader;

		return false;
	}

	// Documents with MIME headers ...

	if (mime::email.match(data) || mime::mime.match(data))
	{
		std::shared_ptr<mime_headers> mime = std::make_shared<mime_headers>(data, subject, events, aGraph, flags);

		std::shared_ptr<cainteoir::buffer> decoded;
		if (mime->encoding == "quoted-printable")
			decoded = cainteoir::decode_quoted_printable(*mime, 0);
		else if (mime->encoding == "base64")
			decoded = cainteoir::decode_base64(*mime, 0);
		else if (mime->encoding == "7bit" || mime->encoding == "7BIT")
			decoded = mime;
		else if (mime->encoding == "8bit" || mime->encoding == "8BIT")
			decoded = mime;
		else if (mime->encoding == "binary")
			decoded = mime;
		else
			throw std::runtime_error(i18n("unsupported content-transfer-encoding"));

		flags = mime->flags;
		if (mime->begin() != data->begin()) // Avoid an infinite loop when there is just the mime header.
			return parseDocumentBuffer(decoded, subject, events, aGraph, flags);
	}

	// Zip/Compressed documents ...

	if (mime::zip.match(data))
	{
		auto archive = create_zip_archive(data, subject);

		for (const ParseZipDocument *parse = std::begin(zip_handlers); parse != std::end(zip_handlers); ++parse)
		{
			if (parse->mimetype->match(data))
			{
				parse->parser(archive, subject, events, aGraph);
				if ((flags & include_document_mimetype) == include_document_mimetype)
					aGraph.statement(subject, rdf::tts("mimetype"), rdf::literal(parse->mimetype->mime_type));
				return true;
			}
		}

		bool parsed = false;
		const auto &files = archive->files();
		for (auto file = files.begin(), last = files.end(); file != last; ++file)
		{
			auto buffer   = archive->read(file->c_str());
			auto location = archive->location(*file, std::string());
			parsed |= parseDocumentBuffer(buffer, location, events, aGraph, needs_document_title);
		}

		return parsed;
	}

create_reader:
	// Reader-based document parsers (new-style) ...

	auto reader = createDocumentReader(data, subject, aGraph);
	if (!reader) return false;

	while (reader->read())
	{
		if ((flags & needs_document_title) == needs_document_title)
		{
			if (reader->type & cainteoir::events::toc_entry)
				events.toc_entry((int)reader->parameter, reader->anchor, reader->text->str());
			if (reader->type & cainteoir::events::anchor)
				events.anchor(reader->anchor, std::string());
		}
		if (reader->type & cainteoir::events::begin_context)
			events.begin_context(reader->context, reader->parameter);
		if (reader->type & cainteoir::events::text)
			events.text(reader->text);
		if (reader->type & cainteoir::events::end_context)
			events.end_context();
	}

	return true;
}

void cainteoir::supportedDocumentFormats(rdf::graph &metadata, capability_types capabilities)
{
	std::string baseuri = "http://rhdunn.github.com/cainteoir/formats/document";

	if (capabilities & (cainteoir::metadata_support | cainteoir::text_support))
	{
		mime::epub .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::xhtml.metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::html .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::mhtml.metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::email.metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::rtf  .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::ssml .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::gzip .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::zip  .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
	}

	if (capabilities & cainteoir::metadata_support)
	{
		mime::rdfxml.metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::opf   .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::ncx   .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
		mime::smil  .metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
	}

	if (capabilities & cainteoir::text_support)
	{
		mime::text.metadata(metadata, baseuri, rdf::tts("DocumentFormat"));
	}
}

bool cainteoir::parseDocument(const char *aFilename, cainteoir::document_events &events, rdf::graph &aGraph)
{
	const rdf::uri subject = rdf::uri(aFilename ? aFilename : "stdin", std::string());

	std::shared_ptr<cainteoir::buffer> data;
	if (aFilename)
		data = std::make_shared<cainteoir::mmap_buffer>(aFilename);
	else
		data = buffer_from_stdin();

	return parseDocumentBuffer(data, subject, events, aGraph,
	                           parser_flags(include_document_mimetype|needs_document_title));
}
