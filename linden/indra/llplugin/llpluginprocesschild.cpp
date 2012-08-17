/** 
 * @file llpluginprocesschild.cpp
 * @brief LLPluginProcessChild handles the child side of the external-process plugin API. 
 *
 * @cond
 * $LicenseInfo:firstyear=2008&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 * @endcond
 */

#include "linden_common.h"

#include "llpluginprocesschild.h"
#include "llplugininstance.h"
#include "llpluginmessagepipe.h"
#include "llpluginmessageclasses.h"

#if LL_WINDOWS
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif

static const F32 HEARTBEAT_SECONDS = 1.0f;
static const F32 PLUGIN_IDLE_SECONDS = 1.0f / 100.0f;  // Each call to idle will give the plugin this much time.

LLPluginProcessChild::LLPluginProcessChild()
{
	mState = STATE_UNINITIALIZED;
	mInstance = NULL;
	mSocket = LLSocket::create(gAPRPoolp, LLSocket::STREAM_TCP);
	mSleepTime = PLUGIN_IDLE_SECONDS;	// default: send idle messages at 100Hz
	mCPUElapsed = 0.0f;
	mBlockingRequest = false;
	mBlockingResponseReceived = false;
}

LLPluginProcessChild::~LLPluginProcessChild()
{
	if(mInstance != NULL)
	{
		sendMessageToPlugin(LLPluginMessage("base", "cleanup"));

		// IMPORTANT: under some (unknown) circumstances the apr_dso_unload() triggered when mInstance is deleted 
		// appears to fail and lock up which means that a given instance of the slplugin process never exits. 
		// This is bad, especially when users try to update their version of SL - it fails because the slplugin 
		// process as well as a bunch of plugin specific files are locked and cannot be overwritten.
		exit( 0 );
		//delete mInstance;
		//mInstance = NULL;
	}
}

void LLPluginProcessChild::killSockets(void)
{
	killMessagePipe();
	mSocket.reset();
}

void LLPluginProcessChild::init(U32 launcher_port)
{
	mLauncherHost = LLHost("127.0.0.1", launcher_port);
	setState(STATE_INITIALIZED);
}

void LLPluginProcessChild::idle(void)
{
	bool idle_again;
	do
	{
		if(APR_STATUS_IS_EOF(mSocketError))
		{
			// Plugin socket was closed.  This covers both normal plugin termination and host crashes.
			setState(STATE_ERROR);
		}
		else if(mSocketError != APR_SUCCESS)
		{
			LL_INFOS("PluginChild") << "message pipe is in error state (" << mSocketError << "), moving to STATE_ERROR"<< LL_ENDL;
			setState(STATE_ERROR);
		}

		if((mState > STATE_INITIALIZED) && (mMessagePipe == NULL))
		{
			// The pipe has been closed -- we're done.
			// TODO: This could be slightly more subtle, but I'm not sure it needs to be.
			LL_INFOS("PluginChild") << "message pipe went away, moving to STATE_ERROR"<< LL_ENDL;
			setState(STATE_ERROR);
		}

		// If a state needs to go directly to another state (as a performance enhancement), it can set idle_again to true after calling setState().
		// USE THIS CAREFULLY, since it can starve other code.  Specifically make sure there's no way to get into a closed cycle and never return.
		// When in doubt, don't do it.
		idle_again = false;
		
		if(mInstance != NULL)
		{
			// Provide some time to the plugin
			mInstance->idle();
		}
		
		switch(mState)
		{
			case STATE_UNINITIALIZED:
			break;

			case STATE_INITIALIZED:
				if(mSocket->blockingConnect(mLauncherHost))
				{
					// This automatically sets mMessagePipe
					new LLPluginMessagePipe(this, mSocket);
					
					setState(STATE_CONNECTED);
				}
				else
				{
					// connect failed
					setState(STATE_ERROR);
				}
			break;
			
			case STATE_CONNECTED:
				sendMessageToParent(LLPluginMessage(LLPLUGIN_MESSAGE_CLASS_INTERNAL, "hello"));
				setState(STATE_PLUGIN_LOADING);
			break;
						
			case STATE_PLUGIN_LOADING:
				if(!mPluginFile.empty())
				{
					mInstance = new LLPluginInstance(this);
					if(mInstance->load(mPluginFile) == 0)
					{
						mHeartbeat.start();
						mHeartbeat.setTimerExpirySec(HEARTBEAT_SECONDS);
						mCPUElapsed = 0.0f;
						setState(STATE_PLUGIN_LOADED);
					}
					else
					{
						setState(STATE_ERROR);
					}
				}
			break;
			
			case STATE_PLUGIN_LOADED:
				{
					setState(STATE_PLUGIN_INITIALIZING);
					LLPluginMessage message("base", "init");
					sendMessageToPlugin(message);
				}
			break;
			
			case STATE_PLUGIN_INITIALIZING:
				// waiting for init_response...
			break;
			
			case STATE_RUNNING:
				if(mInstance != NULL)
				{
					// Provide some time to the plugin
					LLPluginMessage message("base", "idle");
					message.setValueReal("time", PLUGIN_IDLE_SECONDS);
					sendMessageToPlugin(message);
					
					mInstance->idle();
					
					if(mHeartbeat.hasExpired())
					{
						
						// This just proves that we're not stuck down inside the plugin code.
						LLPluginMessage heartbeat(LLPLUGIN_MESSAGE_CLASS_INTERNAL, "heartbeat");
						
						// Calculate the approximage CPU usage fraction (floating point value between 0 and 1) used by the plugin this heartbeat cycle.
						// Note that this will not take into account any threads or additional processes the plugin spawns, but it's a first approximation.
						// If we could write OS-specific functions to query the actual CPU usage of this process, that would be a better approximation.
						heartbeat.setValueReal("cpu_usage", mCPUElapsed / mHeartbeat.getElapsedTimeF64());
						
						sendMessageToParent(heartbeat);

						mHeartbeat.reset();
						mHeartbeat.setTimerExpirySec(HEARTBEAT_SECONDS);
						mCPUElapsed = 0.0f;
					}
				}
				// receivePluginMessage will transition to STATE_UNLOADING
			break;

			case STATE_UNLOADING:
				if(mInstance != NULL)
				{
					sendMessageToPlugin(LLPluginMessage("base", "cleanup"));
					delete mInstance;
					mInstance = NULL;
				}
				setState(STATE_UNLOADED);
			break;
			
			case STATE_UNLOADED:
				killSockets();
				setState(STATE_DONE);
			break;

			case STATE_ERROR:
				// Close the socket to the launcher
				killSockets();				
				// TODO: Where do we go from here?  Just exit()?
				setState(STATE_DONE);
			break;
			
			case STATE_DONE:
				// just sit here.
			break;
		}
	
	} while (idle_again);
}

