/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/MutableRoom.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;
			using namespace Common::MemoryManagement;

			MutableRoomFactory::~MutableRoomFactory(void)
			{
			}

			MutableRoom* MutableRoomFactory::create(const JString& name, const Hashtable& properties, Client* pClient, const JVector<JString>& propsListedInLobby, int playerTtl, int emptyRoomTtl, bool suppressRoomEvents, const JVector<JString>* pPlugins, bool publishUserID, const JVector<JString>& expectedUsers)
			{
				MutableRoom* p;
				return ALLOCATE(MutableRoom, p, name, properties, pClient, propsListedInLobby, playerTtl, emptyRoomTtl, suppressRoomEvents, pPlugins, publishUserID, expectedUsers);
			}

			void MutableRoomFactory::destroy(const MutableRoom* pRoom)
			{
				deallocate(pRoom);
			}
		}
	}
}