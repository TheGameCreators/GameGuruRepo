/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/SmartPointers/SmartPointerBase.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype>
			class SmartPointerInterface : public SmartPointerBase
			{
			public:
				typedef Etype type;

				virtual Etype* operator->(void);
				virtual const Etype* operator->(void) const;
				virtual Etype& operator*(void);
				virtual const Etype& operator*(void) const;
				virtual operator Etype*(void);
				virtual operator const Etype*(void) const;
			protected:
				SmartPointerInterface(void (*pDeleter)(const Etype*));
				SmartPointerInterface(Etype* pData, void (*pDeleter)(const Etype*));
				virtual ~SmartPointerInterface(void) = 0;

				SmartPointerInterface(const SmartPointerInterface<Etype>& toCopy);

				Etype* mpData;
				void (*mpDeleter)(const Etype*);
			private:
				SmartPointerInterface& operator=(const SmartPointerInterface<Etype>& toCopy);
			};



			template<typename Etype>
			SmartPointerInterface<Etype>::SmartPointerInterface(void (*pDeleter)(const Etype*))
			{
				mpData = NULL;
				mpDeleter = pDeleter;
			}

			template<typename Etype>
			SmartPointerInterface<Etype>::SmartPointerInterface(Etype* pData, void (*pDeleter)(const Etype*))
			{
				mpData = pData;
				mpDeleter = pDeleter;
			}

			template<typename Etype>
			SmartPointerInterface<Etype>::~SmartPointerInterface(void)
			{
			}

			template<typename Etype>
			SmartPointerInterface<Etype>::SmartPointerInterface(const SmartPointerInterface<Etype>& toCopy)
			{
			}

			template<typename Etype>
			Etype* SmartPointerInterface<Etype>::operator->(void)
			{
				return mpData;
			}

			template<typename Etype>
			const Etype* SmartPointerInterface<Etype>::operator->(void) const
			{
				return mpData;
			}

			template<typename Etype>
			Etype& SmartPointerInterface<Etype>::operator*(void)
			{
				return *mpData;
			}

			template<typename Etype>
			const Etype& SmartPointerInterface<Etype>::operator*(void) const
			{
				return *mpData;
			}

			template<typename Etype>
			SmartPointerInterface<Etype>::operator Etype*(void)
			{
				return mpData;
			}

			template<typename Etype>
			SmartPointerInterface<Etype>::operator const Etype*(void) const
			{
				return mpData;
			}
		}
	}
}