void LLPluginProcessChild::sleep(F64 seconds)
{
	deliverQueuedMessages();
	if(mMessagePipe)
	{
		mMessagePipe->pump(seconds);
	}
	else
	{
		ms_sleep((int)(seconds * 1000.0f));
	}
}

void LLPluginProcessChild::pump(void)
{
	deliverQueuedMessages();
	if(mMessagePipe)
	{
		mMessagePipe->pump(0.0f);
	}
	else
	{
		// Should we warn here?
	}
}


bool LLPluginProcessChild::isRunning(void)
{
	bool result = false;
	
	if(mState == STATE_RUNNING)
		result = true;
		
	return result;
}

bool LLPluginProcessChild::isDone(void)
{
	bool result = false;
	
	switch(mState)
	{
		case STATE_DONE:
		result = true;
		break;
		default:
		break;
	}
		
	return result;
}

void LLPluginProcessChild::sendMessageToPlugin(const LLPluginMessage &message)
{
	if (mInstance)
	{
		std::string buffer = message.generate();

		LL_DEBUGS("PluginChild") << "Sending to plugin: " << buffer << LL_ENDL;
		LLTimer elapsed;
	
		mInstance->sendMessage(buffer);

		mCPUElapsed += elapsed.getElapsedTimeF64();
	}
	else
	{
		LL_WARNS("PluginChild") << "mInstance == NULL" << LL_ENDL;
	}
}

void LLPluginProcessChild::sendMessageToParent(const LLPluginMessage &message)
{
	std::string buffer = message.generate();

	LL_DEBUGS("PluginChild") << "Sending to parent: " << buffer << LL_ENDL;

	writeMessageRaw(buffer);
}

