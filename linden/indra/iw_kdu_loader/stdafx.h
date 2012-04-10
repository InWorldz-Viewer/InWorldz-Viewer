 /** 
 * @file iw_kdu_loader.cpp
 * @brief stdafx.h : include file for standard system include files,
 *	or project specific include files that are used frequently, but
 *	are changed infrequently
 * 
 * Copyright (c) 2011, McCabe Maxsted
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 or higher.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	// Windows Header Files:
	#include <windows.h>
	# pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS
	#define OS_WINDOWS // define for Windows that's used in this library only

//See Hack in 'iw_kdu_loader/CMakeLists.txt' - Avian
#elif LL_LINUX
	#define OS_LINUX

//#elif LL_DARWIN //add check for mac here
//	#define OS_MAC

#endif

// TODO: reference additional headers your program requires here

