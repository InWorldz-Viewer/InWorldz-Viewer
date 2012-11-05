/** 
 * @file llversionviewer.h
 * @brief
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2009, Linden Research, Inc.
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

#ifndef LL_LLVERSIONVIEWER_H
#define LL_LLVERSIONVIEWER_H

#include "llpreprocessor.h"

const S32 LL_VERSION_MAJOR = 1;
const S32 LL_VERSION_MINOR = 5;
const S32 LL_VERSION_PATCH = 0;
const S32 LL_VERSION_BUILD = 0;

// Viewer channel. Controls what channel automatic downloads look at 
const char * const LL_CHANNEL = "InWorldz Release";

// Avian - add more build info - hash becomes 'BUILD' number
// Icky awful hack for windows string issue -- MC
#ifdef LL_WINDOWS
#ifdef IW_BUILD_DESC
const char * const IW_VERSION_DESC = IW_BUILD_DESC;
#endif
#else // LL_WINDOWS
	const char * const IW_VERSION_DESC = IW_MACRO_STR(IW_BUILD_DESC);  // added description from command line
#endif

#ifdef IW_REPO_SHA1
	const char * const IW_VERSION_BUILD = IW_MACRO_STR(IW_REPO_SHA1);  // hash from last commit of current branch
#else
	const char * const IW_VERSION_BUILD = "";
#endif

#ifdef IW_REPO_USER
	const char * const IW_VERSION_USER = IW_MACRO_STR(IW_REPO_USER);   // local repo user name
#else
	const char * const IW_VERSION_USER = "";
#endif

#ifdef IW_BYPASS
    const bool iw_bypass = IW_BYPASS;
#else
    const bool iw_bypass = FALSE;
#endif

#if LL_DARWIN
	const char * const LL_VERSION_BUNDLE_ID = "com.inworldz.viewer";
#endif

#endif
