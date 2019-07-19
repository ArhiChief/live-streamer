/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * media-element.cpp
 * Copyright (C) 2017 Watson Xu <xuhuashan@gmail.com>
 *
 * live-streamer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * live-streamer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>

#include "media-element.h"

#if 0
#ifdef HAVE_HI3516CV300_SUPPORT
#define USING_HIMPP_BADVIDEO 1
#include "platform/hi3516cv300/himpp-base.h"
#endif
#ifdef HAVE_HI3518V200_SUPPORT
#define USING_HIMPP_BADVIDEO 1
#include "platform/hi3618v200/himpp-base.h"
#endif
#endif

namespace Ipcam {
namespace Media {


//////////////////////////////////////////////////////////////////////////////
// MediaElement
//////////////////////////////////////////////////////////////////////////////

MediaElement::MediaElement(MediaElement* source)
  : _source(source), _enable_count(0), _flags(0)
{
}

MediaElement::~MediaElement()
{
}

void MediaElement::enable()
{
	assert(_enable_count >= 0);

	if (_enable_count++ == 0) {
		if (_source) {
			try {
				_source->enable();
			}
			catch (IpcamError &e) {
				_enable_count--;
				throw e;
			}
		}

		try {
			doEnableElement();
		}
		catch (IpcamError &e) {
			if (_source) _source->disable();

			_enable_count--;

			throw e;
		}
	}
}

void MediaElement::disable()
{
	assert(_enable_count > 0);

	if (_enable_count-- == 1) {
		doDisableElement();

		if (_source) _source->disable();
	}
}


//////////////////////////////////////////////////////////////////////////////
// VideoElement
//////////////////////////////////////////////////////////////////////////////

VideoElement::VideoElement(VideoElement* upstream)
  : MediaElement(MEDIA_ELEMENT(upstream))
{
}

VideoElement::~VideoElement()
{
}

Resolution VideoElement::resolution()
{
	VideoElement* upstream = VIDEO_ELEMENT(source());

	if (upstream == NULL)
		throw IpcamError("Not implemented");

	return upstream->resolution();
}

uint32_t VideoElement::framerate()
{
	VideoElement* upstream = VIDEO_ELEMENT(source());

	if (upstream == NULL)
		throw IpcamError("Not implemented");

#ifdef USING_HIMPP_BADVIDEO
	// nasty hack to see if we're using a himpp element that erroneously
	// 'doubles' the codec framerate
	try {
		himpp_element = HIMPP_VIDEO_ELEMENT(upstream);
		return upstream->framerate() / 2;
	}
	catch (std::bad_cast &c) {
		return upstream->framerate();
	}
#else
	return upstream->framerate();
#endif
}


//////////////////////////////////////////////////////////////////////////////
// AudioElement
//////////////////////////////////////////////////////////////////////////////

AudioElement::AudioElement(AudioElement* source)
  : MediaElement(MEDIA_ELEMENT(source))
{
}

AudioElement::~AudioElement()
{
}

uint32_t AudioElement::samplerate()
{
	AudioElement* upstream = AUDIO_ELEMENT(source());

	if (!upstream)
		throw IpcamError("Not implemented");

	return upstream->samplerate();
}

uint32_t AudioElement::channels()
{
	AudioElement* upstream = AUDIO_ELEMENT(source());

	if (!upstream)
		throw IpcamError("Not implemented");

	return upstream->channels();
}

} // namespace Media
} // namespace Ipcam
