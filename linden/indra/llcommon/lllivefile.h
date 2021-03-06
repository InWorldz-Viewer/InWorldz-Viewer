/** 
 * @file lllivefile.h
 * @brief Automatically reloads a file whenever it changes or is removed.
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
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

#ifndef LL_LLLIVEFILE_H
#define LL_LLLIVEFILE_H

const F32 configFileRefreshRate = 5.0; // seconds


class LL_COMMON_API LLLiveFile
{
public:
	LLLiveFile(const std::string &filename, const F32 refresh_period = 5.f);
	virtual ~LLLiveFile();

	bool checkAndReload();
		// Returns true if the file changed in any way
		// Call this before using anything that was read & cached from the file

	std::string filename() const;

	void addToEventTimer();
		// Normally, just calling checkAndReload() is enough.  In some cases
		// though, you may need to let the live file periodically check itself.

protected:
	virtual void loadFile() = 0; // Implement this to load your file if it changed

private:
	class Impl;
	Impl& impl;
};

#endif //LL_LLLIVEFILE_H
