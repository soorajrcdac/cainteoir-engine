#!/usr/bin/python

# Copyright (C) 2010 Reece H. Dunn
#
# This file is part of cainteoir-engine.
#
# cainteoir-engine is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cainteoir-engine is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cainteoir-engine.  If not, see <http://www.gnu.org/licenses/>.

import sys
import os

sys.path.append(os.path.join(sys.path[0], '../src/cainteoir-engine'))

import metadata

def ok(cond, message):
	if (not cond):
		raise Exception(message)

def equal(a, b, message):
	ok(a == b, '%s : expected "%s", got "%s"' % (message, b, a))

def check_unpacked_http_resource():
	res = metadata.RDFResource('title', 'http://purl.org/dc/terms/')
	equal(res.ref, 'title', 'res.ref')
	equal(res.base, 'http://purl.org/dc/terms/', 'res.base')
	equal(res.uri, 'http://purl.org/dc/terms/title', 'res.uri')
	equal(str(res), 'http://purl.org/dc/terms/title', 'str(res)')

	res = metadata.RDFResource('type', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#')
	equal(res.ref, 'type', 'res.ref')
	equal(res.base, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'res.base')
	equal(res.uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'res.uri')
	equal(str(res), 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'str(res)')

def check_packed_http_resource():
	res = metadata.RDFResource('http://purl.org/dc/terms/title', None)
	equal(res.ref, 'title', 'res.ref')
	equal(res.base, 'http://purl.org/dc/terms/', 'res.base')
	equal(res.uri, 'http://purl.org/dc/terms/title', 'res.uri')
	equal(str(res), 'http://purl.org/dc/terms/title', 'str(res)')

	res = metadata.RDFResource('http://www.w3.org/1999/02/22-rdf-syntax-ns#type', None)
	equal(res.ref, 'type', 'res.ref')
	equal(res.base, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'res.base')
	equal(res.uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'res.uri')
	equal(str(res), 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'str(res)')

if __name__ == '__main__':
	check_unpacked_http_resource()
	check_packed_http_resource()

