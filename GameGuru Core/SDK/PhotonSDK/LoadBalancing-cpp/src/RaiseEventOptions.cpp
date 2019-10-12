/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/RaiseEventOptions.h"

/** @file LoadBalancing-cpp/inc/RaiseEventOptions.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		/** @class RaiseEventOptions
		   This class aggregates the various optional parameters that can be passed to Client::opRaiseEvent().
		   @sa Client::opRaiseEvent() */

		/**
		   Constructor: Creates a new instance with the specified parameters.
		   @param channelID see setChannelID() - optional, defaults to 0.
		   @param eventCaching see setEventCaching() - optional, defaults to Lite::EventCache::DO_NOT_CACHE.
		   @param targetPlayers see setTargetPlayers() - optional, defaults to NULL.
		   @param numTargetPlayers see setTargetPlayers() - optional, defaults to 0.
		   @param receiverGroup see setReceiverGroup() - optional, defaults to Lite::ReceiverGroup::OTHERS.
		   @param interestGroup see setInterestGroup() - optional, defaults to 0.
		   @param webFlags see setWebFlags() - optional, defaults to a default-constructed WebFlags instance.
		   @param cacheSliceIndex see setCacheSliceIndex() - optional, defaults to 0. */
		RaiseEventOptions::RaiseEventOptions(nByte channelID, nByte eventCaching, const int* targetPlayers, short numTargetPlayers, nByte receiverGroup, nByte interestGroup, const WebFlags& webFlags, int cacheSliceIndex)
			: mChannelID(channelID)
			, mEventCaching(eventCaching)
			, mTargetPlayers(targetPlayers, numTargetPlayers)
			, mReceiverGroup(receiverGroup)
			, mInterestGroup(interestGroup)
			, mWebFlags(webFlags)
			, mCacheSliceIndex(cacheSliceIndex)
		{
		}
		
		/**
		   Destructor. */
		RaiseEventOptions::~RaiseEventOptions(void)
		{
		}
		
		/**
		   Copy-Constructor: Creates a new instance that is a deep copy of the argument instance.
		   @param toCopy The instance to copy. */
		RaiseEventOptions::RaiseEventOptions(const RaiseEventOptions& toCopy)
		{
			*this = toCopy;
		}
		
		/**
		   operator=.
	   
		   Makes a deep copy of its right operand into its left operand.
	   
		   This overwrites old data in the left operand. */
		RaiseEventOptions& RaiseEventOptions::operator=(const RaiseEventOptions& toCopy)
		{
			mChannelID = toCopy.mChannelID;
			mEventCaching = toCopy.mEventCaching;
			mTargetPlayers = toCopy.mTargetPlayers;
			mReceiverGroup = toCopy.mReceiverGroup;
			mInterestGroup = toCopy.mInterestGroup;
			mWebFlags = toCopy.mWebFlags;
			mCacheSliceIndex = toCopy.mCacheSliceIndex;

			return *this;
		}

		/**
		   @returns the currently set channel ID
		   @sa
		   setChannelID() */
		nByte RaiseEventOptions::getChannelID(void) const
		{
			return mChannelID;
		}

		/**
		   Sets the channel ID.
		   @details
		   Please see @link FragmentationAndChannels Fragmentation and Channels\endlink for further information.
		   @param channelID the ID of the channel on which to send the message. Needs to be in the range from 0 to Client::getChannelCountUserChannels()
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getChannelID() */
		RaiseEventOptions& RaiseEventOptions::setChannelID(nByte channelID)
		{
			mChannelID = channelID;
			return *this;
		}
		
		/**
		   @returns the currently set event caching option
		   @sa
		   setEventCaching() */
		nByte RaiseEventOptions::getEventCaching(void) const
		{
			return mEventCaching;
		}
		
		/**
		   Sets the event caching option.
		   @details
		   This option defines if the server should simply send the event, put it in the cache, remove events that are like this one or if the Cache Slice should be modified.
		   Leave this to the default value of DO_NOT_CACHE to not use the EventCache at all.

		   For a more in-depth description about event caching please see <a href="https://doc.photonengine.com/en-us/realtime/current/reference/cached-events">Cached Events</a>
		   @remarks
		   When using one of the options SLICE_SET_INDEX, SLICE_PURGE_INDEX or SLICE_PURGE_UP_TO_INDEX, you also need to provide a value for the CacheSliceIndex by a call to setCacheSliceIndex(). All other options except setChannelID() and also all other parameters
		   of Client::opRaiseEvent() get ignored in this case.
		   @note
		   The value that you set for this option gets ignored if any of the following statements is true:
		   @note
		   getReceiverGroup() == ReceiverGroup::MASTER_CLIENT
		   @note
		   getTargetPlayers() != NULL
		   @note
		   getInterestGroup() != 0
		   @param eventCaching needs to be one of the values from Lite::EventCache
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getEventCaching(), <a href="https://doc.photonengine.com/en-us/realtime/current/reference/cached-events">Cached Events</a> */
		RaiseEventOptions& RaiseEventOptions::setEventCaching(nByte eventCaching)
		{
			mEventCaching = eventCaching;
			return *this;
		}
		
		/**
		   @returns the currently set array of target players
		   @sa
		   getNumTargetPlayers(), setTargetPlayers() */
		const int* RaiseEventOptions::getTargetPlayers(void) const
		{
			return mTargetPlayers.getCArray();
		}

		/**
		   @returns the number of elements in the array that is returned by getTargetPlayers()
		   @sa
		   getTargetPlayers(), setTargetPlayers() */
		short RaiseEventOptions::getNumTargetPlayers(void) const
		{
			return mTargetPlayers.getSize();
		}

		/**
		   Sets the target players.
		   @details
		   Set this to the Player numbers of the clients, which should receive the event. The default value when not setting anything is NULL and equivalent to an array that consists of the player numbers of all clients inside the room except for the sending client
		   itself.
		   Player Numbers that do not correspond to any active player inside the room will get ignored by the server. 
		   @note
		   If you set this option to anything else than NULL, then any value that might have been passed for setEventCaching() will be ignored.
		   @note
		   The options setTargetPlayers(), setInterestGroup() and setReceiverGroup() provide alternative ways of specifying the receivers of an event and can not be combined with each other.
		   @note
		   If getTargetPlayers() evaluates to !NULL, then the value for the target players gets used and the values for the other 2 options get ignored.
		   @note
		   Otherwise, if getInterestGroup() evaluates to !0, then the value for the interest group gets used and the value for the receiver group gets ignored.
		   @note
		   Else the  value for the receiver group gets used.
		   @param targetPlayers either NULL (to reset the value of the option to the default) or an array of integer values that correspond to the player numbers of the intended receivers
		   @param numTargetPlayers the element count of the array that is passed for targetPlayers
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getTargetPlayers(), getNumTargetPlayers() */
		RaiseEventOptions& RaiseEventOptions::setTargetPlayers(const int* targetPlayers, short numTargetPlayers)
		{
			mTargetPlayers = JVector<int>(targetPlayers, numTargetPlayers);
			return *this;
		}
		
		/**
		   @returns the currently set receiver group
		   @sa
		   setReceiverGroup() */
		nByte RaiseEventOptions::getReceiverGroup(void) const
		{
			return mReceiverGroup;
		}

		/**
		   Sets the receiver group.
		   @details
		   Set this to one of the values from Lite::ReceiverGroup. The default value when not setting anything is Lite::ReceiverGroup::OTHERS.
		   @note
		   If you set this option to Lite::ReceiverGroup::MASTER_CLIENT, then any value that might have been passed for setEventCaching() will be ignored.
		   @note
		   The options setTargetPlayers(), setInterestGroup() and setReceiverGroup() provide alternative ways of specifying the receivers of an event and can not be combined with each other.
		   @note
		   If getTargetPlayers() evaluates to !NULL, then the value for the target players gets used and the values for the other 2 options get ignored.
		   @note
		   Otherwise, if getInterestGroup() evaluates to !0, then the value for the interest group gets used and the value for the receiver group gets ignored.
		   @note
		   Else the  value for the receiver group gets used.
		   @param receiverGroup needs to be one of the values from Lite::ReceiverGroup
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getReceiverGroup() */
		RaiseEventOptions& RaiseEventOptions::setReceiverGroup(nByte receiverGroup)
		{
			mReceiverGroup = receiverGroup;
			return *this;
		}
		
		/**
		   @returns the currently set interest group
		   @sa
		   setInterestGroup() */
		nByte RaiseEventOptions::getInterestGroup(void) const
		{
			return mInterestGroup;
		}
		
		/**
		   Sets the interest group.
		   @details
		   Set this to a value between 0 and 255. The default value when not setting anything is 0. More information about interest groups can be found at <a href="https://doc.photonengine.com/en-us/realtime/current/reference/interestgroups">Interest Groups</a>.
		   @note
		   If you set this option to anything else than 0, then any value that might have been passed for setEventCaching() will be ignored.
		   @note
		   The options setTargetPlayers(), setInterestGroup() and setReceiverGroup() provide alternative ways of specifying the receivers of an event and can not be combined with each other.
		   @note
		   If getTargetPlayers() evaluates to !NULL, then the value for the target players gets used and the values for the other 2 options get ignored.
		   @note
		   Otherwise, if getInterestGroup() evaluates to !0, then the value for the interest group gets used and the value for the receiver group gets ignored.
		   @note
		   Else the  value for the receiver group gets used.
		   @param interestGroup the number of the interest group to which the event should be sent
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getInterestGroup(), <a href="https://doc.photonengine.com/en-us/realtime/current/reference/interestgroups">Interest Groups</a> */
		RaiseEventOptions& RaiseEventOptions::setInterestGroup(nByte interestGroup)
		{
			mInterestGroup = interestGroup;
			return *this;
		}
		
		/**
		   @returns the currently set web flags options
		   @sa
		   setWebFlags() */
		const WebFlags& RaiseEventOptions::getWebFlags(void) const
		{
			return mWebFlags;
		}
		
		/**
		   Sets the web flags options.
		   @details
		   For more information see class WebFlags.
		   @param webFlags an instance of class WebFlags
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getWebFlags(), WebFlags */
		RaiseEventOptions& RaiseEventOptions::setWebFlags(const WebFlags& webFlags)
		{
			mWebFlags = webFlags;
			return *this;
		}
		
		/**
		   @returns the currently set cache slice index
		   @sa
		   setCacheSliceIndex() */
		int RaiseEventOptions::getCacheSliceIndex(void) const 
		{
			return mCacheSliceIndex;
		}
		
		/**
		   Sets the index of the cache slice that should be used in conjunction with the value that you have passed to setEventCaching().
		   @details
		   When you pass one of the options SLICE_SET_INDEX, SLICE_PURGE_INDEX or SLICE_PURGE_UP_TO_INDEX to setEventCaching(), then you also need to provide the cache slice index for that option to setCacheSliceIndex().
		   @param cacheSliceIndex the index of the cache slice to which the event should be added
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getCacheSliceIndex(), setEventCaching(), <a href="https://doc.photonengine.com/en-us/realtime/current/reference/cached-events">Cached Events</a> */
		RaiseEventOptions& RaiseEventOptions::setCacheSliceIndex(int cacheSliceIndex)
		{
			mCacheSliceIndex = cacheSliceIndex;
			return *this;
		}

		JString& RaiseEventOptions::toString(JString& retStr, bool withTypes) const
		{
			return retStr += JString(L"{channelID=") + getChannelID() + L", eventCaching=" + getEventCaching() + L", targetPlayers=" + JVector<int>(getTargetPlayers(), static_cast<unsigned int>(getNumTargetPlayers())).toString() + L", receiverGroup=" + getReceiverGroup() + L", interestGroup" + getInterestGroup()+ L", webFlags=" + getWebFlags().toString(withTypes) + L", cacheSliceIndex" + getCacheSliceIndex() + L"}";
		}
	}
}