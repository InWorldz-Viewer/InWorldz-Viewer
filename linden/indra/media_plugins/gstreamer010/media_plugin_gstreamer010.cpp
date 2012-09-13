/** 
 * @file media_plugin_gstreamer010.cpp
 * @brief GStreamer-0.10 plugin for LLMedia API plugin system
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#include "linden_common.h"

// Needed for _getcwd() RC 
#ifdef LL_WINDOWS
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#ifdef LL_DARWIN
#include <Carbon/Carbon.h>
#endif

#include "llgl.h"

#include "llplugininstance.h"
#include "llpluginmessage.h"
#include "llpluginmessageclasses.h"
#include "media_plugin_base.h"

//#if LL_GSTREAMER010_ENABLED

extern "C" {
#include <gst/gst.h>
#include <gst/gstelement.h>
}

#include "llmediaimplgstreamer.h"
#include "llmediaimplgstreamertriviallogging.h"

#include "llmediaimplgstreamervidplug.h"

//////////////////////////////////////////////////////////////////////////////
//
class MediaPluginGStreamer010 : public MediaPluginBase
{
public:
	MediaPluginGStreamer010(LLPluginInstance::sendMessageFunction host_send_func, void *host_user_data);

	/* virtual */ void receiveMessage(const char *message_string);

	static bool startup();
	static bool closedown();

	static void set_gst_plugin_path();

	gboolean processGSTEvents(GstBus     *bus,
				  GstMessage *message);

	// basic log file writing
	static bool writeToLog(const char* str, ...);

private:
	~MediaPluginGStreamer010();

	std::string getVersion();
	bool navigateTo( const std::string urlIn );
	bool seek( double time_sec );
	bool setVolume( float volume );
	
	// misc
	bool pause();
	bool stop();
	bool play(double rate);
	bool getTimePos(double &sec_out);

	#define MIN_LOOP_SEC 1.0F

	bool mIsLooping;

	enum ECommand {
		COMMAND_NONE,
		COMMAND_STOP,
		COMMAND_PLAY,
		COMMAND_FAST_FORWARD,
		COMMAND_FAST_REWIND,
		COMMAND_PAUSE,
		COMMAND_SEEK,
	};
	ECommand mCommand;

private:
	bool unload();
	bool load();

	bool update(int milliseconds);
        void mouseDown( int x, int y );
        void mouseUp( int x, int y );
        void mouseMove( int x, int y );

        void sizeChanged();
	
	static bool mDoneInit;
	
	guint mBusWatchID;
	
	float mVolume;

	int mDepth;

	// media NATURAL size
	int mNaturalWidth;
	int mNaturalHeight;
	// media current size
	int mCurrentWidth;
	int mCurrentHeight;
	int mCurrentRowbytes;
	  // previous media size so we can detect changes
	  int mPreviousWidth;
	  int mPreviousHeight;
	// desired render size from host
	int mWidth;
	int mHeight;
	// padded texture size we need to write into
	int mTextureWidth;
	int mTextureHeight;
	
	int mTextureFormatPrimary;
	int mTextureFormatType;

	bool mSeekWanted;
	double mSeekDestination;

	std::string mLastTitle;
	
	// Very GStreamer-specific
	GMainLoop *mPump; // event pump for this media
	GstElement *mPlaybin;
	GstSLVideo *mVideoSink;
};

//static
bool MediaPluginGStreamer010::mDoneInit = false;

MediaPluginGStreamer010::MediaPluginGStreamer010(
	LLPluginInstance::sendMessageFunction host_send_func,
	void *host_user_data ) :
	MediaPluginBase(host_send_func, host_user_data),
	mBusWatchID ( 0 ),
	mCurrentRowbytes ( 4 ),
	mTextureFormatPrimary ( GL_RGBA ),
	mTextureFormatType ( GL_UNSIGNED_INT_8_8_8_8_REV ),
	mSeekWanted(false),
	mSeekDestination(0.0),
	mPump ( NULL ),
	mPlaybin ( NULL ),
	mVideoSink ( NULL ),
	mCommand ( COMMAND_NONE )
{
	writeToLog("MediaPluginGStreamer010 PID=%u", U32(LL_GETPID()));
}

///////////////////////////////////////////////////////////////////////////////
//
//#define LL_GST_REPORT_STATE_CHANGES
#ifdef LL_GST_REPORT_STATE_CHANGES
static char* get_gst_state_name(GstState state)
{
	switch (state) {
	case GST_STATE_VOID_PENDING: return "VOID_PENDING";
	case GST_STATE_NULL: return "NULL";
	case GST_STATE_READY: return "READY";
	case GST_STATE_PAUSED: return "PAUSED";
	case GST_STATE_PLAYING: return "PLAYING";
	}
	return "(unknown)";
}
#endif // LL_GST_REPORT_STATE_CHANGES

