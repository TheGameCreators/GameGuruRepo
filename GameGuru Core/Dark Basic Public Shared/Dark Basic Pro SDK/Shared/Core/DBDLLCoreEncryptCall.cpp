#include "windows.h"
#include "globstruct.h"
#include "DBDLLCore.h"
#include "..\..\..\..\Guru-MapEditor\Encryptor.h"
#include ".\..\Core\SteamCheckForWorkshop.h"

void CallEncryptDecrypt( char* dwStringAddress, bool bEncryptIfTrue, bool bDoNotUseTempFolder )
{
	// Sorry, no encryption code here.
	g_pGlob->dwEncryptionUniqueKey = 0;
}
