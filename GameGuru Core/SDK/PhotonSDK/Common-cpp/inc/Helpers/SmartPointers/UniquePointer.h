/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/SmartPointers/UniquePointerBase.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype>
			class UniquePointer : public UniquePointerBase<Etype>
			{
			public:
				UniquePointer(void (*pDeleter)(const Etype*)=MemoryManagement::deallocate);
				UniquePointer(Etype* pData, void (*pDeleter)(const Etype*)=MemoryManagement::deallocate);

				virtual inline UniquePointer& operator=(Etype* pData); // 'inline' prevents a wrong positive for MSVC warning 4505
			private:
				UniquePointer(const UniquePointer<Etype>& toCopy);
				UniquePointer& operator=(const UniquePointer<Etype>& toCopy);

				typedef UniquePointerBase<Etype> super;
			};

			template<typename Etype>
			class UniquePointer<Etype[]> : public UniquePointerBase<Etype>
			{
			public:
				UniquePointer(void (*pDeleter)(const Etype*)=MemoryManagement::deallocateArray);
				UniquePointer(Etype* pData, void (*pDeleter)(const Etype*)=MemoryManagement::deallocateArray);

				virtual inline UniquePointer& operator=(Etype* pData); // 'inline' prevents a wrong positive for MSVC warning 4505
			private:
				UniquePointer(const UniquePointer<Etype[]>& toCopy);
				UniquePointer& operator=(const UniquePointer<Etype[]>& toCopy);

				typedef UniquePointerBase<Etype> super;
			};



			template<typename Etype>
			UniquePointer<Etype>::UniquePointer(void (*pDeleter)(const Etype*)) : UniquePointerBase<Etype>(pDeleter)
			{
			}

			template<typename Etype>
			UniquePointer<Etype>::UniquePointer(Etype* pData, void (*pDeleter)(const Etype*)) : UniquePointerBase<Etype>(pData, pDeleter)
			{
			}

			template<typename Etype>
			UniquePointer<Etype>& UniquePointer<Etype>::operator=(Etype* pData)
			{
				super::mpData = pData;
				return *this;
			}



			template<typename Etype>
			UniquePointer<Etype[]>::UniquePointer(void (*pDeleter)(const Etype*)) : UniquePointerBase<Etype>(pDeleter)
			{
			}

			template<typename Etype>
			UniquePointer<Etype[]>::UniquePointer(Etype* pData, void (*pDeleter)(const Etype*)) : UniquePointerBase<Etype>(pData, pDeleter)
			{
			}

			template<typename Etype>
			UniquePointer<Etype[]>& UniquePointer<Etype[]>::operator=(Etype* pData)
			{
				super::mpData = pData;
				return *this;
			}
		}
	}
}