// static
bool MediaPluginGStreamer010::writeToLog(const char* str, ...)
{
	LLFILE* fp = LLFile::fopen("media_plugin_gstreamer010.log", "a");

    if (!fp)
	{
          return false;
	}

	time_t timeptr = time(NULL);
	struct tm* ltime = localtime(&timeptr);
	char strbuf[1024];
	char strmsg[1024];
	sprintf(strbuf, "[%d:%d:%d] ", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
	va_list arglist;
    va_start(arglist, str);
	vsprintf(strmsg, str, arglist);
	strcat(strbuf, strmsg);

	// write to log file
	fprintf(fp, strbuf);
	fprintf(fp, "\n");
	fclose(fp);

	// mirror in console window if we have one
	puts(strbuf);

	return true;
}

gboolean
MediaPluginGStreamer010::processGSTEvents(GstBus     *bus,
					  GstMessage *message)
{
	if (!message) 
		return TRUE; // shield against GStreamer bug

	// TODO: grok 'duration' message type
	if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_STATE_CHANGED &&
		GST_MESSAGE_TYPE(message) != GST_MESSAGE_BUFFERING &&
	    GST_MESSAGE_TYPE(message) != GST_MESSAGE_TAG)
	{
		writeToLog("Got GST message type: %s", GST_MESSAGE_TYPE_NAME (message));
	}

	switch (GST_MESSAGE_TYPE (message))
	{
		case GST_MESSAGE_BUFFERING:
		{
			// NEEDS GST 0.10.11+ and America discovered by C.Columbus
			gint percent = 0;
			gst_message_parse_buffering(message, &percent);
			writeToLog("GST buffering: %d%%", percent);

			break;
		}
		case GST_MESSAGE_STATE_CHANGED: {
			GstState old_state;
			GstState new_state;
			GstState pending_state;
			gst_message_parse_state_changed(message,
							&old_state,
							&new_state,
							&pending_state);
			#ifdef LL_GST_REPORT_STATE_CHANGES
			// not generally very useful, and rather spammy.
			writeToLog("state change (old,<new>,pending): %s,<%s>,%s",
				get_gst_state_name(old_state),
				get_gst_state_name(new_state),
				get_gst_state_name(pending_state));
			#endif // LL_GST_REPORT_STATE_CHANGES

			switch (new_state) 
			{
				case GST_STATE_VOID_PENDING:
					break;
				case GST_STATE_NULL:
					break;
				case GST_STATE_READY:
					setStatus(STATUS_LOADED);
					break;
				case GST_STATE_PAUSED:
					setStatus(STATUS_PAUSED);
					break;
				case GST_STATE_PLAYING:
					setStatus(STATUS_PLAYING);
					break;
			}
			break;
		}
		case GST_MESSAGE_ERROR:
		{
			GError *err = NULL;
			gchar *debug = NULL;
	
			gst_message_parse_error (message, &err, &debug);
			writeToLog("GST error: %s", err?err->message:"(unknown)");
			if (err)
				g_error_free (err);
			g_free (debug);
	
			mCommand = COMMAND_STOP;
	
			setStatus(STATUS_ERROR);
	
			break;
		}
		case GST_MESSAGE_INFO:
		{
			GError *err = NULL;
			gchar *debug = NULL;
			
			gst_message_parse_info (message, &err, &debug);
			writeToLog("GST info: %s", err?err->message:"(unknown)");
			if (err)
				g_error_free (err);
			g_free (debug);

			break;
		}
		case GST_MESSAGE_WARNING:
		{
			GError *err = NULL;
			gchar *debug = NULL;
	
			gst_message_parse_warning (message, &err, &debug);
			writeToLog("GST warning: %s", err?err->message:"(unknown)");
			if (err)
				g_error_free (err);
			g_free (debug);
	
			break;
		}
		case GST_MESSAGE_TAG: 
		{
			GstTagList *new_tags;

			gst_message_parse_tag( message, &new_tags );

			gchar *title = NULL;

			if ( gst_tag_list_get_string(new_tags, GST_TAG_TITLE, &title) )
			{
				//writeToLog("Title: %s", title);
				std::string newtitle(title);
				gst_tag_list_free(new_tags);

				if ( newtitle != mLastTitle && !newtitle.empty() )
				{
					LLPluginMessage message(LLPLUGIN_MESSAGE_CLASS_MEDIA, "name_text");
					message.setValue("name", newtitle );
					sendMessage( message );
					mLastTitle = newtitle;
				}
				g_free(title);
			}

			break;
		}
		case GST_MESSAGE_EOS:
		{
			/* end-of-stream */
			writeToLog("GST end-of-stream.");
			if (mIsLooping)
			{
				//writeToLog("looping media...");
				double eos_pos_sec = 0.0F;
				bool got_eos_position = getTimePos(eos_pos_sec);
	
				if (got_eos_position && eos_pos_sec < MIN_LOOP_SEC)
				{
					// if we know that the movie is really short, don't
					// loop it else it can easily become a time-hog
					// because of GStreamer spin-up overhead
					writeToLog("really short movie (%0.3fsec) - not gonna loop this, pausing instead.", eos_pos_sec);
					// inject a COMMAND_PAUSE
					mCommand = COMMAND_PAUSE;
				}
				else
				{
					#undef LLGST_LOOP_BY_SEEKING
					// loop with a stop-start instead of a seek, because it actually seems rather
					// faster than seeking on remote streams.
					#ifdef LLGST_LOOP_BY_SEEKING
					// first, try looping by an explicit rewind
					bool seeksuccess = seek(0.0);
					if (seeksuccess)
					{
						play(1.0);
					}
					else
					#endif // LLGST_LOOP_BY_SEEKING
					{  // use clumsy stop-start to loop
						writeToLog("didn't loop by rewinding - stopping and starting instead...");
						stop();
						play(1.0);
					}
				}
			}
			else // not a looping media
			{
				// inject a COMMAND_STOP
				mCommand = COMMAND_STOP;
			}
		} break;

		default:
			/* unhandled message */
			break;
	}

	/* we want to be notified again the next time there is a message
	 * on the bus, so return true (false means we want to stop watching
	 * for messages on the bus and our callback should not be called again)
	 */
	return TRUE;
}

