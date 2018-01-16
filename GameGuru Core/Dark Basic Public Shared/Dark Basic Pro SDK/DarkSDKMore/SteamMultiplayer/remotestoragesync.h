//========= Copyright � 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Class for tracking stats and achievements
//
// $NoKeywords: $
//=============================================================================

#ifndef REMOTE_STORAGE_SYNC_H
#define REMOTE_STORAGE_SYNC_H

#include "SteamMultiplayer.h"
#include "RemoteStorage.h"

//-----------------------------------------------------------------------------
// NOTE
//
// The following code is for synchronizing files while the game is running
// This is only necessary when on the Playstation 3, as the Steam client handles
// these operations when on PC/Mac.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Interface for sync screen
//-----------------------------------------------------------------------------
class IRemoteStorageSync
{
public:
	virtual ~IRemoteStorageSync() {}

	virtual void SynchronizeToClient() = 0;
	virtual void SynchronizeToServer() = 0;
	virtual void Render() = 0;
	virtual bool BFinished() = 0;
	virtual void OnMenuSelection( ERemoteStorageSyncMenuCommand selection ) = 0;
};

#endif