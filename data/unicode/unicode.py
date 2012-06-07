#!/usr/bin/python

# Copyright (C) 2012 Reece H. Dunn
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

import os
import sys
import json

from xml.dom import minidom
from operator import itemgetter, attrgetter

unicode_data_path = sys.argv[1]

def utf8(c):
	if c < 0x80:
		return '\\x%02X' % c
	return repr(unichr(c).encode('utf-8'))[1:-1]

def within(c, r):
	r_first, r_last = r
	if isinstance(c, int):
		return r_first <= c and c <= r_last
	c_first, c_last = c
	return r_first <= c_first and c_last <= r_last

def enumerate_codepoints(codepoints):
	if isinstance(codepoints, int):
		yield codepoints
	else:
		first, last = codepoints
		for codepoint in range(first, last+1):
			yield codepoint

def read_data(path, split_char=';'):
	first = None
	with open(path) as f:
		for line in f:
			line = line.replace('\n', '')
			if line == '':
				continue

			if '#' in line:
				line = line.split('#')[0]

			data = [' '.join(x.split()) for x in line.split(split_char)]
			if len(data) == 0 or (len(data) == 1 and data[0] == ''):
				continue

			if data[1].endswith(', First>'):
				first = data
				continue

			if data[1].endswith(', Last>'):
				data[0] = (int(first[0], 16), int(data[0], 16))
				data[1] = data[1].replace(', Last>', '>')
				first = None
			elif '..' in data[0]:
				codepoints = data[0].split('..')
				data[0] = (int(codepoints[0], 16), int(codepoints[1], 16))
			else:
				data[0] = int(data[0], 16)
			yield data

def select(tag, ns, name):
	value = tag.getElementsByTagNameNS(ns, name)[0].firstChild.nodeValue
	if type(value).__name__ == 'unicode':
		return value.encode('utf-8')
	return value

def parse_script_codes(language_data):
	doc = minidom.parse(language_data).documentElement
	unicode_names = { # These are where the names differ between IANA and Unicode:
		'Cans': ['Canadian_Aboriginal'],
		'Egyp': ['Egyptian_Hieroglyphs'],
		'Mtei': ['Meetei_Mayek'],
		'Nkoo': ['Nko'],
		'Phag': ['Phags_Pa'],
		'Zyyy': ['Common', 'Inherited'],
	}
	scripts = {}
	for script in doc.getElementsByTagNameNS('http://rhdunn.github.com/cainteoir/schema/iana#', 'Script'):
		ref = select(script, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'value')
		if ref in unicode_names.keys():
			for name in unicode_names[ref]:
				scripts[name] = ref
		else:
			name = select(script, 'http://purl.org/dc/terms/', 'title')
			if '(' in name:
				name = name.split('(')[0]
			if ',' in name:
				name = name.split(',')[0]
			name = '_'.join(name.split())
			scripts[name] = ref
	return scripts

script_codes = parse_script_codes('../languages.rdf')

unicode_char = {}
for data in read_data(os.path.join(unicode_data_path, 'UnicodeData.txt')):
	if data[1] == '<control>' and data[10] != '':
		name = data[10]
	else:
		name = data[1]
	if name.startswith('<') and not name == '<control>':
		name = name[1:-1]
	for codepoint in enumerate_codepoints(data[0]):
		unicode_char[codepoint] = {
			'name': name,
			'category': data[2],
			'script': 'Zzzz', # Unknown
		}

for data in read_data(os.path.join(unicode_data_path, 'DerivedAge.txt')):
	for codepoint in enumerate_codepoints(data[0]):
		try:
			unicode_char[codepoint]['age'] = data[1]
		except KeyError:
			pass

for data in read_data(os.path.join(unicode_data_path, 'Scripts.txt')):
	script = script_codes[data[1]]
	for codepoint in enumerate_codepoints(data[0]):
		try:
			unicode_char[codepoint]['script'] = script
		except KeyError:
			pass

blocks = {}
for data in read_data(os.path.join(unicode_data_path, 'Blocks.txt')):
	blocks[data[1]] = data[0]

print 'script,category,age,codepoint,utf-8,name'
for codepoint, data in sorted(unicode_char.items()):
	print '%s,%s,%s,%04X,%s,%s' % (data['script'], data['category'], data['age'], codepoint, utf8(codepoint), data['name'])
