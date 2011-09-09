/** 
 * @file InWorldzJ2C.h
 * @brief This is an implementation of JPEG2000 encode/decode using Kakadu
 *
 * $LicenseInfo:firstyear=2010&license=viewerlgpl$
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
 */

#ifndef InWorldzJ2C_H
#define InWorldzJ2C_H

// dll includes
#include "stdafx.h"
#include "iw_kdu_loader.h"
#include "stdtypes.h"

// IW viewer includes
#include "../iw_imagebase/imagebaseforkdu.h"
#include "../llimage/inworldzj2cimpl.h"

//
// KDU core header files
//
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"

class LLKDUDecodeState;
class LLKDUMemSource;

class InWorldzJ2C : public InWorldzJ2CImpl
{	
public:
	enum ECodeStreamMode 
	{
		MODE_FAST = 0,
		MODE_RESILIENT = 1,
		MODE_FUSSY = 2
	};
	InWorldzJ2C();
	/*virtual*/ ~InWorldzJ2C();
	
	/*virtual*/ BOOL getMetadata(ImageBaseForKDU* base);
	/*virtual*/ BOOL decodeImpl(ImageBaseForKDU* base, 
								ImageBaseForKDU* raw_image, F32 decode_time, 
								S32 first_channel, 
								S32 max_channel_count);
	/*virtual*/ BOOL encodeImpl(ImageBaseForKDU* base, 
								ImageBaseForKDU* raw_image, 
								const char* comment_text, 
								F32 encode_time=0.0, 
								BOOL reversible=FALSE);

protected:
	BOOL initDecode(ImageBaseForKDU& base, 
					ImageBaseForKDU& raw_image, 
					S32 discard_level = -1, 
					S32* region = NULL);
	BOOL initEncode(ImageBaseForKDU& base, 
					S32 blocks_size = -1, 
					S32 precincts_size = -1, 
					S32 levels = 0);

private:
	BOOL initDecode(ImageBaseForKDU& base, 
					ImageBaseForKDU& raw_image, 
					F32 decode_time, 
					ECodeStreamMode mode, 
					S32 first_channel, 
					S32 max_channel_count, 
					S32 discard_level = -1, 
					S32* region = NULL);
	void setupCodeStream(ImageBaseForKDU& base, 
							BOOL keep_codestream, 
							ECodeStreamMode mode);
	void cleanupCodeStream();

	// Encode variable
	LLKDUMemSource*		mInputp;
	kdu_codestream*		mCodeStreamp;
	kdu_coords*			mTPosp; // tile position
	kdu_dims*			mTileIndicesp;
	S32					mBlocksSize;
	S32					mPrecinctsSize;
	S32					mLevels;

	// Temporary variables for in-progress decodes...
	ImageBaseForKDU*	mRawImagep;
	LLKDUDecodeState*	mDecodeState;
};

// Avoid using the .def file. We want these to be compiler-independent
extern "C" IW_KDU_LOADER_API const char* engineInfoInWorldzJ2C();
extern "C" IW_KDU_LOADER_API InWorldzJ2C* createInWorldzJ2C();
extern "C" IW_KDU_LOADER_API void destroyInWorldzJ2C(InWorldzJ2C* kdu);

#endif