extern "C" {
gboolean
llmediaimplgstreamer_bus_callback (GstBus     *bus,
				   GstMessage *message,
				   gpointer    data)
{
	MediaPluginGStreamer010 *impl = (MediaPluginGStreamer010*)data;
	return impl->processGSTEvents(bus, message);
}
} // extern "C"



bool
MediaPluginGStreamer010::navigateTo ( const std::string urlIn )
{
	if (!mDoneInit)
		return false; // error

	setStatus(STATUS_LOADING);

	writeToLog("Setting media URI: %s", urlIn.c_str());

	mSeekWanted = false;

	if (NULL == mPump ||
	    NULL == mPlaybin)
	{
		setStatus(STATUS_ERROR);
		return false; // error
	}

	// set URI
	g_object_set (G_OBJECT (mPlaybin), "uri", urlIn.c_str(), NULL);
	//g_object_set (G_OBJECT (mPlaybin), "uri", "file:///tmp/movie", NULL);

	// navigateTo implicitly plays, too.
	play(1.0);

	return true;
}


bool
MediaPluginGStreamer010::update(int milliseconds)
{
	if (!mDoneInit)
		return false; // error

	//writeToLog("updating media...");
	
	// sanity check
	if (NULL == mPump ||
	    NULL == mPlaybin)
	{
		writeToLog("dead media...");
		return false;
	}

	// see if there's an outstanding seek wanted
	if (mSeekWanted &&
	    // bleh, GST has to be happy that the movie is really truly playing
	    // or it may quietly ignore the seek (with rtsp:// at least).
	    (GST_STATE(mPlaybin) == GST_STATE_PLAYING))
	{
		seek(mSeekDestination);
		mSeekWanted = false;
	}

	// *TODO: time-limit - but there isn't a lot we can do here, most
	// time is spent in gstreamer's own opaque worker-threads.  maybe
	// we can do something sneaky like only unlock the video object
	// for 'milliseconds' and otherwise hold the lock.
	while (g_main_context_pending(g_main_loop_get_context(mPump)))
	{
	       g_main_context_iteration(g_main_loop_get_context(mPump), FALSE);
	}

	// check for availability of a new frame
	
	if (mVideoSink)
	{
	        GST_OBJECT_LOCK(mVideoSink);
		if (mVideoSink->retained_frame_ready)
		{
			writeToLog("NEW FRAME READY");

			if (mVideoSink->retained_frame_width != mCurrentWidth ||
			    mVideoSink->retained_frame_height != mCurrentHeight)
				// *TODO: also check for change in format
			{
				// just resize container, don't consume frame
				int neww = mVideoSink->retained_frame_width;
				int newh = mVideoSink->retained_frame_height;

				int newd = 4;
				mTextureFormatPrimary = GL_RGBA;
				mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8_REV;

				/*
				int newd = SLVPixelFormatBytes[mVideoSink->retained_frame_format];
				if (SLV_PF_BGRX == mVideoSink->retained_frame_format)
				{
					mTextureFormatPrimary = GL_BGRA;
					mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8_REV;
				}
				else
				{
					mTextureFormatPrimary = GL_RGBA;
					mTextureFormatType = GL_UNSIGNED_INT_8_8_8_8_REV;
				}
				*/

				GST_OBJECT_UNLOCK(mVideoSink);

				mCurrentRowbytes = neww * newd;
				writeToLog("video container resized to %dx%d",
					 neww, newh);

				mDepth = newd;
				mCurrentWidth = neww;
				mCurrentHeight = newh;
				sizeChanged();
				return true;
			}

			if (mPixels &&
			    mCurrentHeight <= mHeight &&
			    mCurrentWidth <= mWidth &&
			    !mTextureSegmentName.empty())
			{
				// we're gonna totally consume this frame - reset 'ready' flag
				mVideoSink->retained_frame_ready = FALSE;
				int destination_rowbytes = mWidth * mDepth;
				for (int row=0; row<mCurrentHeight; ++row)
				{
					memcpy(&mPixels
					        [destination_rowbytes * row],
					       &mVideoSink->retained_frame_data
					        [mCurrentRowbytes * row],
					       mCurrentRowbytes);
				}

				GST_OBJECT_UNLOCK(mVideoSink);
				writeToLog("NEW FRAME REALLY TRULY CONSUMED, TELLING HOST");

				setDirty(0,0,mCurrentWidth,mCurrentHeight);
			}
			else
			{
				// new frame ready, but we're not ready to
				// consume it.

				GST_OBJECT_UNLOCK(mVideoSink);

				writeToLog("NEW FRAME not consumed, still waiting for a shm segment and/or shm resize");
			}

			return true;
		}
		else
		{
			// nothing to do yet.
			GST_OBJECT_UNLOCK(mVideoSink);
			return true;
		}
	}

	return true;
}


void
MediaPluginGStreamer010::mouseDown( int x, int y )
{
  // do nothing
}

void
MediaPluginGStreamer010::mouseUp( int x, int y )
{
  // do nothing
}

