//========= Copyright © 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Class for tracking stats and achievements
//
// $NoKeywords: $
//=============================================================================

#include "stdafx.h"
#include "remotestoragesync.h"

//-----------------------------------------------------------------------------
// NOTE
//
// The following code is for synchronizing files while the game is running
// This is only necessary when on the Playstation 3, as the Steam client handles
// these operations when on PC/Mac.
//
//-----------------------------------------------------------------------------

