/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/WebFlags.h"

/** @file LoadBalancing-cpp/inc/WebFlags.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		/** @class WebFlags
		   Optional flags to be used with RaiseEventOptions::setWebFlags() and with various property setters of the MutablePlayer and the MutableRoom class to control the behavior of forwarded HTTP requests.
		   @details
		   Please see <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks-v1-2">Webhooks v1.2</a> for further information.
		   @sa <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks-v1-2">Webhooks v1.2</a>, RaiseEventOptions::setWebFlags(), MutablePlayer, MutableRoom */
		
		/**
		   Constructor: Creates a new instance with the specified parameters.
		   @param webFlags see setFlags() - optional, defaults to 0. */
		WebFlags::WebFlags(nByte webFlags)
			: mWebFlags(webFlags)
		{
		}

		/**
		   @returns the currently set flags
		   @sa
		   setFlags() */
		nByte WebFlags::getFlags(void) const
		{
			return mWebFlags; 
		}

		/**
		   Sets the values of all flags at once.
		   @details
		   Internally all boolean flags are encoded as bits into a single byte variable. This byte can be retrieved with getFlags() and set with setFlags().
		   @param webFlags
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getFlags() */
		WebFlags& WebFlags::setFlags(nByte webFlags)
		{
			mWebFlags = webFlags;
			return *this;
		}

		/**
		   @returns the currently set value of the httpWebForward flag
		   @sa
		   setHttpForward() */
		bool WebFlags::getHttpForward(void) const
		{
			return !!(mWebFlags&M_HTTP_FORWARD); 
		}

		/**
		   Sets the value of the httpWebForward flag.
		   @param httpWebForward true or false
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getHttpForward() */
		WebFlags& WebFlags::setHttpForward(bool httpWebForward)
		{
			mWebFlags = httpWebForward?mWebFlags|M_HTTP_FORWARD:mWebFlags&M_HTTP_FORWARD;
			return *this;
		}

		/**
		   @returns the currently set value of the sendAuthCookie flag
		   @sa
		   setSendAuthCookie() */
		bool WebFlags::getSendAuthCookie(void) const
		{
			return !!(mWebFlags&M_SEND_AUTH_COOKIE);
		}

		/**
		   Sets the value of the sendAuthCookie flag.
		   @param sendAuthCookie true or false
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getSendAuthCookie() */
		WebFlags& WebFlags::setSendAuthCookie(bool sendAuthCookie)
		{
			mWebFlags = sendAuthCookie?mWebFlags|M_SEND_AUTH_COOKIE:mWebFlags&M_SEND_AUTH_COOKIE;
			return *this;
		}

		/**
		   @returns the currently set value of the sendSync flag
		   @sa
		   setSendSync() */
		bool WebFlags::getSendSync(void) const
		{
			return !!(mWebFlags&M_SEND_SYNC);
		}

		/**
		   Sets the value of the sendSync flag.
		   @param sendSync true or false
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getSendSync() */
		WebFlags& WebFlags::setSendSync(bool sendSync)
		{
			mWebFlags = sendSync?mWebFlags|M_SEND_SYNC:mWebFlags&M_SEND_SYNC;
			return *this;
		}

		/**
		   @returns the currently set value of the sendState flag
		   @sa
		   setSendState() */
		bool WebFlags::getSendState(void) const
		{
			return !!(mWebFlags&M_SEND_STATE);
		}

		/**
		   Sets the value of the sendState flag.
		   @param sendState true or false
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getSendState() */
		WebFlags& WebFlags::setSendState(bool sendState)
		{
			mWebFlags = sendState?mWebFlags|M_SEND_STATE:mWebFlags&M_SEND_STATE;
			return *this;
		}

		JString& WebFlags::toString(JString& retStr, bool withTypes) const
		{
			return retStr += JString() + mWebFlags + L" {" + L"httpForward: " + getHttpForward() + L", SendAuthCookie: " + getSendAuthCookie() + L", sendSync: " + getSendSync() + L", sendState: " + getSendState() + L"}";
		}
	}
}