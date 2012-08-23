/** 
 * @file llimagej2c.cpp
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2010, Linden Research, Inc.
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
#include "linden_common.h"

#include "apr_pools.h"
#include "apr_dso.h"

#include "lldir.h"
#include "llimagej2c.h"
#include "llmemtype.h"
#include "inworldzj2cimpl.h"

// todo: templatize these
// inworldz kdu support
typedef InWorldzJ2CImpl* (*CreateInWorldzJ2CImpl)();
typedef void (*DestroyInWorldzJ2CImpl)(InWorldzJ2CImpl*);
typedef const char* (*EngineInfoInWorldzJ2CImpl)();

CreateInWorldzJ2CImpl		iw_create_func;
DestroyInWorldzJ2CImpl		iw_destroy_func;
EngineInfoInWorldzJ2CImpl	iw_engineinfo_func;

// for openjpeg support
typedef LLImageJ2CImpl* (*CreateLLImageJ2CFunction)();
typedef void (*DestroyLLImageJ2CFunction)(LLImageJ2CImpl*);
typedef const char* (*EngineInfoLLImageJ2CFunction)();

//some "private static" variables so we only attempt to load
//dynamic libaries once
CreateLLImageJ2CFunction j2cimpl_create_func = NULL;
DestroyLLImageJ2CFunction j2cimpl_destroy_func = NULL;
EngineInfoLLImageJ2CFunction j2cimpl_engineinfo_func = NULL;

apr_pool_t*					iw_dso_memory_pool;
apr_dso_handle_t*			iw_dso_handle;

//Declare the prototype for theses functions here, their functionality
//will be implemented in other files which define a derived LLImageJ2CImpl
//but only ONE static library which has the implementation for this
//function should ever be included
LLImageJ2CImpl* fallbackCreateLLImageJ2CImpl();
void fallbackDestroyLLImageJ2CImpl(LLImageJ2CImpl* impl);
const char* fallbackEngineInfoLLImageJ2CImpl();

bool sInWorldzKDU = false;

//static
//Loads the required "create", "destroy" and "engineinfo" functions needed
void LLImageJ2C::openDSO()
{
	//attempt to load a DSO and get some functions from it
	std::string dso_name;
	std::string dso_path;

	sInWorldzKDU = false;
	apr_status_t rv;

#if LL_WINDOWS
	dso_name = "iw_kdu_loader.dll";
#elif LL_DARWIN
	dso_name = "libiw_kdu_loader.dylib";
#else
	dso_name = "libiw_kdu_loader.so";
#endif

	dso_path = gDirUtilp->findFile(dso_name,
				       gDirUtilp->getAppRODataDir(),
				       gDirUtilp->getExecutableDir());

	iw_dso_handle      = NULL;
	iw_dso_memory_pool = NULL;

	//attempt to load the shared library
	apr_pool_create(&iw_dso_memory_pool, NULL);
	rv = apr_dso_load(&iw_dso_handle,
					  dso_path.c_str(),
					  iw_dso_memory_pool);

	//now, check for success
	if ( rv == APR_SUCCESS )
	{
		//found the dynamic library
		//now we want to load the functions we're interested in
		CreateInWorldzJ2CImpl  create_func = NULL;
		DestroyInWorldzJ2CImpl dest_func = NULL;
		EngineInfoInWorldzJ2CImpl engineinfo_func = NULL;

		rv = apr_dso_sym((apr_dso_handle_sym_t*)&create_func,
						 iw_dso_handle,
						 "createInWorldzJ2C");
		if ( rv == APR_SUCCESS )
		{
			//we've loaded the create function ok
			//we need to delete via the DSO too
			//so lets check for a destruction function
			rv = apr_dso_sym((apr_dso_handle_sym_t*)&dest_func,
							 iw_dso_handle,
						     "destroyInWorldzJ2C");
			if ( rv == APR_SUCCESS )
			{
				//we've loaded the destroy function ok
				rv = apr_dso_sym((apr_dso_handle_sym_t*)&engineinfo_func,
						 iw_dso_handle,
						 "engineInfoInWorldzJ2C");
				if ( rv == APR_SUCCESS )
				{
					//ok, everything is loaded alright
					iw_create_func  = create_func;
					iw_destroy_func = dest_func;
					iw_engineinfo_func = engineinfo_func;
					sInWorldzKDU = true;
					LL_INFOS("LLKDU") << "Optional J2C renderer " << dso_name << " found at " << dso_path << LL_ENDL;
				}
			}
		}
	}

	if ( !sInWorldzKDU )
	{
		//something went wrong with the DSO or function loading..
		//fall back onto our satefy impl creation function

		// precious verbose debugging, sadly we can't use our
		// 'llinfos' stream etc. this early in the initialisation seq.
		// Want to bet? -- MC
		if (dso_path.empty()) dso_path = "not found";
		char errbuf[256];
		LL_INFOS("LLKDU") << "failed to load syms from optional DSO " << dso_name 
			<< " (" << dso_path << ")" << LL_ENDL;
		apr_strerror(rv, errbuf, sizeof(errbuf));
		LL_INFOS("LLKDU") << "error: " << rv << ", " << errbuf << LL_ENDL;
		apr_dso_error(iw_dso_handle, errbuf, sizeof(errbuf));
		LL_INFOS("LLKDU") << "dso-error: " << rv << ", " << errbuf << LL_ENDL;

		if ( iw_dso_handle )
		{
			apr_dso_unload(iw_dso_handle);
			iw_dso_handle = NULL;
		}

		if ( iw_dso_memory_pool )
		{
			apr_pool_destroy(iw_dso_memory_pool);
			iw_dso_memory_pool = NULL;
		}
	}
}

//static
void LLImageJ2C::closeDSO()
{
	if ( iw_dso_handle ) apr_dso_unload(iw_dso_handle);
	if (iw_dso_memory_pool) apr_pool_destroy(iw_dso_memory_pool);
}

//static
std::string LLImageJ2C::getEngineInfo()
{
	if (sInWorldzKDU && iw_engineinfo_func)
	{
		return iw_engineinfo_func();
	}

	if (!j2cimpl_engineinfo_func)
	{
		j2cimpl_engineinfo_func = fallbackEngineInfoLLImageJ2CImpl;
	}

	return j2cimpl_engineinfo_func();
}

LLImageJ2C::LLImageJ2C() : 	LLImageFormatted(IMG_CODEC_J2C),
							mMaxBytes(0),
							mRawDiscardLevel(-1),
							mRate(0.0f),
							mReversible(FALSE),
							mAreaUsedForDataSizeCalcs(0),
							mImpl(NULL),
							mImplKDU(NULL)
{
	//We assume here that if we wanted to create via
	//a dynamic library that the approriate open calls were made
	//before any calls to this constructor.

	//Therefore, a NULL creation function pointer here means
	//we either did not want to create using functions from the dynamic
	//library or there were issues loading it, either way
	//use our fall back
	if (iw_create_func && sInWorldzKDU)
	{
		mImplKDU = iw_create_func();
	}
	else
	{
		j2cimpl_create_func = fallbackCreateLLImageJ2CImpl;
		mImpl = j2cimpl_create_func();
	}

	// Clear data size table
	for( S32 i = 0; i <= MAX_DISCARD_LEVEL; i++)
	{	// Array size is MAX_DISCARD_LEVEL+1
		mDataSizes[i] = 0;
	}
}

// virtual
LLImageJ2C::~LLImageJ2C()
{
	//We assume here that if we wanted to destroy via
	//a dynamic library that the approriate open calls were made
	//before any calls to this destructor.

	//Therefore, a NULL creation function pointer here means
	//we either did not want to destroy using functions from the dynamic
	//library or there were issues loading it, either way
	//use our fall back
	if (iw_destroy_func && mImplKDU)
	{
		iw_destroy_func(mImplKDU);
	}
	else
	{
		j2cimpl_destroy_func = fallbackDestroyLLImageJ2CImpl;
	}

	if ( mImpl )
	{
		j2cimpl_destroy_func(mImpl);
	}
}

// virtual
void LLImageJ2C::resetLastError()
{
	mLastError.clear();
}

//virtual
void LLImageJ2C::setLastError(const std::string& message, const std::string& filename)
{
	LL_DEBUGS("Rendering") << "LLImageJ2C error: " << message << llendl;
	mLastError = message;
	if (!filename.empty())
		mLastError += std::string(" FILE: ") + filename;
}

// virtual
S8  LLImageJ2C::getRawDiscardLevel()
{
	return mRawDiscardLevel;
}

BOOL LLImageJ2C::updateData()
{
	BOOL res = TRUE;
	resetLastError();

	// Check to make sure that this instance has been initialized with data
	if (!getData() || (getDataSize() < 16))
	{
		setLastError("LLImageJ2C uninitialized");
		res = FALSE;
	}
	else 
	{
		if (mImplKDU)
		{
			// LGPL plugin
			ImageBaseForKDU base(this->getData(), this->getDataSize(), this->getWidth(), this->getHeight(), this->getComponents(), this->getRawDiscardLevel(), this->mRate, this->getMaxBytes());

			if (!base.getData())
			{
				llwarns << "cannot allocate temporary image info" << llendl;
				return FALSE;
			}
			else
			{
				res = mImplKDU->getMetadata(&base);
				
				// We don't return here if hwc <=0 || !res
				// so as to set the any possible error message
				// below
				if (res &&
					(base.getData() != this->getData()) &&
					(base.getHeight() > 0) && 
					(base.getWidth() > 0) && 
					(base.getComponents() > 0))
				{
					this->setMaxBytes(base.mMaxBytes);
					this->mRate = base.mRate;
					this->mRawDiscardLevel = base.mRawDiscardLevel;

					this->setSize(base.getWidth(), base.getHeight(), base.getComponents());
					this->copyData(base.getData(), base.getDataSize()); // calls updateData();
				}
				else
				{
					//llinfos << "updateData failed. Either bad data or no data to update!" << llendl;
				}

				if (!(base.mLastErrorMsg.empty()))
				{
					this->setLastError(base.mLastErrorMsg);
				}
			}
		}
		else
		{
			// GPL plugin
			res = mImpl->getMetadata(*this);
		}
	}

	if (res)
	{
		// SJB: override discard based on mMaxBytes elsewhere
		S32 max_bytes = getDataSize(); // mMaxBytes ? mMaxBytes : getDataSize();
		S32 discard = calcDiscardLevelBytes(max_bytes);
		setDiscardLevel(discard);
	}

	if (!mLastError.empty())
	{
		LLImage::setLastError(mLastError);
	}
	return res;
}


BOOL LLImageJ2C::decode(LLImageRaw *raw_imagep, F32 decode_time)
{
	if (raw_imagep && !raw_imagep->isBufferInvalid())
	{
		return decodeChannels(raw_imagep, decode_time, 0, 4);
	}
	else
	{
		llwarns << "Trying to decode a null pointer!" << llendl;
		LLImage::setLastError("trying to decode an image with an invalid buffer, doing nothing with it!");
		return FALSE;
	}
}


// Returns TRUE to mean done, whether successful or not.
BOOL LLImageJ2C::decodeChannels(LLImageRaw *raw_imagep, F32 decode_time, S32 first_channel, S32 max_channel_count )
{
	LLMemType mt1((LLMemType::EMemType)mMemType);

	BOOL res = TRUE;
	
	resetLastError();

	// Check to make sure that this instance has been initialized with data
	if (!getData() || (getDataSize() < 16))
	{
		setLastError("LLImageJ2C uninitialized");
		res = TRUE; // done
	}
	else
	{
		// Update the raw discard level
		updateRawDiscardLevel();
		mDecoding = TRUE;

		// Here we make changes to the raw image pointer in our decoder plugin
		if (mImplKDU) // the version of KDU used with InWorldz
		{
			if (raw_imagep->isBufferInvalid())
			{
				
				return TRUE;
			}

			// LGPL plugin
			ImageBaseForKDU base(this->getData(), this->getDataSize(), this->getWidth(), this->getHeight(), this->getComponents(), this->getRawDiscardLevel(), this->mRate, this->getMaxBytes());

			ImageBaseForKDU raw(raw_imagep->getData(), raw_imagep->getDataSize(), raw_imagep->getWidth(), raw_imagep->getHeight(), raw_imagep->getComponents());

			if (!base.getData() || !raw.getData())
			{
				llwarns << "cannot allocate temporary image info" << llendl;
			}
			else if (base.getData() == raw.getData())
			{
				llwarns << "decode called, but nothing to decode" << llendl;
			}
			else
			{
				res = mImplKDU->decodeImpl(&base, &raw, decode_time, first_channel, max_channel_count);

				if ((raw.getData() != NULL) &&
					((raw.getData() != this->getData()) &&
					//(raw.mRawDiscardLevel > -1) &&
					(raw.getHeight() > 0) && 
					(raw.getWidth() > 0) && 
					(raw.getComponents() > 0)))
				{
					// resize checks to make sure we need to, then updates in a sane way
					raw_imagep->resize(raw.getWidth(), raw.getHeight(), raw.getComponents());
					if (!raw_imagep->isBufferInvalid())
					{
						memcpy(raw_imagep->getData(), raw.getData(), raw.getDataSize());
					}
					memcpy(raw_imagep->getData(), raw.getData(), raw.getDataSize());

					if (!(base.mLastErrorMsg.empty()))
					{
						this->setLastError(base.mLastErrorMsg);
					}
				}
				else
				{
					mDecoding = FALSE;
				}
			}
		}
		else // GPL-compatible plugins
		{
			res = mImpl->decodeImpl(*this, *raw_imagep, decode_time, first_channel, max_channel_count);
		}
	}
	
	if (res)
	{
		if (!mDecoding)
		{
			// Failed
			raw_imagep->deleteData();
		}
		else
		{
			mDecoding = FALSE;
		}
	}

	if (!mLastError.empty())
	{
		LLImage::setLastError(mLastError);
	}
	
	return res;
}


BOOL LLImageJ2C::encode(const LLImageRaw *raw_imagep, F32 encode_time)
{
	return encode(raw_imagep, NULL, encode_time);
}


BOOL LLImageJ2C::encode(const LLImageRaw *raw_imagep, const char* comment_text, F32 encode_time)
{
	LLMemType mt1((LLMemType::EMemType)mMemType);
	resetLastError();
	BOOL res = FALSE;

	// we send data through our plugin interface, then update the this pointer's data to "set" it
	if (mImplKDU)
	{
		// LGPL plugin
		ImageBaseForKDU base(this->getData(), this->getDataSize(), this->getWidth(), this->getHeight(), this->getComponents(), this->getRawDiscardLevel(), this->mRate, this->getMaxBytes());

		// icky const cast
		ImageBaseForKDU raw(const_cast<U8*>(raw_imagep->getData()), raw_imagep->getDataSize(), raw_imagep->getWidth(), raw_imagep->getHeight(), raw_imagep->getComponents());

		if (!raw.getData())
		{
			llwarns << "cannot allocate temporary image info" << llendl;
			res = FALSE; // just in case we change the default
		}
		else
		{
			res = mImplKDU->encodeImpl(&base, &raw, comment_text, encode_time, mReversible);
			
			if ((base.getData() != NULL) &&
				((base.getData() != this->getData()) &&
				//(base.mRawDiscardLevel > -1) &&
				(base.getHeight() > 0) && 
				(base.getWidth() > 0) && 
				(base.getComponents() > 0)))
			{
				this->setMaxBytes(base.mMaxBytes);
				this->mRate = base.mRate;
				this->mRawDiscardLevel = base.mRawDiscardLevel;
				this->setSize(base.getWidth(), base.getHeight(), base.getComponents());
				this->copyData(base.getData(), base.getDataSize()); // calls updateData();
				

				if (!(base.mLastErrorMsg.empty()))
				{
					this->setLastError(base.mLastErrorMsg);
				}
			}
		}
	}
	else
	{
		res = mImpl->encodeImpl(*this, *raw_imagep, comment_text, encode_time, mReversible);
	}

	if (!mLastError.empty())
	{
		LLImage::setLastError(mLastError);
	}
	return res;
}

//static
S32 LLImageJ2C::calcHeaderSizeJ2C()
{
	//return FIRST_PACKET_SIZE; // Hack. just needs to be >= actual header size...
	return 1024; // MC
}

//static
S32 LLImageJ2C::calcDataSizeJ2C(S32 w, S32 h, S32 comp, S32 discard_level, F32 rate)
{
	if (rate <= 0.f) rate = .125f;
	while (discard_level > 0)
	{
		if (w < 1 || h < 1)
			break;
		w >>= 1;
		h >>= 1;
		discard_level--;
	}
	S32 bytes = (S32)((F32)(w*h*comp)*rate);
	bytes = llmax(bytes, calcHeaderSizeJ2C());
	return bytes;
}

S32 LLImageJ2C::calcHeaderSize()
{
	return calcHeaderSizeJ2C();
}


// calcDataSize() returns how many bytes to read 
// to load discard_level (including header and higher discard levels)
S32 LLImageJ2C::calcDataSize(S32 discard_level)
{
	discard_level = llclamp(discard_level, 0, MAX_DISCARD_LEVEL);

	if ( mAreaUsedForDataSizeCalcs != (getHeight() * getWidth()) 
		|| mDataSizes[0] == 0)
	{
		mAreaUsedForDataSizeCalcs = getHeight() * getWidth();
		
		S32 level = MAX_DISCARD_LEVEL;	// Start at the highest discard
		while ( level >= 0 )
		{
			mDataSizes[level] = calcDataSizeJ2C(getWidth(), getHeight(), getComponents(), level, mRate);
			level--;
		}

		/* This is technically a more correct way to calculate the size required
		   for each discard level, since they should include the size needed for
		   lower levels.   Unfortunately, this doesn't work well and will lead to 
		   download stalls.  The true correct way is to parse the header.  This will
		   all go away with http textures at some point.

		// Calculate the size for each discard level.   Lower levels (higher quality)
		// contain the cumulative size of higher levels		
		S32 total_size = calcHeaderSizeJ2C();

		S32 level = MAX_DISCARD_LEVEL;	// Start at the highest discard
		while ( level >= 0 )
		{	// Add in this discard level and all before it
			total_size += calcDataSizeJ2C(getWidth(), getHeight(), getComponents(), level, mRate);
			mDataSizes[level] = total_size;
			level--;
		}
		*/
	}
	return mDataSizes[discard_level];
}

