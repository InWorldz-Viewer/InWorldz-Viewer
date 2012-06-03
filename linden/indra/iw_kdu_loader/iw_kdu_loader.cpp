 /** 
 * @file iw_kdu_loader.cpp
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

/* NOTE: we use different versions of files that are normally found in llcommon.
   Rather than risking updating llcommon and accidentally adding a dependency
   that causes us to violate the LGPL, we maintain these here. Try to update both
   copies if necessary, but they're mostly typedefs and inline functions so
   you shouldn't have any real difficulty with them. 

   If you're compiling this, you need to make sure you've included the kdu source
   and libs at "linden\libraries\kdu\". See "linden\libraries\kdu\README.txt" for
   details. 

   CMake should generate everything else.

	-- MC
*/

#if IW_KDU_ENABLED

#include "stdafx.h"
#include "iw_kdu_loader.h"

#ifdef OS_WINDOWS

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// Note: we load this as a DSO in the viewer, so we don't worry about this
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


//Terrible Hack for now - Avian
//#elif defined(OS_LINUX)	// add Linux main here
#else

//#elif OS_MAC	// add mac main here

#endif

#endif //IW_KDU_ENABLED
