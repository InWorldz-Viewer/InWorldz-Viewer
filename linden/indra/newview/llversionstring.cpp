/**
 * @file llversionstring.cpp
 * @brief A version string compare class based on http://codingcastles.blogspot.com/2009/05/comparing-version-numbers.html
 *
 * @cond
 * $LicenseInfo:firstyear=2012&license=viewerlgpl$
 * InWorldz Viewer Source Code
 * Copyright (C) 2012, InWorldz, LLC.
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
 * @endcond
 */

#include "llviewerprecompiledheaders.h"

#include "llversionstring.h"

// For the viewer's internal version, edit indra/llcommon/llversionviewer.h

//
// Helper functions
//

U32 string_to_digit(std::string str)
{
	if (!str.empty())
	{
		LLStringUtil::trim(str);
		char* end_ptr;
		U32 num = (U32)strtol(str.c_str(), &end_ptr, 10);
		if (*end_ptr != '\0')
		{
			llwarns << "Trying to build a version with a bad string: " << str << llendl;
			return 0;
		}
		return num;
	}
	return 0;
}

//
// Constructors
//

LLVersionString::LLVersionString(std::string version)
: mVersion(version)
{
	LLStringUtil::trim(mVersion);
}

LLVersionString::LLVersionString(const char* version)
: mVersion("")
{
	mVersion = std::string(version);
	LLStringUtil::trim(mVersion);
}

LLVersionString::LLVersionString(std::string major, std::string minor, std::string patch, std::string hotfix, std::string dash_description)
: mVersion("")
{
	setVersion(string_to_digit(major), string_to_digit(minor), string_to_digit(patch), string_to_digit(hotfix), dash_description);
}

LLVersionString::LLVersionString(U32 major, U32 minor, U32 patch, U32 hotfix, std::string dash_description)
: mVersion("")
{
	setVersion(major, minor, patch, hotfix, dash_description);
}

LLVersionString::~LLVersionString()
{
}

//
// Methods
//

void LLVersionString::setVersion(U32 major, U32 minor, U32 patch, U32 hotfix, std::string dash_description)
{
	std::stringstream version;
	version << major << "." << minor << "." << patch << "." << hotfix;
	if (!dash_description.empty())
	{
		LLStringUtil::trim(dash_description);
		version << "-" << dash_description;
	}
	mVersion = version.str();
}

 
// Compare two components of a version string.  Return -1, 0, or 1
// if a is less than, equal to, or greater than b, respectively
int LLVersionString::CompareComponent(const char *a, const char *b)
{
	while (*a && *b) 
	{
		while (*a && *b && !isdigit(*a) && !isdigit(*b)) 
		{
			if (*a != *b) 
			{
				if (*a == '-') return -1;
				if (*b == '-') return 1;
				return *a < *b ? -1 : 1;
			}
			a++;
			b++;
		}
		if (*a && *b && (!isdigit(*a) || !isdigit(*b))) 
		{
			if (*a == '-') return -1;
			if (*b == '-') return 1;
			return isdigit(*a) ? -1 : 1;
		}

		char *next_a, *next_b;
		long int num_a = strtol(a, &next_a, 10);
		long int num_b = strtol(b, &next_b, 10);
		if (num_a != num_b) 
		{
			return num_a < num_b ? -1 : 1;
		}
		a = next_a;
		b = next_b;
	}

	if (!*a && !*b) 
	{
		return 0;
	} 
	else if (*a) 
	{
		return *a == '-' ? -1 : 1;
	} 
	else 
	{
		return *b == '-' ? 1 : -1;
	}
}

//
// Operators
//

std::ostream& operator<<(std::ostream& o, const LLVersionString& ver)
{
	o << ver.getVersion();
	return o;
}

bool LLVersionString::operator<(const LLVersionString& other) const
{
	int result = CompareComponent(getVersion().c_str(), other.getVersion().c_str());
	if (result) 
	{
		return -1 == result;
	}
	return false;
}
