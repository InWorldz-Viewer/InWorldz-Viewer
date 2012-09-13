/** 
 * @file llviewermedia_streamingaudio.h
 * @author Tofu Linden, Sam Kolb
 * @brief LLStreamingAudio_MediaPlugins implementation - an implementation of the streaming audio interface which is implemented as a client of the media plugins API.
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "linden_common.h"
#include "llpluginclassmedia.h"
#include "llpluginclassmediaowner.h"
#include "llviewermedia.h"
#include "llviewercontrol.h"

#include "llviewermedia_streamingaudio.h"

#include "llmimetypes.h"
#include "llvfs.h"
#include "lldir.h"

#include "llchat.h"
#include "llfloaterchat.h"

LLStreamingAudio_MediaPlugins::LLStreamingAudio_MediaPlugins() :
	mMediaPlugin(NULL),
	mGain(1.0)
{
	// nothing interesting to do?
	// we will lazily create a media plugin at play-time, if none exists.
}

LLStreamingAudio_MediaPlugins::~LLStreamingAudio_MediaPlugins()
{
	delete mMediaPlugin;
	mMediaPlugin = NULL;
}

void LLStreamingAudio_MediaPlugins::start(const std::string& url)
{
	if (url.empty()) 
	{
		return;
	}
	
	std::string test_url(url);
	//llinfos << "Starting internet stream: " << test_url << llendl;
	// note: it's okay if mURL is empty here
	if (mURL != test_url)
	{
		// stop any previous stream that was playing
		// this happens on parcel crossings, usually
		stop();

		if (mMediaPlugin)
		{
			mMediaPlugin->reset();
			mMediaPlugin = initializeMedia("audio/mpeg");
		}
	}
	mURL = test_url;

#ifdef LL_DARWIN
	// We need to change http:// streams to icy:// in order to use them with quicktime.
	// This isn't a good place to put this, but none of this is good, so... -- MC
	LLURI uri(test_url);
	std::string scheme = uri.scheme();
	if ((scheme.empty() || "http" == scheme || "https" == scheme) &&
		((test_url.length() > 4) &&
		 (test_url.substr(test_url.length()-4, 4) != ".pls") &&		// Shoutcast listen.pls playlists
		 (test_url.substr(test_url.length()-4, 4) != ".m3u"))		// Icecast liten.m3u playlists
		)
	{
		std::string temp_url = "icy:" + uri.opaque();
		test_url = temp_url; 
	}
#endif //LL_DARWIN
	
	if (!mMediaPlugin) // lazy-init the underlying media plugin
	{
		mMediaPlugin = initializeMedia("audio/mpeg"); // assumes that whatever media implementation supports mp3 also supports vorbis.
		llinfos << "streaming audio mMediaPlugin is now " << mMediaPlugin << llendl;
	}
	else if (mMediaPlugin->isPluginExited()) // If our reset didn't work right, try again
	{
		mMediaPlugin->reset();
		mMediaPlugin = initializeMedia("audio/mpeg");
	}

	mVersion = mMediaPlugin ? mMediaPlugin->getPluginVersion() : std::string();

	if (!mMediaPlugin)
	{
		llinfos << "mMediaPlugin failed to initialize!" << llendl;
		return;
	}
	else
	{
		mMediaPlugin->loadURI(test_url);
		mMediaPlugin->start();
		llinfos << "Attempting to play internet stream: " << mURL << llendl;	
	}
}

void LLStreamingAudio_MediaPlugins::stop()
{
	llinfos << "entered LLStreamingAudio_MediaPlugins::stop()" << llendl;

	if(mMediaPlugin)
	{
		llinfos << "Stopping internet stream: " << mURL << llendl;
		mMediaPlugin->stop();

		// MURDER DEATH KILL -- MC
		mMediaPlugin->forceCleanUpPlugin();
	}

	mURL.clear();
}

void LLStreamingAudio_MediaPlugins::pause(int pause)
{
	if(!mMediaPlugin)
		return;
	
	if(pause)
	{
		llinfos << "Pausing internet stream: " << mURL << llendl;
		mMediaPlugin->pause();
	} 
	else 
	{
		llinfos << "Unpausing internet stream: " << mURL << llendl;
		mMediaPlugin->start();
	}
}

void LLStreamingAudio_MediaPlugins::update()
{
	if (mMediaPlugin)
		mMediaPlugin->idle();
}

int LLStreamingAudio_MediaPlugins::isPlaying()
{
	if (!mMediaPlugin)
		return 0;
	
	LLPluginClassMediaOwner::EMediaStatus status =
		mMediaPlugin->getStatus();

	switch (status)
	{
	case LLPluginClassMediaOwner::MEDIA_LOADING: // but not MEDIA_LOADED
	case LLPluginClassMediaOwner::MEDIA_PLAYING:
		return 1; // Active and playing
	case LLPluginClassMediaOwner::MEDIA_PAUSED:
		return 2; // paused
	default:
		return 0; // stopped
	}
/*
	// *TODO: can probably do better than this
	if (mMediaPlugin->isPluginRunning())
	{
		return 1; // Active and playing
	}	

	if (mMediaPlugin->isPluginExited())
	{
		return 0; // stopped
	}

	return 2; // paused
*/
}

void LLStreamingAudio_MediaPlugins::setGain(F32 vol)
{
	mGain = vol;

	if(!mMediaPlugin)
		return;

	vol = llclamp(vol, 0.f, 1.f);
	mMediaPlugin->setVolume(vol);
}

F32 LLStreamingAudio_MediaPlugins::getGain()
{
	return mGain;
}

std::string LLStreamingAudio_MediaPlugins::getURL()
{
	return mURL;
}

std::string LLStreamingAudio_MediaPlugins::getVersion()
{
	if(mMediaPlugin)
		return mMediaPlugin->getPluginVersion();

	std::string version = LLMIMETypes::implType("audio/mpeg");
	std::replace(version.begin(), version.end(), '_', ' ');
	return version;
}

void LLStreamingAudio_MediaPlugins::handleMediaEvent(LLPluginClassMedia* self, EMediaEvent event)
{
	if (event == MEDIA_EVENT_NAME_CHANGED)
	{
		std::string title = self->getMediaName();
		if (!title.empty() && gSavedSettings.getBOOL("ShowStreamTitle"))
		{
			//llinfos << "Playing: " << title << llendl;
			LLChat chat;
			//TODO: set this in XUI
			std::string playing_msg = "Playing: " + title;
			chat.mText = playing_msg;
			LLFloaterChat::addChat(chat, FALSE, FALSE);
		}
	}
}

LLPluginClassMedia* LLStreamingAudio_MediaPlugins::initializeMedia(const std::string& media_type)
{
	LLPluginClassMediaOwner* owner = (LLPluginClassMediaOwner*)this;
	S32 default_size = 1; // audio-only - be minimal, doesn't matter
	LLPluginClassMedia* media_source = LLViewerMediaImpl::newSourceFromMediaType(media_type, owner, default_size, default_size);

	if (media_source)
	{
		media_source->setLoop(false); // audio streams are not expected to loop
	}

	return media_source;
}

