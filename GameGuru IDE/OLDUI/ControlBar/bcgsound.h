//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#ifndef __BCGSOUND_H
#define __BCGSOUND_H

#define BCGSOUND_NOT_STARTED	(-2)
#define BCGSOUND_TERMINATE		(-1)
#define BCGSOUND_IDLE			0
#define BCGSOUND_MENU_COMMAND	1
#define BCGSOUND_MENU_POPUP		2

void BCGPlaySystemSound (int nSound);

#endif // __BCGSOUND_H
