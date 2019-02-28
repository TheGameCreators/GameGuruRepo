/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/defines.h"

namespace ExitGames
{
	namespace Common
	{
		namespace MemoryManagement
		{
			namespace Internal
			{
				struct RawStorage
				{
					RawStorage* mpNext;
#ifdef _EG_EMSCRIPTEN_PLATFORM
					nByte mPadding[4]; // Emscripten requires arrays of objects that have member variables of type double to be 8 byte aligned, which when storing such arrays in memory that was allocated by MemoryPool is only the case when we add pading bytes to RawStorage
#endif
				};
			}
		}
	}
}