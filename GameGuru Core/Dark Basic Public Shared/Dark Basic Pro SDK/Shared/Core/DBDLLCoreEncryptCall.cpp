#include "windows.h"
#include "globstruct.h"
#include "DBDLLCore.h"
#ifdef WICKEDENGINE
#include "..\..\..\..\Guru-WickedMAX\Encryptor.h"
#else
#include "..\..\..\..\Guru-MapEditor\Encryptor.h"
#endif
#include ".\..\Core\SteamCheckForWorkshop.h"

void CallEncryptDecrypt( char* dwStringAddress, bool bEncryptIfTrue, bool bDoNotUseTempFolder )
{
	// Sorry, no encryption code here.
	g_pGlob->dwEncryptionUniqueKey = 0;
}
