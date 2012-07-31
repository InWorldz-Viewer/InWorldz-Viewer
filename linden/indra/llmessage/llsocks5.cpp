/** 
 * @file llsocks5.cpp
 * @brief Socks 5 implementation
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2009, Linden Research, Inc.
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

#include <string>

#include "linden_common.h"
#include "net.h"
#include "llhost.h"
#include "message.h"
#include "llsocks5.h"


// originally from net.cpp:
#include <stdexcept>

#if LL_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#endif


// originally from net.h:
void tcp_close_channel(S32 handle);
S32 tcp_open_channel(LLHost host);
int tcp_handshake(S32 handle, char * dataout, int outlen, char * datain, int maxinlen);
// Static class variable instances

// We want this to be static to avoid excessive indirection on every
// incomming packet just to do a simple bool test. The getter for this
// member is also static
bool LLSocks::sUdpProxyEnabled;
bool LLSocks::sHttpProxyEnabled;

LLSocks::LLSocks()
{
	sUdpProxyEnabled  = false;
	sHttpProxyEnabled = false;
	mNeedUpdate       = false;
}

// Perform a Socks5 authentication and UDP assioacation to the proxy
// specified by proxy, and assiocate UDP port message_port
int LLSocks::proxyHandshake(LLHost proxy, U32 message_port)
{
	int result;

	/* Socks 5 Auth request */
	socks_auth_request_t  socks_auth_request;
	socks_auth_response_t socks_auth_response;

	socks_auth_request.version     = SOCKS_VERSION;       // Socks version 5
	socks_auth_request.num_methods = 1;                   // Sending 1 method
	socks_auth_request.methods     = mAuthMethodSelected; // send only the selected metho

	result = tcp_handshake(hProxyControlChannel, (char*)&socks_auth_request, sizeof(socks_auth_request_t), (char*)&socks_auth_response, sizeof(socks_auth_response_t));
	if (result != 0)
	{
		llwarns << "Socks authentication request failed, error on TCP control channel : " << result << llendl;
		stopProxy();
		return SOCKS_CONNECT_ERROR;
	}
	
	if (socks_auth_response.method == AUTH_NOT_ACCEPTABLE)
	{
		llwarns << "Socks5 server refused all our authentication methods" << llendl;
		stopProxy();
		return SOCKS_NOT_ACCEPTABLE;
	}

	// SOCKS5 USERNAME/PASSWORD authentication
	if (socks_auth_response.method == METHOD_PASSWORD)
	{
		// The server has requested a username/password combination
		U32 request_size = mSocksUsername.size() + mSocksPassword.size() + 3;
		char * password_auth = (char *)malloc(request_size);
		password_auth[0] = 0x01;
		password_auth[1] = mSocksUsername.size();
		memcpy(&password_auth[2],mSocksUsername.c_str(), mSocksUsername.size());
		password_auth[mSocksUsername.size()+2] = mSocksPassword.size();
		memcpy(&password_auth[mSocksUsername.size()+3], mSocksPassword.c_str(), mSocksPassword.size());

		authmethod_password_reply_t password_reply;

		result = tcp_handshake(hProxyControlChannel, password_auth, request_size, (char*)&password_reply, sizeof(authmethod_password_reply_t));
		free (password_auth);

		if (result != 0)
		{
		llwarns << "Socks authentication failed, error on TCP control channel : " << result << llendl;
			stopProxy();
			return SOCKS_CONNECT_ERROR;
		}

		if (password_reply.status != AUTH_SUCCESS)
		{
			llwarns << "Socks authentication failed" << llendl;
			stopProxy();
			return SOCKS_AUTH_FAIL;
		}
	}

	/* SOCKS5 connect request */

	socks_command_request_t  connect_request;
	socks_command_response_t connect_reply;

	connect_request.version = SOCKS_VERSION;         //Socks V5
	connect_request.command = COMMAND_UDP_ASSOCIATE; // Associate UDP
	connect_request.flag    = FIELD_RESERVED;
	connect_request.atype   = ADDRESS_IPV4;
	connect_request.address = 0; // 0.0.0.0 We are not fussy about address
							     // UDP is promiscious receive for our protocol
	connect_request.port    = 0; // Port must be 0 if you ever want to connect via NAT and your router does port rewrite for you

	result = tcp_handshake(hProxyControlChannel, (char*)&connect_request, sizeof(socks_command_request_t), (char*)&connect_reply, sizeof(socks_command_response_t));
	if (result != 0)
	{
		llwarns << "Socks connect request failed, error on TCP control channel : " << result << llendl;
		stopProxy();
		return SOCKS_CONNECT_ERROR;
	}

	if (connect_reply.reply != REPLY_REQUEST_GRANTED)
	{
		//Something went wrong
		llwarns << "Connection to SOCKS5 server failed, UDP forward request not granted" << llendl;
		stopProxy();
		return SOCKS_UDP_FWD_NOT_GRANTED;
	}

	mUDPProxy.setPort(ntohs(connect_reply.port)); // reply port is in network byte order
	mUDPProxy.setAddress(proxy.getAddress());
	// All good now we have been given the UDP port to send requests that need forwarding.
	llinfos << "Socks 5 UDP proxy connected on " << mUDPProxy << llendl;
	return SOCKS_OK;
}

