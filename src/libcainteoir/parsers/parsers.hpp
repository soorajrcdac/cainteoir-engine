/* Document Parser API.
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

#ifndef CAINTEOIR_ENGINE_PARSERS_HPP
#define CAINTEOIR_ENGINE_PARSERS_HPP

#include <cainteoir/document.hpp>
#include <cainteoir/archive.hpp>
#include <list>
#include <map>

enum parser_flags
{
	no_document_flags = 0,
	include_document_mimetype = 1,
	needs_document_title = 2,
};

bool parseDocumentBuffer(std::shared_ptr<cainteoir::buffer> &data,
                         const cainteoir::rdf::uri &subject,
                         cainteoir::document_events &events,
                         cainteoir::rdf::graph &aGraph,
                         parser_flags flags);

namespace cainteoir
{
	/** @brief Open Container Format (OCF)
	  * @see   http://www.idpf.org/ocf/ocf1.0/download/ocf10.htm
	  * @see   http://www.idpf.org/specs.htm
	  */
	class ocf_reader
	{
	public:
		ocf_reader(std::shared_ptr<cainteoir::buffer> aData);

		bool read();

		const std::string &mediaType() const { return mMediaType; }

		const std::string &path() const { return mPath; }
	private:
		xml::reader mReader;
		std::string mMediaType;
		std::string mPath;
	};

	/** @brief Navigation Control File (NCX)
	  * @see   http://www.niso.org/workrooms/daisy/Z39-86-2005.html#NCX
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The subject to use for any metadata.
	  * @param events   The events callback to handle document events.
	  */
	void parseNcxDocument(xml::reader &reader,
	                      const rdf::uri &aSubject,
	                      document_events &events,
	                      rdf::graph &aGraph);

	/** @brief Open Publication Format (OPF)
	  * @see   http://www.idpf.org/2007/opf/opf2.0/download/
	  * @see   http://www.idpf.org/specs.htm
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The subject to use for any Dublin Core metadata.
	  * @param events   The events callback to handle document events.
	  */
	void parseOpfDocument(xml::reader &reader,
	                      const rdf::uri &aSubject,
	                      document_events &events,
	                      rdf::graph &aGraph);

	/** @brief XML encoded HTML (XHTML)
	  * @see   http://www.w3.org/TR/xhtml1/
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The base to use for any relative URIs.
	  * @param events   The events callback to handle document events.
	  */
	void parseXHtmlDocument(xml::reader &reader,
	                        const rdf::uri &aSubject,
	                        document_events &events,
	                        rdf::graph &aGraph);

	/** @brief RDF/XML
	  * @see   http://www.w3.org/TR/2004/REC-rdf-syntax-grammar-20040210/
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The base to use for any relative URIs.
	  * @param events   The events callback to handle document events.
	  */
	void parseRdfXmlDocument(xml::reader &reader,
	                         const rdf::uri &aSubject,
	                         document_events &events,
	                         rdf::graph &aGraph);

	/** @brief Synchronized Multimedia Integration Language (SMIL)
	  * @see   http://www.w3.org/TR/2008/REC-SMIL3-20081201/
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The base to use for any relative URIs.
	  * @param events   The events callback to handle document events.
	  */
	void parseSmilDocument(xml::reader &reader,
	                       const rdf::uri &aSubject,
	                       document_events &events,
	                       rdf::graph &aGraph);

	/** @brief Speech Synthesis Markup Language (SSML)
	  * @see   http://www.w3.org/TR/speech-synthesis/
	  *
	  * @param reader   The XML document reader.
	  * @param aSubject The base to use for any relative URIs.
	  * @param events   The events callback to handle document events.
	  */
	void parseSsmlDocument(xml::reader &reader,
	                       const rdf::uri &aSubject,
	                       document_events &events,
	                       rdf::graph &aGraph);

	/** @brief ePub
	  * @see   http://www.idpf.org/specs.htm
	  *
	  * @param aData    The document data.
	  * @param aSubject The base to use for any relative URIs.
	  * @param events   The events callback to handle document events.
	  */
	void parseEpubDocument(std::shared_ptr<archive> aData,
	                       const rdf::uri &aSubject,
	                       document_events &events,
	                       rdf::graph &aGraph);

	/** @name Document Readers
	  */
	//@{

	/** @brief HyperText Markup Language (HTML)
	  *
	  * @param[in]  aData            The document content.
	  * @param[in]  aSubject         The RDF subject for the document metadata.
	  * @param[out] aPrimaryMetadata The main metadata that describes the document.
	  *
	  * @return A reader over the document contents.
	  */
	std::shared_ptr<document_reader>
	createHtmlReader(std::shared_ptr<buffer> &aData,
	                 const rdf::uri &aSubject,
	                 rdf::graph &aPrimaryMetadata);

	/** @brief HyperText Markup Language (HTML)
	  *
	  * @param[in]  aReader          The XML document reader.
	  * @param[in]  aSubject         The RDF subject for the document metadata.
	  * @param[out] aPrimaryMetadata The main metadata that describes the document.
	  *
	  * @return A reader over the document contents.
	  */
	std::shared_ptr<document_reader>
	createHtmlReader(xml::reader &aReader,
	                 const rdf::uri &aSubject,
	                 rdf::graph &aPrimaryMetadata);

	/** @brief Plain Text
	  *
	  * @param[in]  aData            The document content.
	  * @param[in]  aSubject         The RDF subject for the document metadata.
	  * @param[out] aPrimaryMetadata The main metadata that describes the document.
	  *
	  * @return A reader over the document contents.
	  */
	std::shared_ptr<document_reader>
	createPlainTextReader(std::shared_ptr<buffer> &aData,
	                      const rdf::uri &aSubject,
	                      rdf::graph &aPrimaryMetadata);

	/** @brief Rich Text Format (RTF)
	  *
	  * @param[in]  aData            The document content.
	  * @param[in]  aSubject         The RDF subject for the document metadata.
	  * @param[out] aPrimaryMetadata The main metadata that describes the document.
	  *
	  * @return A reader over the document contents.
	  */
	std::shared_ptr<document_reader>
	createRtfReader(std::shared_ptr<buffer> &aData,
	                const rdf::uri &aSubject,
	                rdf::graph &aPrimaryMetadata);

	/** @brief XML-based HyperText Markup Language (XHTML)
	  *
	  * @param[in]  aReader          The XML document reader.
	  * @param[in]  aSubject         The RDF subject for the document metadata.
	  * @param[out] aPrimaryMetadata The main metadata that describes the document.
	  *
	  * @return A reader over the document contents.
	  */
	std::shared_ptr<document_reader>
	createXHtmlReader(xml::reader &aReader,
	                  const rdf::uri &aSubject,
	                  rdf::graph &aPrimaryMetadata);

	//@}
}

#endif