void LLPluginProcessChild::receiveMessageRaw(const std::string &message)
{
	// Incoming message from the TCP Socket

	LL_DEBUGS("PluginChild") << "Received from parent: " << message << LL_ENDL;

	// Decode this message
	LLPluginMessage parsed;
	parsed.parse(message);

	if(mBlockingRequest)
	{
		// We're blocking the plugin waiting for a response.

		if(parsed.hasValue("blocking_response"))
		{
			// This is the message we've been waiting for -- fall through and send it immediately. 
			mBlockingResponseReceived = true;
		}
		else
		{
			// Still waiting.  Queue this message and don't process it yet.
			mMessageQueue.push(message);
			return;
		}
	}
	
	bool passMessage = true;
	
	// FIXME: how should we handle queueing here?
	
	{
		std::string message_class = parsed.getClass();
		if(message_class == LLPLUGIN_MESSAGE_CLASS_INTERNAL)
		{
			passMessage = false;
			
			std::string message_name = parsed.getName();
			if(message_name == "load_plugin")
			{
				mPluginFile = parsed.getValue("file");
			}
			else if(message_name == "shm_add")
			{
				std::string name = parsed.getValue("name");
				size_t size = (size_t)parsed.getValueS32("size");
				
				sharedMemoryRegionsType::iterator iter = mSharedMemoryRegions.find(name);
				if(iter != mSharedMemoryRegions.end())
				{
					// Need to remove the old region first
					LL_WARNS("PluginChild") << "Adding a duplicate shared memory segment!" << LL_ENDL;
				}
				else
				{
					// This is a new region
					LLPluginSharedMemory *region = new LLPluginSharedMemory;
					if(region->attach(name, size))
					{
						mSharedMemoryRegions.insert(sharedMemoryRegionsType::value_type(name, region));
						
						std::stringstream addr;
						addr << region->getMappedAddress();
						
						// Send the add notification to the plugin
						LLPluginMessage message("base", "shm_added");
						message.setValue("name", name);
						message.setValueS32("size", (S32)size);
						message.setValuePointer("address", region->getMappedAddress());
						sendMessageToPlugin(message);
						
						// and send the response to the parent
						message.setMessage(LLPLUGIN_MESSAGE_CLASS_INTERNAL, "shm_add_response");
						message.setValue("name", name);
						sendMessageToParent(message);
					}
					else
					{
						LL_WARNS("PluginChild") << "Couldn't create a shared memory segment!" << LL_ENDL;
						delete region;
					}
				}
				
			}
			else if(message_name == "shm_remove")
			{
				std::string name = parsed.getValue("name");
				sharedMemoryRegionsType::iterator iter = mSharedMemoryRegions.find(name);
				if(iter != mSharedMemoryRegions.end())
				{
					// forward the remove request to the plugin -- its response will trigger us to detach the segment.
					LLPluginMessage message("base", "shm_remove");
					message.setValue("name", name);
					sendMessageToPlugin(message);
				}
				else
				{
					LL_WARNS("PluginChild") << "shm_remove for unknown memory segment!" << LL_ENDL;
				}
			}
			else if(message_name == "sleep_time")
			{
				mSleepTime = parsed.getValueReal("time");
			}
     		#if LL_WINDOWS
			else if(message_name == "show_console")
			{
				createConsole();
			}
			#endif
			else if(message_name == "crash")
			{
				// Crash the plugin
				LL_ERRS("PluginChild") << "Plugin crash requested." << LL_ENDL;
			}
			else if(message_name == "hang")
			{
				// Hang the plugin
				LL_WARNS("PluginChild") << "Plugin hang requested." << LL_ENDL;
				while(1)
				{
					// wheeeeeeeee......
				}
			}
			else
			{
				LL_WARNS("PluginChild") << "Unknown internal message from parent: " << message_name << LL_ENDL;
			}
		}
	}
	
	if(passMessage && mInstance != NULL)
	{
		LLTimer elapsed;

		mInstance->sendMessage(message);

		mCPUElapsed += elapsed.getElapsedTimeF64();
	}
}