void
MediaPluginGStreamer010::mouseMove( int x, int y )
{
  // do nothing
}


bool
MediaPluginGStreamer010::pause()
{
	writeToLog("pausing media...");
	// todo: error-check this?
	gst_element_set_state(mPlaybin, GST_STATE_PAUSED);
	return true;
}

bool
MediaPluginGStreamer010::stop()
{
	writeToLog("stopping media...");
	// todo: error-check this?
	gst_element_set_state(mPlaybin, GST_STATE_READY);
	return true;
}

bool
MediaPluginGStreamer010::play(double rate)
{
	// NOTE: we don't actually support non-natural rate.
	writeToLog("playing media... rate=%f", rate);
	// todo: error-check this?
	gst_element_set_state(mPlaybin, GST_STATE_PLAYING);
	return true;
}

bool
MediaPluginGStreamer010::setVolume( float volume )
{
	// we try to only update volume as conservatively as
	// possible, as many gst-plugins-base versions up to at least
	// November 2008 have critical race-conditions in setting volume - sigh
	if (mVolume == volume)
	{
		return true; // nothing to do, everything's fine
	}

	mVolume = volume;
	if (mDoneInit && mPlaybin)
	{
		g_object_set(mPlaybin, "volume", mVolume, NULL);
		return true;
	}

	return false;
}