int LLSocks::startProxy(LLHost proxy, U32 message_port)
{
	int status;

	mTCPProxy   = proxy;
	mNeedUpdate = false;

	stopProxy();
	hProxyControlChannel = tcp_open_channel(proxy);	
	if (hProxyControlChannel == -1)
	{
		return SOCKS_HOST_CONNECT_FAILED;
	}

	status = proxyHandshake(proxy, message_port);	
	if (status == SOCKS_OK)
	{
		sUdpProxyEnabled=true;
	}
	return status;
}

int LLSocks::startProxy(std::string host, U32 port)
{
		mTCPProxy.setHostByName(host);
		mTCPProxy.setPort(port);
		return startProxy(mTCPProxy, (U32)gMessageSystem->mPort);
}

void LLSocks::stopProxy()
{
	sUdpProxyEnabled = false;

	if (hProxyControlChannel)
	{
		tcp_close_channel(hProxyControlChannel);
	}
}

void LLSocks::setAuthNone()
{
	mAuthMethodSelected = METHOD_NOAUTH;
}


void LLSocks::setAuthPassword(std::string username, std::string password)
{
	mAuthMethodSelected = METHOD_PASSWORD;
	mSocksUsername      = username;
	mSocksPassword      = password;
}

void LLSocks::EnableHttpProxy(LLHost httpHost, LLHttpProxyType type)
{ 
	sHttpProxyEnabled = true; 
	mHTTPProxy        = httpHost; 
	mProxyType        = type;
}


// originally from net.cpp:

//////////////////////////////////////////////////////////////////////////////////////////
// Windows Versions
//////////////////////////////////////////////////////////////////////////////////////////

#if LL_WINDOWS

int tcp_handshake(S32 handle, char * dataout, int outlen, char * datain, int maxinlen)
{
	int result;
	result = send(handle, dataout, outlen, 0);
	if (result != outlen)
	{
		S32 err = WSAGetLastError();
		llwarns << "Error sending data to proxy control channel, number of bytes sent were " << result << " error code was " << err << llendl;
		return -1;
	}
	
	result = recv(handle, datain, maxinlen, 0);
	if (result != maxinlen)
	{
		S32 err = WSAGetLastError();
		llwarns << "Error receiving data from proxy control channel, number of bytes received were " << result << " error code was " << err << llendl;
		return -1;
	}
	
	return 0;
}

