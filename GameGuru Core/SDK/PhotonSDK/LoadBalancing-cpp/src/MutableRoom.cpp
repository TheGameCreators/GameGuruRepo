/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/MutableRoom.h"
#include "LoadBalancing-cpp/inc/Internal/PlayerMutableRoomPointerSetter.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Room.h"

/** @file inc/MutableRoom.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Common::MemoryManagement;
		using namespace Internal;

		MutableRoom::MutableRoom(const JString& name, const Hashtable& properties, Client* pClient, const JVector<JString>& propsListedInLobby, int playerTtl, int emptyRoomTtl, bool suppressRoomEvents, const JVector<JString>* pPlugins, bool publishUserID, const JVector<JString>& expectedUsers)
			: super(name, properties)
			, mpLoadBalancingClient(pClient)
			, mIsVisible(true)
			, mPlayers(JVector<Player*>())
			, mMasterClientID(0)
			, mPropsListedInLobby(propsListedInLobby)
			, mLocalPlayerNumber(-1)
			, mPlayerTtl(playerTtl)
			, mEmptyRoomTtl(emptyRoomTtl)
			, mSuppressRoomEvents(suppressRoomEvents)
			, mupPlugins(pPlugins?allocate<JVector<JString> >(*pPlugins):NULL)
			, mPublishUserID(publishUserID)
			, mExpectedUsers(expectedUsers)
		{
			cacheProperties(properties);
		}

		MutableRoom::~MutableRoom(void)
		{
			destroyAllPlayers();
		}

		MutableRoom::MutableRoom(const MutableRoom& toCopy)
			: super(toCopy)
		{
			*this = toCopy;
		}

		MutableRoom& MutableRoom::operator=(const Room& toCopy)
		{
			return assign(toCopy);
		}

		MutableRoom& MutableRoom::operator=(const MutableRoom& toCopy)
		{
			return assign(toCopy);
		}

		bool MutableRoom::getIsMutable(void) const
		{
			return true;
		}

		MutableRoom& MutableRoom::assign(const Room& toCopy)
		{
			super::assign(toCopy);
			const MutableRoom& temp = static_cast<const MutableRoom&>(toCopy);
			if(!temp.getIsMutable())
				return *this;
			mpLoadBalancingClient = temp.mpLoadBalancingClient;
			mIsVisible = temp.mIsVisible;
			mPlayers.removeAllElements();
			for(unsigned int i=0; i<temp.mPlayers.getSize(); ++i)
			{
				mPlayers.addElement(temp.mPlayers[i]->getNumber()==temp.mLocalPlayerNumber?temp.mPlayers[i]:allocate<Player>(*temp.mPlayers[i]));
				PlayerMutableRoomPointerSetter::setMutableRoomPointer(*mPlayers[i], this);
			}
			mMasterClientID = temp.mMasterClientID;
			mPropsListedInLobby = temp.mPropsListedInLobby;
			mLocalPlayerNumber = temp.mLocalPlayerNumber;
			mPlayerTtl = temp.mPlayerTtl;
			mEmptyRoomTtl = temp.mEmptyRoomTtl;
			mSuppressRoomEvents = temp.mSuppressRoomEvents;
			mupPlugins = temp.mupPlugins?allocate<JVector<JString> >(*temp.mupPlugins):NULL;
			mPublishUserID = temp.mPublishUserID;
			mExpectedUsers = temp.mExpectedUsers;
			return *this;
		}

		void MutableRoom::mergeCustomProperties(const Hashtable& customProperties, const Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			Hashtable stripDict = Utils::stripToCustomProperties(customProperties);
			if(!stripDict.getSize())
				return;
			if(!expectedCustomProperties.getSize())
			{
				Hashtable oldDict = mCustomProperties;
				mCustomProperties.put(stripDict);
				mCustomProperties = Utils::stripKeysWithNullValues(mCustomProperties);
				if(mCustomProperties == oldDict)
					return;
			}
			mpLoadBalancingClient->opSetPropertiesOfRoom(stripDict, expectedCustomProperties, webflags);
		}

		void MutableRoom::addCustomProperties(const Hashtable& customProperties, const Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			mergeCustomProperties(Utils::stripKeysWithNullValues(customProperties), expectedCustomProperties, webflags);
		}

		void MutableRoom::cacheProperties(const Hashtable& properties)
		{
			if(properties.contains(Properties::Room::IS_VISIBLE))
				mIsVisible = ValueObject<bool>(properties.getValue(Properties::Room::IS_VISIBLE)).getDataCopy();
			if(properties.contains(Properties::Room::EXPECTED_USERS))
			{
				ValueObject<JString*> obj = ValueObject<JString*>(properties.getValue(Properties::Room::EXPECTED_USERS));
				mExpectedUsers = JVector<JString>(*obj.getDataAddress(), *obj.getSizes());
			}
			if(properties.contains(Properties::Room::MASTER_CLIENT_ID))
				setMasterClientID(ValueObject<int>(properties.getValue(Properties::Room::MASTER_CLIENT_ID)).getDataCopy());
			super::cacheProperties(properties);
		}

		JString MutableRoom::toString(bool withTypes, bool withCustomProperties, bool withPlayers) const
		{
			return mName + L"={" + payloadToString(withTypes, withCustomProperties, withPlayers) + L"}";
		}

		JString MutableRoom::payloadToString(bool withTypes, bool withCustomProperties, bool withPlayers) const
		{
			JString res = JString(L"masterClient: ") + mMasterClientID + L" " + super::payloadToString() + L" visible: " + mIsVisible + L" expectedUsers: " + mExpectedUsers.toString();
			if(mPropsListedInLobby.getSize())
				res += L" propsListedInLobby: " + mPropsListedInLobby.toString(withTypes);
			if(withCustomProperties && mCustomProperties.getSize())
				res += L" props: " + mCustomProperties.toString(withTypes);
			if(withPlayers && mPlayers.getSize())
			{
				res += L" players: ";
				for(unsigned int i=0; i<mPlayers.getSize(); ++i)
					res += mPlayers[i]->toString(withTypes, withCustomProperties) + (i<mPlayers.getSize()-1?L", ":L"");
			}
			return res;
		}

		nByte MutableRoom::getPlayerCount(void) const
		{
			return mPlayers.getSize();
		}

		void MutableRoom::setMaxPlayers(nByte maxPlayers, const WebFlags& webflags)
		{
			if(mMaxPlayers != maxPlayers)
				setRoomProperty(Properties::Room::MAX_PLAYERS, mMaxPlayers=maxPlayers, webflags);
		}

		void MutableRoom::setIsOpen(bool isOpen, const WebFlags& webflags)
		{
			if(mIsOpen != isOpen)
				setRoomProperty(Properties::Room::IS_OPEN, mIsOpen=isOpen, webflags);
		}

		bool MutableRoom::getIsVisible(void) const
		{
			return mIsVisible;
		}

		void MutableRoom::setIsVisible(bool isVisible, const WebFlags& webflags)
		{
			if(mIsVisible != isVisible)
				setRoomProperty(Properties::Room::IS_VISIBLE, mIsVisible=isVisible, webflags);
		}

		template<typename Etype> void MutableRoom::setRoomProperty(nByte key, Etype val, const WebFlags& webflags)
		{
			Hashtable properties;
			properties.put(key, val);
			mpLoadBalancingClient->opSetPropertiesOfRoom(properties, Hashtable(), webflags);
		}

		template<typename Etype> void MutableRoom::setRoomProperty(nByte key, const Etype pValueArray, typename Common::Helpers::ArrayLengthType<Etype>::type arrSize, const WebFlags& webflags)
		{
			Hashtable properties;
			properties.put(key, pValueArray, arrSize);
			mpLoadBalancingClient->opSetPropertiesOfRoom(properties, Hashtable(), webflags);
		}

		template<typename Etype> void MutableRoom::setRoomProperty(nByte key, const Etype pValueArray, const short* pArrSizes, const WebFlags& webflags)
		{
			Hashtable properties;
			properties.put(key, pValueArray, pArrSizes);
			mpLoadBalancingClient->opSetPropertiesOfRoom(properties, Hashtable(), webflags);
		}

		const JVector<Player*>& MutableRoom::getPlayers(void) const
		{
			return mPlayers;
		}
		
		const Player* MutableRoom::getPlayerForNumber(int playerNumber) const
		{
			for(unsigned i=0; i<mPlayers.getSize(); i++)
				if(mPlayers[i]->getNumber() == playerNumber)
					return mPlayers[i];
			return NULL;
		}

		int MutableRoom::getMasterClientID(void) const
		{
			return mMasterClientID;
		}

		bool MutableRoom::setMasterClientID(int id)
		{			
			if(mMasterClientID != id)
			{
				int oldID = mMasterClientID;
				mMasterClientID = id;
				mpLoadBalancingClient->onMasterClientChanged(mMasterClientID, oldID);
				return true;
			}
			else
				return false;
		}

		const JVector<JString>& MutableRoom::getPropsListedInLobby(void) const
		{
			return mPropsListedInLobby;
		}

		void MutableRoom::setPropsListedInLobby(const JVector<JString>& propsListedInLobby, const JVector<JString>& expectedList, const WebFlags& webflags)
		{
			if(mPropsListedInLobby != propsListedInLobby)
			{
				mPropsListedInLobby = propsListedInLobby;
				Hashtable properties;
				properties.put(Properties::Room::PROPS_LISTED_IN_LOBBY, propsListedInLobby.getCArray(), propsListedInLobby.getSize());
				Hashtable expectedProperties;
				expectedProperties.put(Properties::Room::PROPS_LISTED_IN_LOBBY, expectedList.getCArray(), expectedList.getSize());
				mpLoadBalancingClient->opSetPropertiesOfRoom(properties, expectedProperties, webflags);
			}
		}

		int MutableRoom::getPlayerTtl(void) const
		{
			return mPlayerTtl;
		}

		int MutableRoom::getEmptyRoomTtl(void) const
		{
			return mEmptyRoomTtl;
		}
		bool MutableRoom::getSuppressRoomEvents(void) const
		{
			return mSuppressRoomEvents;
		}

		const Common::JVector<Common::JString>* MutableRoom::getPlugins(void) const
		{
			return mupPlugins;
		}

		bool MutableRoom::getPublishUserID(void) const
		{
			return mPublishUserID;
		}

		const JVector<JString>& MutableRoom::getExpectedUsers(void) const
		{
			return mExpectedUsers;
		}

		void MutableRoom::setExpectedUsers(const Common::JVector<Common::JString>& expectedUsers, const WebFlags& webflags)
		{
			if(mExpectedUsers == expectedUsers)
				return;
			Hashtable properties, expectedProperties;
			if(getExpectedUsers().getSize())
				expectedProperties.put(Properties::Room::EXPECTED_USERS, getExpectedUsers().getCArray(), getExpectedUsers().getSize());
			else
				expectedProperties.put(Properties::Room::EXPECTED_USERS, Object());
			properties.put(Properties::Room::EXPECTED_USERS, expectedUsers.getCArray(), expectedUsers.getSize());
			mpLoadBalancingClient->opSetPropertiesOfRoom(properties, expectedProperties, webflags);
		}


		Player* MutableRoom::createPlayer(int number, const Hashtable& properties) const
		{
			return PlayerFactory::create(number, properties, this);
		}

		void MutableRoom::destroyPlayer(const Player* pPlayer) const
		{
			PlayerFactory::destroy(pPlayer);
		}

		void MutableRoom::setPlayers(const JVector<Player*>& players)
		{
			mPlayers = players;
		}

		void MutableRoom::removeAllPlayers(void)
		{
			destroyAllPlayers();
			mPlayers.removeAllElements();
		}

		void MutableRoom::destroyAllPlayers(void)
		{
			for(unsigned int i=0; i<mPlayers.getSize(); ++i)
				if(mPlayers[i]->getNumber() != mLocalPlayerNumber)
					destroyPlayer(mPlayers[i]);
		}
		
		void MutableRoom::addPlayer(Player& player)
		{
			mPlayers.addElement(&player);
		}
		
		void MutableRoom::addLocalPlayer(Player& player)
		{
			mLocalPlayerNumber = player.getNumber();
			addPlayer(player);
		}
		
		void MutableRoom::addPlayer(int number, const Common::Hashtable& properties)
		{
			addPlayer(*createPlayer(number, properties));
		}
		
		bool MutableRoom::removePlayer(int number)
		{
			bool retVal = false;
			for(unsigned int i=0; i<mPlayers.getSize(); ++i)
			{
				if(mPlayers[i]->getNumber() == number)
				{
					destroyPlayer(mPlayers[i]);
					mPlayers.removeElementAt(i);
					retVal = true;
				}
			}
			return retVal;
		}

		JVector<Player*>& MutableRoom::getNonConstPlayers(void)
		{
			return mPlayers;
		}
	}
}