bool
MediaPluginGStreamer010::seek(double time_sec)
{
	bool success = false;
	if (mDoneInit && mPlaybin)
	{
		success = gst_element_seek(mPlaybin, 1.0F, GST_FORMAT_TIME,
				GstSeekFlags(GST_SEEK_FLAG_FLUSH |
					     GST_SEEK_FLAG_KEY_UNIT),
				GST_SEEK_TYPE_SET, gint64(time_sec*GST_SECOND),
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
	writeToLog("MEDIA SEEK REQUEST to %f sec result was %d",
		 float(time_sec), int(success));
	return success;
}

bool
MediaPluginGStreamer010::getTimePos(double &sec_out)
{
	bool got_position = false;
	if (mPlaybin)
	{
		gint64 pos;
		GstFormat timefmt = GST_FORMAT_TIME;
		got_position =	gst_element_query_position(mPlaybin,
						   		&timefmt,
						     		&pos);
		got_position = got_position
			&& (timefmt == GST_FORMAT_TIME);
		// GStreamer may have other ideas, but we consider the current position
		// undefined if not PLAYING or PAUSED
		got_position = got_position &&
			(GST_STATE(mPlaybin) == GST_STATE_PLAYING ||
			 GST_STATE(mPlaybin) == GST_STATE_PAUSED);
		if (got_position && !GST_CLOCK_TIME_IS_VALID(pos))
		{
			if (GST_STATE(mPlaybin) == GST_STATE_PLAYING)
			{
				// if we're playing then we treat an invalid clock time
				// as 0, for complicated reasons (insert reason here)
				pos = 0;
			}
			else
			{
				got_position = false;
			}
			
		}
		// If all the preconditions succeeded... we can trust the result.
		if (got_position)
		{
			sec_out = double(pos) / double(GST_SECOND); // gst to sec
		}
	}
	return got_position;
}

bool
MediaPluginGStreamer010::load()
{
	if (!mDoneInit)
		return false; // error

	setStatus(STATUS_LOADING);

	writeToLog("setting up media...");

	mIsLooping = false;
	mVolume = (float) 0.1234567; // minor hack to force an initial volume update

	// Create a pumpable main-loop for this media
	mPump = g_main_loop_new (NULL, FALSE);
	if (!mPump)
	{
		setStatus(STATUS_ERROR);
		return false; // error
	}

	// instantiate a playbin element to do the hard work
	mPlaybin = gst_element_factory_make ("playbin", "play");
	if (!mPlaybin)
	{
		setStatus(STATUS_ERROR);
		return false; // error
	}

	// get playbin's bus
	GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (mPlaybin));
	if (!bus)
	{
		setStatus(STATUS_ERROR);
		return false; // error
	}
	mBusWatchID = gst_bus_add_watch (bus,
					   llmediaimplgstreamer_bus_callback,
					   this);
	gst_object_unref (bus);

	if (NULL == getenv("LL_GSTREAMER_EXTERNAL")) {
		// instantiate a custom video sink
		mVideoSink =
			GST_SLVIDEO(gst_element_factory_make ("private-slvideo", "slvideo"));
		if (!mVideoSink)
		{
			writeToLog("Could not instantiate private-slvideo element.");
			// todo: cleanup.
			setStatus(STATUS_ERROR);
			return false; // error
		}

		// connect the pieces
		g_object_set(mPlaybin, "video-sink", mVideoSink, NULL);
	}

	return true;
}

bool
MediaPluginGStreamer010::unload ()
{
	if (!mDoneInit)
		return false; // error

	writeToLog("unloading media...");
	
	// stop getting callbacks for this bus
	g_source_remove(mBusWatchID);
	mBusWatchID = 0;

	if (mPlaybin)
	{
		gst_element_set_state (mPlaybin, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (mPlaybin));
		mPlaybin = NULL;
	}

	if (mPump)
	{
		g_main_loop_quit(mPump);
		mPump = NULL;
	}

	mVideoSink = NULL;

	setStatus(STATUS_NONE);

	return true;
}


//static
bool
MediaPluginGStreamer010::startup()
{
	// first - check if GStreamer is explicitly disabled
	if (NULL != getenv("LL_DISABLE_GSTREAMER"))
		return false;

	// only do global GStreamer initialization once.
	if (!mDoneInit)
	{
		g_thread_init(NULL);

		// Init the glib type system - we need it.
		g_type_init();
		set_gst_plugin_path();


/*
		// Get symbols!
#if LL_DARWIN
		if (! grab_gst_syms("libgstreamer-0.10.dylib",
				    "libgstvideo-0.10.dylib") )
#elseif LL_WINDOWS
		if (! grab_gst_syms("libgstreamer-0.10.dll",
				    "libgstvideo-0.10.dll") )
#else // linux or other ELFy unixoid
		if (! grab_gst_syms("libgstreamer-0.10.so.0",
				    "libgstvideo-0.10.so.0") )
#endif
		{
			writeToLog("Couldn't find suitable GStreamer 0.10 support on this system - video playback disabled.");
			return false;
		}
*/
// 		if (gst_segtrap_set_enabled)
// 		{
			gst_segtrap_set_enabled(FALSE);
// 		}
// 		else
// 		{
// 			writeToLog("gst_segtrap_set_enabled() is not available; plugin crashes won't be caught.");
// 		}
/*
#if LL_LINUX
		// Gstreamer tries a fork during init, waitpid-ing on it,
		// which conflicts with any installed SIGCHLD handler...
		struct sigaction tmpact, oldact;
		if (gst_registry_fork_set_enabled) {
			// if we can disable SIGCHLD-using forking behaviour,
			// do it.
			gst_registry_fork_set_enabled(false);
		}
		else {
			// else temporarily install default SIGCHLD handler
			// while GStreamer initialises
			tmpact.sa_handler = SIG_DFL;
			sigemptyset( &tmpact.sa_mask );
			tmpact.sa_flags = SA_SIGINFO;
			sigaction(SIGCHLD, &tmpact, &oldact);
		}
#endif // LL_LINUX
*/
		// Protect against GStreamer resetting the locale, yuck.
		static std::string saved_locale;
		saved_locale = setlocale(LC_ALL, NULL);

		// finally, try to initialize GStreamer!
		GError *err = NULL;
		gboolean init_gst_success = gst_init_check(NULL, NULL, &err);

		// restore old locale
		setlocale(LC_ALL, saved_locale.c_str() );
/*
#if LL_LINUX
		// restore old SIGCHLD handler
		if (!gst_registry_fork_set_enabled)
			sigaction(SIGCHLD, &oldact, NULL);
#endif // LL_LINUX
*/
		if (!init_gst_success) // fail
		{
			if (err)
			{
				writeToLog("GST init failed: %s", err->message);
				g_error_free(err);
			}
			else
			{
				writeToLog("GST init failed for unspecified reason.");
			}
			return false;
		}

		// Set up logging facilities
		gst_debug_remove_log_function( gst_debug_log_default );
//		gst_debug_add_log_function( gstreamer_log, NULL );

		// Init our custom plugins - only really need do this once.
		gst_slvideo_init_class();

		// List the plugins GStreamer can find
		writeToLog("Found GStreamer plugins:");
		GList *list;
		GstRegistry *registry = gst_registry_get_default();
		std::string loaded = "No";
		for (list = gst_registry_get_plugin_list(registry);
		     list != NULL;
		     list = g_list_next(list))
		{	 
			GstPlugin *list_plugin = (GstPlugin *)list->data;
			if (gst_plugin_is_loaded(list_plugin)) loaded = "Yes";
			writeToLog("%s, loaded? %s", gst_plugin_get_name(list_plugin), loaded.c_str());
		}
		gst_plugin_list_free(list);

		mDoneInit = true;
	}

	return true;
}

void MediaPluginGStreamer010::set_gst_plugin_path()
{
	// Linux sets GST_PLUGIN_PATH in wrapper.sh, not here.
#if LL_WINDOWS || LL_DARWIN

	std::string imp_dir = "";

	// Get the current working directory: 
#if LL_WINDOWS
	char* raw_dir;
	raw_dir = _getcwd(NULL,0);
	if( raw_dir != NULL )
	{
		imp_dir = std::string( raw_dir );
		// add a trailing slash to the end if there isn't one
		if (*(imp_dir.rbegin()) != '\\')
		{
			imp_dir += '\\';
		}
	}
#elif LL_DARWIN
	CFBundleRef main_bundle = CFBundleGetMainBundle();
	if( main_bundle != NULL )
	{
		CFURLRef bundle_url = CFBundleCopyBundleURL( main_bundle );
		if( bundle_url != NULL )
		{
			#ifndef MAXPATHLEN
			#define MAXPATHLEN 1024
			#endif
			char raw_dir[MAXPATHLEN];
			if( CFURLGetFileSystemRepresentation( bundle_url, true, (UInt8 *)raw_dir, MAXPATHLEN) )
			{
				imp_dir = std::string( raw_dir ) + "/Contents/MacOS/";
			}
			CFRelease(bundle_url);
		}
	}
#endif

	if( imp_dir == "" )
	{
		writeToLog("Could not get application directory, not setting GST_PLUGIN_PATH.");
		return;
	}

	writeToLog("Imprudence is installed at %s", imp_dir.c_str());

	// ":" on Mac and 'Nix, ";" on Windows
	std::string separator = G_SEARCHPATH_SEPARATOR_S;

	// Grab the current path, if it's set.
	std::string old_plugin_path = "";
	char *old_path = getenv("GST_PLUGIN_PATH");
	if(old_path == NULL)
	{
		writeToLog("Did not find user-set GST_PLUGIN_PATH.");
	}
	else
	{
		old_plugin_path = separator + std::string( old_path );
	}

	// Search both Imprudence and Imprudence\lib\gstreamer-plugins.
	// But we also want to search the path the user has set, if any.
	std::string plugin_path =	
		"GST_PLUGIN_PATH=" +
#if LL_WINDOWS
		// On windows, the path should be maindir\llplugin\lib\gstreamer-plugins
		// but getcwd() isn't reliable and can put us a directory higher sometimes
		// The third path is so we can load the plugins when running in Visual Studio
		imp_dir + "lib\\gstreamer-plugins" + G_SEARCHPATH_SEPARATOR_S +
		imp_dir + "llplugin\\lib\\gstreamer-plugins" + G_SEARCHPATH_SEPARATOR_S +
		imp_dir + "..\\..\\..\\..\\newview\\lib\\gstreamer-plugins" +
#elif LL_DARWIN
		imp_dir + separator +
		imp_dir + "/../Resources/lib/gstreamer-plugins" +
#endif
		old_plugin_path;

	int put_result;

	// Place GST_PLUGIN_PATH in the environment settings
#if LL_WINDOWS
	put_result = _putenv( (char*)plugin_path.c_str() );
#elif LL_DARWIN
	put_result = putenv( (char*)plugin_path.c_str() );
#endif

	if( put_result == -1 )
	{
		writeToLog("Setting GST_PLUGIN_PATH failed!");
	}
	else
	{
		writeToLog("GST_PLUGIN_PATH set to %s", getenv("GST_PLUGIN_PATH"));
	}
		
	// Don't load system plugins. We only want to use ours, to avoid conflicts.
#if LL_WINDOWS
	put_result = _putenv( "GST_PLUGIN_SYSTEM_PATH=\"\"" );
#elif LL_DARWIN
	put_result = putenv( "GST_PLUGIN_SYSTEM_PATH=\"\"" );
#endif

	if( put_result == -1 )
	{
		writeToLog("Setting GST_PLUGIN_SYSTEM_PATH=\"\" failed!");
	}
		
#endif // LL_WINDOWS || LL_DARWIN
}


void
MediaPluginGStreamer010::sizeChanged()
{
	// the shared writing space has possibly changed size/location/whatever

	// Check to see whether the movie's NATURAL size has been set yet
	if (1 == mNaturalWidth &&
	    1 == mNaturalHeight)
	{
		mNaturalWidth = mCurrentWidth;
		mNaturalHeight = mCurrentHeight;
		writeToLog("Media NATURAL size better detected as %dx%d",
			 mNaturalWidth, mNaturalHeight);
	}

	// if the size has changed then the shm has changed and the app needs telling
	if (mCurrentWidth != mPreviousWidth ||
	    mCurrentHeight != mPreviousHeight)
	{
		mPreviousWidth = mCurrentWidth;
		mPreviousHeight = mCurrentHeight;

		LLPluginMessage message(LLPLUGIN_MESSAGE_CLASS_MEDIA, "size_change_request");
		message.setValue("name", mTextureSegmentName);
		message.setValueS32("width", mNaturalWidth);
		message.setValueS32("height", mNaturalHeight);
		writeToLog("<--- Sending size change request to application with name: '%s' - natural size is %d x %d", mTextureSegmentName.c_str(), mNaturalWidth, mNaturalHeight);
		sendMessage(message);
	}
}



//static
bool
MediaPluginGStreamer010::closedown()
{

	if (!mDoneInit)
		return false; // error

//	ungrab_gst_syms();

	mDoneInit = false;

	writeToLog("GStreamer010 closed down");

	return true;
}

MediaPluginGStreamer010::~MediaPluginGStreamer010()
{
	//writeToLog("MediaPluginGStreamer010 destructor");

	closedown();

	writeToLog("GStreamer010 destructor");
}


std::string
MediaPluginGStreamer010::getVersion()
{
	std::string plugin_version = "GStreamer010 media plugin, GStreamer version ";
	if (mDoneInit) // &&   gst_version)
	{
		guint major, minor, micro, nano;
		gst_version(&major, &minor, &micro, &nano);
		plugin_version += llformat("%u.%u.%u.%u (runtime), %u.%u.%u.%u (headers)", (unsigned int)major, (unsigned int)minor, (unsigned int)micro, (unsigned int)nano, (unsigned int)GST_VERSION_MAJOR, (unsigned int)GST_VERSION_MINOR, (unsigned int)GST_VERSION_MICRO, (unsigned int)GST_VERSION_NANO);
	}
	else
	{
		plugin_version += "(unknown)";
	}
	return plugin_version;
}

void MediaPluginGStreamer010::receiveMessage(const char *message_string)
{
	//std::cerr << "MediaPluginGStreamer010::receiveMessage: received message: \"" << message_string << "\"";

	LLPluginMessage message_in;

	if(message_in.parse(message_string) >= 0)
	{
		std::string message_class = message_in.getClass();
		std::string message_name = message_in.getName();
		if(message_class == LLPLUGIN_MESSAGE_CLASS_BASE)
		{
			if(message_name == "init")
			{
				LLPluginMessage message("base", "init_response");
				LLSD versions = LLSD::emptyMap();
				versions[LLPLUGIN_MESSAGE_CLASS_BASE] = LLPLUGIN_MESSAGE_CLASS_BASE_VERSION;
				versions[LLPLUGIN_MESSAGE_CLASS_MEDIA] = LLPLUGIN_MESSAGE_CLASS_MEDIA_VERSION;
				versions[LLPLUGIN_MESSAGE_CLASS_MEDIA_TIME] = LLPLUGIN_MESSAGE_CLASS_MEDIA_TIME_VERSION;
				message.setValueLLSD("versions", versions);

				if ( load() )
				{
					writeToLog("GStreamer010 media instance set up");
				}
				else
				{
					writeToLog("GStreamer010 media instance failed to set up");
				}

				message.setValue("plugin_version", getVersion());
				sendMessage(message);
			}
			else if(message_name == "idle")
			{
				// no response is necessary here.
				double time = message_in.getValueReal("time");
				
				// Convert time to milliseconds for update()
				update((int)(time * 1000.0f));
			}
			else if(message_name == "cleanup")
			{
				writeToLog("MediaPluginGStreamer010::receiveMessage: cleanup");
				unload();
				closedown();

				// Reply once we're done
				LLPluginMessage message("base", "cleanup_reply");
				sendMessage(message);

				// Now suicide. Because It is the only honorable thing to do.
				// JUST BE CAREFUL! 
				// http://www.parashift.com/c++-faq-lite/delete-this.html	
				delete this;
				return;
			}
			else if(message_name == "shm_added")
			{
				SharedSegmentInfo info;
				info.mAddress = message_in.getValuePointer("address");
				info.mSize = (size_t)message_in.getValueS32("size");
				std::string name = message_in.getValue("name");

				std::ostringstream str;
				writeToLog("MediaPluginGStreamer010::receiveMessage: shared memory added, name: %s, size: %d, address: %p", name.c_str(), int(info.mSize), info.mAddress);

				mSharedSegments.insert(SharedSegmentMap::value_type(name, info));
			}
			else if(message_name == "shm_remove")
			{
				std::string name = message_in.getValue("name");

				writeToLog("MediaPluginGStreamer010::receiveMessage: shared memory remove, name = %s", name.c_str());
				
				SharedSegmentMap::iterator iter = mSharedSegments.find(name);
				if(iter != mSharedSegments.end())
				{
					if(mPixels == iter->second.mAddress)
					{
						// This is the currently active pixel buffer.  Make sure we stop drawing to it.
						mPixels = NULL;
						mTextureSegmentName.clear();
						
						// Make sure the movie decoder is no longer pointed at the shared segment.
						sizeChanged();						
					}
					mSharedSegments.erase(iter);
				}
				else
				{
					writeToLog("MediaPluginGStreamer010::receiveMessage: unknown shared memory region!");
				}

				// Send the response so it can be cleaned up.
				LLPluginMessage message("base", "shm_remove_response");
				message.setValue("name", name);
				sendMessage(message);
			}
			else
			{
				std::ostringstream str;
				writeToLog("MediaPluginGStreamer010::receiveMessage: unknown base message: %s", message_name.c_str());
			}
		}
		else if(message_class == LLPLUGIN_MESSAGE_CLASS_MEDIA)
		{
			if(message_name == "init")
			{
				// Plugin gets to decide the texture parameters to use.
				LLPluginMessage message(LLPLUGIN_MESSAGE_CLASS_MEDIA, "texture_params");
				// lame to have to decide this now, it depends on the movie.  Oh well.
				mDepth = 4;

				mCurrentWidth = 1;
				mCurrentHeight = 1;
				mPreviousWidth = 1;
				mPreviousHeight = 1;
				mNaturalWidth = 1;
				mNaturalHeight = 1;
				mWidth = 1;
				mHeight = 1;
				mTextureWidth = 1;
				mTextureHeight = 1;

				message.setValueU32("format", GL_RGBA);
				message.setValueU32("type", GL_UNSIGNED_INT_8_8_8_8_REV);

				message.setValueS32("depth", mDepth);
				message.setValueS32("default_width", mWidth);
				message.setValueS32("default_height", mHeight);
				message.setValueU32("internalformat", GL_RGBA8);
				message.setValueBoolean("coords_opengl", true);	// true == use OpenGL-style coordinates, false == (0,0) is upper left.
				message.setValueBoolean("allow_downsample", true); // we respond with grace and performance if asked to downscale
				sendMessage(message);
			}
			else if(message_name == "size_change")
			{
				std::string name = message_in.getValue("name");
				S32 width = message_in.getValueS32("width");
				S32 height = message_in.getValueS32("height");
				S32 texture_width = message_in.getValueS32("texture_width");
				S32 texture_height = message_in.getValueS32("texture_height");

				std::ostringstream str;
				writeToLog("---->Got size change instruction from application with shm name: %s - size is %d x %d", name.c_str(), width, height);

				LLPluginMessage message(LLPLUGIN_MESSAGE_CLASS_MEDIA, "size_change_response");
				message.setValue("name", name);
				message.setValueS32("width", width);
				message.setValueS32("height", height);
				message.setValueS32("texture_width", texture_width);
				message.setValueS32("texture_height", texture_height);
				sendMessage(message);

				if(!name.empty())
				{
					// Find the shared memory region with this name
					SharedSegmentMap::iterator iter = mSharedSegments.find(name);
					if(iter != mSharedSegments.end())
					{
						writeToLog("*** Got size change with matching shm, new size is %d x %d", width, height);
						writeToLog("*** Got size change with matching shm, texture size size is %d x %d", texture_width, texture_height);

						mPixels = (unsigned char*)iter->second.mAddress;
						mTextureSegmentName = name;
						mWidth = width;
						mHeight = height;

						if (texture_width > 1 ||
						    texture_height > 1) // not a dummy size from the app, a real explicit forced size
						{
							writeToLog("**** = REAL RESIZE REQUEST FROM APP");
							
							GST_OBJECT_LOCK(mVideoSink);
							mVideoSink->resize_forced_always = true;
							mVideoSink->resize_try_width = texture_width;
							mVideoSink->resize_try_height = texture_height;
							GST_OBJECT_UNLOCK(mVideoSink);
 						}

						mTextureWidth = texture_width;
						mTextureHeight = texture_height;
					}
				}
			}
			else if(message_name == "load_uri")
			{
				std::string uri = message_in.getValue("uri");
				navigateTo( uri );
				sendStatus();		
			}
			else if(message_name == "mouse_event")
			{
				std::string event = message_in.getValue("event");
				S32 x = message_in.getValueS32("x");
				S32 y = message_in.getValueS32("y");
				
				if(event == "down")
				{
					mouseDown(x, y);
				}
				else if(event == "up")
				{
					mouseUp(x, y);
				}
				else if(event == "move")
				{
					mouseMove(x, y);
				};
			};
		}
		else if(message_class == LLPLUGIN_MESSAGE_CLASS_MEDIA_TIME)
		{
			if(message_name == "stop")
			{
				stop();
			}
			else if(message_name == "start")
			{
				double rate = 0.0;
				if(message_in.hasValue("rate"))
				{
					rate = message_in.getValueReal("rate");
				}
				// NOTE: we don't actually support rate.
				play(rate);
			}
			else if(message_name == "pause")
			{
				pause();
			}
			else if(message_name == "seek")
			{
				double time = message_in.getValueReal("time");
				// defer the actual seek in case we haven't
				// really truly started yet in which case there
				// is nothing to seek upon
				mSeekWanted = true;
				mSeekDestination = time;
			}
			else if(message_name == "set_loop")
			{
				bool loop = message_in.getValueBoolean("loop");
				mIsLooping = loop;
			}
			else if(message_name == "set_volume")
			{
				double volume = message_in.getValueReal("volume");
				writeToLog("MediaPluginGStreamer010::receiveMessage: set_volume: %f", volume);
				setVolume(volume);
			}
		}
		else
		{
			writeToLog("MediaPluginGStreamer010::receiveMessage: unknown message class: %s", message_class.c_str());
		}
	}
}

int init_media_plugin(LLPluginInstance::sendMessageFunction host_send_func, void *host_user_data, LLPluginInstance::sendMessageFunction *plugin_send_func, void **plugin_user_data)
{
	// init log file
	LLFILE* fp = LLFile::fopen("media_plugin_gstreamer010.log", "w");
	if (fp)
	{
		time_t timeptr = time(NULL);
		fprintf(fp, "%s", asctime(localtime(&timeptr)));
		fprintf(fp, "<--- Begin media_plugin_gstreamer010 initialization --->\n");
		fclose(fp);
	}

	if (MediaPluginGStreamer010::startup())
	{
		MediaPluginGStreamer010 *self = new MediaPluginGStreamer010(host_send_func, host_user_data);
		*plugin_send_func = MediaPluginGStreamer010::staticReceiveMessage;
		*plugin_user_data = (void*)self;
		
		return 0; // okay
	}
	else 
	{
		return -1; // failed to init
	}
}

//#else // LL_GSTREAMER010_ENABLED
//
//// Stubbed-out class with constructor/destructor (necessary or windows linker
//// will just think its dead code and optimize it all out)
//class MediaPluginGStreamer010 : public MediaPluginBase
//{
//public:
//	MediaPluginGStreamer010(LLPluginInstance::sendMessageFunction host_send_func, void *host_user_data);
//	~MediaPluginGStreamer010();
//	/* virtual */ void receiveMessage(const char *message_string);
//};
//
//MediaPluginGStreamer010::MediaPluginGStreamer010(
//	LLPluginInstance::sendMessageFunction host_send_func,
//	void *host_user_data ) :
//	MediaPluginBase(host_send_func, host_user_data)
//{
//    // no-op
//}
//
//MediaPluginGStreamer010::~MediaPluginGStreamer010()
//{
//    // no-op
//}
//
//void MediaPluginGStreamer010::receiveMessage(const char *message_string)
//{
//    // no-op 
//}
//
//// We're building without GStreamer enabled.  Just refuse to initialize.
//int init_media_plugin(LLPluginInstance::sendMessageFunction host_send_func, void *host_user_data, LLPluginInstance::sendMessageFunction *plugin_send_func, void **plugin_user_data)
//{
//    return -1;
//}

//#endif // LL_GSTREAMER010_ENABLED