S32 LLImageJ2C::calcDiscardLevelBytes(S32 bytes)
{
	llassert(bytes >= 0);
	S32 discard_level = 0;
	if (bytes == 0)
	{
		return MAX_DISCARD_LEVEL;
	}
	while (1)
	{
		S32 bytes_needed = calcDataSize(discard_level); // virtual
		if (bytes >= bytes_needed - (bytes_needed>>2)) // For J2c, up the res at 75% of the optimal number of bytes
		{
			break;
		}
		discard_level++;
		if (discard_level >= MAX_DISCARD_LEVEL)
		{
			break;
		}
	}
	return discard_level;
}

void LLImageJ2C::setRate(F32 rate)
{
	mRate = rate;
}

void LLImageJ2C::setMaxBytes(S32 max_bytes)
{
	mMaxBytes = max_bytes;
}

void LLImageJ2C::setReversible(const BOOL reversible)
{
 	mReversible = reversible;
}


BOOL LLImageJ2C::loadAndValidate(const std::string &filename)
{
	BOOL res = TRUE;
	
	resetLastError();

	S32 file_size = 0;
	LLAPRFile infile ;
	infile.open(filename, LL_APR_RB, LLAPRFile::global, &file_size);
	apr_file_t* apr_file = infile.getFileHandle() ;
	if (!apr_file)
	{
		setLastError("Unable to open file for reading", filename);
		res = FALSE;
	}
	else if (file_size == 0)
	{
		setLastError("File is empty",filename);
		res = FALSE;
	}
	else
	{
		U8 *data = new U8[file_size];
		apr_size_t bytes_read = file_size;
		apr_status_t s = apr_file_read(apr_file, data, &bytes_read); // modifies bytes_read	
		infile.close() ;

		if (s != APR_SUCCESS || (S32)bytes_read != file_size)
		{
			delete[] data;
			setLastError("Unable to read entire file");
			res = FALSE;
		}
		else
		{
			res = validate(data, file_size);
		}
	}
	
	if (!mLastError.empty())
	{
		LLImage::setLastError(mLastError);
	}
	
	return res;
}


BOOL LLImageJ2C::validate(U8 *data, U32 file_size)
{
	LLMemType mt1((LLMemType::EMemType)mMemType);

	resetLastError();
	
	setData(data, file_size);

	BOOL res = updateData();
	if ( res )
	{
		// Check to make sure that this instance has been initialized with data
		if (!getData() || (getDataSize() < 16))
		{
			setLastError("LLImageJ2C uninitialized");
			res = FALSE;
		}
		else
		{
			if (mImplKDU)
			{
				// InWorldz' kdu should validate when updateData is called. 
				// Avoid calling getMetaData a second time, we don't apply
				// updates that aren't validated
				return true;
			}
			else
			{
				// GPL plugin
				res = mImpl->getMetadata(*this);
			}
		}
	}
	
	if (!mLastError.empty())
	{
		LLImage::setLastError(mLastError);
	}
	return res;
}

void LLImageJ2C::decodeFailed()
{
	mDecoding = FALSE;
}

void LLImageJ2C::updateRawDiscardLevel()
{
	mRawDiscardLevel = mMaxBytes ? calcDiscardLevelBytes(mMaxBytes) : mDiscardLevel;
}

LLImageJ2CImpl::~LLImageJ2CImpl()
{
}
