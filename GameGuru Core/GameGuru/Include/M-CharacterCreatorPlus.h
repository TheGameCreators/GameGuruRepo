//----------------------------------------------------
//--- GAMEGURU - M-CharacterCreatorPlus
//----------------------------------------------------

// Includes
#include "cstr.h"

// Data type
struct sCharacterCreatorPlusMouthData
{
	float fTimeStamp;
	int iMouthShape;
};
struct sCharacterCreatorPlusSpeak
{
	std::vector <sCharacterCreatorPlusMouthData> mouthData;
	float fMouthTimeStamp;
	int iMouthDataIndex;
	int iMouthDataShape;
	float fSmouthDataSpeedToNextShape;
	float fNeedToBlink;
};
struct sCharacterCreatorPlusSettings
{
	cstr script_s;
	cstr voice_s;
	int iSpeakRate;
	int iGender;
	int iNeckBone;
};
struct sCharacterCreatorPlusObject
{
	sCharacterCreatorPlusSettings settings;
	sCharacterCreatorPlusSpeak speak;
};
struct sCharacterCreatorPlus
{
	bool bInitialised;
	float fTimerTrigger;
	sCharacterCreatorPlusObject obj;
};

void charactercreatorplus_preloadinitialcharacter ( void );
void charactercreatorplus_preloadallcharacterpartchoices ( void );

void charactercreatorplus_refreshtype(void);
void charactercreatorplus_init ( void );
void charactercreatorplus_free ( void );
bool charactercreatorplus_savecharacterentity ( int iCharObj, LPSTR pOptionalDBOSaveFile, int iThumbnailImage );
void charactercreatorplus_loop ( void );
void charactercreatorplus_imgui(void);
void charactercreatorplus_preparechange(char *path, int part, char* tag);
void charactercreatorplus_waitforpreptofinish(void);
void charactercreatorplus_change(char *path, int part, char* tag);
void charactercreatorplus_loadannotationlist ( void );
LPSTR charactercreatorplus_findannotation ( LPSTR pSearchStr );
LPSTR charactercreatorplus_findannotationtag ( LPSTR pSearchStr );

void characterkitplus_chooseOnlineAvatar(void);
void characterkitplus_saveAvatarInfo(void);
void characterkitplus_checkAvatarExists(void);
void characterkitplus_loadMyAvatarInfo(void);
void characterkitplus_makeMultiplayerCharacterCreatorAvatar(void);
void characterkitplus_removeMultiplayerCharacterCreatorAvatar(void);