/* virtual */
void LLPluginProcessChild::receivePluginMessage(const std::string &message)
{
	LL_DEBUGS("PluginChild") << "Received from plugin: " << message << LL_ENDL;
	
	if(mBlockingRequest)
	{
		// 
		LL_ERRS("Plugin") << "Can't send a message while already waiting on a blocking request -- aborting!" << LL_ENDL;
	}
	
	// Incoming message from the plugin instance
	bool passMessage = true;

	// FIXME: how should we handle queueing here?
	
	// Intercept certain base messages (responses to ones sent by this class)
	{
		// Decode this message
		LLPluginMessage parsed;
		parsed.parse(message);
		
		if(parsed.hasValue("blocking_request"))
		{
			mBlockingRequest = true;
		}

		std::string message_class = parsed.getClass();
		if(message_class == "base")
		{
			std::string message_name = parsed.getName();
			if(message_name == "init_response")
			{
				// The plugin has finished initializing.
				setState(STATE_RUNNING);

				// Don't pass this message up to the parent
				passMessage = false;
				
				LLPluginMessage new_message(LLPLUGIN_MESSAGE_CLASS_INTERNAL, "load_plugin_response");
				LLSD versions = parsed.getValueLLSD("versions");
				new_message.setValueLLSD("versions", versions);
				
				if(parsed.hasValue("plugin_version"))
				{
					std::string plugin_version = parsed.getValue("plugin_version");
					new_message.setValueLLSD("plugin_version", plugin_version);
				}

				// Let the parent know it's loaded and initialized.
				sendMessageToParent(new_message);
			}
			else if(message_name == "shm_remove_response")
			{
				// Don't pass this message up to the parent
				passMessage = false;

				std::string name = parsed.getValue("name");
				sharedMemoryRegionsType::iterator iter = mSharedMemoryRegions.find(name);				
				if(iter != mSharedMemoryRegions.end())
				{
					// detach the shared memory region
					iter->second->detach();
					
					// and remove it from our map
					mSharedMemoryRegions.erase(iter);
					
					// Finally, send the response to the parent.
					LLPluginMessage message(LLPLUGIN_MESSAGE_CLASS_INTERNAL, "shm_remove_response");
					message.setValue("name", name);
					sendMessageToParent(message);
				}
				else
				{
					LL_WARNS("PluginChild") << "shm_remove_response for unknown memory segment!" << LL_ENDL;
				}
			}
		}
	}

	if(passMessage)
	{
		LL_DEBUGS("PluginChild") << "Passing through to parent: " << message << LL_ENDL;
		writeMessageRaw(message);
	}
	
	while(mBlockingRequest)
	{
		// The plugin wants to block and wait for a response to this message.
		sleep(mSleepTime);	// this will pump the message pipe and process messages

		if(mBlockingResponseReceived || mSocketError != APR_SUCCESS || (mMessagePipe == NULL))
		{
			// Response has been received, or we've hit an error state.  Stop waiting.
			mBlockingRequest = false;
			mBlockingResponseReceived = false;
		}
	}
}


void LLPluginProcessChild::setState(EState state)
{
	LL_DEBUGS("PluginChild") << "setting state to " << state << LL_ENDL;
	mState = state;
};

void LLPluginProcessChild::deliverQueuedMessages()
{
	if(!mBlockingRequest)
	{
		while(!mMessageQueue.empty())
		{
			receiveMessageRaw(mMessageQueue.front());
			mMessageQueue.pop();
		}
	}
}

#if LL_WINDOWS
void LLPluginProcessChild::createConsole()
{
	if (!AllocConsole())
	{
		// error creating console
		TCHAR buffer[1024];
		if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL , GetLastError(), 0, buffer, 1024, NULL))
		{
			LL_WARNS("PluginChild") << "Error creating console: " << buffer << LL_ENDL;
		}
		else
		{
			LL_WARNS("PluginChild") << "Error creating console, error code: " << GetLastError() << LL_ENDL;
		}
		return;
	}

	HANDLE std_o_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE std_e_handle = GetStdHandle(STD_ERROR_HANDLE);
	if ((std_o_handle == INVALID_HANDLE_VALUE) || (std_e_handle == INVALID_HANDLE_VALUE))
	{
		LL_WARNS("PluginChild") << "couldn't create console, std out or err not available" << LL_ENDL;
		return;
	}

	int crt_o_filedesc = _open_osfhandle((long)std_o_handle, _O_TEXT);
	int crt_e_filedesc = _open_osfhandle((long)std_e_handle, _O_TEXT);
	if ((crt_o_filedesc == -1) || (crt_e_filedesc == -1))
	{
		LL_WARNS("PluginChild") << "_open_osfhandle failed" << LL_ENDL;
		return;
	}

	// replace stdout handle
	FILE* fp_crt_o = _fdopen(crt_o_filedesc, "w");
	if (!fp_crt_o)
	{
		LL_WARNS("PluginChild") << "_fdopen stdout handle failed" << LL_ENDL;
		return;
	}
	*stdout = *fp_crt_o;
	std::cout.clear(); // clear anything before AllocConsole();

	// replace stderr handle
	FILE* fp_crt_e = _fdopen(crt_e_filedesc, "w");
	if (!fp_crt_e)
	{
		LL_WARNS("PluginChild") << "_fdopen stderr handle failed" << LL_ENDL;
		return;
	}
	*stderr = *fp_crt_e;
	std::cerr.clear(); // clear anything before AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	GetConsoleScreenBufferInfo(std_o_handle, &coninfo);
	coninfo.dwSize.Y = 500; // max console lines
	SetConsoleScreenBufferSize(std_o_handle, coninfo.dwSize);
}
#endif
