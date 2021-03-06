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

#ifndef CAINTEOIR_ENGINE_TTS_ENGINE_HPP
#define CAINTEOIR_ENGINE_TTS_ENGINE_HPP

#include <cainteoir/buffer.hpp>
#include <cainteoir/audio.hpp>
#include <cainteoir/metadata.hpp>
#include <cainteoir/engines.hpp>

#include <stdexcept>
#include <memory>

namespace cainteoir { namespace tts
{
	enum state
	{
		stopped,
		speaking,
		paused,
	};

	struct engine_callback
	{
		virtual ~engine_callback() {}

		virtual tts::state state() const = 0;

		virtual void onaudiodata(short *data, int nsamples) = 0;

		virtual void onspeaking(size_t pos, size_t len) = 0;
	};

	struct engine
	{
		virtual ~engine() {}

		virtual bool select_voice(const char *voicename, const std::string &phonemeset) = 0;

		virtual void speak(buffer *text, size_t offset, engine_callback *callback) = 0;

		virtual std::shared_ptr<phoneme_reader> pronunciation() = 0;

		virtual std::shared_ptr<cainteoir::tts::parameter> parameter(cainteoir::tts::parameter::type aType) = 0;
	};

	engine *create_espeak_engine(rdf::graph &aMetadata, std::string &uri, std::string &default_voice);

	engine *create_pico_engine(rdf::graph &aMetadata, std::string &uri, std::string &default_voice);
}}

#endif
