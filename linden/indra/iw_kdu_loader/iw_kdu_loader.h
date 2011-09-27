 /** 
 * @file iw_kdu_loader.h
 * @brief Defines the entry point for the DLL application
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

#if IW_KDU_ENABLED

#ifndef IW_KDU_LOADER
#define IW_KDU_LOADER

#include "stdafx.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IW_KDU_LOADER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IW_KDU_LOADER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef OS_WINDOWS
	#ifdef IW_KDU_LOADER_EXPORTS
		#define IW_KDU_LOADER_API __declspec(dllexport)
	#else
		#define IW_KDU_LOADER_API __declspec(dllimport)
	#endif
//#elif OS_LINUX	// add Linux macro here?
//#elif OS_MAC	// add mac macro here?
#endif


// Note: we keep the exported functions in their specific files. Move them here
// if this proves more difficult to maintain -- MC


#endif // IW_KDU_LOADER

#endif //IW_KDU_ENABLED
