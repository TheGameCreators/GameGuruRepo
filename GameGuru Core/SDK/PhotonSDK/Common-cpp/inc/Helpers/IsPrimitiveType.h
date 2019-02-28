/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype> struct IsPrimitiveType{static const bool is = false;};
			template<> struct IsPrimitiveType<char>{static const bool is = true;};
			template<> struct IsPrimitiveType<signed char>{static const bool is = true;};
			template<> struct IsPrimitiveType<unsigned char>{static const bool is = true;};
			template<> struct IsPrimitiveType<short>{static const bool is = true;};
			template<> struct IsPrimitiveType<unsigned short>{static const bool is = true;};
			template<> struct IsPrimitiveType<int>{static const bool is = true;};
			template<> struct IsPrimitiveType<unsigned int>{static const bool is = true;};
			template<> struct IsPrimitiveType<long>{static const bool is = true;};
			template<> struct IsPrimitiveType<unsigned long>{static const bool is = true;};
			template<> struct IsPrimitiveType<long long>{static const bool is = true;};
			template<> struct IsPrimitiveType<unsigned long long>{static const bool is = true;};
			template<> struct IsPrimitiveType<float>{static const bool is = true;};
			template<> struct IsPrimitiveType<double>{static const bool is = true;};
			template<> struct IsPrimitiveType<long double>{static const bool is = true;};
			template<> struct IsPrimitiveType<bool>{static const bool is = true;};
			template<> struct IsPrimitiveType<wchar_t>{static const bool is = true;};

			/** @file */
		}
	}
}