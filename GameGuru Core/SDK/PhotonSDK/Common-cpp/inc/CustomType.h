/* Exit Games Common - C++ Client Lib
* Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
* http://www.photonengine.com
* mailto:developer@photonengine.com
*/

#pragma once

#include "Common-cpp/inc/CustomTypeBase.h"
#include "Common-cpp/inc/CustomTypeFactory.h"

namespace ExitGames
{
	namespace Common
	{
		template<nByte typeCode>
		class CustomType : public CustomTypeBase
		{
		public:
			static void constructClass(const CustomTypeFactory<typeCode>& factory);
			static void deconstructClass(void);

			static const nByte TypeCode = typeCode;
		protected:
			CustomType(void);
			CustomType(const CustomType& toCopy);
			~CustomType(void);
		private:
			typedef CustomTypeBase super;

			static void* ccalloc(short count, nByte customTypeCode);
			static void cfree(const void* pData, nByte customTypeCode);
			static unsigned int csizeOf(nByte customTypeCode);

			static CustomTypeFactory<typeCode>* mFactory;
		};



		/** @file */

		template<nByte typeCode>
		CustomTypeFactory<typeCode>* CustomType<typeCode>::mFactory = NULL;

		/** @class CustomType
		   The CustomType class provides you with an interface, to add support for additional data-types.
		   @details
		   We only support a certain subset of @link Datatypes\endlink out of the box.
		   If you need support for further datatypes, then you can easily add this support yourself by subclassing this
		   class template and providing suitable implementations for the pure virtual functions, which are inherited from CustomTypeBase,
		   in your subclass.
		   You should only subclass every typecode once. typeCode 0 should be considered as reserved.
		   So your first custom type would inherit from CustomType<1>, the second one from CustomType<2> and so on.
		   Subclassing the same typecode multiple times will lead into undefined behavior as the typecode will determine the class
		   as instance of which serialized data should be interpreted.
		   @remarks
		   When you are subclassing a specialization of CustomType, then you will also have to subclass the according specialization of
		   CustomTypeFactory (the one for the same typecode).
		   @sa CustomTypeBase, CustomTypeFactory
		   */

		/** @var CustomType::TypeCode
		    Check this public constant to find out the typecode of a custom type at runtime.
			This should normally not be of any interest.*/
		
		/**
		   This static function initializes the class and has to be called once before any instance of a concrete subclass gets created.
		   It registers the typecode and sets the factory-class to a copy of the passed parameter.
		   @sa deconstructClass()
		   @param factory an instance of the factory class, which will be used to create instances of this class*/
		template<nByte typeCode>
		void CustomType<typeCode>::constructClass(const CustomTypeFactory<typeCode>& factory)
		{
			super::constructClass(typeCode, &ccalloc, &cfree, &csizeOf);
			mFactory = factory.copyFactory();
		}
		
		/**
		   This static function cleans up the class and has to be called once after the last instance of a concrete subclass has been
		   deallocated. It will then deallocate the shared instance of the according CustomTypeFactory subclass.
		   @sa constructClass() */
		template<nByte typeCode>
		void CustomType<typeCode>::deconstructClass(void)
		{
			super::deconstructClass(typeCode);
			mFactory->destroyFactory(); // not just deleting it, as subclasses could potentially implement copyFactory() by using something else then new
		}

		template<nByte typeCode>
		CustomType<typeCode>::CustomType(void)
		{
		}

		template<nByte typeCode>
		CustomType<typeCode>::CustomType(const CustomType& /*toCopy*/)
		{
		}

		template<nByte typeCode>
		CustomType<typeCode>::~CustomType(void)
		{
		}

		template<nByte typeCode>
		void* CustomType<typeCode>::ccalloc(short count, nByte /*customTypeCode*/)
		{
			return mFactory->create(count);
		}

		template<nByte typeCode>
		void CustomType<typeCode>::cfree(const void* pData, nByte /*customTypeCode*/)
		{
			mFactory->destroy(static_cast<const CustomType<typeCode>*>(pData));
		}

		template<nByte typeCode>
		unsigned int CustomType<typeCode>::csizeOf(nByte /*customTypeCode*/)
		{
			return mFactory->sizeOf();
		}
	}
}