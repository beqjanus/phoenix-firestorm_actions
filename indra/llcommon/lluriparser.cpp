/** 
 * @file lluriparser.cpp
 * @author Protey
 * @date 2014-10-07
 * @brief Implementation of the LLUriParser class.
 *
 * $LicenseInfo:firstyear=2014&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2014, Linden Research, Inc.
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

#include "linden_common.h"
#include "lluriparser.h"

LLUriParser::LLUriParser(const std::string& u) : mTmpScheme(false), mRes(0)
{
	mState.uri = &mUri;

	if (u.find("://") == std::string::npos)
	{
		mNormalizedUri = "http://";
		mTmpScheme = true;
	}

	mNormalizedUri += u.c_str();

	mRes = parse();
}

LLUriParser::~LLUriParser()
{
	uriFreeUriMembersA(&mUri);
}

S32 LLUriParser::parse()
{
	mRes = uriParseUriA(&mState, mNormalizedUri.c_str());
	return mRes;
}

const char * LLUriParser::scheme() const
{
	return mScheme.c_str();
}

void LLUriParser::sheme(const std::string& s)
{
	mTmpScheme = !s.size();
	mScheme = s;
}

const char * LLUriParser::port() const
{
	return mPort.c_str();
}

void LLUriParser::port(const std::string& s)
{
	mPort = s;
}

const char * LLUriParser::host() const
{
	return mHost.c_str();
}

void LLUriParser::host(const std::string& s)
{
	mHost = s;
}

const char * LLUriParser::path() const
{
	return mPath.c_str();
}

void LLUriParser::path(const std::string& s)
{
	mPath = s;
}

const char * LLUriParser::query() const
{
	return mQuery.c_str();
}

void LLUriParser::query(const std::string& s)
{
	mQuery = s;
}

const char * LLUriParser::fragment() const
{
	return mFragment.c_str();
}

void LLUriParser::fragment(const std::string& s)
{
	mFragment = s;
}

void LLUriParser::textRangeToString(UriTextRangeA& textRange, std::string& str)
{
	// <FS> Fix for pointer arithmetics by Drake Arconis
	//S32 len = textRange.afterLast - textRange.first;
	//if (len)
	//{
	//	str = textRange.first;
	//	str = str.substr(0, len);
	//}

	if (textRange.first != NULL && textRange.afterLast != NULL && !(textRange.first >= textRange.afterLast))
	{
		const ptrdiff_t len = textRange.afterLast - textRange.first;
		if (len)
		{
			str.assign(textRange.first, uintptr_t(len));
			return;
		}
	}

	str = LLStringUtil::null;
	// </FS>
}

void LLUriParser::extractParts()
{
	if (mTmpScheme)
	{
		mScheme.clear();
	}
	else
	{
		textRangeToString(mUri.scheme, mScheme);
	}
	
	textRangeToString(mUri.hostText, mHost);
	textRangeToString(mUri.portText, mPort);
	textRangeToString(mUri.query, mQuery);
	textRangeToString(mUri.fragment, mFragment);

	UriPathSegmentA * pathHead = mUri.pathHead;
	while (pathHead)
	{
		std::string partOfPath;
		textRangeToString(pathHead->text, partOfPath);

		mPath += '/';
		mPath += partOfPath;

		pathHead = pathHead->next;
	}
}

S32 LLUriParser::normalize()
{
	if (!mRes)
	{
		mRes = uriNormalizeSyntaxExA(&mUri, URI_NORMALIZE_SCHEME | URI_NORMALIZE_HOST);

		if (!mRes)
		{
			S32 chars_required;
			mRes = uriToStringCharsRequiredA(&mUri, &chars_required);

			if (!mRes)
			{
				chars_required++;
				std::vector<char> label_buf(chars_required);
				mRes = uriToStringA(&label_buf[0], &mUri, chars_required, NULL);

				if (!mRes)
				{
					mNormalizedUri = &label_buf[mTmpScheme ? 7 : 0];
				}
			}
		}
	}

	return mRes;
}

void LLUriParser::glue(std::string& uri) const
{
	if (mScheme.size())
	{
		uri = mScheme;
		uri += "://";
	}

	uri += mHost;

	if (mPort.size())
	{
		uri += ':';
		uri += mPort;
	}

	uri += mPath;

	if (mQuery.size())
	{
		uri += '?';
		uri += mQuery;

		if (mFragment.size())
		{
			uri += '#';
			uri += mFragment;
		}
	}
}

bool LLUriParser::test() const
{
	std::string uri;
	glue(uri);

	return uri == mNormalizedUri;
}

const char * LLUriParser::normalizedUri() const
{
	return mNormalizedUri.c_str();
}
