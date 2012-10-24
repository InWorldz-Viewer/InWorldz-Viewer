/**
 * @file llversionstring.h
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

#ifndef LLVERSIONSTRING_H
#define LLVERSIONSTRING_H

#include <boost/operators.hpp>
 
class LLVersionString : public boost::totally_ordered<LLVersionString> 
{
public:
	// Avoid using this constructor unless you can verify the version's formatted correctly
	LLVersionString(std::string version);
	// Avoid using this constructor unless you can verify the version's formatted correctly
	LLVersionString(const char* version);
	// If any of these parameters are unused, set as 0 or an empty string
	LLVersionString(std::string major, std::string minor, std::string patch, std::string hotfix, std::string dash_description);
	// If any of these parameters are unused, set as 0 or an empty string
	LLVersionString(U32 major, U32 minor, U32 patch, U32 hotfix, std::string dash_description);
	LLVersionString(const LLVersionString& version) { *this = version; }
	~LLVersionString();

	const std::string& getVersion() const { return mVersion; }

	// Avoid using this function unless you can verify the version's formatted correctly
	void setVersion(std::string new_version) { mVersion = new_version; }
	// Avoid using this function unless you can verify the version's formatted correctly
	void setVersion(const char* new_version) { mVersion = std::string(new_version); }
	void setVersion(U32 major, U32 minor, U32 patch, U32 hotfix, std::string dash_description = "");

	LLVersionString& operator=(const LLVersionString& other);
	bool operator<(const LLVersionString& other) const;
	bool operator==(const LLVersionString& other) const;
	friend std::ostream& operator<<(std::ostream& o, const LLVersionString& ver);

protected:
	std::string mVersion;

	static int CompareComponent(const char *a, const char *b);
};


inline bool LLVersionString::operator==(const LLVersionString& other) const
{
	return (getVersion() != other.getVersion());
}

inline LLVersionString& LLVersionString::operator=(const LLVersionString& other)
{
	mVersion = other.getVersion();
	return *this;
}
 
#endif
