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
		template<nByte typeCode> class CustomType;

		template<nByte typeCode> class CustomTypeFactory : public Base
		{
		public:
			using ToString::toString;

			virtual ~CustomTypeFactory(void) {};
			virtual CustomTypeFactory<typeCode>* copyFactory(void) const = 0;
			virtual void destroyFactory(void) = 0;

			virtual CustomType<typeCode>* create(short amount) const = 0;
			virtual CustomType<typeCode>* copy(const CustomType<typeCode>* pToCopy, short amount) const = 0;
			virtual void destroy(const CustomType<typeCode>* pToDestroy) const = 0;
			virtual unsigned int sizeOf(void) const = 0;

			virtual JString& toString(JString& retStr, bool withTypes=false) const;
		};



		/** @file */

		/** @class CustomTypeFactory
		   This is the factory interface class template for the CustomType interface class template and offers an interface to create,
		   copy and delete instances of CustomType subclasses, without the caller needing to know the names of the subclasses.
		   @details
		   For every specialization of the CustomType template, that you subclass, you have to subclass the according specialization
		   (meaning the one for the same typecode) of this class. Please refer to class SampleCustomTypeFactory in demo_typeSupport for an
		   example implementation.
		   @remarks
		   You normally won't have to call functions from this class yourself, but the library does this for you.
		   @sa CustomType, CustomTypeBase */

		/** @fn CustomTypeFactory::~CustomTypeFactory()
		   Destructor. */

		/** @fn CustomTypeFactory::copyFactory()
		    This function shall return a pointer to a freshly allocated copy of the instance, on which it has been called.
			@returns a pointer to a copy of the instance*/

		/** @fn CustomTypeFactory::destroyFactory()
		    This function shall deallocate the instance, on which it has been called on. */

		/** @fn CustomTypeFactory::create()
		    This function shall allocate an array of the class, for which the template parameter specialization has been registered.
			@param amount the amount of elements to allocate
			@returns a pointer to the created array of CustomTypes*/

		/** @fn CustomTypeFactory::copy()
		    This function shall return a pointer to a freshly allocated copy of the passed array.
			@param pToCopy a pointer to the original array, which should be copied
			@param amount the amount of elements of the array, pointed to by pToCopy
			@returns the created copy of the array*/

		/** @fn CustomTypeFactory::destroy()
		    This function shall deallocate the array, to which the passed pointer points.
			@param pToDestroy a pointer to an array, which has previously been allocated with create() or copy()*/

		/** @fn CustomTypeFactory::sizeOf()
		    @returns the size of a single ínstance as determined by calling the sizeof()-operator, for the class, which is fabricated by this specialization of the factory  */

		/**
		     */
		template<nByte typeCode>
		JString& CustomTypeFactory<typeCode>::toString(JString& retStr, bool /*withTypes*/) const
		{
			return retStr;
		}
	}
}