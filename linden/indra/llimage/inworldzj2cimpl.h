/** 
 * @file llimagej2cimpl.h
 * @brief declared class for jpeg2000 LGPL implementation
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

#ifndef LL_LLIMAGEJ2CIMPL_H
#define LL_LLIMAGEJ2CIMPL_H


#include "imagebaseforkdu.h"

// Derive from this class to implement JPEG2000 decoding in an LGPL plugin
// (see LLImageJ2CImpl for the details of each function)
// We keep these implementations separate rather than inheriting them
// to keep the licensing distinction clear
class InWorldzJ2CImpl
{
public:
	virtual ~InWorldzJ2CImpl() = 0;

	virtual BOOL getMetadata(ImageBaseForKDU& base) = 0; // LGPL-compatible
	virtual BOOL decodeImpl(ImageBaseForKDU& base,
							ImageBaseForKDU& raw,
							float decode_time, 
							int first_channel, 
							int max_channel_count) = 0; // LGPL-compatible
	virtual BOOL encodeImpl(ImageBaseForKDU& base, 
							ImageBaseForKDU& raw, 
							const char* comment_text, 
							float encode_time=0.0,							
							BOOL reversible=FALSE) = 0; // LGPL-compatible
};
InWorldzJ2CImpl::~InWorldzJ2CImpl() { assert(0); }

#endif //LL_LLIMAGEJ2CIMPL_H
