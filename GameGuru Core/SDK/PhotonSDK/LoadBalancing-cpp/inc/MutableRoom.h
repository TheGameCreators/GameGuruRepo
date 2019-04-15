/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "LoadBalancing-cpp/inc/Peer.h"
#include "LoadBalancing-cpp/inc/Player.h"
#include "LoadBalancing-cpp/inc/Room.h"
#include "LoadBalancing-cpp/inc/Internal/MutableRoomFactory.h"
#include "LoadBalancing-cpp/inc/Internal/PlayerPropertiesUpdateInformant.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			class PlayerMovementInformant;
		}

		class MutableRoom : public Room
		{
		public:
			using Room::toString;

			virtual ~MutableRoom(void);

			MutableRoom(const MutableRoom& toCopy);
			virtual MutableRoom& operator=(const Room& toCopy);
			virtual MutableRoom& operator=(const MutableRoom& toCopy);

			nByte getPlayerCount(void) const;
			void setMaxPlayers(nByte maxPlayers, const WebFlags& webflags=WebFlags());
			void setIsOpen(bool isOpen, const WebFlags& webflags=WebFlags());
			bool getIsVisible(void) const;
			void setIsVisible(bool isVisible, const WebFlags& webflags=WebFlags());
			const Common::JVector<Player*>& getPlayers(void) const;
			const Player* getPlayerForNumber(int playerNumber) const;
			int getMasterClientID(void) const;
			const Common::JVector<Common::JString>& getPropsListedInLobby(void) const;
			void setPropsListedInLobby(const Common::JVector<Common::JString>& propsListedInLobby, const Common::JVector<Common::JString>& expectedList=Common::JVector<Common::JString>(), const WebFlags& webflags=WebFlags());
			int getPlayerTtl(void) const;
			int getEmptyRoomTtl(void) const;
			bool getSuppressRoomEvents(void) const;
			const Common::JVector<Common::JString>* getPlugins(void) const;
			bool getPublishUserID(void) const;
			const Common::JVector<Common::JString>& getExpectedUsers(void) const;
			void setExpectedUsers(const Common::JVector<Common::JString>& expectedUsers, const WebFlags& webflags=WebFlags());

			void mergeCustomProperties(const Common::Hashtable& customProperties, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			template<typename ktype, typename vtype> void addCustomProperty(const ktype& key, const vtype& value, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			template<typename ktype, typename vtype> void addCustomProperty(const ktype& key, const vtype pValueArray, typename Common::Helpers::ArrayLengthType<vtype>::type arrSize, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			template<typename ktype, typename vtype> void addCustomProperty(const ktype& key, const vtype pValueArray, const short* pArrSizes, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			void addCustomProperties(const Common::Hashtable& customProperties, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			template<typename ktype> void removeCustomProperty(const ktype& key, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			template<typename ktype> void removeCustomProperties(const ktype* keys, unsigned int count, const Common::Hashtable& expectedCustomProperties=Common::Hashtable(), const WebFlags& webflags=WebFlags());
			virtual Common::JString toString(bool withTypes=false, bool withCustomProperties=false, bool withPlayers=false) const;
		protected:
			using Room::payloadToString;

			MutableRoom(const Common::JString& name, const Common::Hashtable& properties, Client* pClient, const Common::JVector<Common::JString>& propsListedInLobby, int playerTtl, int emptyRoomTtl, bool suppressRoomEvents, const Common::JVector<Common::JString>* pPlugins, bool publishUserID, const Common::JVector<Common::JString>& expectedUsers);

			virtual bool getIsMutable(void) const;

			virtual MutableRoom& assign(const Room& toCopy);
			virtual Player* createPlayer(int number, const Common::Hashtable& properties) const;
			virtual void destroyPlayer(const Player* pPlayer) const;
			virtual Common::JString payloadToString(bool withCustomProperties, bool withTypes, bool withPlayers) const;
		private:
			void cacheProperties(const Common::Hashtable& properties);
			template<typename Etype> void setRoomProperty(nByte key, Etype val, const WebFlags& webflags);
			template<typename Etype> void setRoomProperty(nByte key, const Etype pValueArray, typename Common::Helpers::ArrayLengthType<Etype>::type arrSize, const WebFlags& webflags);
			template<typename Etype> void setRoomProperty(nByte key, const Etype pValueArray, const short* pArrSizes, const WebFlags& webflags);

			void setPlayers(const Common::JVector<Player*>& players);
			void removeAllPlayers(void);
			void destroyAllPlayers(void);
			void addPlayer(Player& player);
			void addLocalPlayer(Player& player);
			void addPlayer(int number, const Common::Hashtable& properties);
			bool removePlayer(int number);
			Common::JVector<Player*>& getNonConstPlayers(void);
			bool setMasterClientID(int id);

			typedef Room super;

			Client* mpLoadBalancingClient;
			bool mIsVisible;
			Common::JVector<Player*> mPlayers;
			int mMasterClientID;
			Common::JVector<Common::JString> mPropsListedInLobby;
			int mLocalPlayerNumber;
			int mPlayerTtl;
			int mEmptyRoomTtl;
			bool mSuppressRoomEvents;
			Common::Helpers::UniquePointer<Common::JVector<Common::JString> > mupPlugins;
			bool mPublishUserID;
			Common::JVector<Common::JString> mExpectedUsers;

			friend class Internal::MutableRoomFactory;
			friend class Internal::RoomPropertiesCacher;
			friend class Internal::PlayerMovementInformant;
			friend class Internal::PlayerPropertiesUpdateInformant;
		};



		template<typename ktype, typename vtype>
		void MutableRoom::addCustomProperty(const ktype& key, const vtype& value, const Common::Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			Common::Hashtable hash;
			hash.put(key, value);
			addCustomProperties(hash, expectedCustomProperties, webflags);
		}
		
		template<typename ktype, typename vtype>
		void MutableRoom::addCustomProperty(const ktype& key, const vtype pValueArray, typename Common::Helpers::ArrayLengthType<vtype>::type arrSize, const Common::Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			Common::Hashtable hash;
			hash.put(key, pValueArray, arrSize);
			addCustomProperties(hash, expectedCustomProperties, webflags);
		}

		template<typename ktype, typename vtype>
		void MutableRoom::addCustomProperty(const ktype& key, const vtype pValueArray, const short* pArrSizes, const Common::Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			Common::Hashtable hash;
			hash.put(key, pValueArray, pArrSizes);
			addCustomProperties(hash, expectedCustomProperties, webflags);
		}


		template<typename ktype>
		void MutableRoom::removeCustomProperty(const ktype& key, const Common::Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			removeCustomProperties(&key, 1, expectedCustomProperties, webflags);
		}

		template<typename ktype>
		void MutableRoom::removeCustomProperties(const ktype* const keys, unsigned int count, const Common::Hashtable& expectedCustomProperties, const WebFlags& webflags)
		{
			Common::Hashtable hash;
			for(unsigned int i=0; i<count; i++)
				hash.put(keys[i]);
			mergeCustomProperties(hash, expectedCustomProperties, webflags);
		}
	}
}