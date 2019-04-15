/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Internal/PlayerMutableRoomPointerSetter.h"
#include "LoadBalancing-cpp/inc/Player.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			void PlayerMutableRoomPointerSetter::setMutableRoomPointer(Player& player, const MutableRoom* pRoom)
			{
				player.setMutableRoomPointer(pRoom);
			}
		}
	}
}