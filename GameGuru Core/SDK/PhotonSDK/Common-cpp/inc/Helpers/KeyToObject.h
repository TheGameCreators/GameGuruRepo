/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/KeyObject.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			class KeyToObject
			{
			public:
				template<typename Ftype> static Object get(const Ftype& key);
				static const Object& get(const Object& key);
				static Object get(const char* key);
				static Object get(const wchar_t* key);
			};



			template<typename Ftype>
			Object KeyToObject::get(const Ftype& key)
			{
				return KeyObject<Ftype>(key);
			}
		}
	}
}