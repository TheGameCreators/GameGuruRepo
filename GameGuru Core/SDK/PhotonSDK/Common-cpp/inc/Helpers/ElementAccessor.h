/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/JString.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype, bool isSmartPointer>
			class ElementAccessor
			{
			public:
				static const Etype& getElement(Etype* data, unsigned int index);
			};

			template<typename Etype, bool isSmartPointer>
			class ElementAccessor<Etype*, isSmartPointer>
			{
			public:
				static const Etype& getElement(Etype** pData, unsigned int index);
			};

			template<typename Etype>
			class ElementAccessor<Etype, true>
			{
			public:
				static const typename Etype::type& getElement(Etype* spData, unsigned int index);
			};



			template<typename Etype, bool isSmartPointer>
			const Etype& ElementAccessor<Etype, isSmartPointer>::getElement(Etype* data, unsigned int index)
			{
				return data[index];
			}

			template<typename Etype, bool isSmartPointer>
			const Etype& ElementAccessor<Etype*, isSmartPointer>::getElement(Etype** pData, unsigned int index)
			{
				return *(pData[index]);
			}

			template<typename Etype>
			const typename Etype::type& ElementAccessor<Etype, true>::getElement(Etype* spData, unsigned int index)
			{
				return *(spData[index]);
			}
		}
	}
}