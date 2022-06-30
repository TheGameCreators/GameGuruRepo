//----------------------------------------------------
//--- GAMEGURU - M-Entity
//----------------------------------------------------

#include "cstr.h"
#include "types.h"

void entity_addtoselection_core ( void );
void entity_addtoselection ( void );
void entity_adduniqueentity ( bool bAllowDuplicates );
void entity_validatearraysize ( void );
bool entity_load_thread_prepare(LPSTR pFpeFile);
bool entity_load ( void );
void entity_loaddata ( void );
void entity_loadvideoid ( void );
void entity_loadactivesoundsandvideo ( void );
void entity_fillgrideleproffromprofile ( void );
void entity_updatetextureandeffectfromeleprof ( void );
void entity_updatetextureandeffectfromgrideleprof ( void );
void entity_getgunidandflakid ( void );
void entity_loadtexturesandeffect ( void );
void entity_saveelementsdata ( void );
void entity_savebank ( void );
void entity_savebank_ebe ( void );
void entity_loadbank ( void );
void entity_loadelementsdata ( void );
void entity_loadentitiesnow ( void );
void entity_deletebank ( void );
void entity_deleteelementsdata ( void );
void entity_deleteelements ( void );
void entity_assignentityparticletodecalelement ( void );
void entity_addentitytomap_core ( void );
void entity_addentitytomap ( void );
void entity_deleteentityfrommap ( void );
void entity_recordbuffer_add ( void );
void entity_recordbuffer_delete ( void );
void entity_recordbuffer_move ( void );
void entity_undo ( void );
void entity_redo ( void );
void entity_updateparticleemitterbyID(entityeleproftype* pEleprof, int iObj, float fScale, float fX, float fY, float fZ);
void entity_updateparticleemitter(int e);

bool ObjectIsEntity(void* pTestObject);

int GetLodLevels(int obj);

void entity_placeprobe(int obj, float fLightProbeScale);
void entity_deleteprobe(int obj);
