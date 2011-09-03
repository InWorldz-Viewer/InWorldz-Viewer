/** 
 * @file imagebaseforkdu.cpp
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

#include "imagebaseforkdu.h"

ImageBaseForKDU::ImageBaseForKDU() :
		mWidth(0),
		mHeight(0),
		mComponents(-1),
		mData(NULL),
		mDataSize(-1),
		mIsAvatarBake(false),
		mLastErrorMsg(""),
		mRawDiscardLevel(-1),
		mRate(0.f),
		mDecodeSuccessful(false),
		mMaxBytes(0)
{
}

// Create data when all you know is the data_size
ImageBaseForKDU::ImageBaseForKDU(S16 width, S16 height, S8 components) :
		mWidth(width),
		mHeight(height),
		mComponents(components),
		mData(NULL),
		mDataSize(-1),
		mIsAvatarBake(false),
		mLastErrorMsg(""),
		mRawDiscardLevel(-1),
		mRate(0.f),
		mDecodeSuccessful(false),
		mMaxBytes(0)
{
	allocateDataSize(mWidth, mHeight, mComponents);
}

// Use this as your main entry point when you only have data
// but data_size is just width*height*components -- prefer that ctor
ImageBaseForKDU::ImageBaseForKDU(U8* data, S32 data_size) :
		mWidth(0),
		mHeight(0),
		mComponents(-1),
		mData(NULL),
		mDataSize(-1),
		mIsAvatarBake(false),
		mLastErrorMsg(""),
		mRawDiscardLevel(-1),
		mRate(0.f),
		mDecodeSuccessful(false),
		mMaxBytes(0)
{
	copyData(data, data_size);
}

// Use this as your main entry point whenever possible
ImageBaseForKDU::ImageBaseForKDU(U8* data, S32 data_size, S16 width, S16 height, S8 components) :
		mWidth(width),
		mHeight(height),
		mComponents(components),
		mData(NULL),
		mDataSize(-1),
		mIsAvatarBake(false),
		mLastErrorMsg(""),
		mRawDiscardLevel(-1),
		mRate(0.f),
		mDecodeSuccessful(false),
		mMaxBytes(0)
{
	copyData(data, data_size);
}

ImageBaseForKDU::ImageBaseForKDU(U8* data, S32 data_size, S16 width, S16 height, S8 components, S32 raw_discard_level, F32 rate, S32 max_bytes) :
		mWidth(width),
		mHeight(height),
		mComponents(components),
		mData(NULL),
		mDataSize(-1),
		mIsAvatarBake(false),
		mLastErrorMsg(""),
		mRawDiscardLevel(raw_discard_level),
		mRate(rate),
		mDecodeSuccessful(false),
		mMaxBytes(max_bytes)
{
	copyData(data, data_size);
}

ImageBaseForKDU::~ImageBaseForKDU()
{
	deleteData();
}

void ImageBaseForKDU::setSize(S16 width, S16 height, S8 components)
{
	mWidth = width;
	mHeight = height;
	mComponents = components;
}

bool ImageBaseForKDU::copyData(U8* data, S32 data_size)
{
	if (data && ((data != mData) || (data_size != mDataSize)))
	{
		deleteData();
		allocateData(data_size);
		memcpy(mData, data, data_size);
	}
	return true;
}

void ImageBaseForKDU::deleteData()
{
	if (mData)
	{
		delete[] mData;
		mData = NULL;
		mDataSize = 0;
	}
}

U8* ImageBaseForKDU::allocateDataSize(S32 width, S32 height, S32 components, S32 data_size)
{
	setSize(width, height, components);
	return allocateData(data_size);
}

U8* ImageBaseForKDU::allocateData(S32 data_size)
{
	if (data_size < 0)
	{
		data_size = mWidth * mHeight * mComponents;
		if (data_size <= 0)
		{
			return NULL;
		}
	}
	else if (data_size <= 0 || (data_size > 4096*4096*16))
	{
		return NULL;
	}
	
	if (!mData || data_size != mDataSize)
	{
		deleteData();
		mData = new U8[data_size];
		if (!mData)
		{
			data_size = 0;
			mWidth = mHeight = 0;
			return NULL;
		}
		mDataSize = data_size;
	}
	return mData;
}

void ImageBaseForKDU::resize(U16 width, U16 height, S8 components)
{
	if ((mWidth == width) && (mHeight == height) && (mComponents == components))
	{
		return;
	}

	deleteData();
	// Reallocate the data buffer.
	allocateDataSize(width, height, components);
}
