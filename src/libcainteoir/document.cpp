/* Document.
 *
 * Copyright (C) 2010-2013 Reece H. Dunn
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

#include <cainteoir/document.hpp>

namespace rdf = cainteoir::rdf;

static inline size_t
anchor(const std::map<std::string, size_t> &anchors, const rdf::uri &anchor)
{
	auto at = anchors.find(anchor.str());
	return (at == anchors.end()) ? size_t(-1) : at->second;
}

static inline cainteoir::document::const_iterator
get_child(const cainteoir::document::list_type &children, size_t index)
{
	if (index == size_t(-1)) return children.end();

	cainteoir::document::const_iterator pos = children.begin();
	std::advance(pos, index);
	return pos;
}

void cainteoir::document::clear()
{
	mLength = 0;
	mChildren.clear();
	mAnchors.clear();
}

void cainteoir::document::add(const document_item &aItem)
{
	mChildren.push_back(aItem);
	if (aItem.type & cainteoir::events::anchor)
		mAnchors[aItem.anchor.str()] = mChildren.size();
	if (aItem.type & cainteoir::events::text)
		mLength += aItem.text->size();
}

cainteoir::document::range_type
cainteoir::document::children(const std::pair<const rdf::uri, const rdf::uri> &aAnchors) const
{
	size_t from = anchor(mAnchors, aAnchors.first);
	size_t to   = anchor(mAnchors, aAnchors.second);

	if (from == size_t(-1)) from = 0;
	if (from > to) std::swap(from, to);

	return range_type(get_child(mChildren, from), get_child(mChildren, to));
}

struct document_range : public cainteoir::document_reader
{
	document_range(const cainteoir::document::range_type &aDocumentRange)
		: mCurrent(aDocumentRange.first)
		, mLast(aDocumentRange.second)
	{
	}

	bool read();

	cainteoir::document::const_iterator mCurrent;
	cainteoir::document::const_iterator mLast;
};

bool document_range::read()
{
	if (mCurrent == mLast) return false;

	*((document_item *)this) = *mCurrent;
	++mCurrent;

	return true;
}

std::shared_ptr<cainteoir::document_reader>
cainteoir::createDocumentReader(const document::range_type &aDocumentRange)
{
	return std::make_shared<document_range>(aDocumentRange);
}
