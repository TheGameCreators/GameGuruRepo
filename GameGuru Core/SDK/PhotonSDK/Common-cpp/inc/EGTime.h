/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Base.h"

namespace ExitGames
{
	namespace Common
	{
		class EGTime : public Base
		{
		public:
			using ToString::toString;

			EGTime(int time);
			~EGTime(void);

			EGTime(const EGTime& toCopy);
			EGTime& operator=(const EGTime& toCopy);

			EGTime& operator=(const int &time);
			const EGTime& operator+=(const EGTime &time);
			const EGTime& operator-=(const EGTime &time);
			EGTime operator+(const EGTime &time);
			EGTime operator-(const EGTime &time);

			bool operator<(const EGTime &time) const;
			bool operator>(const EGTime &time) const;
			bool operator<=(const EGTime &time) const;
			bool operator>=(const EGTime &time) const;
			bool operator==(const EGTime &time) const;
			bool operator!=(const EGTime &time) const;

			bool overflowed(const EGTime &time) const;
			JString& toString(JString& retStr, bool withTypes=false) const;
		private:
			static const unsigned int TIME_OVERFLOW = 86400000u;

			int t;
		};
	}
}