/** 
 * @file imagebaseforkdu.h
 * @brief interface inworldz kdu class
 * @author McCabe Maxsted
 *
 * InWorldz Viewer Source Code
 * Copyright (C) 2011, InWorldz LLC.
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
 */

#ifndef IW_IMAGEBASEFORKDU_H
#define IW_IMAGEBASEFORKDU_H

#include "../iw_kdu_loader/includes/stdtypes.h" // pure LGPL -- MC
#include <string>

// our LGPL "interface" class. We use this to send data to any non-GPLv2 decoders
// that we load as DSOs. This makes error reporting very difficult, but we need to
// keep GPL header files from contaminating any LGPL plugin
class ImageBaseForKDU
{
public:
	// creating objects on the stack can risk overflow when decoding a lot of data
	// even though it shouldn't -- consider refactoring LLImageRaw
	ImageBaseForKDU();
	ImageBaseForKDU(S16 width, S16 height, S8 components);
	ImageBaseForKDU(U8* data, S32 data_size);
	ImageBaseForKDU(U8* data, S32 data_size, S16 width, S16 height, S8 components);
	ImageBaseForKDU(U8* data, S32 data_size, S16 width, S16 height, S8 components, S32 raw_discard_level, F32 rate, S32 max_bytes);

	~ImageBaseForKDU();

	// Use this as your main entry point when modifying data
	// preferrably along with setSize()
	bool copyData(U8* data, S32 data_size);

	// sets the sizes of an image; doesn't modify it's data or data_size
	void setSize(S16 width, S16 height, S8 components);
	// sets the width and height of an image while not touching its components
	// be very careful when calling; prefer setting components together if you can
	void setSize(S16 width, S16 height);

	// sets the data_size of an image and modifies its data
	void resize(U16 width, U16 height, S8 components);

	// todo: turn these into getters/setters
	bool		mIsAvatarBake;
	std::string mLastErrorMsg;
	S8			mRawDiscardLevel;
	F32			mRate;
	bool		mDecodeSuccessful;
	S32			mMaxBytes; // max bytes of data, not size

private:
	U16	mWidth;			// todo: <- move to their own struct
	U16	mHeight;		// todo: <- move to their own struct
	S8	mComponents;	// todo: <- move to their own struct
	U8*	mData;			// this NEEDS TO BE FIXED to use std::vector in llimage
	S32 mDataSize;		// convenience variable, not actually w * h * components


	// deletes image data if it exists
	void deleteData();

	// Create data when all you know is the size
	U8* allocateDataSize(S32 width, S32 height, S32 components, S32 data_size = -1);

	// subsystem -- avoid calling directly
	U8* allocateData(S32 data_size = -1);

public:
	U16	getWidth()		{ return mWidth;		}
	U16	getHeight()		{ return mHeight;		}
	S8	getComponents()	{ return mComponents;	}
	U8*	getData()		{ return mData;			}
	S32 getDataSize()	{ return mDataSize;		}
};

#endif // IW_IMAGEBASEFORKDU_H
