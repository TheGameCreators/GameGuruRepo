/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/AuthenticationValues.h"

/** @file LoadBalancing-cpp/inc/AuthenticationValues.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		/** @class AuthenticationValues
		   Container for user authentication in Photon.
		   @remarks
		   On Photon, user authentication is optional but can be useful in many cases.
		   If you want to use Client::opFindFriends(), a unique ID per user is very practical.

		   There are basically three options for user authentication: None at all, the client sets some UserId
		   or you can use some account web-service to authenticate a user (and set the UserId server-side).

		   Custom Authentication lets you verify end-users by some kind of login or token. It sends those
		   values to Photon which will verify them before granting access or disconnecting the client.

		   If you don't set a user ID through setUserID() for the AuthenticationValues instance that you pass to Client::connect(), then Photon generates a unique user ID (which fulfills the requirements of a GUID) for you, which can be retrieved through
		   Client::getUserID(), once the Client instance has notified Listener::connectReturn() about having successfully finished the connection procedure.
		   Once you have set a user ID, the Client instance caches it until you either override it or until the end of the lifetime of the Client instance.
		   
		   To be able to rejoin a room and to be recognized there as the previous user it is critical to continue to use the same user ID.
		   
		   Therefor you should store the user ID in permanent storage and set it to that same stored value whenever you want to connect as that user, even if you let Photon initially generate that ID.
		   Otherwise Photon would generate a new user ID for you whenever you construct a new Client instance (i.e. when the user restarts your app). */

		/**
		   Constructor. */
		AuthenticationValues::AuthenticationValues(void)
			: mType(CustomAuthenticationType::NONE)
		{
		}

		/**
		   @returns the type of the "Custom Authentication"service that will be used.
		   @sa
		   setType() */
		nByte AuthenticationValues::getType(void) const
		{
			return mType;
		}

		/**
		   Sets the type of the "Custom Authentication" service that will be used. The initial value before the first call to this function is CustomAuthenticationType::NONE.
		   @note
		   Any custom authentication type aside from CustomAuthenticationType::NONE requires you to set up an authentication service of matching type for your appID at https://www.photonengine.com/dashboard
		   @param type needs to match one of the values in CustomAuthenticationType
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getType(), CustomAuthenticationType */
		AuthenticationValues& AuthenticationValues::setType(nByte type)
		{
			mType = type;
			return *this;
		}
		
		/**
		   @returns the HTTP GET parameters that will be forwarded to the authentication service.
		   @sa
		   setParameters(), setParametersWithUsernameAndToken(), getData(), setData() */
		const JString& AuthenticationValues::getParameters(void) const
		{
			return mParameters;
		}
		
		/**
		   Sets the HTTP GET parameters that will be forwarded to the authentication service to the provided parameters.
		   @details
		   The provided parameter string must contain any (HTTP GET) parameters that are expected by the used authentication service.
		   @remarks
		   Standard HTTP GET parameters are used here and passed on to the authentication service that's defined for the provided authentication type in the Photon Cloud Dashboard.
		   @param parameters needs to be a valid HTTP GET string (i.e. param1=value1&param2=value2&param3=value3)
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getParameters(), setParametersWithUsernameAndToken(), getData(), setData()  */
		AuthenticationValues& AuthenticationValues::setParameters(const JString& parameters)
		{
			mParameters = parameters;
			return *this;
		}
		
		/**
		   Sets the HTTP GET parameters that will be forwarded to the authentication service to the provided username and token.
		   @details
		   Calling this function is equivalent to setParameters(Common::JString(L"username=") + username + "&token=" + token).
		   @param username the username of the user that should be authenticated
		   @param token the authentication token needed by the authentication service to verify the user
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getParameters(), setParameters(), getData(), setData()  */
		AuthenticationValues& AuthenticationValues::setParametersWithUsernameAndToken(const JString& username, const JString&  token)
		{
			mParameters = L"username=" + username + "&token=" + token;
			return *this;
		}
		
		/**
		   @returns the HTTP POST data that will be forwarded to the authentication service.
		   @sa
		   getParameters(), setParameters(), setParametersWithUsernameAndToken(), setData() */
		const JVector<nByte>& AuthenticationValues::getData(void) const
		{
			return mData;
		}
		
		/**
		   Sets the HTTP POST data, that will be forwarded to the authentication service, to the provided data.
		   @details
		   The provided data needs to match what is expected by the used authentication service.
		   @remarks
		   The provided data is passed on to the authentication service that's defined for the provided authentication type in the Photon Cloud Dashboard.
		   @param data the data to be used in the body of the POST request.
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getParameters(), setParameters(), setParametersWithUsernameAndToken(), getData() */
		AuthenticationValues& AuthenticationValues::setData(const JVector<nByte>& data)
		{
			mData = data;
			return *this;
		}
		
		/**
		   After initial authentication, Photon provides a secret for this client / user, which is subsequently used as (cached) validation internally.
		   @remarks
		   This is publicly read-accessible only for debugging purposes. For normal operations it is entirely unnecessary for the app code to ever access this value.
		   @returns the cached secret */
		const JString& AuthenticationValues::getSecret(void) const
		{
			return mSecret;
		}

		AuthenticationValues& AuthenticationValues::setSecret(const JString& secret)
		{
			mSecret = secret;
			return *this;
		}
		
		/**
		   @returns the unique user ID
		   @sa
		   setUserID() */
		const JString& AuthenticationValues::getUserID(void) const
		{
			return mUserID;
		}

		/**
		   Sets the unique user ID.
		   @param userID a string that needs to be unique per user among all users of your app
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getUserID() */
		AuthenticationValues& AuthenticationValues::setUserID(const JString& userID)
		{
			mUserID = userID;
			return *this;
		}
		
		JString& AuthenticationValues::toString(JString& retStr, bool withTypes) const
		{
			return retStr += JString(L"{") + static_cast<int>(getType()) + L" " + getParameters().toString() + L" " + getData().toString(withTypes) + L" " + getSecret().toString() + L" " + getUserID().toString() + L"}";
		}
	}
}