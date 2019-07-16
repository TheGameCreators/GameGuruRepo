#ifndef _STEAMCHECKFORWORKSHOP_H_
#define _STEAMCHECKFORWORKSHOP_H_

#include "directx-macros.h"

void SetWorkshopFolder ( LPSTR pFolder );
bool CheckForWorkshopFile ( LPSTR VirtualFilename);
bool Steam_CanIUse_W_();
bool CanIUse_E_();
void SetCanUse_e_ ( int flag );

#endif