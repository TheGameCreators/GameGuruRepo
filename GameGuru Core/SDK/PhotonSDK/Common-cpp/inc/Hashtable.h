/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/KeyToObject.h"
#include "Common-cpp/inc/Helpers/ValueToObject.h"
#include "Common-cpp/inc/JVector.h"

namespace ExitGames
{
	namespace Common
	{
		class Hashtable : public Base
		{
		public:
			using ToString::toString;

			Hashtable(void);
			~Hashtable(void);

			Hashtable(const Hashtable& toCopy);
			Hashtable& operator=(const Hashtable& toCopy);

			bool operator==(const Hashtable& toCompare) const;
			bool operator!=(const Hashtable& toCompare) const;
			const Object& operator[](unsigned int index) const;
			Object& operator[](unsigned int index);

			void put(const Hashtable& src);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType& val);
			template<typename FKeyType> void put(const FKeyType& key);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType pVal, typename Common::Helpers::ArrayLengthType<FValueType>::type size);
			template<typename FKeyType, typename FValueType> void put(const FKeyType& key, const FValueType pVal, const short* sizes);

			template<typename FKeyType> const Object* getValue(const FKeyType& key) const;
			unsigned int getSize(void) const;
			const JVector<Object>& getKeys(void) const;
			template<typename FKeyType> void remove(const FKeyType& key);
			template<typename FKeyType> bool contains(const FKeyType& key) const;
			void removeAllElements(void);
			JString& toString(JString& retStr, bool withTypes=false) const;
		private:
			static bool haveSameKey(const Object& one, const Object& two);

			void putImplementation(const Object& key, const Object& val);
			const Object* getValueImplementation(const Object& key) const;
			void removeImplementation(const Object& key);
			bool containsImplementation(const Object& key) const;

			JVector<Object> mKeytable;
			JVector<Object> mValtable;
		};



		/** 
		   Adds a pair of a key and a corresponding value to the instance.
		   @details
		   If the key is already existing, then it's old value will be
		   replaced with the new one.
		   Please have a look at the @link Datatypes table of datatypes\endlink for a list of supported types for keys and values
		   @param key the key to add
		   @param val the value to add
		   @returns
		   nothing. */
		template<typename FKeyType, typename FValueType>
		void Hashtable::put(const FKeyType& key, const FValueType& val)
		{
			typename Helpers::ConfirmAllowedKey<FKeyType>::type forceCompilationToFailForUnsupportedKeyTypes; (void)forceCompilationToFailForUnsupportedKeyTypes;
			typename Helpers::ConfirmAllowed<FValueType>::type forceCompilationToFailForUnsupportedValueTypes; (void)forceCompilationToFailForUnsupportedValueTypes;
			COMPILE_TIME_ASSERT2_TRUE_MSG(!Helpers::ConfirmAllowed<typename Helpers::ConfirmAllowed<FValueType>::type>::dimensions, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_SINGLE_VALUES);
			putImplementation(Helpers::KeyToObject::get(key), Helpers::ValueToObject::get(val));
		}

		/**
		   @overload
		   @details
		   This overload adds an empty object as value for the provided key. */
		template<typename FKeyType>
		void Hashtable::put(const FKeyType& key)
		{
			putImplementation(Helpers::KeyToObject::get(key), Object());
		}

		/**
		   @overload
		   @details
		   This overload accepts singledimensional arrays and NULL-pointers passed for parameter pVal. NULL pointers are only legal input, if size is 0
		   @param key the key to add
		   @param pVal the value array to add
		   @param size the size of the value array */
		template<typename FKeyType, typename FValueType>
		void Hashtable::put(const FKeyType& key, const FValueType pVal, typename Common::Helpers::ArrayLengthType<FValueType>::type size)
		{
			typename Helpers::ConfirmAllowedKey<FKeyType>::type forceCompilationToFailForUnsupportedKeyTypes; (void)forceCompilationToFailForUnsupportedKeyTypes;
			typename Helpers::ConfirmAllowed<FValueType>::type forceCompilationToFailForUnsupportedValueTypes; (void)forceCompilationToFailForUnsupportedValueTypes;
			COMPILE_TIME_ASSERT2_TRUE_MSG(Helpers::ConfirmAllowed<typename Helpers::ConfirmAllowed<FValueType>::type>::dimensions==1, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_1D_ARRAYS);
			putImplementation(Helpers::KeyToObject::get(key), Helpers::ValueToObject::get(pVal, size));
		}

		/**
		   @overload
		   @details
		   This overload accepts multidimensional arrays and NULL-pointers passed for parameter pVal.
		   The array that is passed for parameter pVal has to be a pointer of the correct abstraction level, meaning a normal pointer for
		   a singledimensional array, a doublepointer for a twodimensional array, a triplepointer for a threedimensional array and so on.
		   For pVal NULL pointers are only legal input, if sizes[0] is 0.
		   For sizes NULL is no valid input.
		   @param key the key to add
		   @param pVal the value array to add
		   @param sizes the sizes for every dimension of the value array - the length of this array has to match the dimensions of pVal */
		template<typename FKeyType, typename FValueType>
		void Hashtable::put(const FKeyType& key, const FValueType pVal, const short* const sizes)
		{
			typename Helpers::ConfirmAllowedKey<FKeyType>::type forceCompilationToFailForUnsupportedKeyTypes; (void)forceCompilationToFailForUnsupportedKeyTypes;
			typename Helpers::ConfirmAllowed<FValueType>::type forceCompilationToFailForUnsupportedValueTypes; (void)forceCompilationToFailForUnsupportedValueTypes;
			COMPILE_TIME_ASSERT2_TRUE_MSG((bool)Helpers::ConfirmAllowed<typename Helpers::ConfirmAllowed<FValueType>::type>::dimensions, ERROR_THIS_OVERLOAD_IS_ONLY_FOR_FOR_ARRAYS);
			putImplementation(Helpers::KeyToObject::get(key), Helpers::ValueToObject::get(pVal, sizes));
		}


		
		/**
		   Deletes the specified key and the corresponding value, if
		   found in the Hashtable.
		   @param key Pointer to the key of the key/value-pair to remove.
		   @returns
		   nothing.
		   @sa
		   removeAllElements()    */
		template<typename FKeyType>
		void Hashtable::remove(const FKeyType& key)
		{
			removeImplementation(Helpers::KeyToObject::get(key));
		}
		
		/**
		   Checks, whether the Hashtable contains a certain key.
		   @param key Pointer to the key to look up.
		   @returns
		   true if the specified key was found, false otherwise. */
		template<typename FKeyType>
		bool Hashtable::contains(const FKeyType& key) const
		{
			return containsImplementation(Helpers::KeyToObject::get(key));
		}

		/**
		   Returns the corresponding value for a specified key.
		   @param key  Reference to the key to return the corresponding value for.
		   @returns
		   a pointer to the corresponding value if the Hashtable
		   contains the specified key, NULL otherwise.
		   
		   @sa
		   put()                                                               */
		template<typename FKeyType>
		const Object* Hashtable::getValue(const FKeyType& key) const
		{
			return getValueImplementation(Helpers::KeyToObject::get(key));
		}
	}
}