S32 tcp_open_channel(LLHost host)
{
	// Open a TCP channel
	// Jump through some hoops to ensure that if the request hosts is down
	// or not reachable connect() does not block
	
	S32 handle;
	handle = socket(AF_INET, SOCK_STREAM, 0);
	if (!handle)
	{
		llwarns << "Error opening TCP control socket, socket() returned " << handle << llendl;
		return -1;
	}
	
	struct sockaddr_in address;
	address.sin_port        = htons(host.getPort());
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = host.getAddress();
	
	// Non blocking 
	WSAEVENT hEvent=WSACreateEvent();
	WSAEventSelect(handle, hEvent, FD_CONNECT) ;
	connect(handle, (struct sockaddr*)&address, sizeof(address)) ;
	// Wait fot 5 seconds, if we can't get a TCP channel open in this
	// time frame then there is something badly wrong.
	WaitForSingleObject(hEvent, 1000*5); // 5 seconds time out
	
	WSANETWORKEVENTS netevents;
	WSAEnumNetworkEvents(handle,hEvent,&netevents);
	
	// Check the async event status to see if we connected
	if ((netevents.lNetworkEvents & FD_CONNECT) == FD_CONNECT)
	{
		if (netevents.iErrorCode[FD_CONNECT_BIT] != 0)
		{
			llwarns << "Unable to open TCP channel, WSA returned an error code of " << netevents.iErrorCode[FD_CONNECT_BIT] << llendl;
			WSACloseEvent(hEvent);
			return -1;
		}
		
		// Now we are connected disable non blocking
		// we don't need support an async interface as
		// currently our only consumer (socks5) will make one round
		// of packets then just hold the connection open
		WSAEventSelect(handle, hEvent, NULL) ;
		unsigned long NonBlock = 0;
		ioctlsocket(handle, FIONBIO, &NonBlock);
		
		return handle;
	}
	
	llwarns << "Unable to open TCP channel, Timeout is the host up?" << netevents.iErrorCode[FD_CONNECT_BIT] << llendl;
	return -1;
}

void tcp_close_channel(S32 handle)
{
	llinfos << "Closing TCP channel" << llendl;
	shutdown(handle, SD_BOTH);
	closesocket(handle);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Linux Versions
//////////////////////////////////////////////////////////////////////////////////////////

#else


int tcp_handshake(S32 handle, char * dataout, int outlen, char * datain, int maxinlen)
{
	if (send(handle, dataout, outlen, 0) != outlen)
	{
		llwarns << "Error sending data to proxy control channel" << llendl;
		return -1;
	}
	
	if (recv(handle, datain, maxinlen, 0) != maxinlen)
	{
		llwarns << "Error receiving data to proxy control channel" << llendl;		
		return -1;
	}
	
	return 0;
}

S32 tcp_open_channel(LLHost host)
{
	S32 handle;
	handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!handle)
	{
		llwarns << "Error opening TCP control socket, socket() returned " << handle << llendl;
		return -1;
	}
	
	struct sockaddr_in address;
	address.sin_port        = htons(host.getPort());
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = host.getAddress();
	
	// Set the socket to non blocking for the connect()
	int flags = fcntl(handle, F_GETFL, 0);
	fcntl(handle, F_SETFL, flags | O_NONBLOCK);
	
	S32 error = connect(handle, (sockaddr*)&address, sizeof(address));
	if (error && (errno != EINPROGRESS))
	{
		llwarns << "Unable to open TCP channel, error code: " << errno << llendl;
		return -1;
	}
	
	struct timeval timeout;
	timeout.tv_sec  = 5; // Maximum time to wait for the connect() to complete
	timeout.tv_usec = 0;
    fd_set fds;
	FD_ZERO(&fds);
	FD_SET(handle, &fds);
	
	// See if we have connectde or time out after 5 seconds
	U32 rc = select(sizeof(fds)*8, NULL, &fds, NULL, &timeout);	
	
	if (rc != 1) // we require exactly one descriptor to be set
	{
		llwarns << "Unable to open TCP channel" << llendl;
		return -1;
	}
	
	// Return the socket to blocking operations
	fcntl(handle, F_SETFL, flags);
	
	return handle;
}

void tcp_close_channel(S32 handle)
{
	close(handle);
}
#endif

