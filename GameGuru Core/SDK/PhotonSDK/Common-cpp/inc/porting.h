/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/defines.h"

#if defined _EG_WINDOWS_PLATFORM || defined _EG_WINDOWSSTORE_PLATFORM || defined _EG_XB1_PLATFORM
#	include <process.h>
#	include <mbstring.h>
#	if !defined _EG_XB1_PLATFORM
#		include <tchar.h>
#	endif
#	include <winsock2.h>
#	include <windows.h>
#	include <time.h>
#	include <ws2tcpip.h>
#	if defined _EG_WINDOWS_PLATFORM
#		include <wininet.h>
#	endif
#	include <urlmon.h>
#	include <string.h>
#elif defined _EG_UNIX_PLATFORM && !defined _EG_SWITCH_PLATFORM
#	if defined _EG_PSVITA_PLATFORM
#		include <kernel.h>
#	else
#		include <pthread.h>
#	endif
#endif

#if defined _EG_SWITCH_PLATFORM
#	include <chrono>
#	include <nn/time.h>
#endif