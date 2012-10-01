/* Text-to-Speech Engine API.
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

#ifndef CAINTEOIR_ENGINE_ENGINES_HPP
#define CAINTEOIR_ENGINE_ENGINES_HPP

#include "audio.hpp"
#include "document.hpp"

namespace cainteoir { namespace tts
{
	struct engine;

	struct speech
	{
		virtual ~speech() {}

		virtual bool is_speaking() const = 0;

		virtual void stop() = 0;
		virtual void wait() = 0;

		virtual double elapsedTime() const = 0;
		virtual double totalTime() const = 0;

		virtual double completed() const = 0;

		virtual size_t position() const = 0;

		virtual std::string error_message() const = 0;
	};

	struct parameter
	{
		virtual ~parameter() {}

		virtual const char *name() const = 0;

		virtual const char *units() const = 0;

		virtual int minimum() const = 0;

		virtual int maximum() const = 0;

		virtual int default_value() const = 0;

		virtual int value() const = 0;

		virtual bool set_value(int value) = 0;

		enum type
		{
			rate,
			volume,
			pitch,
			pitch_range,
			word_gap,
			number_of_parameters
		};
	};

	struct engines
	{
		engines(rdf::graph &metadata);
		~engines();

		bool select_voice(const rdf::graph &aMetadata, const rdf::uri &aVoice);

		const rdf::uri & voice() const { return *selectedVoice; }

		std::shared_ptr<speech>
		speak(const std::shared_ptr<document> &doc,
		      std::shared_ptr<audio> out,
		      size_t offset = 0);

		std::shared_ptr<speech>
		speak(const std::shared_ptr<document> &doc,
		      std::shared_ptr<audio> out,
		      cainteoir::document::const_iterator from,
		      cainteoir::document::const_iterator to);

		std::shared_ptr<cainteoir::tts::parameter>
		parameter(cainteoir::tts::parameter::type aType);
	private:
		std::map<std::string, engine *> enginelist;
		engine *active;
		const rdf::uri *selectedVoice;
	};
}}

#endif
