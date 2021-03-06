/* Test driver for style manager (CSS) tests.
 *
 * Copyright (C) 2012 Reece H. Dunn
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

#include <cainteoir/content.hpp>
#include <stdexcept>

namespace css = cainteoir::css;

std::string get_counter_style_name(const css::style_manager &aStyles, const css::counter_style *aStyle)
{
	for (auto &item : aStyles.counter_styles())
	{
		if (item.second == aStyle)
			return item.first;
	}
	return "decimal";
}

const char *get_counter_system_string(css::counter_system system)
{
	switch (system)
	{
	case css::counter_system::cyclic:     return "cyclic";
	case css::counter_system::fixed:      return "fixed";
	case css::counter_system::symbolic:   return "symbolic";
	case css::counter_system::alphabetic: return "alphabetic";
	case css::counter_system::numeric:    return "numeric";
	case css::counter_system::additive:   return "additive";
	default: return nullptr;
	}
}

int main(int argc, char ** argv)
{
	try
	{
		argc -= 1;
		argv += 1;

		if (argc == 0)
			throw std::runtime_error("no document specified");

		css::style_manager styles;
		styles.parse(cainteoir::make_file_buffer(argv[0]));
		for (auto &item : styles.counter_styles())
		{
			fprintf(stdout, "@counter-style %s {\n", item.first.c_str());
			auto &style = *item.second;
			if (style.system == css::counter_system::fixed)
				fprintf(stdout, "\tsystem: fixed %d;\n", style.initial_symbol_value);
			else
				fprintf(stdout, "\tsystem: %s;\n", get_counter_system_string(style.system));
			fprintf(stdout, "\tnegative: '%s' '%s';\n", style.negative_prefix.c_str(), style.negative_suffix.c_str());
			fprintf(stdout, "\tprefix: '%s';\n", style.prefix.c_str());
			fprintf(stdout, "\tsuffix: '%s';\n", style.suffix.c_str());
			fprintf(stdout, "\trange:");
			if (style.range == css::counter_style::get_auto_range(style.system))
				fprintf(stdout, " auto");
			else
			{
				if (style.range.first == css::counter_style::infinite.first)
					fprintf(stdout, " infinite");
				else
					fprintf(stdout, " %d", style.range.first);
				if (style.range.second == css::counter_style::infinite.second)
					fprintf(stdout, " infinite");
				else
					fprintf(stdout, " %d", style.range.second);
			}
			fprintf(stdout, ";\n");
			fprintf(stdout, "\tsymbols:");
			for (auto &symbol : style.symbols)
				fprintf(stdout, " '%s'", symbol.c_str());
			fprintf(stdout, " ;\n");
			fprintf(stdout, "\tadditive-symbols:");
			bool need_comma = false;
			for (auto &symbol : style.additive_symbols)
			{
				if (need_comma)
					fprintf(stdout, ",");
				fprintf(stdout, " %d '%s'", symbol.first, symbol.second.c_str());
				need_comma = true;
			}
			fprintf(stdout, " ;\n");
			fprintf(stdout, "\tfallback: %s;\n", get_counter_style_name(styles, style.fallback).c_str());
			fprintf(stdout, "}\n\n");
		}
	}
	catch (std::runtime_error &e)
	{
		fprintf(stderr, "error: %s\n", e.what());
	}

	return 0;
}
