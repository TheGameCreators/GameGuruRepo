/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/JString.h"
#include "Common-cpp/inc/Helpers/ElementAccessor.h"
#include "Common-cpp/inc/Helpers/IsDerivedFrom.h"
#include "Common-cpp/inc/Helpers/SmartPointers/SmartPointerBase.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype>
			class ObjectToStringConverterBase
			{
			public:
				static JString toString(Etype* pData, unsigned int size, bool withTypes=false);
			private:
				typedef ElementAccessor<Etype, IsDerivedFrom<Etype, SmartPointerBase>::Is> EA;
			};



			template<typename Etype>
			JString ObjectToStringConverterBase<Etype>::toString(Etype* pData, unsigned int size, bool withTypes)
			{
				JString retStr = L"[";
				for(unsigned int i=0; i<size; ++i)
				{
					retStr += EA::getElement(pData, i).toString(withTypes);
					if(i < size-1)
						retStr += L", ";
				}
				retStr += L"]";
				return retStr;
			}
		}
	}
}