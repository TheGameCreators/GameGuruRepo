/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

// clang rtti
#if defined __clang__
#	if !defined __GXX_RTTI && __has_feature(cxx_rtti)
#		define __GXX_RTTI 1
#	endif
#endif

// msvc rtti || gcc rtti || gcc version from a time, when it hasn't offered an rtti define ( == llvm gcc frontend for Marmalade OS X x86 builds!)
#if defined _CPPRTTI || defined __GXX_RTTI || !defined __clang__ && !defined _EG_PS4_PLATFORM && !defined _EG_PSVITA_PLATFORM && defined __GNUC__ && (__GNUC__ < 4 || __GNUC__ == 4 && (__GNUC_MINOR__ < 3 || __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ < 2))
#	include <typeinfo>
#	define TYPENAME(arg) cut((typeid(arg).name()))
// else assume rtti to be disabled
#else
#	define TYPENAME(arg) ("?")
#endif

#include "Common-cpp/inc/JString.h"

#if !defined _EG_WINDOWS_PLATFORM && !defined _EG_MARMALADE_PLATFORM && !defined _EG_ANDROID_PLATFORM && !defined _EG_BLACKBERRY_PLATFORM && !defined _EG_PS3_PLATFORM && !defined _EG_PS4_PLATFORM && !defined _EG_WINDOWSSTORE_PLATFORM && !defined _EG_EMSCRIPTEN_PLATFORM && !defined _EG_XB1_PLATFORM && !defined _EG_PSVITA_PLATFORM && !defined _EG_SWITCH_WINDOWS_PLATFORM
#	include <cxxabi.h>
#elif defined _EG_MARMALADE_PLATFORM && (defined __GNUC__ && !defined I3D_ARCH_MIPS && !defined S3E_ANDROID_X86) || defined _EG_BLACKBERRY_PLATFORM
	namespace abi
	{
		extern "C" char* __cxa_demangle(const char* mangled_name, char* buf, size_t* n, int* status);
	}
#endif

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			namespace TypeName
			{
				template<typename Etype> JString get(void);
				template<typename Etype> JString get(Etype& arg);
				template<typename Etype> JString get(Etype* pArg);
				JString cut(const char* rawTypeName);
			}



			template<typename Etype>
			JString TypeName::get(void)
			{
				return TYPENAME(Etype);
			}

			template<typename Etype>
			JString TypeName::get(Etype& arg)
			{
				return TYPENAME(arg);
			}

			template<typename Etype>
			JString TypeName::get(Etype* pArg)
			{
				return TYPENAME(*pArg);
			}
		}
	}
}