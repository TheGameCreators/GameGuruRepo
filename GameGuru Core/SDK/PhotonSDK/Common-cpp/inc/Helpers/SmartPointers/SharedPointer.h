/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/SmartPointers/SharedPointerBase.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype>
			class SharedPointer : public SharedPointerBase<Etype>
			{
			public:
				SharedPointer(void (*pDeleter)(const Etype*)=MemoryManagement::deallocate);
				SharedPointer(Etype* pData, void (*pDeleter)(const Etype*)=MemoryManagement::deallocate);

				typedef SharedPointerBase<Etype> super;
			};

			template<typename Etype>
			class SharedPointer<Etype[]> : public SharedPointerBase<Etype>
			{
			public:
				SharedPointer(void (*pDeleter)(const Etype*)=MemoryManagement::deallocateArray);
				SharedPointer(Etype* pData, void (*pDeleter)(const Etype*)=MemoryManagement::deallocateArray);

				typedef SharedPointerBase<Etype> super;
			};



			template<typename Etype>
			SharedPointer<Etype>::SharedPointer(void (*pDeleter)(const Etype*)) : SharedPointerBase<Etype>(pDeleter)
			{
			}

			template<typename Etype>
			SharedPointer<Etype>::SharedPointer(Etype* pData, void (*pDeleter)(const Etype*)) : SharedPointerBase<Etype>(pData, pDeleter)
			{
			}



			template<typename Etype>
			SharedPointer<Etype[]>::SharedPointer(void (*pDeleter)(const Etype*)) : SharedPointerBase<Etype>(pDeleter)
			{
			}

			template<typename Etype>
			SharedPointer<Etype[]>::SharedPointer(Etype* pData, void (*pDeleter)(const Etype*)) : SharedPointerBase<Etype>(pData, pDeleter)
			{
			}
		}
	}
}