/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Hashtable.h"

namespace ExitGames
{
	namespace Common
	{
		class DictionaryBase: public Base
		{
		public:
			using ToString::toString;

			virtual ~DictionaryBase(void);

			DictionaryBase(const DictionaryBase& toCopy);
			DictionaryBase& operator=(const DictionaryBase& toCopy);

			bool operator==(const DictionaryBase &toCompare) const;
			bool operator!=(const DictionaryBase &toCompare) const;

			template<typename FKeyType> void remove(const FKeyType& key);
			template<typename FKeyType> bool contains(const FKeyType& key) const;
			void removeAllElements(void);
			JString typeToString(void) const;
			JString& toString(JString& retStr, bool withTypes=false) const;
			const Hashtable& getHashtable(void) const;
			unsigned int getSize(void) const;
			virtual const nByte* getKeyTypes(void) const;
			virtual const nByte* getValueTypes(void) const;
			template<typename FKeyType> const short* getValueSizes(const FKeyType& key) const;
			virtual const unsigned int* getValueDimensions(void) const;
			template<typename FKeyType, typename FValueType> const FValueType* getValue(const FKeyType& key, const FValueType* /*dummyDeducer*/) const;
			template<typename FKeyType> const Object* getValue(const FKeyType& key, const Object* /*dummyDeducer*/) const;
			template<typename FKeyType> JVector<FKeyType> getKeys(const FKeyType* /*dummyDeducer*/) const;
			JVector<Object> getKeys(const Object* /*dummyDeducer*/) const;
		protected:
			DictionaryBase(void);

			virtual DictionaryBase& assign(const DictionaryBase& toCopy);

			void put(const DictionaryBase& src);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType& val);
			template<typename FKeyType> void put(const FKeyType& key);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType pVal, typename Common::Helpers::ArrayLengthType<FValueType>::type size);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType pVal, const short* sizes);
			template<typename FValueType> const FValueType& getElementAt(unsigned int index, const FValueType* /*dummyDeducer*/) const;
			const Object& getElementAt(unsigned int index, const Object* /*dummyDeducer*/) const;
			template<typename FValueType> FValueType& getElementAt(unsigned int index, const FValueType* /*dummyDeducer*/);
			Object& getElementAt(unsigned int index, const Object* /*dummyDeducer*/);
		private:
			DictionaryBase(const nByte* pKeyTypes, const nByte* pValueTypes, const unsigned int* pValueDimensions);
			virtual bool compare(const DictionaryBase &toCompare) const;
			virtual DictionaryBase* copy(short amount) const;

			class TypeInfo : public Base
			{
			public:
				using ToString::toString;
				using ToString::typeToString;

				TypeInfo(const nByte* pKeyTypes, const nByte* pValueTypes, const unsigned int* pValueDimensions);
				TypeInfo(const TypeInfo& toCopy);
				TypeInfo& operator=(const TypeInfo& toCopy);
				~TypeInfo(void);

				JString& toString(JString& retStr, bool withTypes=false) const;
				const nByte* getKeyTypes(void) const;
				const nByte* getValueTypes(void) const;
				const unsigned int* getValueDimensions(void) const;
			private:
				void init(const nByte* pKeyTypes, const nByte* pValueTypes, const unsigned int* pValueDimensions);
				void cleanup(void);
				JString toStringRecursor(bool withTypes, unsigned int recursionDepth=0) const;
				static JString typeToString(nByte typeCode);

				nByte* mpKeyTypes;
				nByte* mpValueTypes;
				unsigned int* mpValueDimensions;
			};

			Hashtable mHashtable;
			TypeInfo* mpTypeInfo;

			friend class Object;
			friend class Helpers::DeSerializerImplementation;
			friend class ::EGUtilsHelper;
		};



		/** @copydoc Hashtable::remove() */
		template<typename FKeyType>
		void DictionaryBase::remove(const FKeyType& key)
		{
			return mHashtable.remove(key);
		}

		/** @copydoc Hashtable::contains() */
		template<typename FKeyType>
		bool DictionaryBase::contains(const FKeyType& key) const
		{
			return getHashtable().contains(key);
		}

		/** @returns Object::getSizes() of the value, that corresponds to the passed key.
		    @param key Reference to the key to return the corresponding value sizes for
			*/
		template<typename FKeyType>
		const short* DictionaryBase::getValueSizes(const FKeyType& key) const
		{
			return getHashtable().getValue(key)->getSizes();
		}

		template<typename FKeyType, typename FValueType>
		void DictionaryBase::put(const FKeyType& key, const FValueType& val)
		{
			mHashtable.put(key, val);
		}

		template<typename FKeyType>
		void DictionaryBase::put(const FKeyType& key)
		{
			mHashtable.put(key);
		}

		template<typename FKeyType, typename FValueType>
		void DictionaryBase::put(const FKeyType& key, FValueType pVal, typename Common::Helpers::ArrayLengthType<FValueType>::type size)
		{
			mHashtable.put(key, pVal, size);
		}

		template<typename FKeyType, typename FValueType>
		void DictionaryBase::put(const FKeyType& key, FValueType pVal, const short* const sizes)
		{
			mHashtable.put(key, pVal, sizes);
		}

		/** @copydoc Hashtable::getValue() */
		template<typename FKeyType, typename FValueType>
		const FValueType* DictionaryBase::getValue(const FKeyType& key, const FValueType* /*dummyDeducer*/) const
		{
			ValueObject<FValueType>* retObject = (ValueObject<FValueType>*)getHashtable().getValue(key);
			return retObject?retObject->getDataAddress():NULL;
		}
		
		/** @copydoc Hashtable::getValue() */
		template<typename FKeyType>
		const Object* DictionaryBase::getValue(const FKeyType& key, const Object* /*dummyDeducer*/) const
		{
			return getHashtable().getValue(key);
		}

		/** @copydoc Hashtable::getKeys() */
		template<typename FKeyType>
		JVector<FKeyType> DictionaryBase::getKeys(const FKeyType* /*dummyDeducer*/) const
		{
			JVector<FKeyType> keys;
			JVector<Object> keyObjs = getHashtable().getKeys();
			for(unsigned int i=0; i<keyObjs.getSize(); ++i)
				keys.addElement(static_cast<KeyObject<FKeyType> >(keyObjs[i]).getDataCopy());
			return keys;
		}

		template<typename FValueType>
		const FValueType& DictionaryBase::getElementAt(unsigned int index, const FValueType* /*dummyDeducer*/) const
		{
			return *((ValueObject<FValueType>&)getHashtable()[index]).getDataAddress();
		}

		template<typename FValueType>
		FValueType& DictionaryBase::getElementAt(unsigned int index, const FValueType* /*dummyDeducer*/)
		{
			return *((ValueObject<FValueType>&)getHashtable()[index]).getDataAddress();
		}
	}
}