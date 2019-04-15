/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/DeSerializerImplementation.h"

namespace ExitGames
{
	namespace Common
	{
		class DeSerializer : public Base
		{
		public:
			using ToString::toString;

			DeSerializer(const nByte* data, int size);

			bool pop(Object& object);

			JString& toString(JString& retStr, bool withTypes=false) const;
		private:
			Helpers::DeSerializerImplementation mImp;
			int mSize;
		};
	}
}