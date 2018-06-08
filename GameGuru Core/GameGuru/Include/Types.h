//----------------------------------------------------
//--- GAMEGURU - Types
//----------------------------------------------------

#ifndef __GG_TYPES_INCLUDE__
#define __GG_TYPES_INCLUDE__
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include "cstr.h"

//  Master settings (MAXTEXTURESIZEMULTIPLIER is 1024*? to get MAXTEXTURESIZE)
#define MAXTEXTURESIZE 2048
#define MAXTEXTURESIZEMULTIPLIER 2
	
//  AI Freeze Distances
#define MAXNEVERFREEZEDISTANCE 999000
#define MAXFREEZEDISTANCE 3000

//  Importer constants (tab indices exclusively used by the importer)
#define IMPORTERTEXTURESMAX 100
#define IMPORTERTABPAGE1 -50
#define IMPORTERTABPAGE2 -51
#define IMPORTERTABPAGE3 -52
#define IMPORTERTABPAGE4 -53
#define IMPORTERSAVEFILE 0
#define IMPORTEROPENFILE 1
#define MACIMPORTERCOLLISIONSHAPES 50
#define IMPORTERSHADERFILESMAX 20
#define IMPORTERSCRIPTFILESMAX 200
#define IMPORTERSCRIPTFILESSHOW 20
#define IMPORTERZPOSITION 400

//  Character Kit constants (tab indices exclusively used by the importer)
#define CHARACTERKITTABPAGE1 -60
#define CHARACTERKITSAVEFILE 0
#define CHARACTERKITOPENFILE 1

//  weapon system constants
//  These will be initial maximums for the data structures, but we can dynamically grow the arrays when required
#define WEAPON_MAXWEAPONS      20
#define WEAPON_MAXANIMATIONS   30
#define WEAPON_PROJECTILETYPES 100
#define WEAPON_MAXPROJECTILES  100
#define WEAPON_MAXSOUNDS       100

//  These are the states the weapon can be in
#define WEAPON_STATE_IDLEHIP         1
#define WEAPON_STATE_IDLEIRON        2
#define WEAPON_STATE_IDLESCOPE       3
#define WEAPON_STATE_FIREHIP         4
#define WEAPON_STATE_FIREIRON        5
#define WEAPON_STATE_FIRESCOPE       6
#define WEAPON_STATE_STARTFIREHIP    7
#define WEAPON_STATE_STARTFIREIRON   8
#define WEAPON_STATE_STARTFIRESCOPE  9
#define WEAPON_STATE_ENDFIREHIP      10
#define WEAPON_STATE_ENDFIREIRON     11
#define WEAPON_STATE_ENDFIRESCOPE    12
#define WEAPON_STATE_RELOAD          13
#define WEAPON_STATE_RELOADFROMEMPTY 14
#define WEAPON_STATE_PULLOUT         15
#define WEAPON_STATE_PUTAWAY         16
#define WEAPON_STATE_MOVEHIPTOIRON   17
#define WEAPON_STATE_MOVEHIPTOSCOPE  18
#define WEAPON_STATE_MOVEIRONTOSCOPE 19
#define WEAPON_STATE_MOVEIRONTOHIP   20
#define WEAPON_STATE_MOVESCOPETOHIP  21
#define WEAPON_STATE_MOVESCOPETOIRON 22

//  These are the modes a projectile can be in (and determines how the system processes it)
#define WEAPON_PROJECTILEMODE_RAYCAST 0
#define WEAPON_PROJECTILEMODE_DYNAMIC 1
#define WEAPON_PROJECTILEMODE_STATIC  2

//  These are the events a projectile can trigger when it collides with something, or at certain intervals etc.
#define WEAPON_PROJECTILERESULT_NULL     0
#define WEAPON_PROJECTILERESULT_DAMAGE   1
#define WEAPON_PROJECTILERESULT_EXPLODE  2
#define WEAPON_PROJECTILERESULT_CUSTOM   99

//  These are the animations a weapon can perform
#define WEAPON_ANIMATION_NULL            0
#define WEAPON_ANIMATION_IDLEHIP         1
#define WEAPON_ANIMATION_IDLEIRON        2
#define WEAPON_ANIMATION_IDLESCOPE       3
#define WEAPON_ANIMATION_FIREHIP         4
#define WEAPON_ANIMATION_FIREIRON        5
#define WEAPON_ANIMATION_FIRESCOPE       6
#define WEAPON_ANIMATION_STARTFIREHIP    7
#define WEAPON_ANIMATION_STARTFIREIRON   8
#define WEAPON_ANIMATION_STARTFIRESCOPE  9
#define WEAPON_ANIMATION_ENDFIREHIP      10
#define WEAPON_ANIMATION_ENDFIREIRON     11
#define WEAPON_ANIMATION_ENDFIRESCOPE    12
#define WEAPON_ANIMATION_RELOAD          13
#define WEAPON_ANIMATION_RELOADFROMEMPTY 14
#define WEAPON_ANIMATION_PULLOUT         15
#define WEAPON_ANIMATION_PUTAWAY         16
#define WEAPON_ANIMATION_MOVEHIPTOIRON   17
#define WEAPON_ANIMATION_MOVEHIPTOSCOPE  18
#define WEAPON_ANIMATION_MOVEIRONTOSCOPE 19
#define WEAPON_ANIMATION_MOVEIRONTOHIP   20
#define WEAPON_ANIMATION_MOVESCOPETOHIP  21
#define WEAPON_ANIMATION_MOVESCOPETOIRON 22

//  These are the type of particle the weapon or projectile can create (stock defaults)
#define WEAPON_PARTICLETYPE_NONE         0
#define WEAPON_PARTICLETYPE_SMOKE        1
#define WEAPON_PARTICLETYPE_FLARE        2
#define WEAPON_PARTICLETYPE_FIRE         3

//  Ravey Particles constants
#define RAVEY_PARTICLE_EMITTERS_MAX 10
#define RAVEY_PARTICLES_MAX 100
#define RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER 20
#define RAVEY_PARTICLES_IMAGETYPE_FLARE      0
#define RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE 1
#define RAVEY_PARTICLES_IMAGETYPE_FLAME      2
#define RAVEY_PARTICLES_IMAGETYPE_LASTONE    3

//  Widget keys
#define WIDGET_KEY_TRANSLATE 88
#define WIDGET_KEY_ROTATE 67
#define WIDGET_KEY_SCALE 86

//  bitbob constants
#define BITBOBS_DEFAULT_MAXSIZE 120.0
#define BITBOBS_DEFAULT_FADEIN  90.0
#define BITBOBS_DEFAULT_FADEOUT 100.0
#define BITBOBS_FADETIME        2000.0

//  terrain constants
#define TERRAIN_SUPERFLAT_HEIGHT 1000.0
#define TERRAIN_WATERLINE_SWIMOFFSET 20.0

//  conKit Edit constants
#define CONKITEDIT_CURSORRAYS 30
#define CONKITEDIT_CURSORRAYDIST 1000.0
#define CONKITEDIT_CURSORRAYSPREAD 5.0

//  ConKit Make constants
//  paths
#define CONKIT_FILEVERSION 1
#define CONKIT_PATH        "conkitbank\\"
#define CONKIT_IMAGEPATH   "conkitbank\\images\\"
#define CONKIT_PARTSPATH   "conkitbank\\parts\\"
#define CONKIT_FEATUREPATH "conkitbank\\features\\"
#define CONKIT_MENUPATH    "conkitbank\\menu\\"
#define CONKIT_SHADERPATH  "effectbank\\reloaded\\entity_basic.fx"
//  conkit top level modes
#define CONKIT_MODE_OFF     0
#define CONKIT_MODE_BUILD   1
#define CONKIT_MODE_DELETE  2
#define CONKIT_MODE_PAINT   3
#define CONKIT_MODE_FEATURE 4
//  size and scale variables
#define CONKIT_PREFAB_SCALEFACTOR 1000
#define CONKIT_FEATURE_SCALEFACTOR 1000
#define CONKIT_GRID_BORDERSIZE 4
//  maximums for our different object types
#define CONKIT_ENTITYPARTS_MAX 6000
#define CONKIT_ENTITIES_MAX 2000
#define CONKIT_BUILDINGSITES_MAX 10
#define CONKIT_CONSOLIDATED_GRIDSIZE 8
#define CONKIT_CONSOLIDATED_MAX 100
#define CONKIT_FEATURES_MAX 500
//  conkit menu constants
#define CONKIT_PAINT_MENURESOLUTION 5
#define CONKIT_MENU_DIVIDER 1400.0
//  conkit types of objects
#define CONKIT_TYPE_FLOOR 1
#define CONKIT_TYPE_FLOORDIAG 2
#define CONKIT_TYPE_WALL 3
#define CONKIT_TYPE_WALLDIAG 4
#define CONKIT_TYPE_POST 5
#define CONKIT_TYPE_EDGESTRAIGHT 6
#define CONKIT_TYPE_EDGEDIAGONAL 7
#define CONKIT_TYPE_EDGEJOIN 8
#define CONKIT_TYPE_EDGEDIAGONALJOIN 9
#define CONKIT_TYPE_EDGECORNER45 10
#define CONKIT_TYPE_EDGECORNER45LEFT 11
#define CONKIT_TYPE_EDGECORNER90 12
#define CONKIT_TYPE_EDGECORNER135 13
#define CONKIT_TYPE_EDGECORNER135LEFT 14
#define CONKIT_TYPE_EDGECORNER225 15
#define CONKIT_TYPE_EDGECORNER225LEFT 16
#define CONKIT_TYPE_EDGECORNER270 17
#define CONKIT_TYPE_EDGECORNER315 18
#define CONKIT_TYPE_EDGECORNER315LEFT 19
#define CONKIT_TYPE_EDGECORNERDIAG 20
//  conkit feature classes
#define CONKIT_FEATURECLASS_NORMAL         0
#define CONKIT_FEATURECLASS_TRANS          1
#define CONKIT_FEATURECLASS_TRANSBREAKABLE 2
#define CONKIT_FEATURECLASS_SWINGDOOR      3
#define CONKIT_FEATURECLASS_LAST           3
//  conkit feature states
#define CONKIT_FEATURESTATE_START     0
#define CONKIT_FEATURESTATE_ANIMATING 1
#define CONKIT_FEATURESTATE_END       2
//  conkit physics types
#define CONKIT_PHYSICSTYPE_NONE 0
#define CONKIT_PHYSICSTYPE_BOX  1
#define CONKIT_PHYSICSTYPE_MESH 2

//  Max physics boxes
#define MAX_ENTITY_PHYSICS_BOXES  100

//  Steam constants
#define STEAM_MODE_NONE  -1
#define STEAM_MODE_MAIN_MENU  0
#define STEAM_MODE_LOBBY_CREATED  1
#define STEAM_MODE_LOBBY  2
#define STEAM_WAITING_FOR_LOBBY_CREATION  3
#define STEAM_WAITING_FOR_SERVER_CREATION  4
#define STEAM_SERVER_CREATED  5
#define STEAM_JOINING_LOBBY  6
#define STEAM_IN_GAME_SERVER  7
#define STEAM_IN_GAME_CLIENT  8
#define STEAM_IN_GAME_SERVER_SENDING_OTHER_PLAYERS_INFO  9
#define STEAM_IN_GAME_CLIENT_WAITING_FOR_OTHER_PLAYERS_INFO  10
#define STEAM_SERVER_CHOOSING_TYPE_OF_LEVEL  11
#define STEAM_SERVER_CHOOSING_FPM_TO_USE  12
#define STEAM_SERVER_CHOOSING_WORKSHOP_TO_USE  13
#define STEAM_SERVER_CHOOSING_TO_MAKE_FPS_WORKSHOP  14
#define STEAM_CREATING_WORKSHOP_ITEM  15
#define STEAM_ASKING_IF_SUBSCRIBE_TO_WORKSHOP_ITEM  16
#define STEAM_ASKING_IF_SUBSCRIBE_TO_WORKSHOP_ITEM_WAITING_FOR_RESULTS  17
#define STEAM_ASKING_IF_SUBSCRIBE_TO_WORKSHOP_ITEM_RESULTS  18
#define STEAM_TELLING_THEY_NEED_TO_RESTART  19

#define STEAM_MAX_NUMBER_OF_LOBBIES  100
#define STEAM_DESTROYED_OBJECT_LIST_SIZE  1000
#define STEAM_RESPAWN_TIME_OBJECT_LIST_SIZE  100
#define STEAM_RESPAWN_TIME_DELAY  10000

#define STEAM_ANIMATION_NONE  0
#define STEAM_ANIMATION_IDLE  1
#define STEAM_ANIMATION_WALKING  2
#define STEAM_ANIMATION_WALKINGBACKWARDS  3
#define STEAM_ANIMATION_DUCKING  4
#define STEAM_ANIMATION_DUCKINGWALKING  5
#define STEAM_ANIMATION_DUCKINGWALKINGBACKWARDS  6
#define STEAM_ANIMATION_STRAFELEFT  7
#define STEAM_ANIMATION_STRAFERIGHT  8
#define STEAM_ANIMATION_RELOAD  9

#define STEAM_MAX_NUMBER_OF_PLAYERS  8
//  we send position data at 60fps but multiplayer dll sends it when it needs to
#define STEAM_POSITION_UPDATE_DELAY  1000/60
#define STEAM_INPUT_UPDATE_DELAY  1000/4
#define STEAM_APPEARANCE_UPDATE_DELAY  1000/1

#define STEAM_PROJECTILE_UPDATE_DELAY  1000/15
#define STEAM_ALIVE_UPDATE_DELAY  1000/2

#define STEAM_JOIN_LOBBY_TIMEOUT  1000*10
#define STEAM_MESSAGE_TIMOUT  1000*4

#define STEAM_MAX_CHAT_LINES 5
#define STEAM_CHAT_DELAY  1000*15

#define STEAM_LUA_SetActivated 1
#define STEAM_LUA_SetAnimation 2
#define STEAM_LUA_PlayAnimation 3
#define STEAM_LUA_ActivateIfUsed 4
#define STEAM_LUA_PlaySound 5
#define STEAM_LUA_StartTimer 6
#define STEAM_LUA_CollisionOff 7
#define STEAM_LUA_CollisionOn 8
#define STEAM_LUA_ServerSetLuaGameMode 9
#define STEAM_LUA_ServerSetPlayerKills 10
#define STEAM_LUA_ServerSetPlayerDeaths 11
#define STEAM_LUA_ServerSetPlayerAddKill 12
#define STEAM_LUA_ServerSetPlayerRemoveKill 13
#define STEAM_LUA_ServerSetPlayerAddDeath 14
#define STEAM_LUA_SetServerTimer 15
#define STEAM_LUA_ServerRespawnAll 16
#define STEAM_LUA_ServerEndPlay 17
#define STEAM_LUA_AiGoToX 18
#define STEAM_LUA_AiGoToZ 19
#define Steam_LUA_setcharactertowalkrun 20
#define Steam_LUA_DarkAI_AIEntityStop  21
#define Steam_LUA_CharacterControlArmed 22
#define Steam_LUA_LookAtPlayer 23
#define Steam_LUA_TakenAggro 24
#define Steam_LUA_FireWeaponEffectOnly 25
#define Steam_LUA_RotateToPlayer 26
#define Steam_LUA_CharacterControlLimbo  27
#define Steam_LUA_CharacterControlUnarmed  28
#define Steam_LUA_SetAnimationFrames  29
#define Steam_LUA_AISetEntityControl  30
#define Steam_LUA_AIMoveX  31
#define Steam_LUA_AIMoveZ  32
#define Steam_LUA_SendAvatar  33
#define Steam_LUA_SendAvatarName  34
#define Steam_LUA_HaveAggro  35
#define Steam_LUA_CharacterControlManual  36

//  Memblock Resources
//  NOTE; Importer will DELETE memblocks 1-50!!
//  1   = Used to load terrain height data / load entity bin files
//  3   = Used to load grass data in
//  9   = Used to collect POT data for mega texture mini-texture references
//  10  = Used to collect data for terrain super texture
//  11  = To create waypoint polygons
//  30  = Old terrain height loader
//  44  = Grass Memblock
//  51  = Memblock to replace POTREF (used for mega texture generation)
//  123 = TERRAIN VEGMAP IMAGE (terrain.imagestart+2 copy)
//  124 = Extra copy for Undo Buffer
//  125 = Extra copy for Redo Buffer
//  255 = Used to find texture files in object limbs

//  data struture for interactive tutorial mode
struct interactivetype
{
	int active;
	int insidepanel;
	int showonstartup;
	int clicked;
	int editorlogicevent;
	int pageindex;
	int itemhighlightindex;
	int sceneindex;
	int sceneselectednow;
	int sequencegroup;
	int sequenceindex;
	int sequencemax;
	int sequencegroupmax;
	int alphafadestage;
	int alphafade;
	int alphafadecommand;
	int alphalasttime;
	int alphainitial;
	int applychangesused;
	int testgameused;
	int savestandaloneused;

	// Constructor
	interactivetype ( )
	{
		 savestandaloneused = 0;
		 testgameused = 0;
		 applychangesused = 0;
		 alphainitial = 0;
		 alphalasttime = 0;
		 alphafadecommand = 0;
		 alphafade = 0;
		 alphafadestage = 0;
		 sequencegroupmax = 0;
		 sequencemax = 0;
		 sequenceindex = 0;
		 sequencegroup = 0;
		 sceneselectednow = 0;
		 sceneindex = 0;
		 itemhighlightindex = 0;
		 pageindex = 0;
		 editorlogicevent = 0;
		 clicked = 0;
		 showonstartup = 0;
		 insidepanel = 0;
		 active = 0;
	}
	// End of Constructor

};

//  promptimage data structure for script image prompts
struct promptimagetype
{
	float x;
	float y;
	int img;
	int show;
	int alignment;

	// Constructor
	promptimagetype ( )
	{
		 alignment = 0;
		 show = 0;
		 img = 0;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

struct luatexttype
{
	float x;
	float y;
	int size;
	cstr txt;

	// Constructor
	luatexttype ( )
	{
		 txt = "";
		 size = 0;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

struct luapaneltype
{
	float x;
	float y;
	float x2;
	float y2;
	luapaneltype ( )
	{
		 y2 = 0.0f;
		 x2 = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
	}
};

struct characterkitcontroltype
{
	int count;
	int offset;
	int bankOffset;
	int minEntity;
	int maxEntity;
	int init;
	cstr originalDir_s;
	int oldtabmode;
	int loaded;
	int oldF9CursorEntid;
	int gameHasCharacterCreatorIn;
	int isMale;
	int customHeadMode;
	int customHeadMouseMode;
	int samplerMode;
	float origPosX_f;
	float origPosY_f;
	float origPosZ_f;
	float origAngX_f;
	float origAngY_f;
	float origAngZ_f;
	int showmyhead;
	int fmaleProjection;
	int hasCustomHead;

	// Constructor
	characterkitcontroltype ( )
	{
		 hasCustomHead = 0;
		 fmaleProjection = 0;
		 showmyhead = 0;
		 origAngZ_f = 0.0f;
		 origAngY_f = 0.0f;
		 origAngX_f = 0.0f;
		 origPosZ_f = 0.0f;
		 origPosY_f = 0.0f;
		 origPosX_f = 0.0f;
		 samplerMode = 0;
		 customHeadMouseMode = 0;
		 customHeadMode = 0;
		 isMale = 0;
		 gameHasCharacterCreatorIn = 0;
		 oldF9CursorEntid = 0;
		 loaded = 0;
		 oldtabmode = 0;
		 originalDir_s = "";
		 init = 0;
		 maxEntity = 0;
		 minEntity = 0;
		 bankOffset = 0;
		 offset = 0;
		 count = 0;
	}
	// End of Constructor

};

struct steamworkstype
{
	int isRunning;
	cstr playerName;
	cstr playerID;
	int maxHealth;
	int launchServer;
	int mode;
	int isLobbyCreated;
	int isServerCreated;
	int lobbyoffset;
	int lobbyscrollbarOn;
	float lobbyscrollbarOldY;
	float lobbyscrollbarOffset;
	int isGameHost;
	int voiceChatOn;
	int lobbycount;
	int selectedLobby;
	cstr selectedLobbyName;
	int gameAlreadySpawnedBefore;
	int killedByPlayer;
	cstr previousMessage_s;
	//int syncedWithServer;
	int syncedWithServerMode;
	int oldtime;
	int me;
	int playedMyDeathAnim;
	int fileLoaded;
	int playGame;
	int oldSpawnTimeLeft;
	int respawnLeft;
	int crouchOn;
	int meleeOn;
	int isAnimating;
	int okayToLoadLevel;
	int iHaveSaidIAmReady;
	int attachmentcount;
	int gunCount;
	int gunid;
	int lastSendTime;
	int lastSendTimeAppearance;
	int appearance;
	int dyingTime;
	int spawnrnd;
	int reloading;
	int syncedWithServer;
	int sentreadytime;
	int AttemptedToJoinLobbyTime;
	cstr message;
	int messageTime;
	int lastSendProjectileTime;
	int dontApplyDamage;
	int lastSendAliveTime;
	int lastSendPositionTime;
	float lastx;
	float lasty;
	float lastz;
	float lastangley;
	float camrotate;
	int ragdollon;
	float spectatorfollowdistance;
	int spectatorfollowdistancedelay;
	int ignoreDamageToEntity;
	int reloadingCount;
	int killedByPlayerFlag;
	int checkedWhoKilledMe;
	int playerThatKilledMe;
	int endplay;
	int destroyedObjectCount;
	int showscoresdelay;
	int setserverkillstowin;
	int oldfootfloortime;
	int footfloor;
	cstr lobbyjoinedname;
	int howmanyjoinedatstart;
	int howmanyfpefiles;
	int listboxmode;
	cstr fpmpicked;
	int resetcore;
	int usersInServersLobbyAtServerCreation;
	int levelContainsCustomContent;
	int buildingWorkshopItemMode;
	cstr workshopid;
	cstr originalpath;
	int serverusingworkshop;
	int needToResetOnStartup;
	cstr workshopidtojoin;
	cstr levelnametojoin;
	cstr workshopVersionNumberToJoin;
	int steamDoColorText;
	int steamColorRed;
	int steamColorGreen;
	int steamColorBlue;
	int sentmyname;
	int lastsendmynametime;
	int initialSpawnmoveDownCharacterFlag;
	int resetSteamFlag;
	cstr buildingDots;
	int backtoeditorforyou;
	int workshopItemVersionNumber;
	int workshopItemChangedFlag;
	int dontDrawTitles;
	cstr messageDots;
	int messageTimeDots;
	int haveToldAboutSolo;
	int chattimer;
	int chaton;
	cstr chatstring;
	int chatbackspacedelay;
	int chatcursortime;
	int oldchatscancode;
	int cursoron;
	int steamdotsoldtime;
	//int invincibleTimer;
	int haveshowndeath;
	int goBackToEditor;
	int haveSentSteamIDToEditor;
	int lastTimeISentMySteamID;
	int lastTimeTriedToConnectToSteamFromEditor;
	int checkiflobbiesavailablemode;
	int noplayermovement;
	int team;
	int friendlyfireoff;
	int invincibleTimer;
	int lastSpawnedTime;
	int nameplatesOff;
	int coop;
	int madeArrays;
	int damageWasFromAI;
	int originalEntitycount;
	cstr myAvatar_s;
	cstr myAvatarHeadTexture_s;
	cstr myAvatarName_s;
	int haveSentMyAvatar;
	cstr myFileToSendToServer_s;
	int myOriginalSpawnPoint;
	int realfirsttimespawn;
	int finishedLoadingMap;

	// Constructor
	steamworkstype ( )
	{
		 finishedLoadingMap = 0;
		 realfirsttimespawn = 0;
		 myOriginalSpawnPoint = 0;
		 myFileToSendToServer_s = "";
		 haveSentMyAvatar = 0;
		 myAvatarName_s = "";
		 myAvatarHeadTexture_s = "";
		 myAvatar_s = "";
		 originalEntitycount = 0;
		 damageWasFromAI = 0;
		 madeArrays = 0;
		 coop = 0;
		 nameplatesOff = 0;
		 lastSpawnedTime = 0;
		 invincibleTimer = 0;
		 friendlyfireoff = 0;
		 team = 0;
		 noplayermovement = 0;
		 checkiflobbiesavailablemode = 0;
		 lastTimeTriedToConnectToSteamFromEditor = 0;
		 lastTimeISentMySteamID = 0;
		 haveSentSteamIDToEditor = 0;
		 goBackToEditor = 0;
		 haveshowndeath = 0;
		 steamdotsoldtime = 0;
		 cursoron = 0;
		 oldchatscancode = 0;
		 chatcursortime = 0;
		 chatbackspacedelay = 0;
		 chatstring = "";
		 chaton = 0;
		 chattimer = 0;
		 haveToldAboutSolo = 0;
		 messageTimeDots = 0;
		 messageDots = "";
		 dontDrawTitles = 0;
		 workshopItemChangedFlag = 0;
		 workshopItemVersionNumber = 0;
		 backtoeditorforyou = 0;
		 buildingDots = "";
		 resetSteamFlag = 0;
		 initialSpawnmoveDownCharacterFlag = 0;
		 lastsendmynametime = 0;
		 sentmyname = 0;
		 steamColorBlue = 0;
		 steamColorGreen = 0;
		 steamColorRed = 0;
		 steamDoColorText = 0;
		 workshopVersionNumberToJoin = "";
		 levelnametojoin = "";
		 workshopidtojoin = "";
		 needToResetOnStartup = 0;
		 serverusingworkshop = 0;
		 originalpath = "";
		 workshopid = "";
		 buildingWorkshopItemMode = 0;
		 levelContainsCustomContent = 0;
		 usersInServersLobbyAtServerCreation = 0;
		 resetcore = 0;
		 fpmpicked = "";
		 listboxmode = 0;
		 howmanyfpefiles = 0;
		 howmanyjoinedatstart = 0;
		 lobbyjoinedname = "";
		 footfloor = 0;
		 oldfootfloortime = 0;
		 setserverkillstowin = 0;
		 showscoresdelay = 0;
		 destroyedObjectCount = 0;
		 endplay = 0;
		 playerThatKilledMe = 0;
		 checkedWhoKilledMe = 0;
		 killedByPlayerFlag = 0;
		 reloadingCount = 0;
		 ignoreDamageToEntity = 0;
		 spectatorfollowdistancedelay = 0;
		 spectatorfollowdistance = 0.0f;
		 ragdollon = 0;
		 camrotate = 0.0f;
		 lastangley = 0.0f;
		 lastz = 0.0f;
		 lasty = 0.0f;
		 lastx = 0.0f;
		 lastSendPositionTime = 0;
		 lastSendAliveTime = 0;
		 dontApplyDamage = 0;
		 lastSendProjectileTime = 0;
		 messageTime = 0;
		 message = "";
		 AttemptedToJoinLobbyTime = 0;
		 sentreadytime = 0;
		 syncedWithServer = 0;
		 reloading = 0;
		 spawnrnd = 0;
		 dyingTime = 0;
		 appearance = 0;
		 lastSendTimeAppearance = 0;
		 lastSendTime = 0;
		 gunid = 0;
		 gunCount = 0;
		 attachmentcount = 0;
		 iHaveSaidIAmReady = 0;
		 okayToLoadLevel = 0;
		 isAnimating = 0;
		 meleeOn = 0;
		 crouchOn = 0;
		 respawnLeft = 0;
		 oldSpawnTimeLeft = 0;
		 playGame = 0;
		 fileLoaded = 0;
		 playedMyDeathAnim = 0;
		 me = 0;
		 oldtime = 0;
		 syncedWithServerMode = 0;
		 previousMessage_s = "";
		 killedByPlayer = 0;
		 gameAlreadySpawnedBefore = 0;
		 selectedLobbyName = "";
		 selectedLobby = 0;
		 lobbycount = 0;
		 voiceChatOn = 0;
		 isGameHost = 0;
		 lobbyscrollbarOffset = 0.0f;
		 lobbyscrollbarOldY = 0.0f;
		 lobbyscrollbarOn = 0;
		 lobbyoffset = 0;
		 isServerCreated = 0;
		 isLobbyCreated = 0;
		 mode = 0;
		 launchServer = 0;
		 maxHealth = 0;
		 playerID = "";
		 playerName = "";
		 isRunning = 0;
	}
	// End of Constructor

};


struct steamworksbullettype
{
	int btype;
	float x_f;
	float y_f;
	float z_f;
	float xrot_f;
	float yrot_f;
	float zrot_f;
	int on;
	int particles;
	int sound;
	float soundDistFromPlayer;

	// Constructor
	steamworksbullettype ( )
	{
		 soundDistFromPlayer = 0.0f;
		 sound = 0;
		 particles = 0;
		 on = 0;
		 zrot_f = 0.0f;
		 yrot_f = 0.0f;
		 xrot_f = 0.0f;
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
		 btype = 0;
	}
	// End of Constructor

};


struct steamworksrespawntype
{
	int inuse;
	int e;
	int time;

	// Constructor
	steamworksrespawntype ( )
	{
		 time = 0;
		 e = 0;
		 inuse = 0;
	}
	// End of Constructor

};


struct steamworksmultiplayerstarttype
{
	int active;
	int ghostentityindex;
	float x;
	float y;
	float z;
	float angle;

	// Constructor
	steamworksmultiplayerstarttype ( )
	{
		 angle = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 ghostentityindex = 0;
		 active = 0;
	}
	// End of Constructor

};

/*
//  RealSense Data Structures
struct realsensetype
{
	int init;
	int grammermax;
	int grammer;
	int disconnected;
	float bMassX_f;
	float bMassY_f;
	float bMassCalibrateX_f;
	int id;
	int confidence;
	cstr currentCommand_s;

	// Constructor
	realsensetype ( )
	{
		 currentCommand_s = "";
		 confidence = 0;
		 id = 0;
		 bMassCalibrateX_f = 0.0f;
		 bMassY_f = 0.0f;
		 bMassX_f = 0.0f;
		 disconnected = 0;
		 grammer = 0;
		 grammermax = 0;
		 init = 0;
	}
	// End of Constructor

};
*/

//  Weapon System Data Structures
//  Top level variables for the weapon system
struct WeaponSystemType
{
	int objTracer;
	int numWeapons;
	int numProjectileBases;
	int numProjectiles;
	int numSounds;
	int initialorbitprojectiledefeatstall;

	// Constructor
	WeaponSystemType ( )
	{
		 initialorbitprojectiledefeatstall = 0;
		 numSounds = 0;
		 numProjectiles = 0;
		 numProjectileBases = 0;
		 numWeapons = 0;
		 objTracer = 0;
	}
	// End of Constructor

};


//  This is a definition for a particle emitter. Anything from ejecting brass, to smoke, muzzle flare etc.
//  This forms the basis for the data passed to the particle system. Not all values are used by all particle types
struct weaponParticleEmitterBaseType
{
	int particleType;
	float delay;
	int joint;

	float xOffsetMin_f;
	float xOffsetMax_f;
	float yOffsetMin_f;
	float yOffsetMax_f;
	float zOffsetMin_f;
	float zOffsetMax_f;
	float xAngOffsetMin_f;
	float xAngOffsetMax_f;
	float yAngOffsetMin_f;
	float yAngOffsetMax_f;
	float zAngOffsetMin_f;
	float zAngOffsetMax_f;

	float scaleMin_f;
	float scaleMax_f;
	float scaleSpeed_f;

	float xSpdMin_f;
	float xSpdMax_f;
	float ySpdMin_f;
	float ySpdMax_f;
	float zSpdMin_f;
	float zSpdMax_f;
	float xAngSpdMin_f;
	float xAngSpdMax_f;
	float yAngSpdMin_f;
	float yAngSpdMax_f;
	float zAngSpdMin_f;
	float zAngSpdMax_f;

	int lifeMin;
	int lifeMax;
	int alpha;


	// Constructor
	weaponParticleEmitterBaseType ( )
	{
		 alpha = 0;
		 lifeMax = 0;
		 lifeMin = 0;
		 zAngSpdMax_f = 0.0f;
		 zAngSpdMin_f = 0.0f;
		 yAngSpdMax_f = 0.0f;
		 yAngSpdMin_f = 0.0f;
		 xAngSpdMax_f = 0.0f;
		 xAngSpdMin_f = 0.0f;
		 zSpdMax_f = 0.0f;
		 zSpdMin_f = 0.0f;
		 ySpdMax_f = 0.0f;
		 ySpdMin_f = 0.0f;
		 xSpdMax_f = 0.0f;
		 xSpdMin_f = 0.0f;
		 scaleSpeed_f = 0.0f;
		 scaleMax_f = 0.0f;
		 scaleMin_f = 0.0f;
		 zAngOffsetMax_f = 0.0f;
		 zAngOffsetMin_f = 0.0f;
		 yAngOffsetMax_f = 0.0f;
		 yAngOffsetMin_f = 0.0f;
		 xAngOffsetMax_f = 0.0f;
		 xAngOffsetMin_f = 0.0f;
		 zOffsetMax_f = 0.0f;
		 zOffsetMin_f = 0.0f;
		 yOffsetMax_f = 0.0f;
		 yOffsetMin_f = 0.0f;
		 xOffsetMax_f = 0.0f;
		 xOffsetMin_f = 0.0f;
		 joint = 0;
		 delay = 0.0f;
		 particleType = 0;
	}
};


//  This is the realtime changing data for a particle emitter type defined with weaponParticleEmitterBaseType
struct weaponParticleEmitterType
{
	int lastEmitStamp;

	// Constructor
	weaponParticleEmitterType ( )
	{
		 lastEmitStamp = 0;
	}
};

//  Settings for the currently held weapon and global high level settings
struct CurrentWeaponType
{

	int currentWeapon;
	int previousWeapon;

	int state;
	int stateTimeStamp;

	int autoROFDelay;
	int tracerCount;
	int ammoInClip;

	int animation;
	float animationFrame_f;

	float xAngOffset_f;
	float yAngOffset_f;
	float zAngOffset_f;
	float xPosOffset_f;
	float yPosOffset_f;
	float zPosOffset_f;
	float xRecoilSpeed_f;
	float yRecoilSpeed_f;
	float zRecoilSpeed_f;
	float xRecoilAngSpeed_f;
	float yRecoilAngSpeed_f;
	float zRecoilAngSpeed_f;

	weaponParticleEmitterType particle1;
	weaponParticleEmitterType particle2;
	weaponParticleEmitterType particle3;
	weaponParticleEmitterType particle4;


	// Constructor
	CurrentWeaponType ( )
	{
		 zRecoilAngSpeed_f = 0.0f;
		 yRecoilAngSpeed_f = 0.0f;
		 xRecoilAngSpeed_f = 0.0f;
		 zRecoilSpeed_f = 0.0f;
		 yRecoilSpeed_f = 0.0f;
		 xRecoilSpeed_f = 0.0f;
		 zPosOffset_f = 0.0f;
		 yPosOffset_f = 0.0f;
		 xPosOffset_f = 0.0f;
		 zAngOffset_f = 0.0f;
		 yAngOffset_f = 0.0f;
		 xAngOffset_f = 0.0f;
		 animationFrame_f = 0.0f;
		 animation = 0;
		 ammoInClip = 0;
		 tracerCount = 0;
		 autoROFDelay = 0;
		 stateTimeStamp = 0;
		 state = 0;
		 previousWeapon = 0;
		 currentWeapon = 0;
	}
	// End of Constructor

};


struct weaponType
{

	int activeFlag;
	int availableFlag;
	cstr name_s;
	cstr projectileName_s;
	int obj;
	int hudGraphicType;
	int scopeGraphicType;

	int ammoPerClip;
	int ammoPoolID;
	int reloadTime;
	int reloadAutoFlag;
	int holsterAmmoEmptyFlag;
	int switchAmmoEmptyFlag;

	float rangeModifier_f;
	float damageModifier_f;
	float hipMoveSpeedModifier_f;
	float hipTurnSpeedModifier_f;
	float ironMoveSpeedModifier_f;
	float ironTurnSpeedModifier_f;
	float scopeMoveSpeedModifier_f;
	float scopeTurnSpeedModifier_f;

	int hipFireAllowed;
	int ironSightAllowed;
	int scopeAllowed;
	int singleFireAllowed;
	int burstFireAllowed;
	int burstFireCount;
	int autoFireAllowed;
	int twoStageFireAllowed;

	int singleROFDelay;
	int autoROFDelay;
	int autoROFDelayMax;
	int autoROFDelayMin;
	int autoROFDelayAccel;

	int projectileType;
	int projectileCount;
	int tracerFrequency;

	float xMuzzleAngleMin_f;
	float xMuzzleAngleMax_f;
	float yMuzzleAngleMin_f;
	float yMuzzleAngleMax_f;
	float zMuzzleAngleMin_f;
	float zMuzzleAngleMax_f;
	float xMuzzleAccSpeed_f;
	float yMuzzleAccSpeed_f;
	float zMuzzleAccSpeed_f;
	float muzzleAccModHip_f;
	float muzzleAccModIron_f;
	float muzzleAccModScope_f;
	float muzzleAccModWalk_f;
	float muzzleAccModRun_f;
	float muzzleAccModCrouch_f;
	float muzzleAccModJump_f;
	float muzzleAccModHealth_f;

	float xRecoilSpeedMin_f;
	float xRecoilSpeedMax_f;
	float yRecoilSpeedMin_f;
	float yRecoilSpeedMax_f;
	float zRecoilSpeedMin_f;
	float zRecoilSpeedMax_f;
	float xRecoilSpeedDecel_f;
	float yRecoilSpeedDecel_f;
	float zRecoilSpeedDecel_f;
	float xRecoilSpeedReturn_f;
	float yRecoilSpeedReturn_f;
	float zRecoilSpeedReturn_f;
	int recoilLimb;
	float xRecoilAngSpeedMin_f;
	float xRecoilAngSpeedMax_f;
	float yRecoilAngSpeedMin_f;
	float yRecoilAngSpeedMax_f;
	float zRecoilAngSpeedMin_f;
	float zRecoilAngSpeedMax_f;
	float xRecoilAngSpeedDecel_f;
	float yRecoilAngSpeedDecel_f;
	float zRecoilAngSpeedDecel_f;
	float xRecoilAngSpeedReturn_f;
	float yRecoilAngSpeedReturn_f;
	float zRecoilAngSpeedReturn_f;
	float recoilModHip_f;
	float recoilModIron_f;
	float recoilModScope_f;
	float recoilModWalk_f;
	float recoilModRun_f;
	float recoilModCrouch_f;
	float recoilModJump_f;
	float recoilModHealth_f;

	float xHipPosition_f;
	float yHipPosition_f;
	float zHipPosition_f;
	float xIronPosition_f;
	float yIronPosition_f;
	float zIronPosition_f;
	float xScopePosition_f;
	float yScopePosition_f;
	float zScopePosition_f;
	int hipToIronTime;
	int hipToScopeTime;
	int ironToHipTime;
	int ironToScopeTime;
	int scopeToHipTime;
	int scopeToIronTime;

	int soundFire1;
	int soundFire2;
	int soundFire3;
	int soundFire4;
	int soundReload;
	int soundDryFire;
	int soundPullOut;
	int soundPutAway;
	int soundToHip;
	int soundToIron;
	int soundToScope;
	int soundBackground;

	weaponParticleEmitterBaseType particle1;
	weaponParticleEmitterBaseType particle2;
	weaponParticleEmitterBaseType particle3;
	weaponParticleEmitterBaseType particle4;


	// Constructor
	weaponType ( )
	{
		 soundBackground = 0;
		 soundToScope = 0;
		 soundToIron = 0;
		 soundToHip = 0;
		 soundPutAway = 0;
		 soundPullOut = 0;
		 soundDryFire = 0;
		 soundReload = 0;
		 soundFire4 = 0;
		 soundFire3 = 0;
		 soundFire2 = 0;
		 soundFire1 = 0;
		 scopeToIronTime = 0;
		 scopeToHipTime = 0;
		 ironToScopeTime = 0;
		 ironToHipTime = 0;
		 hipToScopeTime = 0;
		 hipToIronTime = 0;
		 zScopePosition_f = 0.0f;
		 yScopePosition_f = 0.0f;
		 xScopePosition_f = 0.0f;
		 zIronPosition_f = 0.0f;
		 yIronPosition_f = 0.0f;
		 xIronPosition_f = 0.0f;
		 zHipPosition_f = 0.0f;
		 yHipPosition_f = 0.0f;
		 xHipPosition_f = 0.0f;
		 recoilModHealth_f = 0.0f;
		 recoilModJump_f = 0.0f;
		 recoilModCrouch_f = 0.0f;
		 recoilModRun_f = 0.0f;
		 recoilModWalk_f = 0.0f;
		 recoilModScope_f = 0.0f;
		 recoilModIron_f = 0.0f;
		 recoilModHip_f = 0.0f;
		 zRecoilAngSpeedReturn_f = 0.0f;
		 yRecoilAngSpeedReturn_f = 0.0f;
		 xRecoilAngSpeedReturn_f = 0.0f;
		 zRecoilAngSpeedDecel_f = 0.0f;
		 yRecoilAngSpeedDecel_f = 0.0f;
		 xRecoilAngSpeedDecel_f = 0.0f;
		 zRecoilAngSpeedMax_f = 0.0f;
		 zRecoilAngSpeedMin_f = 0.0f;
		 yRecoilAngSpeedMax_f = 0.0f;
		 yRecoilAngSpeedMin_f = 0.0f;
		 xRecoilAngSpeedMax_f = 0.0f;
		 xRecoilAngSpeedMin_f = 0.0f;
		 recoilLimb = 0;
		 zRecoilSpeedReturn_f = 0.0f;
		 yRecoilSpeedReturn_f = 0.0f;
		 xRecoilSpeedReturn_f = 0.0f;
		 zRecoilSpeedDecel_f = 0.0f;
		 yRecoilSpeedDecel_f = 0.0f;
		 xRecoilSpeedDecel_f = 0.0f;
		 zRecoilSpeedMax_f = 0.0f;
		 zRecoilSpeedMin_f = 0.0f;
		 yRecoilSpeedMax_f = 0.0f;
		 yRecoilSpeedMin_f = 0.0f;
		 xRecoilSpeedMax_f = 0.0f;
		 xRecoilSpeedMin_f = 0.0f;
		 muzzleAccModHealth_f = 0.0f;
		 muzzleAccModJump_f = 0.0f;
		 muzzleAccModCrouch_f = 0.0f;
		 muzzleAccModRun_f = 0.0f;
		 muzzleAccModWalk_f = 0.0f;
		 muzzleAccModScope_f = 0.0f;
		 muzzleAccModIron_f = 0.0f;
		 muzzleAccModHip_f = 0.0f;
		 zMuzzleAccSpeed_f = 0.0f;
		 yMuzzleAccSpeed_f = 0.0f;
		 xMuzzleAccSpeed_f = 0.0f;
		 zMuzzleAngleMax_f = 0.0f;
		 zMuzzleAngleMin_f = 0.0f;
		 yMuzzleAngleMax_f = 0.0f;
		 yMuzzleAngleMin_f = 0.0f;
		 xMuzzleAngleMax_f = 0.0f;
		 xMuzzleAngleMin_f = 0.0f;
		 tracerFrequency = 0;
		 projectileCount = 0;
		 projectileType = 0;
		 autoROFDelayAccel = 0;
		 autoROFDelayMin = 0;
		 autoROFDelayMax = 0;
		 autoROFDelay = 0;
		 singleROFDelay = 0;
		 twoStageFireAllowed = 0;
		 autoFireAllowed = 0;
		 burstFireCount = 0;
		 burstFireAllowed = 0;
		 singleFireAllowed = 0;
		 scopeAllowed = 0;
		 ironSightAllowed = 0;
		 hipFireAllowed = 0;
		 scopeTurnSpeedModifier_f = 0.0f;
		 scopeMoveSpeedModifier_f = 0.0f;
		 ironTurnSpeedModifier_f = 0.0f;
		 ironMoveSpeedModifier_f = 0.0f;
		 hipTurnSpeedModifier_f = 0.0f;
		 hipMoveSpeedModifier_f = 0.0f;
		 damageModifier_f = 0.0f;
		 rangeModifier_f = 0.0f;
		 switchAmmoEmptyFlag = 0;
		 holsterAmmoEmptyFlag = 0;
		 reloadAutoFlag = 0;
		 reloadTime = 0;
		 ammoPoolID = 0;
		 ammoPerClip = 0;
		 scopeGraphicType = 0;
		 hudGraphicType = 0;
		 obj = 0;
		 projectileName_s = "";
		 name_s = "";
		 availableFlag = 0;
		 activeFlag = 0;
	}
	// End of Constructor

};


struct weaponSoundType
{
	int sndID;
	int minVolume;
	int maxVolume;
	int minSpeed;
	int maxSpeed;

	// Constructor
	weaponSoundType ( )
	{
		 maxSpeed = 0;
		 minSpeed = 0;
		 maxVolume = 0;
		 minVolume = 0;
		 sndID = 0;
	}
};


struct weaponProjectileBaseType
{
	int activeFlag;
	cstr name_s;
	int mode;

	int baseObj;
	int textureD;
	int textureN;
	int textureS;
	int effectid;
	int noZWrite;
	float baseObjScaleMinX_f;
	float baseObjScaleMaxX_f;
	float baseObjScaleMinY_f;
	float baseObjScaleMaxY_f;
	float baseObjScaleMinZ_f;
	float baseObjScaleMaxZ_f;

	float avoidPlayerPenetration_f;
	int attachToWeaponLimb;

	float xAccuracy_f;
	float yAccuracy_f;

	int life;
	int resultEndOfLife;

	float damage_f;
	float damageRandom_f;
	float range_f;
	float fullDamageRange_f;
	float damageRadius_f;

	int resultIfDamaged;
	int resultIfDamagedDelay;
	int resultOnInterval;
	int resultOnIntervalTime;
	int resultBounce;

	float speedMin_f;
	float speedMax_f;
	float speedAngMinX_f;
	float speedAngMaxX_f;
	float speedAngMinY_f;
	float speedAngMaxY_f;
	float speedAngMinZ_f;
	float speedAngMaxZ_f;
	float speedTurnMinX_f;
	float speedTurnMaxX_f;
	float speedTurnMinY_f;
	float speedTurnMaxY_f;
	float speedTurnMinZ_f;
	float speedTurnMaxZ_f;

	int usingRealPhysics;
	float gravityModifier_f;
	float airFrictionModifier_f;
	float groundFrictionModifier_f;
	float thrustModifier_f;

	int bounceFlag;
	float bounceModifier_f;
	int bounceResult;

	int sound;
	int soundLoopFlag;
	int soundDopplerFlag;
	int soundDopplerBaseSpeed;
	int soundInterval;
	int soundDeath;

	int particleType;
	cstr particleName;
	int particleImageID;
	int explosionType;
	cstr explosionName;
	int explosionImageID;
	int explosionLightFlag;
	cstr explosionSmokeName;
	int explosionSmokeImageID;
	int explosionSparksCount;
	int projectileEventType;

	int thrustTime;
	int thrustDelay;
	weaponParticleEmitterBaseType thrustParticle1;
	weaponParticleEmitterBaseType thrustParticle2;
	weaponParticleEmitterBaseType particle1;
	weaponParticleEmitterBaseType particle2;

	int overridespotlighting;

	// Constructor
	weaponProjectileBaseType ( )
	{
		 overridespotlighting = 0;
		 thrustDelay = 0;
		 thrustTime = 0;
		 projectileEventType = 0;
		 explosionSmokeName = "";
		 explosionLightFlag = 0;
		 explosionSmokeImageID = 0;
		 explosionSparksCount = 0;
		 explosionImageID = 0;
		 explosionName = "";
		 explosionType = 0;
		 particleImageID = 0;
		 particleName = "";
		 particleType = 0;
		 soundDeath = 0;
		 soundInterval = 0;
		 soundDopplerBaseSpeed = 0;
		 soundDopplerFlag = 0;
		 soundLoopFlag = 0;
		 sound = 0;
		 bounceResult = 0;
		 bounceModifier_f = 0.0f;
		 bounceFlag = 0;
		 thrustModifier_f = 0.0f;
		 groundFrictionModifier_f = 0.0f;
		 airFrictionModifier_f = 0.0f;
		 gravityModifier_f = 0.0f;
		 usingRealPhysics = 0;
		 speedTurnMaxZ_f = 0.0f;
		 speedTurnMinZ_f = 0.0f;
		 speedTurnMaxY_f = 0.0f;
		 speedTurnMinY_f = 0.0f;
		 speedTurnMaxX_f = 0.0f;
		 speedTurnMinX_f = 0.0f;
		 speedAngMaxZ_f = 0.0f;
		 speedAngMinZ_f = 0.0f;
		 speedAngMaxY_f = 0.0f;
		 speedAngMinY_f = 0.0f;
		 speedAngMaxX_f = 0.0f;
		 speedAngMinX_f = 0.0f;
		 speedMax_f = 0.0f;
		 speedMin_f = 0.0f;
		 resultBounce = 0;
		 resultOnIntervalTime = 0;
		 resultOnInterval = 0;
		 resultIfDamagedDelay = 0;
		 resultIfDamaged = 0;
		 damageRadius_f = 0.0f;
		 fullDamageRange_f = 0.0f;
		 range_f = 0.0f;
		 damageRandom_f = 0.0f;
		 damage_f = 0.0f;
		 resultEndOfLife = 0;
		 life = 0;
		 yAccuracy_f = 0.0f;
		 xAccuracy_f = 0.0f;
		 attachToWeaponLimb = 0;
		 avoidPlayerPenetration_f = 0.0f;
		 baseObjScaleMaxZ_f = 0.0f;
		 baseObjScaleMinZ_f = 0.0f;
		 baseObjScaleMaxY_f = 0.0f;
		 baseObjScaleMinY_f = 0.0f;
		 baseObjScaleMaxX_f = 0.0f;
		 baseObjScaleMinX_f = 0.0f;
		 effectid = 0;
		 textureS = 0;
		 textureN = 0;
		 textureD = 0;
		 baseObj = 0;
		 mode = 0;
		 name_s = "";
		 activeFlag = 0;
	}
	// End of Constructor

};


struct weaponProjectileType
{

	int baseType;
	int activeFlag;
	int obj;
	int sourceEntity;
	int tracerFlag;
	int tracerObj;
	int sound;
	int soundDeath;
	float soundDistFromPlayer_f;
	int createStamp;
	int resultIntervalStamp;
	int soundIntervalStamp;
	weaponParticleEmitterType thrustParticle1;
	weaponParticleEmitterType thrustParticle2;
	weaponParticleEmitterType particle1;
	weaponParticleEmitterType particle2;
	int tempEmitter;

	int usespotlighting;

	float startXPos_f;
	float startYPos_f;
	float startZPos_f;
	float xPos_f;
	float yPos_f;
	float zPos_f;
	float xOldPos_f;
	float yOldPos_f;
	float zOldPos_f;
	float xAng_f;
	float yAng_f;
	float zAng_f;
	float xTurn_f;
	float yTurn_f;
	float zTurn_f;
	float xSpeed_f;
	float ySpeed_f;
	float zSpeed_f;
	float xAngSpeed_f;
	float yAngSpeed_f;
	float zAngSpeed_f;
	float xTurnSpeed_f;
	float yTurnSpeed_f;
	float zTurnSpeed_f;
	float acceleration_f;


	// Constructor
	weaponProjectileType ( )
	{
		 acceleration_f = 0.0f;
		 zTurnSpeed_f = 0.0f;
		 yTurnSpeed_f = 0.0f;
		 xTurnSpeed_f = 0.0f;
		 zAngSpeed_f = 0.0f;
		 yAngSpeed_f = 0.0f;
		 xAngSpeed_f = 0.0f;
		 zSpeed_f = 0.0f;
		 ySpeed_f = 0.0f;
		 xSpeed_f = 0.0f;
		 zTurn_f = 0.0f;
		 yTurn_f = 0.0f;
		 xTurn_f = 0.0f;
		 zAng_f = 0.0f;
		 yAng_f = 0.0f;
		 xAng_f = 0.0f;
		 zOldPos_f = 0.0f;
		 yOldPos_f = 0.0f;
		 xOldPos_f = 0.0f;
		 zPos_f = 0.0f;
		 yPos_f = 0.0f;
		 xPos_f = 0.0f;
		 startZPos_f = 0.0f;
		 startYPos_f = 0.0f;
		 startXPos_f = 0.0f;
		 usespotlighting = 0;
		 tempEmitter = 0;
		 soundIntervalStamp = 0;
		 resultIntervalStamp = 0;
		 createStamp = 0;
		 soundDistFromPlayer_f = 0.0f;
		 soundDeath = 0;
		 sound = 0;
		 tracerObj = 0;
		 tracerFlag = 0;
		 sourceEntity = 0;
		 obj = 0;
		 activeFlag = 0;
		 baseType = 0;
	}
	// End of Constructor

};


struct weaponAnimationType
{

	int startFrame;
	int endFrame;
	int speed;
	int loopFlag;


	// Constructor
	weaponAnimationType ( )
	{
		 loopFlag = 0;
		 speed = 0;
		 endFrame = 0;
		 startFrame = 0;
	}
	// End of Constructor

};

//  Construction Kit type structures
struct conkitfeaturetype
{
	int featureType;
	int objTrimmed;
	int obj;
	int objEnd;
	int objAnimation;
	int objNonShaded;
	int animationStamp;
	int buildingIndex;
	int gridAng;
	int gridX;
	int gridY;
	int gridZ;
	int state;
	int physicsEnabled;

	// Constructor
	conkitfeaturetype ( )
	{
		 physicsEnabled = 0;
		 state = 0;
		 gridZ = 0;
		 gridY = 0;
		 gridX = 0;
		 gridAng = 0;
		 buildingIndex = 0;
		 animationStamp = 0;
		 objNonShaded = 0;
		 objAnimation = 0;
		 objEnd = 0;
		 obj = 0;
		 objTrimmed = 0;
		 featureType = 0;
	}
	// End of Constructor

};

struct conkitfeaturebasetype
{
	int obj;
	int texture_d;
	int texture_n;
	int texture_s;
	int texture_i;
	int texture_o;
	int objEnd;
	int objAnimation;
	int objCue;
	int imgSprIcon;
	int attributeClass;

	// Constructor
	conkitfeaturebasetype ( )
	{
		 attributeClass = 0;
		 imgSprIcon = 0;
		 objCue = 0;
		 objAnimation = 0;
		 objEnd = 0;
		 texture_o = 0;
		 texture_i = 0;
		 texture_s = 0;
		 texture_n = 0;
		 texture_d = 0;
		 obj = 0;
	}
	// End of Constructor

};

struct conkitheldfeaturetype
{
	int featureJustPlaced;
	int featureIndex;
	int menuOpen;
	int menuJustChanged;

	// Constructor
	conkitheldfeaturetype ( )
	{
		 menuJustChanged = 0;
		 menuOpen = 0;
		 featureIndex = 0;
		 featureJustPlaced = 0;
	}
	// End of Constructor

};

struct conkitgridpostype
{
	int x;
	int y;
	int z;

	// Constructor
	conkitgridpostype ( )
	{
		 z = 0;
		 y = 0;
		 x = 0;
	}
	// End of Constructor

};

struct conkitworldpostype
{
	float x_f;
	float y_f;
	float z_f;

	// Constructor
	conkitworldpostype ( )
	{
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
	}
	// End of Constructor

};

struct conkitmakeconsolidatedtype
{
	int obj;
	int buildingSite;

	// Constructor
	conkitmakeconsolidatedtype ( )
	{
		 buildingSite = 0;
		 obj = 0;
	}
	// End of Constructor

};

struct conkitmakebuildingsitetype
{
	int active;
	int saveArrayIndex;
	float ang_f;
	float x_f;
	float y_f;
	float z_f;
	int cursorGridX;
	int cursorGridZ;

	// Constructor
	conkitmakebuildingsitetype ( )
	{
		 cursorGridZ = 0;
		 cursorGridX = 0;
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
		 ang_f = 0.0f;
		 saveArrayIndex = 0;
		 active = 0;
	}
	// End of Constructor

};

struct conkitmakeentityparttype
{
	int entityIndex;
	int texStyle;
	int obj;
	int objNonShaded;
	int objTrimmed;
	int objSource;

	// Constructor
	conkitmakeentityparttype ( )
	{
		 objSource = 0;
		 objTrimmed = 0;
		 objNonShaded = 0;
		 obj = 0;
		 texStyle = 0;
		 entityIndex = 0;
	}
	// End of Constructor

};

struct conkitmakestyletype
{
	int imgSprIcon;
	int objTileFloor;
	int objTileCeiling;
	int objPost1;
	int objPost2;
	int objPost3;
	int objPost4;
	int objWallInt;
	int objWallExt;
	int objTileDiagFloor;
	int objTileDiagCeiling;
	int objWallDiagInt;
	int objWallDiagExt;
	int objEdgeStraight;
	int objEdgeDiagonal;
	int objEdgeJoin;
	int objEdgeDiagonalJoin;
	int objEdgeCorner45;
	int objEdgeCorner45Left;
	int objEdgeCorner90;
	int objEdgeCorner135;
	int objEdgeCorner135Left;
	int objEdgeCorner225;
	int objEdgeCorner225Left;
	int objEdgeCorner270;
	int objEdgeCorner315;
	int objEdgeCorner315Left;
	int objEdgeCornerDiag;
	int objCueTile;
	int objCuePost;
	int objCueWall;
	int objCueTileDiag;
	int objCueWallDiag;
	int objCueEdgeStraight;
	int objCueEdgeDiagonal;
	int objCueEdgeJoin;
	int objCueEdgeDiagonalJoin;
	int objCueEdgeCorner45;
	int objCueEdgeCorner45Left;
	int objCueEdgeCorner90;
	int objCueEdgeCorner135;
	int objCueEdgeCorner135Left;
	int objCueEdgeCorner225;
	int objCueEdgeCorner225Left;
	int objCueEdgeCorner270;
	int objCueEdgeCorner315;
	int objCueEdgeCorner315Left;
	int objCueEdgeCornerDiag;

	// Constructor
	conkitmakestyletype ( )
	{
		 objCueEdgeCornerDiag = 0;
		 objCueEdgeCorner315Left = 0;
		 objCueEdgeCorner315 = 0;
		 objCueEdgeCorner270 = 0;
		 objCueEdgeCorner225Left = 0;
		 objCueEdgeCorner225 = 0;
		 objCueEdgeCorner135Left = 0;
		 objCueEdgeCorner135 = 0;
		 objCueEdgeCorner90 = 0;
		 objCueEdgeCorner45Left = 0;
		 objCueEdgeCorner45 = 0;
		 objCueEdgeDiagonalJoin = 0;
		 objCueEdgeJoin = 0;
		 objCueEdgeDiagonal = 0;
		 objCueEdgeStraight = 0;
		 objCueWallDiag = 0;
		 objCueTileDiag = 0;
		 objCueWall = 0;
		 objCuePost = 0;
		 objCueTile = 0;
		 objEdgeCornerDiag = 0;
		 objEdgeCorner315Left = 0;
		 objEdgeCorner315 = 0;
		 objEdgeCorner270 = 0;
		 objEdgeCorner225Left = 0;
		 objEdgeCorner225 = 0;
		 objEdgeCorner135Left = 0;
		 objEdgeCorner135 = 0;
		 objEdgeCorner90 = 0;
		 objEdgeCorner45Left = 0;
		 objEdgeCorner45 = 0;
		 objEdgeDiagonalJoin = 0;
		 objEdgeJoin = 0;
		 objEdgeDiagonal = 0;
		 objEdgeStraight = 0;
		 objWallDiagExt = 0;
		 objWallDiagInt = 0;
		 objTileDiagCeiling = 0;
		 objTileDiagFloor = 0;
		 objWallExt = 0;
		 objWallInt = 0;
		 objPost4 = 0;
		 objPost3 = 0;
		 objPost2 = 0;
		 objPost1 = 0;
		 objTileCeiling = 0;
		 objTileFloor = 0;
		 imgSprIcon = 0;
	}
	// End of Constructor

};

struct conkitmakeentitytype
{
	int buildingIndex;
	int saveArrayIndex;
	int entType;
	int entStyle;
	int autoAdded;
	int gridAng;
	int gridX;
	int gridY;
	int gridZ;
	int physicsEnabled;

	// Constructor
	conkitmakeentitytype ( )
	{
		 physicsEnabled = 0;
		 gridZ = 0;
		 gridY = 0;
		 gridX = 0;
		 gridAng = 0;
		 autoAdded = 0;
		 entStyle = 0;
		 entType = 0;
		 saveArrayIndex = 0;
		 buildingIndex = 0;
	}
	// End of Constructor

};

struct conkitmakeheldobjecttype
{
	int obj;

	// Constructor
	conkitmakeheldobjecttype ( )
	{
		 obj = 0;
	}
	// End of Constructor

};

struct conkitmakemenutype
{
	int mode;
	int imgSprFrame;
	int imgSprSubFrame;
	int imgSprTopButton;
	int imgSprTopButtonSel;
	int imgSprBigButton;
	int imgSprSmallButton;

	// Constructor
	conkitmakemenutype ( )
	{
		 imgSprSmallButton = 0;
		 imgSprBigButton = 0;
		 imgSprTopButtonSel = 0;
		 imgSprTopButton = 0;
		 imgSprSubFrame = 0;
		 imgSprFrame = 0;
		 mode = 0;
	}
	// End of Constructor

};

struct conkitmakematerialtype
{
	int imgTextureD;
	int imgTextureN;
	int imgTextureS;
	int imgTextureI;
	int imgTextureO;
	int imgSprIcon;

	// Constructor
	conkitmakematerialtype ( )
	{
		 imgSprIcon = 0;
		 imgTextureO = 0;
		 imgTextureI = 0;
		 imgTextureS = 0;
		 imgTextureN = 0;
		 imgTextureD = 0;
	}
	// End of Constructor

};

struct conkitmakeheldtype
{
	int numObjects;
	int heldType;
	int heldStyle;
	int newHeldJustSelected;
	int newStyleJustSelected;
	float ang_f;
	float xOffset_f;
	float yOffset_f;
	float zOffset_f;
	int rotationKey;
	int menuOpen;
	int menuJustChanged;
	int imgSprIconTile;
	int imgSprIconTileDiag;
	int imgSprIconWall;
	int imgSprIconWallDiag;
	int imgSprIconPost;
	int imgSprIconEdge;
	int imgSprIconEdgeDiag;
	int imgSprIconEdgeJoin;
	int imgSprIconEdgeJoinDiag;
	int imgSprIconEdgeCorn45;
	int imgSprIconEdgeCorn45Left;
	int imgSprIconEdgeCorn90;
	int imgSprIconEdgeCorn135;
	int imgSprIconEdgeCorn135Left;
	int imgSprIconEdgeCorn225;
	int imgSprIconEdgeCorn225Left;
	int imgSprIconEdgeCorn270;
	int imgSprIconEdgeCorn315;
	int imgSprIconEdgeCorn315Left;
	int imgSprIconEdgeCornDiag;

	// Constructor
	conkitmakeheldtype ( )
	{
		 imgSprIconEdgeCornDiag = 0;
		 imgSprIconEdgeCorn315Left = 0;
		 imgSprIconEdgeCorn315 = 0;
		 imgSprIconEdgeCorn270 = 0;
		 imgSprIconEdgeCorn225Left = 0;
		 imgSprIconEdgeCorn225 = 0;
		 imgSprIconEdgeCorn135Left = 0;
		 imgSprIconEdgeCorn135 = 0;
		 imgSprIconEdgeCorn90 = 0;
		 imgSprIconEdgeCorn45Left = 0;
		 imgSprIconEdgeCorn45 = 0;
		 imgSprIconEdgeJoinDiag = 0;
		 imgSprIconEdgeJoin = 0;
		 imgSprIconEdgeDiag = 0;
		 imgSprIconEdge = 0;
		 imgSprIconPost = 0;
		 imgSprIconWallDiag = 0;
		 imgSprIconWall = 0;
		 imgSprIconTileDiag = 0;
		 imgSprIconTile = 0;
		 menuJustChanged = 0;
		 menuOpen = 0;
		 rotationKey = 0;
		 zOffset_f = 0.0f;
		 yOffset_f = 0.0f;
		 xOffset_f = 0.0f;
		 ang_f = 0.0f;
		 newStyleJustSelected = 0;
		 newHeldJustSelected = 0;
		 heldStyle = 0;
		 heldType = 0;
		 numObjects = 0;
	}
	// End of Constructor

};

struct conkitmakegridtype
{
	int obj;
	int objMarker;
	int imgMarker;
	int layer;
	int justChangedLayer;
	int minX;
	int maxX;
	int minZ;
	int maxZ;

	// Constructor
	conkitmakegridtype ( )
	{
		 maxZ = 0;
		 minZ = 0;
		 maxX = 0;
		 minX = 0;
		 justChangedLayer = 0;
		 layer = 0;
		 imgMarker = 0;
		 objMarker = 0;
		 obj = 0;
	}
	// End of Constructor

};

struct conkitmakepainttype
{
	int partJustPainted;
	int material;
	int materialCount;
	int materialJustChanged;
	int menuOpen;
	int menuJustChanged;

	// Constructor
	conkitmakepainttype ( )
	{
		 menuJustChanged = 0;
		 menuOpen = 0;
		 materialJustChanged = 0;
		 materialCount = 0;
		 material = 0;
		 partJustPainted = 0;
	}
	// End of Constructor

};

struct conkitmaketype
{
	conkitheldfeaturetype feature;
	conkitmakeheldtype held;
	conkitmakegridtype grid;
	conkitmakepainttype paint;
	conkitmakemenutype menu;
	int numStyles;
	int numFeatureBases;
	int activeBuildingSite;
	int buildingSiteJustSelected;
	int entityJustDeleted;
	int initialised;
	int entityEffectID;
	int mode;
	int lastMouse;
	int mouseclicked;
	int mouseClickStamp;
	int lastKey;
	int keyPressed;
	int imgBlankTexture;
	int imgGridTexture;
	int imgSprDeleteCrossHair;
	int imgSprPaintCrossHair;
	int imgSprCursor;
	float pointerx_f;
	float pointery_f;
	float camvecx_f;
	float camvecy_f;
	float camvecz_f;
	int lateralMove;
	int lateralMoveJustChanged;
	float oldCamX_f;
	float oldCamY_f;
	float oldCamZ_f;

	// Constructor
	conkitmaketype ( )
	{
		 oldCamZ_f = 0.0f;
		 oldCamY_f = 0.0f;
		 oldCamX_f = 0.0f;
		 lateralMoveJustChanged = 0;
		 lateralMove = 0;
		 camvecz_f = 0.0f;
		 camvecy_f = 0.0f;
		 camvecx_f = 0.0f;
		 pointery_f = 0.0f;
		 pointerx_f = 0.0f;
		 imgSprCursor = 0;
		 imgSprPaintCrossHair = 0;
		 imgSprDeleteCrossHair = 0;
		 imgGridTexture = 0;
		 imgBlankTexture = 0;
		 keyPressed = 0;
		 lastKey = 0;
		 mouseClickStamp = 0;
		 mouseclicked = 0;
		 lastMouse = 0;
		 mode = 0;
		 entityEffectID = 0;
		 initialised = 0;
		 entityJustDeleted = 0;
		 buildingSiteJustSelected = 0;
		 activeBuildingSite = 0;
		 numFeatureBases = 0;
		 numStyles = 0;
	}
	// End of Constructor

};

struct conkitedittype
{
	int conkitkeypressed;
	int entityeditmodepress;
	int entityeditgrabbed;
	int entityeditaddpress;
	int entityeditstaticmode;
	int entityeditposoffground;
	int entityindex;
	int entityindexmax;
	int entitytakenfromlevel;
	int entityhighlighted;
	float scalex;
	float scaley;
	float scalez;
	float rotx;
	float roty;
	float rotz;
	float posoffx;
	float posoffy;
	float posoffz;
	float entityheightadjust;
	int insideentity;
	int timeDelay;

	// Constructor
	conkitedittype ( )
	{
		 timeDelay = 0;
		 insideentity = 0;
		 entityheightadjust = 0.0f;
		 posoffz = 0.0f;
		 posoffy = 0.0f;
		 posoffx = 0.0f;
		 rotz = 0.0f;
		 roty = 0.0f;
		 rotx = 0.0f;
		 scalez = 0.0f;
		 scaley = 0.0f;
		 scalex = 0.0f;
		 entityhighlighted = 0;
		 entitytakenfromlevel = 0;
		 entityindexmax = 0;
		 entityindex = 0;
		 entityeditposoffground = 0;
		 entityeditstaticmode = 0;
		 entityeditaddpress = 0;
		 entityeditgrabbed = 0;
		 entityeditmodepress = 0;
		 conkitkeypressed = 0;
	}
	// End of Constructor

};

struct conkittype
{
	int editmodeactive;
	int entityeditmode;
	int objectstartnumber;
	int imagestartnumber;
	int cooldown;
	int modified;
	conkitedittype edit;
	conkitmaketype make;
	int forceaction;

	// Constructor
	conkittype ( )
	{
		 forceaction = 0;
		 modified = 0;
		 cooldown = 0;
		 imagestartnumber = 0;
		 objectstartnumber = 0;
		 entityeditmode = 0;
		 editmodeactive = 0;
	}
	// End of Constructor

};

//  +0 = projected object to mark Floor (  position of edit coordinate )
//  +1 = semi-transparent entity cursor object (to scroll through)

//  Global Lighting Structure
struct lightingtype
{
	int override;

	// Constructor
	lightingtype ( )
	{
		 override = 0;
	}
	// End of Constructor

};

//  LUA Global Vars
struct luaglobaltype
{
	int gamestatechange;
	int setanim;
	int loopmode;
	int setanimstart;
	int setanimfinish;
	int lastsoundnumber;
	int lastvideonumber;
	cstr scriptprompt_s;
	DWORD scriptprompttime;
	int scriptprompttextsize;
	char scriptprompt3dtext[256];
	DWORD scriptprompt3dtime;
	float scriptprompt3dX;
	float scriptprompt3dY;
	float scriptprompt3dZ;
	float scriptprompt3dAY;

	// Constructor
	luaglobaltype ( )
	{
		 scriptprompttextsize = 0;
		 scriptprompttime = 0;
		 scriptprompt_s = "";
		 lastvideonumber = 0;
		 lastsoundnumber = 0;
		 setanimfinish = 0;
		 setanimstart = 0;
		 loopmode = 0;
		 setanim = 0;
		 gamestatechange = 0;
	}
	// End of Constructor

};

//  Widget type
struct widgettype
{
	int mclickpress;
	int protoineffect;
	int propertybuttonselected;
	int duplicatebuttonselected;
	int deletebuttonselected;
	int oldActiveObject;
	int pickedEntityIndex;
	int pickedObject;
	float offsetX;
	float offsetY;
	float offsetZ;
	float originalX;
	float originalY;
	float originalZ;
	int mode;
	int grabbed;
	int oldScanCode;
	float distance;
	int activeObject;
	int widgetXObj;
	int widgetYObj;
	int widgetZObj;
	int widgetXYObj;
	int widgetXZObj;
	int widgetYZObj;
	int widgetXRotObj;
	int widgetYRotObj;
	int widgetZRotObj;
	int widgetXScaleObj;
	int widgetYScaleObj;
	int widgetZScaleObj;
	int widgetXYZScaleObj;
	int widgetXColObj;
	int widgetYColObj;
	int widgetZColObj;
	int widgetPOSObj;
	int widgetROTObj;
	int widgetSCLObj;
	int widgetPRPObj;
	int widgetDUPObj;
	int widgetDELObj;
	int widgetLCKObj;
	int widgetMAXObj;
	int widgetPlaneObj;
	int pickedSection;
	int oldMouseClick;
	int imagestart;
	float offsetx;
	float offsety;
	float offsetz;

	// Constructor
	widgettype ( )
	{
		 offsetz = 0.0f;
		 offsety = 0.0f;
		 offsetx = 0.0f;
		 imagestart = 0;
		 oldMouseClick = 0;
		 pickedSection = 0;
		 widgetPlaneObj = 0;
		 widgetMAXObj = 0;
		 widgetLCKObj = 0;
		 widgetDELObj = 0;
		 widgetDUPObj = 0;
		 widgetPRPObj = 0;
		 widgetSCLObj = 0;
		 widgetROTObj = 0;
		 widgetPOSObj = 0;
		 widgetZColObj = 0;
		 widgetYColObj = 0;
		 widgetXColObj = 0;
		 widgetXYZScaleObj = 0;
		 widgetZScaleObj = 0;
		 widgetYScaleObj = 0;
		 widgetXScaleObj = 0;
		 widgetZRotObj = 0;
		 widgetYRotObj = 0;
		 widgetXRotObj = 0;
		 widgetYZObj = 0;
		 widgetXZObj = 0;
		 widgetXYObj = 0;
		 widgetZObj = 0;
		 widgetYObj = 0;
		 widgetXObj = 0;
		 activeObject = 0;
		 distance = 0.0f;
		 oldScanCode = 0;
		 grabbed = 0;
		 mode = 0;
		 originalZ = 0.0f;
		 originalY = 0.0f;
		 originalX = 0.0f;
		 offsetZ = 0.0f;
		 offsetY = 0.0f;
		 offsetX = 0.0f;
		 pickedObject = 0;
		 pickedEntityIndex = 0;
		 oldActiveObject = 0;
		 deletebuttonselected = 0;
		 duplicatebuttonselected = 0;
		 propertybuttonselected = 0;
		 protoineffect = 0;
		 mclickpress = 0;
	}
	// End of Constructor

};

//  Character Kit
struct characterkitttype
{
	int loaded;
	int slidersmenumax;
	int oldMouseClick;
	int objectstart;
	int imagestart;
	int effectforcharacter;
	int effectforcharacterHighlight;
	int effectforAttachments;
	int effectforAttachmentsHighlight;
	int effectforBeard;
	int effectforBeardHighlight;
	int panelsExist;
	int properties1Index;
	cstr body_s;
	int bodyindex;
	int bodyindexloaded;
	cstr head_s;
	int headindex;
	int headindexloaded;
	cstr facialhair_s;
	int facialhairindex;
	int facialhairindexloaded;
	cstr hat_s;
	int hatindex;
	int hatindexloaded;
	int thumbGadgetOn;
	int selected;
	int loadthumbs;
	int picked;
	int skinPickOn;
	int haveSkinColor;
	int haveShirtColor;
	int haveBeardColor;
	int haveTrousersColor;
	int haveShoesColor;
	int haveHatColor;
	float scrollPosition;
	float howManyRows;
	int scrollBarOn;
	float scrollBarOffsetY;
	int inUse;
	int originalHeadIndex;

	// Constructor
	characterkitttype ( )
	{
		 originalHeadIndex = 0;
		 inUse = 0;
		 scrollBarOffsetY = 0.0f;
		 scrollBarOn = 0;
		 howManyRows = 0.0f;
		 scrollPosition = 0.0f;
		 haveHatColor = 0;
		 haveShoesColor = 0;
		 haveTrousersColor = 0;
		 haveBeardColor = 0;
		 haveShirtColor = 0;
		 haveSkinColor = 0;
		 skinPickOn = 0;
		 picked = 0;
		 loadthumbs = 0;
		 selected = 0;
		 thumbGadgetOn = 0;
		 hatindexloaded = 0;
		 hatindex = 0;
		 hat_s = "";
		 facialhairindexloaded = 0;
		 facialhairindex = 0;
		 facialhair_s = "";
		 headindexloaded = 0;
		 headindex = 0;
		 head_s = "";
		 bodyindexloaded = 0;
		 bodyindex = 0;
		 body_s = "";
		 properties1Index = 0;
		 panelsExist = 0;
		 effectforBeardHighlight = 0;
		 effectforBeard = 0;
		 effectforAttachmentsHighlight = 0;
		 effectforAttachments = 0;
		 effectforcharacterHighlight = 0;
		 effectforcharacter = 0;
		 imagestart = 0;
		 objectstart = 0;
		 oldMouseClick = 0;
		 slidersmenumax = 0;
		 loaded = 0;
	}
	// End of Constructor

};

//  Importer structures
struct importertabtype
{
	int x;
	int y;
	cstr label;
	int selected;
	int tabpage;

	// Constructor
	importertabtype ( )
	{
		 tabpage = 0;
		 selected = 0;
		 label = "";
		 y = 0;
		 x = 0;
	}
	// End of Constructor

};

struct importerTexture
{
	int imageID;
	int spriteID;
	int spriteID2;
	cstr fileName;
	cstr originalName;

	// Constructor
	importerTexture ( )
	{
		 originalName = "";
		 fileName = "";
		 spriteID2 = 0;
		 spriteID = 0;
		 imageID = 0;
	}
	// End of Constructor

};

struct importerCollisionType
{
	int object;
	int object2;
	int shapetype;
	float sizex;
	float sizey;
	float sizez;
	float offx;
	float offy;
	float offz;
	float rotx;
	float roty;
	float rotz;

	// Constructor
	importerCollisionType ( )
	{
		 rotz = 0.0f;
		 roty = 0.0f;
		 rotx = 0.0f;
		 offz = 0.0f;
		 offy = 0.0f;
		 offx = 0.0f;
		 sizez = 0.0f;
		 sizey = 0.0f;
		 sizex = 0.0f;
		 shapetype = 0;
		 object2 = 0;
		 object = 0;
	}
	// End of Constructor

};

struct importerFPEtype
{
//  `;header

	cstr desc;
//  `;visualinfo

	cstr textured;
	cstr effect;
	cstr castshadow;
	cstr transparency;
//  `;orientation

	cstr model;
	cstr offx;
	cstr offy;
	cstr offz;
	cstr rotx;
	cstr roty;
	cstr rotz;
	cstr scale;
	cstr collisionmode;
	cstr defaultstatic;
	cstr cullmode;
	cstr materialindex;
	cstr matrixmode;

//  `;CHARACTER

//  `;identity details

	cstr ischaracter;
	cstr hasweapon;
	cstr isobjective;
	cstr cantakeweapon;

//  `;statistics

	cstr strength;
	cstr explodable;
	cstr debrisshape;

//  `;ai

	cstr aiinit;
	cstr aimain;
	cstr aidestroy;

//  `;spawn

	cstr spawnmax;
	cstr spawndelay;
	cstr spawnqty;

//  `;CHARACTER

//  `;anim

	cstr animmax;
	cstr anim0;
	cstr playanimineditor;


	// Constructor
	importerFPEtype ( )
	{
		 playanimineditor = "";
		 anim0 = "";
		 animmax = "";
		 spawnqty = "";
		 spawndelay = "";
		 spawnmax = "";
		 aidestroy = "";
		 aimain = "";
		 aiinit = "";
		 debrisshape = "";
		 explodable = "";
		 strength = "";
		 cantakeweapon = "";
		 isobjective = "";
		 hasweapon = "";
		 ischaracter = "";
		 materialindex = "";
		 matrixmode = "";
		 defaultstatic = "";
		 cullmode = "";
		 collisionmode = "";
		 scale = "";
		 rotz = "";
		 roty = "";
		 rotx = "";
		 offz = "";
		 offy = "";
		 offx = "";
		 model = "";
		 transparency = "";
		 castshadow = "";
		 effect = "";
		 textured = "";
		 desc = "";
	}
	// End of Constructor

};

//  Importer type structure
struct importertype
{
// `stop built in media being overwritten

	int dropDownListNumber;
	int oldTime;
	int slidersmenumax;
	int fpeIsMainFile;
	int isProtected;
	int unknownFPELineCount;
	float scaleMulti;
	int showCollisionOnly;
	int oldShowCollision;
	float originalObjectSize;
	int changeSizeKeyDown;
	int reload;
	int importerActive;
	int selectedCollisionObject;
	int selectedMarker;
	float markerPickX;
	float markerPickY;
	float markerPickZ;
	float lastPickX;
	float lastPickY;
	float lastPickZ;
	int helpSprite;
	int helpSprite2;
	int helpSprite3;
	int helpSprite4;
	int helpFade;
	int helpShow;
	int cancel;
	int cancelCount;
	float markerPickDistance;
	int collisionObjectMode;
	cstr startDir;
	int buttonPressedCount;
	int shaderFileCount;
	int scriptFileCount;
	int objectnumber;
	int objectnumberpreeffectcopy;
	int blankTexture;
	importerFPEtype objectFPE;
	float objectTopOffset;
	float objectBottomOffset;
	float objectCenterOffset;
	float objectLeftOffset;
	float objectFrontOffset;
	float objectScaleForEditing;
	int collisionShapeCount;
	int dummyCharacterObjectNumber;
	cstr objectFilename;
	cstr objectFilenameFPE;
	cstr objectFilenameExtension;
	cstr objectFileOriginalPath;
	cstr tFPESaveName;
	int panelsExist;
	int loaded;
	int objectRotateMode;
	float objectAngleY;
	float objectAngleY2;
	float oldScale;
	int showScaleChange;
	int MouseX;
	int MouseY;
	int oldMouseClick;
	int oldTabMode;
	int properties1Index;
	int properties2Index;
	int properties3Index;
	int properties4Index;
	int mouseMoveSnap;
	cstr message;
	cstr viewMessage;
	float cameraheight;
	float originalcameraheight;
	float lastcameraheightforshift;
	float camerazoom;

	// Constructor
	importertype ( )
	{
		 viewMessage = "";
		 message = "";
		 mouseMoveSnap = 0;
		 properties4Index = 0;
		 properties3Index = 0;
		 properties2Index = 0;
		 properties1Index = 0;
		 oldTabMode = 0;
		 oldMouseClick = 0;
		 MouseY = 0;
		 MouseX = 0;
		 showScaleChange = 0;
		 oldScale = 0.0f;
		 objectAngleY2 = 0.0f;
		 objectAngleY = 0.0f;
		 objectRotateMode = 0;
		 loaded = 0;
		 panelsExist = 0;
		 tFPESaveName = "";
		 objectFileOriginalPath = "";
		 objectFilenameExtension = "";
		 objectFilenameFPE = "";
		 objectFilename = "";
		 dummyCharacterObjectNumber = 0;
		 collisionShapeCount = 0;
		 objectScaleForEditing = 0.0f;
		 objectFrontOffset = 0.0f;
		 objectLeftOffset = 0.0f;
		 objectCenterOffset = 0.0f;
		 objectBottomOffset = 0.0f;
		 objectTopOffset = 0.0f;
		 blankTexture = 0;
		 objectnumber = 0;
		 scriptFileCount = 0;
		 shaderFileCount = 0;
		 buttonPressedCount = 0;
		 startDir = "";
		 collisionObjectMode = 0;
		 markerPickDistance = 0.0f;
		 cancelCount = 0;
		 cancel = 0;
		 helpShow = 0;
		 helpFade = 0;
		 helpSprite4 = 0;
		 helpSprite3 = 0;
		 helpSprite2 = 0;
		 helpSprite = 0;
		 lastPickZ = 0.0f;
		 lastPickY = 0.0f;
		 lastPickX = 0.0f;
		 markerPickZ = 0.0f;
		 markerPickY = 0.0f;
		 markerPickX = 0.0f;
		 selectedMarker = 0;
		 selectedCollisionObject = 0;
		 importerActive = 0;
		 reload = 0;
		 changeSizeKeyDown = 0;
		 originalObjectSize = 0.0f;
		 oldShowCollision = 0;
		 showCollisionOnly = 0;
		 scaleMulti = 0.0f;
		 unknownFPELineCount = 0;
		 isProtected = 0;
		 fpeIsMainFile = 0;
		 slidersmenumax = 0;
		 oldTime = 0;
		 dropDownListNumber = 0;
	}
	// End of Constructor

};

// Easy Building Editor Structure
struct ebeglobaltype
{
	int on;
	int active;
	int entityelementindex;
	int lastentityelementindex;
	bool bReleaseMouseFirst;
	int iWrittenCubeData;

	ebeglobaltype ( )
	{
		on = 0;
		active = 0;
		entityelementindex = 0;
		lastentityelementindex = 0;
		bReleaseMouseFirst = false;
		iWrittenCubeData = 0;
	}
};

//  Lightmapper Structure
struct lightmappertype
{
	cstr lmpath_s;
	cstr lmobjectfile_s;
	int onlyloadstaticentitiesduringlightmapper;

	// Constructor
	lightmappertype ( )
	{
		 onlyloadstaticentitiesduringlightmapper = 0;
		 lmobjectfile_s = "";
		 lmpath_s = "";
	}
	// End of Constructor

};


//  Lightmapper data structure
struct lmsceneobjtype
{
	int startobj;
	int finishobj;
	int bankindex;
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float sx;
	float sy;
	float sz;
	int lmindex;
	int lmvalid;
	int needsaving;
	int includerotandscale;
	int reverseframes;

	// Constructor
	lmsceneobjtype ( )
	{
		 reverseframes = 0;
		 includerotandscale = 0;
		 needsaving = 0;
		 lmvalid = 0;
		 lmindex = 0;
		 sz = 0.0f;
		 sy = 0.0f;
		 sx = 0.0f;
		 rz = 0.0f;
		 ry = 0.0f;
		 rx = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 bankindex = 0;
		 finishobj = 0;
		 startobj = 0;
	}
	// End of Constructor

};


//  OBS and TerrainDot structures
struct mooreneighborhoodtype
{
	int mode;
	int startx;
	int startz;
	int startd;
	int currentx;
	int currentz;
	int currentd;
	int currentclockwise;
	int countneighborscan;
	int countlaps;
	float laststretchlen_f;

	// Constructor
	mooreneighborhoodtype ( )
	{
		 laststretchlen_f = 0.0f;
		 countlaps = 0;
		 countneighborscan = 0;
		 currentclockwise = 0;
		 currentd = 0;
		 currentz = 0;
		 currentx = 0;
		 startd = 0;
		 startz = 0;
		 startx = 0;
		 mode = 0;
	}
	// End of Constructor

};

struct obstype
{
	float x;
	float z;

	// Constructor
	obstype ( )
	{
		 z = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

//  UNDO Buffer Structures
struct terrainundotype
{
	int mode;
	int bufferfilled;
	int tux1;
	int tux2;
	int tuz1;
	int tuz2;

	// Constructor
	terrainundotype ( )
	{
		 tuz2 = 0;
		 tuz1 = 0;
		 tux2 = 0;
		 tux1 = 0;
		 bufferfilled = 0;
		 mode = 0;
	}
	// End of Constructor

};


struct entityundopostype
{
	int staticflag;
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float scalex;
	float scaley;
	float scalez;

	// Constructor
	entityundopostype ( )
	{
		 scalez = 0.0f;
		 scaley = 0.0f;
		 scalex = 0.0f;
		 rz = 0.0f;
		 ry = 0.0f;
		 rx = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 staticflag = 0;
	}
	// End of Constructor

};

struct entityundotype
{
	int undoperformed;
	int action;
	int entityindex;
	int bankindex;
	entityundopostype pos;
	entityundopostype posbkup;

	// Constructor
	entityundotype ( )
	{
		 bankindex = 0;
		 entityindex = 0;
		 action = 0;
		 undoperformed = 0;
	}
	// End of Constructor

};

//  HUD Blood Splat & Directional Damage
struct tscreenblood
{
	float x;
	float y;
	int time;
	int scale;
	float alpha;
	int used;
	int image;
	int life;
	float fadeout;
	int fadetime;
	float fadelife;

	// Constructor
	tscreenblood ( )
	{
		 fadelife = 0.0f;
		 fadetime = 0;
		 fadeout = 0.0f;
		 life = 0;
		 image = 0;
		 used = 0;
		 alpha = 0.0f;
		 scale = 0;
		 time = 0;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

struct damagetype
{
	float x;
	float y;
	int time;
	float alpha;
	int used;
	float fadeout;
	int fadetime;
	float fadelife;
	float angle;
	int image;
	int range;
	int life;
	int scale;
	float lx;
	float ly;
	float lz;
	int entity;

	// Constructor
	damagetype ( )
	{
		 entity = 0;
		 lz = 0.0f;
		 ly = 0.0f;
		 lx = 0.0f;
		 scale = 0;
		 life = 0;
		 range = 0;
		 image = 0;
		 angle = 0.0f;
		 fadelife = 0.0f;
		 fadetime = 0;
		 fadeout = 0.0f;
		 used = 0;
		 alpha = 0.0f;
		 time = 0;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};


struct huddamagetype
{
	int bloodstart;
	int bloodtimestart;
	int bloodtimeend;
	int bloodon;
	int damageindicatoron;
	int damagetimestart;
	int damagetimeend;
	int fadespeed;
	int damagetrackshooter;
	int indicator;
	int bloodtotal;
	int maxbloodsplats;
	int immunity;

	// Constructor
	huddamagetype ( )
	{
		 immunity = 0;
		 maxbloodsplats = 0;
		 bloodtotal = 0;
		 indicator = 0;
		 damagetrackshooter = 0;
		 fadespeed = 0;
		 damagetimeend = 0;
		 damagetimestart = 0;
		 damageindicatoron = 0;
		 bloodon = 0;
		 bloodtimeend = 0;
		 bloodtimestart = 0;
		 bloodstart = 0;
	}
	// End of Constructor

};

//  Main game data structure (to avoid globals, place new game globals here)
struct globalstype
{
	int riftmode;
	int occlusionmode;
	int occlusionsize;
	int riftmoderesult;
	int ideinputmode;
	int smoothcamerakeys;
	int memorydetector;
	int hidelowfpswarning;
	int deactivateconkit;
	int disablefreeflight;
	int fulldebugview;
	int enableplrspeedmods;
	int disableweaponjams;
	int showdebugcollisonboxes;
	int hideebe;
	int hidedistantshadows;
	int realshadowresolution;
	int realshadowcascadecount;
	int realshadowcascade[8];

	// Constructor
	globalstype ( )
	{
		 realshadowcascade[0] = 2;
		 realshadowcascade[1] = 8;
		 realshadowcascade[2] = 16;
		 realshadowcascade[3] = 75;
		 realshadowcascade[4] = 100;
		 realshadowcascade[5] = 100;
		 realshadowcascade[6] = 100;
		 realshadowcascade[7] = 100;
		 realshadowcascadecount = 4;
		 realshadowresolution = 1024;
		 hidedistantshadows = 1;
		 hideebe = 0;
		 showdebugcollisonboxes = 0;
		 disableweaponjams = 0;
		 enableplrspeedmods = 0;
		 fulldebugview = 0;
		 disablefreeflight = 0;
		 deactivateconkit = 0;
		 hidelowfpswarning = 0;
		 memorydetector = 0;
		 smoothcamerakeys = 0;
		 ideinputmode = 0;
		 riftmoderesult = 0;
		 occlusionsize = 0;
		 occlusionmode = 0;
		 riftmode = 0;
	}
	// End of Constructor

};

//  Hardware Info Testing structure
struct hardwareinfoglobalstype
{
	int noterrain;
	int nowater;
	int noguns;
	int nolmos;
	int nosky;
	int nophysics;
	int noai;
	int nograss;
	int noentities;
	int togglepress;
	int gotgraphicsinfo;
	cstr card_s;
	cstr dxversion_s;
	cstr monitorrefresh_s;

	// Constructor
	hardwareinfoglobalstype ( )
	{
		 monitorrefresh_s = "";
		 dxversion_s = "";
		 card_s = "";
		 gotgraphicsinfo = 0;
		 togglepress = 0;
		 noentities = 0;
		 nograss = 0;
		 noai = 0;
		 nophysics = 0;
		 nosky = 0;
		 nolmos = 0;
		 noguns = 0;
		 nowater = 0;
		 noterrain = 0;
	}
	// End of Constructor

};

//  Postprocessing globals
struct postprocessingstype
{
	float fadeinvalue_f;
	int fadeinvalueupdate;
	int spinfill;
	float spinfillStartAng_f;

	// Constructor
	postprocessingstype ( )
	{
		 spinfillStartAng_f = 0.0f;
		 spinfill = 0;
		 fadeinvalueupdate = 0;
		 fadeinvalue_f = 0.0f;
	}
	// End of Constructor

};

//  Game Initial Setup globals structure
struct gamesetuptype
{
	int ismapeditormode;
	int shaderrequirecheapshadow;
	int resolution;
	int initialsplashscreen;
	int endsplash;

	// Constructor
	gamesetuptype ( )
	{
		 endsplash = 0;
		 initialsplashscreen = 0;
		 resolution = 0;
		 shaderrequirecheapshadow = 0;
		 ismapeditormode = 0;
	}
	// End of Constructor

};

struct gameperftype
{
	DWORD resttosync;
	DWORD misc;
	DWORD ai;
	DWORD ai1;
	DWORD ai2;
	DWORD ai3;
	DWORD ai4;
	DWORD ai5;
	DWORD ai6;
	DWORD physics;
	DWORD gun;
	DWORD terrain1;
	DWORD terrain2;
	DWORD terrain3;
	DWORD occlusion;
	DWORD postprocessing;
	DWORD synctime;
	DWORD overall;
	int smoothcounter;

	// Constructor
	gameperftype ( )
	{
		 smoothcounter = 0;
		 overall = 0;
		 synctime = 0;
		 postprocessing = 0;
		 occlusion = 0;
		 terrain3 = 0;
		 terrain2 = 0;
		 terrain1 = 0;
		 gun = 0;
		 physics = 0;
		 ai6 = 0;
		 ai5 = 0;
		 ai4 = 0;
		 ai3 = 0;
		 ai2 = 0;
		 ai1 = 0;
		 ai = 0;
		 misc = 0;
		 resttosync = 0;
	}
	// End of Constructor

};

struct gametype
{
	int gameisexe;
	int ignoretitle;
	gamesetuptype set;
	gameperftype perf;
	char pAdvanceWarningOfLevelFilename[256];
	char pSwitchToPage[256];
	char pSwitchToLastPage[256];
	int allowfragmentation;
	int masterloop;
	int titleloop;
	int levelloop;
	int level;
	int levelplrstatsetup;
	int levelmax;
	int levelloadprogress;
	int lostthegame;
	int gameloop;
	float levelendingcycle;
	int quitflag;
	int runasmultiplayer;
	int cancelmultiplayer;
	cstr jumplevel_s;
	int onceonlyshadow;
	int takingsnapshotpress;

	// Constructor
	gametype ( )
	{
		 takingsnapshotpress = 0;
		 onceonlyshadow = 0;
		 jumplevel_s = "";
		 cancelmultiplayer = 0;
		 runasmultiplayer = 0;
		 quitflag = 0;
		 levelendingcycle = 0.0f;
		 gameloop = 0;
		 lostthegame = 0;
		 levelloadprogress = 0;
		 levelmax = 0;
		 level = 0;
		 levelplrstatsetup = 0;
		 levelloop = 0;
		 titleloop = 0;
		 masterloop = 0;
		 allowfragmentation = 0;
		 ignoretitle = 0;
		 gameisexe = 0;
		 memset ( pSwitchToPage, 0, sizeof(pSwitchToPage) );
		 memset ( pSwitchToLastPage, 0, sizeof(pSwitchToLastPage) );
		 memset ( pAdvanceWarningOfLevelFilename, 0, sizeof(pAdvanceWarningOfLevelFilename) );
	}
	// End of Constructor

};

struct gamesoundstype
{
	int sounds;
	int music;
	float titlemusicvolume;
	int titlemusicvolumetarget;
	int titlemusicvolumestart;
	int titlemusicfadestamp;

	// Constructor
	gamesoundstype ( )
	{
		 titlemusicfadestamp = 0;
		 titlemusicvolumestart = 0;
		 titlemusicvolumetarget = 0;
		 titlemusicvolume = 0.0f;
		 music = 0;
		 sounds = 0;
	}
	// End of Constructor

};

//  Titles data structure
struct titlesbuttontype
{
	int img;
	int imghigh;
	cstr name_s;
	int specialmode;
	int specialmodevalue;
	int value;
	int x1;
	int y1;
	int x2;
	int y2;

	// Constructor
	titlesbuttontype ( )
	{
		 y2 = 0;
		 x2 = 0;
		 y1 = 0;
		 x1 = 0;
		 value = 0;
		 specialmodevalue = 0;
		 specialmode = 0;
		 name_s = "";
		 imghigh = 0;
		 img = 0;
	}
	// End of Constructor

};

struct titlesbartype
{
	int mode;
	int img;
	int x1;
	int y1;
	int x2;
	int y2;
	int fill;

	// Constructor
	titlesbartype ( )
	{
		 fill = 0;
		 y2 = 0;
		 x2 = 0;
		 y1 = 0;
		 x1 = 0;
		 img = 0;
		 mode = 0;
	}
	// End of Constructor

};

struct titlesettingstype
{
	int graphicsettingslevel;
	int updateshadowsaswell;

	// Constructor
	titlesettingstype ( )
	{
		 updateshadowsaswell = 0;
		 graphicsettingslevel = 0;
	}
	// End of Constructor

};

struct shaderlevelstype
{
	int terrain;
	int entities;
	int vegetation;
	int lighting;

	// Constructor
	shaderlevelstype ( )
	{
		 lighting = 0;
		 vegetation = 0;
		 entities = 0;
		 terrain = 0;
	}
	// End of Constructor

};

struct visualstype
{
	shaderlevelstype shaderlevels;
	int mode;
	float value_f;
	int pressed;
	float stepspeed_f;
	int generalpromptstatetimer;
	cstr generalprompt_s;
	int generalpromptalignment;
	int promptstatetimer;
	int showpromptssavestate;
	int refreshshaders;
	int refreshvegetation;
	int refreshskysettings;
	int refreshterraintexture;
	int refreshvegtexture;
	int refreshterrainsupertexture;
	int refreshcountdown;
	float BloomThreshold_f;
	float PostContrast_f;
	float PostBrightness_f;
	float LightRayFactor_f;
	float FogNearest_f;
	float FogDistance_f;
	float FogR_f;
	float FogG_f;
	float FogB_f;
	float FogA_f;
	float AmbienceIntensity_f;
	float AmbienceRed_f;
	float AmbienceGeen_f;
	float AmbienceBlue_f;
	float SurfaceIntensity_f;
	float SurfaceRed_f;
	float SurfaceGreen_f;
	float SurfaceBlue_f;
	float CameraNEAR_f;
	float CameraFAR_f;
	float CameraASPECT_f;
	float CameraFOV_f;
	float WeaponFOV_f;
	float CameraFOVZoomed_f;
	float TerrainLOD1_f;
	float TerrainLOD2_f;
	float TerrainLOD3_f;
	float TerrainSize_f;
	float VegQuantity_f;
	float VegWidth_f;
	float VegHeight_f;
	float SurfaceSunFactor_f;
	float Specular_f;
	float DistanceTransitionStart_f;
	float DistanceTransitionRange_f;
	float DistanceTransitionMultiplier_f;
	int skyindex;
	cstr sky_s;
	int terrainindex;
	cstr terrain_s;
	int vegetationindex;
	cstr vegetation_s;
	int reflectionmode;
	int reflectionmodepixelsrendered;
	DWORD reflectionmodepixelsstamp;
	int reflectionmodemodified;
	int shadowmode;
	int bloommode;
	int lightraymode;
	int vegetationmode;
	int occlusionvalue;
	int debugvisualsmode;
	int underwatermode;
	float VignetteRadius_f;
	float VignetteIntensity_f;
	float MotionDistance_f;
	float MotionIntensity_f;
	float DepthOfFieldDistance_f;
	float DepthOfFieldIntensity_f;
	float LightrayLength_f;
	float LightrayQuality_f;
	float LightrayDecay_f;
	float SAORadius_f;
	float SAOIntensity_f;
	float SAOQuality_f;
	float LensFlare_f;

	// Constructor
	visualstype ( )
	{
		 LensFlare_f = 0.5f;
		 SAORadius_f = 0.0f;
		 SAOIntensity_f = 0.0f;
		 SAOQuality_f = 0.0f;
		 LightrayDecay_f = 0.0f;
		 LightrayQuality_f = 0.0f;
		 LightrayLength_f = 0.0f;
		 DepthOfFieldIntensity_f = 0.0f;
		 DepthOfFieldDistance_f = 0.0f;
		 MotionIntensity_f = 0.0f;
		 MotionDistance_f = 0.0f;
		 VignetteIntensity_f = 0.0f;
		 VignetteRadius_f = 0.0f;
		 underwatermode = 0;
		 debugvisualsmode = 0;
		 occlusionvalue = 0;
		 vegetationmode = 0;
		 lightraymode = 0;
		 bloommode = 0;
		 shadowmode = 0;
		 reflectionmodemodified = 0;
		 reflectionmodepixelsstamp = 0;
		 reflectionmodepixelsrendered = 0;
		 reflectionmode = 0;
		 vegetation_s = "";
		 vegetationindex = 0;
		 terrain_s = "";
		 terrainindex = 0;
		 sky_s = "";
		 skyindex = 0;
		 DistanceTransitionMultiplier_f = 0.0f;
		 DistanceTransitionRange_f = 0.0f;
		 DistanceTransitionStart_f = 0.0f;
		 SurfaceSunFactor_f = 0.0f;
		 Specular_f = 0.0f;
		 VegHeight_f = 0.0f;
		 VegWidth_f = 0.0f;
		 VegQuantity_f = 0.0f;
		 TerrainSize_f = 0.0f;
		 TerrainLOD3_f = 0.0f;
		 TerrainLOD2_f = 0.0f;
		 TerrainLOD1_f = 0.0f;
		 CameraFOVZoomed_f = 0.0f;
		 WeaponFOV_f = 0.0f;
		 CameraFOV_f = 0.0f;
		 CameraASPECT_f = 0.0f;
		 CameraFAR_f = 0.0f;
		 CameraNEAR_f = 0.0f;
		 SurfaceBlue_f = 0.0f;
		 SurfaceGreen_f = 0.0f;
		 SurfaceRed_f = 0.0f;
		 SurfaceIntensity_f = 0.0f;
		 AmbienceBlue_f = 0.0f;
		 AmbienceGeen_f = 0.0f;
		 AmbienceRed_f = 0.0f;
		 AmbienceIntensity_f = 0.0f;
		 FogA_f = 0.0f;
		 FogB_f = 0.0f;
		 FogG_f = 0.0f;
		 FogR_f = 0.0f;
		 FogDistance_f = 0.0f;
		 FogNearest_f = 0.0f;
		 LightRayFactor_f = 0.0f;
		 PostBrightness_f = 0.0f;
		 PostContrast_f = 0.0f;
		 BloomThreshold_f = 0.0f;
		 refreshcountdown = 0;
		 refreshterrainsupertexture = 0;
		 refreshvegtexture = 0;
		 refreshterraintexture = 0;
		 refreshskysettings = 0;
		 refreshvegetation = 0;
		 refreshshaders = 0;
		 showpromptssavestate = 0;
		 promptstatetimer = 0;
		 generalpromptalignment = 0;
		 generalprompt_s = "";
		 generalpromptstatetimer = 0;
		 stepspeed_f = 0.0f;
		 pressed = 0;
		 value_f = 0.0f;
		 mode = 0;
	}
	// End of Constructor

};

struct editortype
{
	int objectstartindex;
	int entitycursorcode;
	int entityworkobjectchoice;
	int entitytoselect;
	DWORD shadowupdatepacer;
//  `mousemodifierx# as float

//  `mousemodifiery# as float

	cstr replacefilepresent_s;

	// Constructor
	editortype ( )
	{
		 replacefilepresent_s = "";
		 shadowupdatepacer = 0;
		 entitytoselect = 0;
		 entityworkobjectchoice = 0;
		 entitycursorcode = 0;
		 objectstartindex = 0;
	}
	// End of Constructor

};

//  HUDLAYER database (jet packs)
struct hudlayertype
{
	cstr name_s;
	int obj;
	int img;
	int hidden;

	// Constructor
	hudlayertype ( )
	{
		 hidden = 0;
		 img = 0;
		 obj = 0;
		 name_s = "";
	}
	// End of Constructor

};


//  Ragdoll data structure
struct tHingeJoint
{
	int rotationVec3;
	int limitsVec2;

	// Constructor
	tHingeJoint ( )
	{
		 limitsVec2 = 0;
		 rotationVec3 = 0;
	}
	// End of Constructor

};

struct tTwistJoint
{
	int rotationVec3;
	int limitsVec3;

	// Constructor
	tTwistJoint ( )
	{
		 limitsVec3 = 0;
		 rotationVec3 = 0;
	}
	// End of Constructor

};

struct tRagdoll
{
	int obj;
	int fixedJointID;

	// Constructor
	tRagdoll ( )
	{
		 fixedJointID = 0;
		 obj = 0;
	}
	// End of Constructor

};

struct tBphys_CollisionGroups
{
	int group0;
	int group1;
	int group2;

	// Constructor
	tBphys_CollisionGroups ( )
	{
		 group2 = 0;
		 group1 = 0;
		 group0 = 0;
	}
	// End of Constructor

};

struct tBphys_CollisionMasks
{
	int mask0;
	int mask1;
	int mask2;

	// Constructor
	tBphys_CollisionMasks ( )
	{
		 mask2 = 0;
		 mask1 = 0;
		 mask0 = 0;
	}
	// End of Constructor

};

//  FPI scripting
struct uservar
{
	unsigned char used;
	cstr name;
	float value;
	unsigned char newsystem;

	// Constructor
	uservar ( )
	{
		 newsystem = 0;
		 value = 0.0f;
		 name = "";
		 used = 0;
	}
	// End of Constructor

};

struct containertype
{
	float y_f;
	int id;

	// Constructor
	containertype ( )
	{
		 id = 0;
		 y_f = 0.0f;
	}
	// End of Constructor

};


//  Animating textures
struct ani
{
	int img;
	int entid;
	int speed;
	int looped;
	int paused;

	// Constructor
	ani ( )
	{
		 paused = 0;
		 looped = 0;
		 speed = 0;
		 entid = 0;
		 img = 0;
	}
	// End of Constructor

};


//  Hockeykid - 160310 - Dark AI path type
struct pathtype
{
	int id;
	float x;
	float y;
	float z;

	// Constructor
	pathtype ( )
	{
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 id = 0;
	}
	// End of Constructor

};


//  Hockeykid - 300310 - Dark AI sound type
struct aisoundtype
{
	int alive;
	float x;
	float y;
	float z;
	int time;
	int team;

	// Constructor
	aisoundtype ( )
	{
		 team = 0;
		 time = 0;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 alive = 0;
	}
	// End of Constructor

};


//  Hockeykid - 050410 - Safe Zone UDT
struct AISafeZoneType
{
	float x;
	float y;
	float z;
	int occupied;

	// Constructor
	AISafeZoneType ( )
	{
		 occupied = 0;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

struct aifactiontype
{
	int team;
	int enemy;
	int ally;
	int neutral;

	// Constructor
	aifactiontype ( )
	{
		 neutral = 0;
		 ally = 0;
		 enemy = 0;
		 team = 0;
	}
	// End of Constructor

};


//  FPGC - 050909 - game memory tracker (test game creates, editor uses to show in meter)
struct gamememtabletype
{
	cstr name_s;
	int memused;

	// Constructor
	gamememtabletype ( )
	{
		 memused = 0;
		 name_s = "";
	}
	// End of Constructor

};


//  Data structure to old player save data
struct saveplayerstatetype
{
	int lives;
	int health;
	int weaponindexholding;

	// Constructor
	saveplayerstatetype ( )
	{
		 weaponindexholding = 0;
		 health = 0;
		 lives = 0;
	}
	// End of Constructor

};


//  Team structure
struct team_frag
{
	cstr teamname;
	int teamscore;

	// Constructor
	team_frag ( )
	{
		 teamscore = 0;
		 teamname = "";
	}
	// End of Constructor

};


//FPSCV10X SAVE LOAD Data Structures
struct saveloadtype
{
	cstr saveloaddir_s;
	int slothighlight;
	int saveloadimg;
	int saveloadx;
	int saveloady;
	int liney;
	int slotselected;
	int areyousure;
	int loadswitch;

	// Constructor
	saveloadtype ( )
	{
		 loadswitch = 0;
		 areyousure = 0;
		 slotselected = 0;
		 liney = 0;
		 saveloady = 0;
		 saveloadx = 0;
		 saveloadimg = 0;
		 slothighlight = 0;
		 saveloaddir_s = "";
	}
	// End of Constructor

};


//FPGC Pause Mode Globals
struct frozentype
{
	int frames;

	// Constructor
	frozentype ( )
	{
		 frames = 0;
	}
	// End of Constructor

};


//  FPSCV10X Game Position Data Structure
struct saveloadgamepositiontype
{
	int level;
	int playerx;
	int playery;
	int playerz;
	int playerrx;
	int playerry;
	int playerhealth;
	int playerlives;
	int playerinventorymax;
	int missiongoaltotal;
	int playerholdinggun;
	int meridinglift;
	int ambience;

	// Constructor
	saveloadgamepositiontype ( )
	{
		 ambience = 0;
		 meridinglift = 0;
		 playerholdinggun = 0;
		 missiongoaltotal = 0;
		 playerinventorymax = 0;
		 playerlives = 0;
		 playerhealth = 0;
		 playerry = 0;
		 playerrx = 0;
		 playerz = 0;
		 playery = 0;
		 playerx = 0;
		 level = 0;
	}
	// End of Constructor

};


//  Save position
struct saveloadgamepositionentitytype
{
	int spawnleaderid;
	int ailibindex;
	cstr aistate_s;
	cstr aioldstate_s;
	int active;
	int dormant;
	int beenkilled;
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float orx;
	float ory;
	float orz;
	float floorposy;
	int animset;
	int animdo;
	float animframe;
	int currentammo;
	int health;
	int collected;
	int activated;
	int collisionactive;
	int actualtarget;
	int waypointstate;
	int waypointcurrent;
	int waypointtracker;
	int waypointdirection;
	float waypointxstart;
	float waypointystart;
	float waypointzstart;
	float waypointxinc;
	float waypointyinc;
	float waypointzinc;
	int aiusinginternalai;
	int aioldwaypointstate;
	float aioldanimframe;
	float aiolddestanimframe;
	int spawnatstart;
	int spawnupto;
	int decalindex;
	int decalmode;
	int physics;
	int alttextureused;
	int videotexture;
	int etimerpaused;
	int etimer;
	int lastdamagetimer;
	int lastdamagepaused;
	float entitydammult_f;
	int ishidden;
	int cullstate;
	unsigned char usefade;
	int onradar;
	int radardistance;
	int shotby;
	cstr shotbyammo_s;
	int isimmune;
	int camuse;
	int camoffsetx;
	int camoffsety;
	int camoffsetz;
	float offsetanglex;
	float offsetangley;
	float offsetanglez;
	int camrotx;
	int camroty;
	int camrotz;
	int positionchanged;
	int cullimmobile;
	int isanobjective;
	int objectiveradarrange;
	int objectivecurrentrange;
	float entityacc;
	int soundplaying;
	frozentype frozen;
	int delaydestroy;
	int delaydestroytimer;
	int delaytimeactive;

	// Constructor
	saveloadgamepositionentitytype ( )
	{
		 delaytimeactive = 0;
		 delaydestroytimer = 0;
		 delaydestroy = 0;
		 soundplaying = 0;
		 entityacc = 0.0f;
		 objectivecurrentrange = 0;
		 objectiveradarrange = 0;
		 isanobjective = 0;
		 cullimmobile = 0;
		 positionchanged = 0;
		 camrotz = 0;
		 camroty = 0;
		 camrotx = 0;
		 offsetanglez = 0.0f;
		 offsetangley = 0.0f;
		 offsetanglex = 0.0f;
		 camoffsetz = 0;
		 camoffsety = 0;
		 camoffsetx = 0;
		 camuse = 0;
		 isimmune = 0;
		 shotbyammo_s = "";
		 shotby = 0;
		 radardistance = 0;
		 onradar = 0;
		 usefade = 0;
		 cullstate = 0;
		 ishidden = 0;
		 entitydammult_f = 0.0f;
		 lastdamagepaused = 0;
		 lastdamagetimer = 0;
		 etimer = 0;
		 etimerpaused = 0;
		 videotexture = 0;
		 alttextureused = 0;
		 physics = 0;
		 decalmode = 0;
		 decalindex = 0;
		 spawnupto = 0;
		 spawnatstart = 0;
		 aiolddestanimframe = 0.0f;
		 aioldanimframe = 0.0f;
		 aioldwaypointstate = 0;
		 aiusinginternalai = 0;
		 waypointzinc = 0.0f;
		 waypointyinc = 0.0f;
		 waypointxinc = 0.0f;
		 waypointzstart = 0.0f;
		 waypointystart = 0.0f;
		 waypointxstart = 0.0f;
		 waypointdirection = 0;
		 waypointtracker = 0;
		 waypointcurrent = 0;
		 waypointstate = 0;
		 actualtarget = 0;
		 collisionactive = 0;
		 activated = 0;
		 collected = 0;
		 health = 0;
		 currentammo = 0;
		 animframe = 0.0f;
		 animdo = 0;
		 animset = 0;
		 floorposy = 0.0f;
		 orz = 0.0f;
		 ory = 0.0f;
		 orx = 0.0f;
		 rz = 0.0f;
		 ry = 0.0f;
		 rx = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 beenkilled = 0;
		 dormant = 0;
		 active = 0;
		 aioldstate_s = "";
		 aistate_s = "";
		 ailibindex = 0;
		 spawnleaderid = 0;
	}
	// End of Constructor

};


//  Save position weapon slots
struct saveloadgamepositionweaponslottype
{
	int pref;
	int got;
	int invpos;
	int noselect;
	int weaponammo;
	int weaponclipammo;
	int weaponhud;

	// Constructor
	saveloadgamepositionweaponslottype ( )
	{
		 weaponhud = 0;
		 weaponclipammo = 0;
		 weaponammo = 0;
		 noselect = 0;
		 invpos = 0;
		 got = 0;
		 pref = 0;
	}
	// End of Constructor

};


//  Decal Globals
struct decalglobaltype
{
	int splashdecalrippleid;
	int splashdecalsmallid;
	int splashdecallargeid;
	int splashdecalmistyid;
	int splashdecaldropletsid;
	int splashdecalfoamid;
	int dustflumeid;
	int impactid;
	int bloodsplatid;

	// Constructor
	decalglobaltype ( )
	{
		 bloodsplatid = 0;
		 impactid = 0;
		 dustflumeid = 0;
		 splashdecalfoamid = 0;
		 splashdecaldropletsid = 0;
		 splashdecalmistyid = 0;
		 splashdecallargeid = 0;
		 splashdecalsmallid = 0;
		 splashdecalrippleid = 0;
	}
	// End of Constructor

};

//  Material sound and decal settings
struct materialsettingstype
{
	cstr name_s;
	cstr tred0_s;
	int tred0id;
	cstr tred1_s;
	int tred1id;
	cstr tred2_s;
	int tred2id;
	cstr tred3_s;
	int tred3id;
	cstr scrape_s;
	int scrapeid;
	cstr impact_s;
	int impactid;
	cstr destroy_s;
	int destroyid;
	int freq;
	cstr decal_s;
	int decalid;
	int usedinlevel;

	// Constructor
	materialsettingstype ( )
	{
		 usedinlevel = 0;
		 decalid = 0;
		 decal_s = "";
		 freq = 0;
		 destroyid = 0;
		 destroy_s = "";
		 impactid = 0;
		 impact_s = "";
		 scrapeid = 0;
		 scrape_s = "";
		 tred3id = 0;
		 tred3_s = "";
		 tred2id = 0;
		 tred2_s = "";
		 tred1id = 0;
		 tred1_s = "";
		 tred0id = 0;
		 tred0_s = "";
		 name_s = "";
	}
	// End of Constructor

};

//  Entity Physics Box (  type )
struct entityphysicsboxtype
{
	float SizeX;
	float SizeY;
	float SizeZ;
	float OffX;
	float OffY;
	float OffZ;
	float RotX;
	float RotY;
	float RotZ;

	// Constructor
	entityphysicsboxtype ( )
	{
		 RotZ = 0.0f;
		 RotY = 0.0f;
		 RotX = 0.0f;
		 OffZ = 0.0f;
		 OffY = 0.0f;
		 OffX = 0.0f;
		 SizeZ = 0.0f;
		 SizeY = 0.0f;
		 SizeX = 0.0f;
	}
	// End of Constructor

};


//  AirMod - Advanced Blood Effects >>
#define BLOODMAX 10
struct entitybloodtype
{
	float x_f;
	float y_f;
	float z_f;
	float oldx_f;
	float oldy_f;
	float oldz_f;
	float angley_f;
	float anglex_f;
	int delay;
	int spawndelay;
	unsigned char active;
	int mode;

	// Constructor
	entitybloodtype ( )
	{
		 mode = 0;
		 active = 0;
		 spawndelay = 0;
		 delay = 0;
		 anglex_f = 0.0f;
		 angley_f = 0.0f;
		 oldz_f = 0.0f;
		 oldy_f = 0.0f;
		 oldx_f = 0.0f;
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
	}
	// End of Constructor

};


//  Waypoint Component of entity element
struct aiwaypointtype
{
	int state;
	int current;
	int tracker;
	int direction;
	DWORD timefrom;
	float xstart;
	float ystart;
	float zstart;
	float xinc;
	float yinc;
	float zinc;
	DWORD timeslice;

	// Constructor
	aiwaypointtype ( )
	{
		 timeslice = 0;
		 zinc = 0.0f;
		 yinc = 0.0f;
		 xinc = 0.0f;
		 zstart = 0.0f;
		 ystart = 0.0f;
		 xstart = 0.0f;
		 timefrom = 0;
		 direction = 0;
		 tracker = 0;
		 current = 0;
		 state = 0;
	}
	// End of Constructor

};


//  AI Component of entity element
struct aistatustype
{
	int libinit;
	int libmain;
	int libdestroy;
	int libshoot;
	int libindex;
	cstr state_s;
	float alphafade;
	float destalphafade;
	int alphafadeupdate;
	float headangle;
	float headdestangle;
	float headdownangle;
	aiwaypointtype waypoint;
	int usinginternalai;
	cstr oldstate_s;
	float oldalphafade;
	int oldwaypointstate;
	int headshot;
	float oldanimframe;
	float olddestanimframe;

	// Constructor
	aistatustype ( )
	{
		 olddestanimframe = 0.0f;
		 oldanimframe = 0.0f;
		 headshot = 0;
		 oldwaypointstate = 0;
		 oldalphafade = 0.0f;
		 oldstate_s = "";
		 usinginternalai = 0;
		 headdownangle = 0.0f;
		 headdestangle = 0.0f;
		 headangle = 0.0f;
		 alphafadeupdate = 0;
		 destalphafade = 0.0f;
		 alphafade = 0.0f;
		 state_s = "";
		 libindex = 0;
		 libshoot = 0;
		 libdestroy = 0;
		 libmain = 0;
		 libinit = 0;
	}
	// End of Constructor

};


//  SPAWN Component of entity element
struct spawntype
{
	int atstart;
	int max;
	int delay;
	int qty;
	int upto;
	int afterdelay;
	int whendead;
	int delayrandom;
	int qtyrandom;
	int vel;
	int velrandom;
	int angle;
	int anglerandom;
	DWORD life;
	int leader;
	float leadercount;
	int leaderid;
	float x;
	float y;
	float z;

	// Constructor
	spawntype ( )
	{
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 leaderid = 0;
		 leadercount = 0.0f;
		 leader = 0;
		 life = 0;
		 anglerandom = 0;
		 angle = 0;
		 velrandom = 0;
		 vel = 0;
		 qtyrandom = 0;
		 delayrandom = 0;
		 whendead = 0;
		 afterdelay = 0;
		 upto = 0;
		 qty = 0;
		 delay = 0;
		 max = 0;
		 atstart = 0;
	}
	// End of Constructor

};


//  FORCE Component of entity element
struct forcetype
{
	int active;
	float ix;
	float iy;
	float iz;

	// Constructor
	forcetype ( )
	{
		 iz = 0.0f;
		 iy = 0.0f;
		 ix = 0.0f;
		 active = 0;
	}
	// End of Constructor

};


//  MOVEMENT Component of entity element
struct movertype
{
	int moved;
	float stepcount;
	float dx;
	float dy;
	float dz;
	float da;
	float ix;
	float iy;
	float iz;
	float grav;
	float gravlasty;
	int inmotion;
	float viewconeused;
	int run;
	int strafe;
	float slidevel;

	// Constructor
	movertype ( )
	{
		 slidevel = 0.0f;
		 strafe = 0;
		 run = 0;
		 viewconeused = 0.0f;
		 inmotion = 0;
		 gravlasty = 0.0f;
		 grav = 0.0f;
		 iz = 0.0f;
		 iy = 0.0f;
		 ix = 0.0f;
		 da = 0.0f;
		 dz = 0.0f;
		 dy = 0.0f;
		 dx = 0.0f;
		 stepcount = 0.0f;
		 moved = 0;
	}
	// End of Constructor

};


//  Decal Particle Structure
struct decalparticletype
{
	float offsety;
	int scale;
	float randomstartx;
	float randomstarty;
	float randomstartz;
	float linearmotionx;
	float linearmotiony;
	float linearmotionz;
	float lineargravity;
	float randommotionx;
	float randommotiony;
	float randommotionz;
	int mirrormode;
	float camerazshift;
	float scaleonlyx;
	float lifeincrement;
	float alphaintensity;
	int animated;

	// Constructor
	decalparticletype ( )
	{
		 animated = 0;
		 alphaintensity = 0.0f;
		 lifeincrement = 0.0f;
		 scaleonlyx = 0.0f;
		 camerazshift = 0.0f;
		 mirrormode = 0;
		 randommotionz = 0.0f;
		 randommotiony = 0.0f;
		 randommotionx = 0.0f;
		 lineargravity = 0.0f;
		 linearmotionz = 0.0f;
		 linearmotiony = 0.0f;
		 linearmotionx = 0.0f;
		 randomstartz = 0.0f;
		 randomstarty = 0.0f;
		 randomstartx = 0.0f;
		 scale = 0;
		 offsety = 0.0f;
	}
	// End of Constructor

};


//  Entity Profile Data Structure
struct entityprofileheadertype
{
	cstr desc_s;

	// Constructor
	entityprofileheadertype ( )
	{
		 desc_s = "";
	}
	// End of Constructor

};

struct entityanimtype
{
	int start;
	int finish;
	int found;

	// Constructor
	entityanimtype ( )
	{
		 found = 0;
		 finish = 0;
		 start = 0;
	}
	// End of Constructor

};

struct tentityfootfall
{
	int keyframe;
	int soundtype;

	// Constructor
	tentityfootfall ( )
	{
		 soundtype = 0;
		 keyframe = 0;
	}
	// End of Constructor

};

struct entitylighttype
{
	int index;
	int islit;
	DWORD color;
	int range;
	int offsetup;
	int offsetz;

	// Constructor
	entitylighttype ( )
	{
		 offsetz = 0;
		 offsetup = 0;
		 range = 0;
		 color = 0;
		 islit = 0;
		 index = 0;
	}
	// End of Constructor

};

struct entitytriggertype
{
	int waypointzoneindex;
	int stylecolor;

	// Constructor
	entitytriggertype ( )
	{
		 stylecolor = 0;
		 waypointzoneindex = 0;
	}
	// End of Constructor

};

// EBE structure for inside entityelement
struct ebeType
{
	DWORD dwRLESize;
	DWORD* pRLEData;
	bool bHasChanged;
	DWORD dwMatRefCount;
	int* iMatRef;
	DWORD dwTexRefCount;
	LPSTR* pTexRef;
};

struct entityprofiletype
{
	cstr aiinit_s;
	cstr aimain_s;
	cstr aidestroy_s;
	cstr aishoot_s;
	cstr soundset_s;
	cstr soundset1_s;
	cstr soundset2_s;
	cstr soundset3_s;
	cstr soundset4_s;
	cstr usekey_s;
	cstr ifused_s;
	cstr ifusednear_s;
	int spawnatstart;
	int spawnmax;
	int spawnupto;
	int spawnafterdelay;
	int spawnwhendead;
	int spawndelay;
	int spawnqty;
	int spawndelayrandom;
	int spawnqtyrandom;
	int spawnvel;
	int spawnvelrandom;
	int spawnangle;
	int spawnanglerandom;
	DWORD spawnlife;
	cstr model_s;
	float offx;
	float offy;
	float offz;
	float rotx;
	float roty;
	float rotz;
	float scale;
	float fixnewy;
	int forwardfacing;
	int dontfindfloor;
	float defaultheight;
	int defaultstatic;
	int collisionmode;
	int collisionscaling;
	int collisionoverride;
	int forcesimpleobstacle;
	float forceobstaclepolysize;
	float forceobstaclesliceheight;
	float forceobstaclesliceminsize;
	int notanoccluder;
	int materialindex;
	int disablebatch;
	int lod1distance;
	int lod2distance;
	int bitbobon;
	float bitbobdistweight_f;
	float bitbobfar_f;
	float bitbobnear_f;
	int debrisshapeindex;
	float coneheight;
	float coneangle;
	float conerange;
	int hurtfall;
	int onetexture;
	cstr texpath_s;
	cstr texd_s;
	int texdid;
	int texnid;
	int texsid;
	int texiid;
	int texgid;
	int texhid;
	int texlid;
	cstr texaltd_s;
	int texaltdid;
	int texidmax;
	cstr effect_s;
	int effectprofile;
	int usingeffect;
	int castshadow;
	int smoothangle;
	int transparency;
	int canseethrough;
	int specular;
	int specularperc;
	float uvscrollu;
	float uvscrollv;
	float uvscaleu;
	float uvscalev;
	int invertnormal;
	int preservetangents;
	int zdepth;
	int cullmode;
	int reducetexture;
	int strength;
	int lives;
	int speed;
	int animspeed;
	int animstyle;
	int isviolent;
	int isimmobile;
	int lodmodifier;
	int isocluder;
	int isocludee;
	int colondeath;
	int parententityindex;
	int parentlimbindex;
	int ischaracter;
	int isspinetracker;
	int noXZrotation;
	int ismultiplayercharacter;
	int canfight;
	int cantakeweapon;
	cstr isweapon_s;
	int isweapon;
	cstr ishudlayer_s;
	int ishudlayer;
	int isammo;
	cstr hasweapon_s;
	int hasweapon;
	int ishealth;
	int isflak;
	int limbmax;
	int headlimb;
	int spine;
	int spine2;
	int firespotlimb;
	int animmax;
	int startofaianim;
	int footfallmax;
	int quantity;
	int ismarker;
	int isebe;
	int offyoverride;
	int markerindex;
	int islightmarker;
	int isobjective;
	entitylighttype light;
	entitytriggertype trigger;
	int decalmax;
	int bloodscorch;
	int rateoffire;
	int damage;
	int accuracy;
	int reloadqty;
	int fireiterations;
	int range;
	int dropoff;
	int usespotlighting;
	float lifespan;
	float throwspeed;
	float throwangle;
	int bounceqty;
	int explodeonhit;
	int weaponisammo;
	int hoverfactor;
	int physics;
	int phyalways;
	int phyweight;
	int phyfriction;
	int phyforcedamage;
	int rotatethrow;
	int explodable;
	int explodedamage;
	int teamfield;
	int headframestart;
	int headframefinish;
	int hairframestart;
	int hairframefinish;
	int hideframestart;
	int hideframefinish;
	float decaloffsetangle;
	float decaloffsetdist;
	float decaloffsety;
	int ragdoll;
	int nothrowscript;
	int reserve1;
	int reserve2;
	int reserve3;
	int reserve4;
	int reserve5;
	int reserve6;
	unsigned char custombiped;
	int approxmemused;
	int playanimineditor;
	int autoflatten;
	int physicsobjectcount;
	int fatness;
	int skipfvfconvert;
	int matrixmode;
	int resetlimbmatrix;
	int usesweapstyleanims;
	int reverseframes;
	int fullbounds;
	int cpuanims;
	int ignoredefanim;
	cstr addhandle_s;
	int addhandlelimb;
	int ischaractercreator;
	cstr charactercreator_s;
	float fJumpModifier;
	int jumphold;
	int jumpresume;
	int jumpvaulttrim;
	int meleerange;
	int meleehitangle;
	int meleestrikest;
	int meleestrikefn;
	int meleedamagest;
	int meleedamagefn;
	ebeType ebe;

	// Constructor
	entityprofiletype ( )
	{
		 ebe.dwTexRefCount = 0;
		 ebe.pTexRef = NULL;
		 ebe.dwMatRefCount = 0;
		 ebe.iMatRef = NULL;
		 ebe.dwRLESize = 0;
		 ebe.pRLEData = NULL;
		 ebe.bHasChanged = false;
		 fJumpModifier = 1.0f;
		 jumphold = 0;
		 jumpresume = 0;
		 jumpvaulttrim = 1;
		 meleerange = 0;
		 meleehitangle = 0;
		 meleestrikest = 0;
		 meleestrikefn = 0;
		 meleedamagest = 0;
		 meleedamagefn = 0;
		 charactercreator_s = "";
		 ischaractercreator = 0;
		 addhandlelimb = 0;
		 addhandle_s = "";
		 ignoredefanim = 0;
		 cpuanims = 0;
		 fullbounds = 0;
		 reverseframes = 0;
		 usesweapstyleanims = 0;
		 resetlimbmatrix = 0;
		 matrixmode = 0;
		 skipfvfconvert = 0;
		 fatness = 0;
		 physicsobjectcount = 0;
		 autoflatten = 0;
		 playanimineditor = 0;
		 approxmemused = 0;
		 custombiped = 0;
		 reserve6 = 0;
		 reserve5 = 0;
		 reserve4 = 0;
		 reserve3 = 0;
		 reserve2 = 0;
		 reserve1 = 0;
		 nothrowscript = 0;
		 ragdoll = 0;
		 decaloffsety = 0.0f;
		 decaloffsetdist = 0.0f;
		 decaloffsetangle = 0.0f;
		 headframefinish = 0;
		 headframestart = 0;
		 hairframestart = 0;
		 hairframefinish = 0;
		 hideframestart = 0;
		 hideframefinish = 0;
		 teamfield = 0;
		 explodedamage = 0;
		 explodable = 0;
		 rotatethrow = 0;
		 phyforcedamage = 0;
		 phyfriction = 0;
		 phyweight = 0;
		 phyalways = 0;
		 physics = 0;
		 hoverfactor = 0;
		 weaponisammo = 0;
		 explodeonhit = 0;
		 bounceqty = 0;
		 throwangle = 0.0f;
		 throwspeed = 0.0f;
		 lifespan = 0.0f;
		 usespotlighting = 0;
		 dropoff = 0;
		 range = 0;
		 fireiterations = 0;
		 reloadqty = 0;
		 accuracy = 0;
		 damage = 0;
		 rateoffire = 0;
		 bloodscorch = 0;
		 decalmax = 0;
		 isobjective = 0;
		 islightmarker = 0;
		 markerindex = 0;
		 ismarker = 0;
		 isebe = 0;
		 offyoverride = 0;
		 quantity = 0;
		 footfallmax = 0;
		 startofaianim = 0;
		 animmax = 0;
		 firespotlimb = 0;
		 spine2 = 0;
		 spine = 0;
		 headlimb = 0;
		 limbmax = 0;
		 isflak = 0;
		 ishealth = 0;
		 hasweapon = 0;
		 hasweapon_s = "";
		 isammo = 0;
		 ishudlayer = 0;
		 ishudlayer_s = "";
		 isweapon = 0;
		 isweapon_s = "";
		 cantakeweapon = 0;
		 canfight = 0;
		 ismultiplayercharacter = 0;
		 noXZrotation = 0;
		 ischaracter = 0;
		 isspinetracker = 0;
		 isocluder = 1;
		 isocludee = 1;
		 specularperc = 100;
		 colondeath = 1;
		 parententityindex = 0;
		 parentlimbindex = 0;
		 lodmodifier = 0;
		 isimmobile = 0;
		 isviolent = 0;
		 animstyle = 0;
		 animspeed = 0;
		 speed = 0;
		 lives = 0;
		 strength = 0;
		 reducetexture = 0;
		 cullmode = 0;
		 zdepth = 1;
		 specular = 0;
		 specularperc = 100;
		 canseethrough = 0;
		 transparency = 0;
		 smoothangle = 0;
		 castshadow = 0;
		 usingeffect = 0;
		 effect_s = "";
		 texidmax = 0;
		 texaltdid = 0;
		 texaltd_s = "";
		 texiid = 0;
		 texsid = 0;
		 texnid = 0;
		 texdid = 0;
		 texd_s = "";
		 texpath_s = "";
		 onetexture = 0;
		 hurtfall = 0;
		 conerange = 0.0f;
		 coneangle = 0.0f;
		 coneheight = 0.0f;
		 debrisshapeindex = 0;
		 bitbobnear_f = 0.0f;
		 bitbobfar_f = 0.0f;
		 bitbobdistweight_f = 0.0f;
		 bitbobon = 0;
		 lod2distance = 0;
		 lod1distance = 0;
		 disablebatch = 0;
		 materialindex = 0;
		 notanoccluder = 0;
		 forcesimpleobstacle = 0;
		 forceobstaclepolysize = 0.0f;
		 forceobstaclesliceheight = 0.0f;
		 forceobstaclesliceminsize = 0.0f;
		 collisionoverride = 0;
		 collisionscaling = 0;
		 collisionmode = 0;
		 defaultstatic = 0;
		 defaultheight = 0.0f;
		 forwardfacing = 0;
		 dontfindfloor = 0;
		 fixnewy = 0.0f;
		 scale = 0.0f;
		 rotz = 0.0f;
		 roty = 0.0f;
		 rotx = 0.0f;
		 offz = 0.0f;
		 offy = 0.0f;
		 offx = 0.0f;
		 model_s = "";
		 spawnlife = 0;
		 spawnanglerandom = 0;
		 spawnangle = 0;
		 spawnvelrandom = 0;
		 spawnvel = 0;
		 spawnqtyrandom = 0;
		 spawndelayrandom = 0;
		 spawnqty = 0;
		 spawndelay = 0;
		 spawnwhendead = 0;
		 spawnafterdelay = 0;
		 spawnupto = 0;
		 spawnmax = 0;
		 spawnatstart = 0;
		 ifusednear_s = "";
		 ifused_s = "";
		 usekey_s = "";
		 soundset4_s = "";
		 soundset3_s = "";
		 soundset2_s = "";
		 soundset1_s = "";
		 soundset_s = "";
		 aishoot_s = "";
		 aidestroy_s = "";
		 aimain_s = "";
		 aiinit_s = "";
	}
	// End of Constructor

};

//  Entity Element Custom Profile Data
struct entityeleproftype
{
	cstr name_s;
	cstr aiinit_s;
	cstr aiinitname_s;
	int aiinit;
	cstr aimain_s;
	cstr aimainname_s;
	int aimain;
	cstr aidestroy_s;
	cstr aidestroyname_s;
	int aidestroy;
	cstr aishoot_s;
	cstr aishootname_s;
	int aishoot;
	cstr usekey_s;
	cstr ifused_s;
	cstr ifusednear_s;
	float scale;
	float coneheight;
	float coneangle;
	float conerange;
	int uniqueelement;
	cstr texd_s;
	int texdid;
	int texnid;
	int texsid;
	cstr texaltd_s;
	int texaltdid;
	int texidmax;
	cstr effect_s;
	int usingeffect;
	int transparency;
	int strength;
	int isimmobile;
	int isocluder;
	int isocludee;
	float specularperc;
	int colondeath;
	int parententityindex;
	int parentlimbindex;
	int lodmodifier;
	int cantakeweapon;
	cstr hasweapon_s;
	int hasweapon;
	int quantity;
	int isviolent;
	int isobjective;
	int lives;
	int markerindex;
	entitylighttype light;
	entitytriggertype trigger;
	cstr basedecal_s;
	int particleoverride;
	decalparticletype particle;
	cstr soundset_s;
	cstr soundset1_s;
	cstr soundset2_s;
	cstr soundset3_s;
	cstr soundset4_s;
	int spawnatstart;
	int spawnmax;
	int spawnupto;
	int spawnafterdelay;
	int spawnwhendead;
	int spawndelay;
	int spawnqty;
	int spawndelayrandom;
	int spawnqtyrandom;
	int spawnvel;
	int spawnvelrandom;
	int spawnangle;
	int spawnanglerandom;
	DWORD spawnlife;
	int hurtfall;
	int castshadow;
	int reducetexture;
	int speed;
	int animspeed;
	int rateoffire;
	int damage;
	int accuracy;
	int reloadqty;
	int fireiterations;
	int range;
	int dropoff;
	int usespotlighting;
	float lifespan;
	float throwspeed;
	float throwangle;
	int bounceqty;
	int explodeonhit;
	int weaponisammo;
	DWORD spare1;
	int physics;
	int phyalways;
	int phyweight;
	int phyfriction;
	int phyforcedamage;
	int rotatethrow;
	int explodable;
	int explodedamage;
	int teamfield;
	DWORD phydw1;
	DWORD phydw2;
	DWORD phydw3;
	DWORD phydw4;
	DWORD phydw5;

	// Constructor
	entityeleproftype ( )
	{
		 phydw5 = 0;
		 phydw4 = 0;
		 phydw3 = 0;
		 phydw2 = 0;
		 phydw1 = 0;
		 teamfield = 0;
		 explodedamage = 0;
		 explodable = 0;
		 rotatethrow = 0;
		 phyforcedamage = 0;
		 phyfriction = 0;
		 phyweight = 0;
		 phyalways = 0;
		 physics = 0;
		 spare1 = 0;
		 weaponisammo = 0;
		 explodeonhit = 0;
		 bounceqty = 0;
		 throwangle = 0.0f;
		 throwspeed = 0.0f;
		 lifespan = 0.0f;
		 usespotlighting = 0;
		 dropoff = 0;
		 range = 0;
		 fireiterations = 0;
		 reloadqty = 0;
		 accuracy = 0;
		 damage = 0;
		 rateoffire = 0;
		 animspeed = 0;
		 speed = 0;
		 reducetexture = 0;
		 castshadow = 0;
		 hurtfall = 0;
		 spawnlife = 0;
		 spawnanglerandom = 0;
		 spawnangle = 0;
		 spawnvelrandom = 0;
		 spawnvel = 0;
		 spawnqtyrandom = 0;
		 spawndelayrandom = 0;
		 spawnqty = 0;
		 spawndelay = 0;
		 spawnwhendead = 0;
		 spawnafterdelay = 0;
		 spawnupto = 0;
		 spawnmax = 0;
		 spawnatstart = 0;
		 soundset4_s = "";
		 soundset3_s = "";
		 soundset2_s = "";
		 soundset1_s = "";
		 soundset_s = "";
		 particleoverride = 0;
		 basedecal_s = "";
		 markerindex = 0;
		 lives = 0;
		 isobjective = 0;
		 isviolent = 0;
		 quantity = 0;
		 hasweapon = 0;
		 hasweapon_s = "";
		 cantakeweapon = 0;
		 isocluder = 1;
		 isocludee = 1;
		 specularperc = 100;
		 colondeath = 1;
		 parententityindex = 0;
		 parentlimbindex = 0;
		 lodmodifier = 0;
		 isimmobile = 0;
		 strength = 0;
		 transparency = 0;
		 usingeffect = 0;
		 effect_s = "";
		 texidmax = 0;
		 texaltdid = 0;
		 texaltd_s = "";
		 texsid = 0;
		 texnid = 0;
		 texdid = 0;
		 texd_s = "";
		 uniqueelement = 0;
		 conerange = 0.0f;
		 coneangle = 0.0f;
		 coneheight = 0.0f;
		 scale = 0.0f;
		 ifusednear_s = "";
		 ifused_s = "";
		 usekey_s = "";
		 aishoot = 0;
		 aishootname_s = "";
		 aishoot_s = "";
		 aidestroy = 0;
		 aidestroyname_s = "";
		 aidestroy_s = "";
		 aimain = 0;
		 aimainname_s = "";
		 aimain_s = "";
		 aiinit = 0;
		 aiinitname_s = "";
		 aiinit_s = "";
		 name_s = "";
	}
	// End of Constructor

};

//  Entity flags for LUA control/states
struct entityluastatestype
{
	int flagschanged;
	int firsttime;
	int plrinzone;
	int entityinzone;
	int haskey;
	int animating;
	int dynamicavoidance;
	float dynamicavoidancestuckclock;

	// Constructor
	entityluastatestype ( )
	{
		 dynamicavoidancestuckclock = 0.0f;
		 dynamicavoidance = 0;
		 animating = 0;
		 haskey = 0;
		 plrinzone = 0;
		 firsttime = 0;
		 flagschanged = 0;
	}
	// End of Constructor

};

/*
//  Entity LUA DATA structure
struct entityluadatastatestype
{
	float x;
	float y;
	float z;
	float anglex;
	float angley;
	float anglez;
	float obj;
	float active;
	float activated;
	float collected;
	float haskey;
	float plrinzone;
	float entityinzone;
	float plrvisible;
	float health;
	float frame;
	int timer;
	float plrdist;
	float avoid;
	cstr limbhit;
	float limbhitindex;
	float animating;
	entityluadatastatestype ( )
	{
		 x = 0;
		 y = 0;
		 z = 0;
		 anglex = 0;
		 angley = 0;
		 anglez = 0;
		 obj = 0;
		 active = 0;
		 activated = 0;
		 collected = 0;
		 haskey = 0;
		 plrinzone = 0;
		 entityinzone = 0;
		 plrvisible = 0;
		 health = 0;
		 frame = 0;
		 timer = 0;
		 plrdist = 0;
		 avoid = 0;
		 limbhit = "";
		 limbhitindex = 0;
		 animating = 0;
	}
};
*/

//  Entity Elements Data (entityelement array)
struct entitytype
{
	float animspeedmod;
	int enterwater;
	int isflak;
	int curvcmx;
	int curvcmy;
	int curvcmz;
	int spine;
	float spiner;
	float spineraim;
	int eyeoffset;
	int aimoved;
	float aimovex;
	float aimovez;
	int ttarget;
	int etimer;
	int etimerpaused;
	int lastdamagetimer;
	int lastdamagepaused;
	int editorfixed;
	int servercontrolled;
	int fakeplrindex;
	int active;
	int dormant;
	entityeleproftype eleprof;
	movertype mover;
	aistatustype ai;
	spawntype spawn;
	forcetype force;
	int maintype;
	int bankindex;
	int profileobj;
	int staticflag;
	int obj;
	int attachmentobj;
	int attachmentbaseobj;
	int attachmentweapontype;
	int attachmentobjfirespotlimb;
	int attachmentblobobj;
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float scalex;
	float scaley;
	float scalez;
	float lastx;
	float lasty;
	float lastz;
	int norotate;
	int nogravity;
	float dry;
	float floorposy;
	int colr;
	int colg;
	int colb;
	int limbslerp;
	float logiccount;
	int logiccountburst;
	DWORD logictimestamp;
	int priorityai;
	int priorityduration;
	int raycastcount;
	int raycastlaststate;
	int nofloorlogic;
	int crouchprofile;
	float plrdist;
	int decalindex;
	int decalmode;
	float decalloop;
	int decalslotused;
	float decalsizex;
	float decalsizey;
	int animset;
	int animdir;
	int animdo;
	int animtime;
	float animframe;
	int animonce;
	float destanimframe;
	int animframeupdate;
	int spinrate;
	float spinvalue;
	int floatrate;
	float floatvalue;
	int health;
	int briefimmunity;
	int lifecode;
	int beenkilled;
	int shotdamage;
	int shotdamagesource;
	int shotdamagetype;
	float delaydamagecount;
	float delaydamagetimer;
	int delaydamagesource;
	int delaydamage;
	int delayimpact;
	float delaydamagex_f;
	float delaydamagey_f;
	float delaydamagez_f;
	int delaydir;
	int collected;
	int activated;
	int collisionactive;
	int invincibleactive;
	int blockedby;
	int blockedtox;
	int blockedtoy;
	int blockedtoz;
	int soundset;
	int soundset1;
	int soundset2;
	int soundset3;
	int soundset4;
	int soundisnonthreedee;
	int soundlooping;
	int soundistalking;
	int alttextureused;
	int videotexture;
	int usingphysicsnow;
	int doorobsactive;
	int destroyme;
	int explodefusetime;
	int reserve1;
	int reserve2;
	int reserve3;
	int reserve4;
	int reserve5;
	float lastdamagex_f;
	float lastdamagey_f;
	float lastdamagez_f;
	unsigned char nobulletcol;
	int isaltammo;
	int hideshadow;
	float floorpositiony;
	float entitydammult_f;
	int ishidden;
	int cullstate;
	unsigned char usefade;
	int onradar;
	int radardistance;
	int shotby;
	cstr shotbyammo_s;
	int isimmune;
	int camuse;
	int camoffsetx;
	int camoffsety;
	int camoffsetz;
	float offsetanglex;
	float offsetangley;
	float offsetanglez;
	int camrotx;
	int camroty;
	int camrotz;
	int positionchanged;
	int cullimmobile;
	int isanobjective;
	int objectiveradarrange;
	int objectivecurrentrange;
	int entityacc;
	int soundplaying;
	frozentype frozen;
	int distance;
	int delaydestroy;
	int delaydestroytimer;
	int delaytimeactive;
	entityluastatestype lua;
	//entityluadatastatestype luadata;
	int isclone;
	int plrvisible;
	int limbhurt;
	float limbhurta_f;
	float speedmodulator_f;
	int donotreflect;
	int hasbeenbatched;
	float abscolx_f;
	float abscolz_f;
	float abscolradius_f;
	int characterSoundBankNumber;
	int particleemitterid;
	int ragdollified;
	float ragdollifiedforcex_f;
	float ragdollifiedforcey_f;
	float ragdollifiedforcez_f;
	float ragdollifiedforcevalue_f;
	int ragdollifiedforcelimb;
	int editorlock;
	cstr overprompt_s;
	DWORD overprompttimer;
	int mp_networkkill;
	int mp_killedby;
	int mp_coopControlledByPlayer;
	int mp_coopLastTimeSwitchedTarget;
	int mp_updateOn;
	int mp_lastUpdateSent;
	int mp_isLuaChar;
	int mp_rotateType;
	int mp_rotateTimer;
	int underground;
	int beenmoved;
	int detectedlimbhit;
	int iHasParentIndex;
	int lastfootfallframeindex;
	int lastfootfallsound;
	float climbgravity;

	// Constructor
	entitytype ( )
	{
		 climbgravity = 0.0f;
		 lastfootfallsound = 0;
		 lastfootfallframeindex = -1;
		 iHasParentIndex = 0;
		 detectedlimbhit = 0;
		 beenmoved = 0;
		 underground = 0;
		 mp_rotateTimer = 0;
		 mp_rotateType = 0;
		 mp_isLuaChar = 0;
		 mp_lastUpdateSent = 0;
		 mp_updateOn = 0;
		 mp_coopLastTimeSwitchedTarget = 0;
		 mp_coopControlledByPlayer = 0;
		 mp_killedby = 0;
		 mp_networkkill = 0;
		 overprompttimer = 0;
		 overprompt_s = "";
		 editorlock = 0;
		 ragdollifiedforcelimb = 0;
		 ragdollifiedforcevalue_f = 0.0f;
		 ragdollifiedforcez_f = 0.0f;
		 ragdollifiedforcey_f = 0.0f;
		 ragdollifiedforcex_f = 0.0f;
		 ragdollified = 0;
		 particleemitterid = 0;
		 characterSoundBankNumber = 0;
		 abscolradius_f = 0.0f;
		 abscolz_f = 0.0f;
		 abscolx_f = 0.0f;
		 hasbeenbatched = 0;
		 donotreflect = 0;
		 speedmodulator_f = 1.0f;
		 limbhurta_f = 0.0f;
		 limbhurt = 0;
		 plrvisible = 0;
		 isclone = 0;
		 delaytimeactive = 0;
		 delaydestroytimer = 0;
		 delaydestroy = 0;
		 distance = 0;
		 soundplaying = 0;
		 entityacc = 0;
		 objectivecurrentrange = 0;
		 objectiveradarrange = 0;
		 isanobjective = 0;
		 cullimmobile = 0;
		 positionchanged = 0;
		 camrotz = 0;
		 camroty = 0;
		 camrotx = 0;
		 offsetanglez = 0.0f;
		 offsetangley = 0.0f;
		 offsetanglex = 0.0f;
		 camoffsetz = 0;
		 camoffsety = 0;
		 camoffsetx = 0;
		 camuse = 0;
		 isimmune = 0;
		 shotbyammo_s = "";
		 shotby = 0;
		 radardistance = 0;
		 onradar = 0;
		 usefade = 0;
		 cullstate = 0;
		 ishidden = 0;
		 entitydammult_f = 0.0f;
		 floorpositiony = 0.0f;
		 hideshadow = 0;
		 isaltammo = 0;
		 nobulletcol = 0;
		 lastdamagez_f = 0.0f;
		 lastdamagey_f = 0.0f;
		 lastdamagex_f = 0.0f;
		 reserve5 = 0;
		 reserve4 = 0;
		 reserve3 = 0;
		 reserve2 = 0;
		 reserve1 = 0;
		 explodefusetime = 0;
		 destroyme = 0;
		 usingphysicsnow = 0;
		 doorobsactive = 0;
		 videotexture = 0;
		 alttextureused = 0;
		 soundistalking = 0;
		 soundlooping = 0;
		 soundisnonthreedee = 0;
		 soundset4 = 0;
		 soundset3 = 0;
		 soundset2 = 0;
		 soundset1 = 0;
		 soundset = 0;
		 blockedtoz = 0;
		 blockedtoy = 0;
		 blockedtox = 0;
		 blockedby = 0;
		 invincibleactive = 0;
		 collisionactive = 0;
		 activated = 0;
		 collected = 0;
		 delaydir = 0;
		 delaydamagez_f = 0.0f;
		 delaydamagey_f = 0.0f;
		 delaydamagex_f = 0.0f;
		 delayimpact = 0;
		 delaydamage = 0;
		 delaydamagesource = 0;
		 delaydamagetimer = 0.0f;
		 delaydamagecount = 0.0f;
		 shotdamagetype = 0;
		 shotdamagesource = 0;
		 shotdamage = 0;
		 beenkilled = 0;
		 lifecode = 0;
		 briefimmunity = 0;
		 health = 0;
		 floatvalue = 0.0f;
		 floatrate = 0;
		 spinvalue = 0.0f;
		 spinrate = 0;
		 animframeupdate = 0;
		 destanimframe = 0.0f;
		 animonce = 0;
		 animframe = 0.0f;
		 animtime = 0;
		 animdo = 0;
		 animdir = 0;
		 animset = 0;
		 decalsizey = 0.0f;
		 decalsizex = 0.0f;
		 decalslotused = 0;
		 decalloop = 0.0f;
		 decalmode = 0;
		 decalindex = 0;
		 plrdist = 0.0f;
		 crouchprofile = 0;
		 nofloorlogic = 0;
		 raycastlaststate = 0;
		 raycastcount = 0;
		 priorityduration = 0;
		 priorityai = 0;
		 logictimestamp = 0;
		 logiccountburst = 0;
		 logiccount = 0.0f;
		 limbslerp = 0;
		 colb = 0;
		 colg = 0;
		 colr = 0;
		 floorposy = 0.0f;
		 dry = 0.0f;
		 nogravity = 0;
		 norotate = 0;
		 scalez = 0.0f;
		 scaley = 0.0f;
		 scalex = 0.0f;
		 rz = 0.0f;
		 ry = 0.0f;
		 rx = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 lastx = 0.0f;
		 lasty = 0.0f;
		 lastz = 0.0f;
		 attachmentblobobj = 0;
		 attachmentobjfirespotlimb = 0;
		 attachmentweapontype = 0;
		 attachmentbaseobj = 0;
		 attachmentobj = 0;
		 obj = 0;
		 staticflag = 0;
		 profileobj = 0;
		 bankindex = 0;
		 maintype = 0;
		 dormant = 0;
		 active = 0;
		 fakeplrindex = 0;
		 servercontrolled = 0;
		 editorfixed = 0;
		 lastdamagepaused = 0;
		 lastdamagetimer = 0;
		 etimerpaused = 0;
		 etimer = 0;
		 ttarget = 0;
		 aimovez = 0.0f;
		 aimovex = 0.0f;
		 aimoved = 0;
		 eyeoffset = 0;
		 spineraim = 0.0f;
		 spiner = 0.0f;
		 spine = 0;
		 curvcmz = 0;
		 curvcmy = 0;
		 curvcmx = 0;
		 isflak = 0;
		 enterwater = 0;
		 animspeedmod = 1.0f;
	}
	// End of Constructor
};


//  Entity Breadcrumbs Data
struct entitybreadcrumbstype
{
	float x;
	float y;
	float z;

	// Constructor
	entitybreadcrumbstype ( )
	{
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};


//  AI actions
struct aiactiontype
{
	int type;
	float value;
	int valueisvar;
	float valueb;
	cstr filename;
	cstr string1;
	cstr string2;

	// Constructor
	aiactiontype ( )
	{
		 string2 = "";
		 string1 = "";
		 filename = "";
		 valueb = 0.0f;
		 valueisvar = 0;
		 value = 0.0f;
		 type = 0;
	}
	// End of Constructor

};


//  AI item
struct aiactionitemtype
{
	int first;
	int last;

	// Constructor
	aiactionitemtype ( )
	{
		 last = 0;
		 first = 0;
	}
	// End of Constructor

};


//  AI conditions
struct aiconditiontype
{
	int type;
	float valuea;
	float valueb;
	float valuec;
	cstr string1;
	cstr string2;

	// Constructor
	aiconditiontype ( )
	{
		 string2 = "";
		 string1 = "";
		 valuec = 0.0f;
		 valueb = 0.0f;
		 valuea = 0.0f;
		 type = 0;
	}
	// End of Constructor

};

struct aiconditionitemtype
{
	int first;
	int last;
	int action;

	// Constructor
	aiconditionitemtype ( )
	{
		 action = 0;
		 last = 0;
		 first = 0;
	}
	// End of Constructor

};


//  AI Library List
struct ailiblisttype
{
	int loaded;
	cstr file_s;
	cstr desc_s;
	int conditionfirst;
	int conditionlast;

	// Constructor
	ailiblisttype ( )
	{
		 conditionlast = 0;
		 conditionfirst = 0;
		 desc_s = "";
		 file_s = "";
		 loaded = 0;
	}
	// End of Constructor

};



//Waypoint Data Structures


//  Waypoint data structure
struct waypointtype
{
	int active;
	int style;
	int fillcolor;
	int linkedtoentityindex;
	int count;
	int start;
	int finish;
	int polycount;

	// Constructor
	waypointtype ( )
	{
		 polycount = 0;
		 finish = 0;
		 start = 0;
		 count = 0;
		 linkedtoentityindex = 0;
		 fillcolor = 0;
		 style = 0;
		 active = 0;
	}
	// End of Constructor

};

struct waypointcoordtype
{
	float x;
	float y;
	float z;
	int link;
	int index;

	// Constructor
	waypointcoordtype ( )
	{
		 index = 0;
		 link = 0;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};


//  Gun Sounds Data Structure
struct gunsoundbanktype
{
	cstr name_s;
	int soundid1;
	int soundid2;
	int soundid3;
	int soundid4;
	cstr altname_s;
	int altsoundid;

	// Constructor
	gunsoundbanktype ( )
	{
		 altsoundid = 0;
		 altname_s = "";
		 soundid4 = 0;
		 soundid3 = 0;
		 soundid2 = 0;
		 soundid1 = 0;
		 name_s = "";
	}
	// End of Constructor

};

struct gunsoundbankcompaniontype
{
	int soundid;

	// Constructor
	gunsoundbankcompaniontype ( )
	{
		 soundid = 0;
	}
	// End of Constructor

};

struct gunsounditemtype
{
	int keyframe;
	int playsound;
	int lastplay;

	// Constructor
	gunsounditemtype ( )
	{
		 lastplay = 0;
		 playsound = 0;
		 keyframe = 0;
	}
	// End of Constructor

};

struct gunsoundstype
{
	int soundframes;
	int fireloopend;
	int altfireloopend;

	// Constructor
	gunsoundstype ( )
	{
		 altfireloopend = 0;
		 fireloopend = 0;
		 soundframes = 0;
	}
	// End of Constructor

};


//  Gun Pose Used By Character Waste Twist
struct charactergunposetype
{
	float x;
	float y;
	float z;

	// Constructor
	charactergunposetype ( )
	{
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
	}
	// End of Constructor

};

//  Gun and Melee Globals
struct gunandmeleetype
{
	int tmouseheld;
	int pressedtrigger;
	int hudbankcrosshairtexture;

	// Constructor
	gunandmeleetype ( )
	{
		 hudbankcrosshairtexture = 0;
		 pressedtrigger = 0;
		 tmouseheld = 0;
	}
	// End of Constructor

};

//  Gun Settings Data Structure
struct gunsettingstype
{
	int emptyreloadonly;
	int gravitygun;
	int gravitystrength;
	int magnet;
	int magnetstrength;
	float jamchance;
	float jamchancetime;
	int jammed;
	int overheatafter;
	int shotsfired;
	int cooldown;
	int cooltimer;
	int nosubmergedfire;
	int addtimer;
	int mintimer;
	int modessharemags;
	int bulletamount;
	int currentbullet;
	int bulletlimbsmax;
	int bulletlimbstart;
	int bulletlimbend;
	int bulletreset;
	int bulletmod;
	int meleedamage;
	int meleerange;
	int minpolytrim;
	int decalforward;
	int meleenoscorch;
//  `altmeleekey as integer

	int altmeleedamage;
	int altmeleerange;
	int altmeleenoscorch;
	int ismelee;
	int disablerunandshoot;
	int altpoolindex;
	int poolindex;
	int isempty;
	int muzzleflash;
	float muzzlex_f;
	float muzzley_f;
	float muzzlez_f;
	float muzzlesize_f;
	int muzzlecolorr;
	int muzzlecolorg;
	int muzzlecolorb;
	int brass;
	float brasslife;
	float brassangle;
	float brassanglerand;
	float brassspeed;
	float brassspeedrand;
	float brassupward;
	float brassupwardrand;
	float brassrotx;
	float brassrotxrand;
	float brassroty;
	float brassrotyrand;
	float brassrotz;
	float brassrotzrand;
	int smoke;
	int flashlimb;
	int brasslimb;
	int smokelimb;
	int flashlimb2;
	int brasslimb2;
	int smokelimb2;
	int handlimb;
	int flashimg;
	int brassobjmaster;
	int smokeimg;
	int crosshairimg;
	cstr flakname_s;
	int flakindex;
	int flaklimb;
	int flakrearmframe;
	int seconduse;
	int damage;
	int damagetype;
	int scorchtype;
	int reloadqty;
	float reloadspeed;
	int weaponisammo;
	int iterate;
	int accuracy;
	int detectcoloff;
	int ignorematerial;
	int zoommode;
	int zoomaccuracy;
	int zoomaccuracybreathhold;
	int zoomaccuracybreath;
	int zoomaccuracyheld;
	int range;
	int dropoff;
	int usespotlighting;
	int smokespeed;
	cstr smokedecal_s;
	int firerate;
	int altfirerate;
	int shotgun;
	int emptyshotgun;
	float rotx_f;
	float roty_f;
	float rotz_f;
	int equipment;
	int equipmententityelementindex;
	int lockcamera;
	int burst;
	float gunlagSpeed;
	float gunlagXmax;
	float gunlagYmax;
	unsigned char noscorch;
	int soundstrength;
	int simplezoom;
	int simplezoommod;
	int simplezoomacc;
	float simplezoomx_f;
	float simplezoomy_f;
	int simplezoomspeed;
	int simplezoomanim;
	int simplezoomflash;
	float zoommuzzlex_f;
	float zoommuzzley_f;
	float zoommuzzlez_f;
	int alternateisflak;
	int alternateisray;
	int altreloadqty;
	int altiterate;
	int altaccuracy;
	int altdamage;
	int altrange;
	float altmuzzlex_f;
	float altmuzzley_f;
	float altmuzzlez_f;
	int altmuzzleflash;
	int altflashimg;
	int alternate;
	float recoily_f;
	float recoilx_f;
	float recoilycorrect_f;
	float recoilxcorrect_f;
	float zoomrecoily_f;
	float zoomrecoilx_f;
	float zoomrecoilycorrect_f;
	float zoomrecoilxcorrect_f;
	float altrecoily_f;
	float altrecoilx_f;
	float altrecoilycorrect_f;
	float altrecoilxcorrect_f;
	int forcezoomout;
	int ammomax;
	float runy_f;
	float runx_f;
	int runaccuracy;
	int runanimdelay;
	unsigned char noautoreload;
	unsigned char nofullreload;
	unsigned char chamberedround;
	unsigned char altchamberedround;
	unsigned char movespeedmod;
	float zoomgunlagSpeed;
	float zoomgunlagXmax;
	float zoomgunlagYmax;
	float zoomwalkspeed;
	float zoomturnspeed;
	float plrmovespeedmod;
	float plremptyspeedmod;
	float plrturnspeedmod;
	float plrjumpspeedmod;
	float plrreloadspeedmod;
	int hasempty;
	int canaddtospare;
	int addtospare;
	int force;
	int meleeforce;
	int npcignorereload;
	int dofdistance;
	int dofintensity;
	int zoomhidecrosshair;

	// Constructor
	gunsettingstype ( )
	{
		 zoomhidecrosshair = 0;
		 dofintensity = 0;
		 dofdistance = 0;
		 npcignorereload = 0;
		 meleeforce = 0;
		 force = 0;
		 addtospare = 0;
		 canaddtospare = 0;
		 hasempty = 0;
		 plrreloadspeedmod = 0.0f;
		 plrjumpspeedmod = 0.0f;
		 plrturnspeedmod = 0.0f;
		 plremptyspeedmod = 0.0f;
		 plrmovespeedmod = 0.0f;
		 zoomturnspeed = 0.0f;
		 zoomwalkspeed = 0.0f;
		 zoomgunlagYmax = 0.0f;
		 zoomgunlagXmax = 0.0f;
		 zoomgunlagSpeed = 0.0f;
		 movespeedmod = 0;
		 altchamberedround = 0;
		 chamberedround = 0;
		 nofullreload = 0;
		 noautoreload = 0;
		 runaccuracy = 0;
		 runanimdelay = 0;
		 runx_f = 0.0f;
		 runy_f = 0.0f;
		 ammomax = 0;
		 forcezoomout = 0;
		 altrecoilxcorrect_f = 0.0f;
		 altrecoilycorrect_f = 0.0f;
		 altrecoilx_f = 0.0f;
		 altrecoily_f = 0.0f;
		 zoomrecoilxcorrect_f = 0.0f;
		 zoomrecoilycorrect_f = 0.0f;
		 zoomrecoilx_f = 0.0f;
		 zoomrecoily_f = 0.0f;
		 recoilxcorrect_f = 0.0f;
		 recoilycorrect_f = 0.0f;
		 recoilx_f = 0.0f;
		 recoily_f = 0.0f;
		 alternate = 0;
		 altflashimg = 0;
		 altmuzzleflash = 0;
		 altmuzzlez_f = 0.0f;
		 altmuzzley_f = 0.0f;
		 altmuzzlex_f = 0.0f;
		 altrange = 0;
		 altdamage = 0;
		 altaccuracy = 0;
		 altiterate = 0;
		 altreloadqty = 0;
		 alternateisray = 0;
		 alternateisflak = 0;
		 zoommuzzlez_f = 0.0f;
		 zoommuzzley_f = 0.0f;
		 zoommuzzlex_f = 0.0f;
		 simplezoomflash = 0;
		 simplezoomanim = 0;
		 simplezoomspeed = 0;
		 simplezoomy_f = 0.0f;
		 simplezoomx_f = 0.0f;
		 simplezoomacc = 0;
		 simplezoommod = 0;
		 simplezoom = 0;
		 soundstrength = 0;
		 noscorch = 0;
		 gunlagYmax = 0.0f;
		 gunlagXmax = 0.0f;
		 gunlagSpeed = 0.0f;
		 burst = 0;
		 lockcamera = 0;
		 equipmententityelementindex = 0;
		 equipment = 0;
		 rotz_f = 0.0f;
		 roty_f = 0.0f;
		 rotx_f = 0.0f;
		 emptyshotgun = 0;
		 shotgun = 0;
		 altfirerate = 0;
		 firerate = 0;
		 smokedecal_s = "";
		 smokespeed = 0;
		 usespotlighting = 0;
		 dropoff = 0;
		 range = 0;
		 zoomaccuracyheld = 0;
		 zoomaccuracybreath = 0;
		 zoomaccuracybreathhold = 0;
		 zoomaccuracy = 0;
		 zoommode = 0;
		 detectcoloff = 0;
		 ignorematerial = 0;
		 accuracy = 0;
		 iterate = 0;
		 weaponisammo = 0;
		 reloadspeed = 0.0f;
		 reloadqty = 0;
		 scorchtype = 0;
		 damagetype = 0;
		 damage = 0;
		 seconduse = 0;
		 flakrearmframe = 0;
		 flaklimb = 0;
		 flakindex = 0;
		 flakname_s = "";
		 crosshairimg = 0;
		 smokeimg = 0;
		 brassobjmaster = 0;
		 flashimg = 0;
		 handlimb = 0;
		 smokelimb2 = 0;
		 brasslimb2 = 0;
		 flashlimb2 = 0;
		 smokelimb = 0;
		 brasslimb = 0;
		 flashlimb = 0;
		 smoke = 0;
		 brassrotzrand = 0.0f;
		 brassrotz = 0.0f;
		 brassrotyrand = 0.0f;
		 brassroty = 0.0f;
		 brassrotxrand = 0.0f;
		 brassrotx = 0.0f;
		 brassupwardrand = 0.0f;
		 brassupward = 0.0f;
		 brassspeedrand = 0.0f;
		 brassspeed = 0.0f;
		 brassanglerand = 0.0f;
		 brassangle = 0.0f;
		 brasslife = 0.0f;
		 brass = 0;
		 muzzlecolorb = 0;
		 muzzlecolorg = 0;
		 muzzlecolorr = 0;
		 muzzlesize_f = 0.0f;
		 muzzlez_f = 0.0f;
		 muzzley_f = 0.0f;
		 muzzlex_f = 0.0f;
		 muzzleflash = 0;
		 isempty = 0;
		 poolindex = 0;
		 altpoolindex = 0;
		 disablerunandshoot = 0;
		 ismelee = 0;
		 altmeleenoscorch = 0;
		 altmeleerange = 0;
		 altmeleedamage = 0;
		 meleenoscorch = 0;
		 decalforward = 0;
		 minpolytrim = 0;
		 meleerange = 0;
		 meleedamage = 0;
		 bulletmod = 0;
		 bulletreset = 0;
		 bulletlimbend = 0;
		 bulletlimbstart = 0;
		 bulletlimbsmax = 0;
		 currentbullet = 0;
		 bulletamount = 0;
		 modessharemags = 0;
		 mintimer = 0;
		 addtimer = 0;
		 nosubmergedfire = 0;
		 cooltimer = 0;
		 cooldown = 0;
		 shotsfired = 0;
		 overheatafter = 0;
		 jammed = 0;
		 jamchancetime = 0.0f;
		 jamchance = 0.0f;
		 magnetstrength = 0;
		 magnet = 0;
		 gravitystrength = 0;
		 gravitygun = 0;
		 emptyreloadonly = 0;
	}
	// End of Constructor

};


struct gunparticletype
{
	int id;
	float velocity;
	cstr decal_s;

	// Constructor
	gunparticletype ( )
	{
		 decal_s = "";
		 velocity = 0.0f;
		 id = 0;
	}
	// End of Constructor

};


//  Gun Actions Data Structure
struct gunanimtype
{
	int s;
	int e;

	// Constructor
	gunanimtype ( )
	{
		 e = 0;
		 s = 0;
	}
	// End of Constructor

};

struct gunactionstype
{
	gunanimtype show;
	gunanimtype idle;
	gunanimtype move;
	gunanimtype run;
	gunanimtype flattentochest;
	gunanimtype start;
	gunanimtype automatic;
	gunanimtype finish;
	gunanimtype laststart;
	gunanimtype lastfinish;
	gunanimtype startreload;
	gunanimtype reloadloop;
	gunanimtype endreload;
	gunanimtype cock; // (Michael Johnson)
	gunanimtype hide;
	gunanimtype block;
	gunanimtype start2;
	gunanimtype finish2;
	gunanimtype start3;
	gunanimtype finish3;
	gunanimtype to;
	gunanimtype from;
	gunanimtype dryfire;

	// Constructor
	gunactionstype ( )
	{
	}
	// End of Constructor

};


struct gunanimextras
{
	gunanimtype clearjam;
	gunanimtype handpush;
	gunanimtype handdead;
	gunanimtype jammed;
	gunanimtype handdoor;
	gunanimtype swaptoalt;
	gunanimtype handbutton;
	gunanimtype handtake;
	gunanimtype pullup;
	gunanimtype pulldown;
	gunanimtype pullleft;
	gunanimtype pullright;

	// Constructor
	gunanimextras ( )
	{
	}
	// End of Constructor

};


//  Gun Data Structure (is 10 our maximum for the entire game, ie weapon slots max?)
struct guntype
{
	gunparticletype particle;
	int firemode;
	int activeingame;
	int obj;
	int secondobj;
	cstr name_s;
	cstr projectile_s;
	int projectileframe;
	int weapontype;
	cstr texd_s;
	cstr effect_s;
	cstr zoomscope_s;
	int zoomscope;
	cstr decal_s;
	int decalid;
	int effectidused;
	int transparency;
	gunsettingstype settings;
	gunsoundstype sound;
	gunactionstype action;
	gunanimextras action2;
	int hudimage;
	float horiz_f;
	float vert_f;
	float forward_f;
	gunactionstype zoomaction;
	gunactionstype altaction;
	gunactionstype emptyaction;
	gunanimtype emptyzoomactionhide;
	gunanimtype emptyzoomactionshow;
	gunanimtype emptyzoomactionidle;
	gunanimtype emptyzoomactionmove;
	gunanimtype emptyzoomactiondryfire;
	gunanimtype emptyaltactionto;
	gunanimtype emptyaltactionfrom;
	gunactionstype meleeaction;
	gunactionstype altmeleeaction;
	int statuspanelcode;
	int texdid;
	int texnid;
	int texmid;
	int texgid;
	int texaoid;
	int texiid;
	int texhid;
	int extraformp;
	cstr vweaptex_s;
	int ammoimg;
	int iconimg;
	float keyframespeed_f;
	int invertnormal;
	int preservetangents;
	float boostintensity;

	// Constructor
	guntype ( )
	{
		 keyframespeed_f = 1.0f;
		 iconimg = 0;
		 ammoimg = 0;
		 vweaptex_s = "";
		 extraformp = 0;
		 texhid = 0;
		 texaoid = 0;
		 texgid = 0;
		 texiid = 0;
		 texmid = 0;
		 texnid = 0;
		 texdid = 0;
		 statuspanelcode = 0;
		 forward_f = 0.0f;
		 vert_f = 0.0f;
		 horiz_f = 0.0f;
		 hudimage = 0;
		 transparency = 0;
		 effectidused = 0;
		 decalid = 0;
		 decal_s = "";
		 zoomscope = 0;
		 zoomscope_s = "";
		 effect_s = "";
		 texd_s = "";
		 weapontype = 0;
		 projectileframe = 0;
		 projectile_s = "";
		 name_s = "";
		 secondobj = 0;
		 obj = 0;
		 activeingame = 0;
		 firemode = 0;
	}
	// End of Constructor

};

//  Ammo Pool
struct ammopooltype
{
	cstr name_s;
	int ammo;
	int used;

	// Constructor
	ammopooltype ( )
	{
		 used = 0;
		 ammo = 0;
		 name_s = "";
	}
	// End of Constructor

};

//  Decal Structure (explosions, lightdlare, smoke, steam, plasma)
struct decaltype
{
	int active;
	int imageid;
	cstr name_s;
	int across;
	int down;
	int variants;
	float playspeed_f;
	int framemax;
	decalparticletype particle;

	// Constructor
	decaltype ( )
	{
		 framemax = 0;
		 playspeed_f = 0.0f;
		 variants = 0;
		 down = 0;
		 across = 0;
		 name_s = "";
		 imageid = 0;
		 active = 0;
	}
	// End of Constructor

};


//  Decal Element Structure (reusable instances)
struct decalelementtype
{
	int decalid;
	int active;
	int obj;
	int uvgridsize;
	float xpos;
	float ypos;
	float zpos;
	float frame;
	float framefinish;
	float framedelay;
	DWORD fadestarttime;
	int fadespan;
	int orient;
	int originator;
	int originatorobj;
	float scalemodx;
	float scalemody;
	decalparticletype particle;
	float particleix;
	float particleiy;
	float particleiz;
	int particlemirror;
	int burstloop;
	int decalforward;

	// Constructor
	decalelementtype ( )
	{
		 decalforward = 0;
		 burstloop = 0;
		 particlemirror = 0;
		 particleiz = 0.0f;
		 particleiy = 0.0f;
		 particleix = 0.0f;
		 scalemody = 0.0f;
		 scalemodx = 0.0f;
		 originatorobj = 0;
		 originator = 0;
		 orient = 0;
		 fadespan = 0;
		 fadestarttime = 0;
		 framedelay = 0.0f;
		 framefinish = 0.0f;
		 frame = 0.0f;
		 zpos = 0.0f;
		 ypos = 0.0f;
		 xpos = 0.0f;
		 uvgridsize = 0;
		 obj = 0;
		 active = 0;
		 decalid = 0;
	}
	// End of Constructor

};

//  Input Command Structure
struct inputcommandtype
{
	int take;
	int drop;
	int newweapon;

	// Constructor
	inputcommandtype ( )
	{
		 newweapon = 0;
		 drop = 0;
		 take = 0;
	}
	// End of Constructor

};

struct playerstatetype
{
	int moving;
	int firingmode;
	int blockingaction;
	int empty;

	// Constructor
	playerstatetype ( )
	{
		 empty = 0;
		 blockingaction = 0;
		 firingmode = 0;
		 moving = 0;
	}
	// End of Constructor

};


//  Weapon Data Structure
struct weaponslottype
{
	int pref;
	int got;
	int invpos;
	int noselect;

	// Constructor
	weaponslottype ( )
	{
		 noselect = 0;
		 invpos = 0;
		 got = 0;
		 pref = 0;
	}
	// End of Constructor

};

//  Inventory Data Structure
struct inventorytype
{
	int isnew;
	int drop;
	int index;

	// Constructor
	inventorytype ( )
	{
		 index = 0;
		 drop = 0;
		 isnew = 0;
	}
	// End of Constructor

};


//  Players Data Structure
struct playerpowerstype
{
	int level;

	// Constructor
	playerpowerstype ( )
	{
		 level = 0;
	}
	// End of Constructor

};

struct playertype
{
	inputcommandtype command;
	playerstatetype state;
	int health;
	int lives;
	playerpowerstype powers;

	// Constructor
	playertype ( )
	{
		 lives = 0;
		 health = 0;
	}
	// End of Constructor

};

struct playercheckpointtype
{
	int stored;
	float x;
	float y;
	float z;
	float a;
	int soundloopcheckpointcountdown;

	// Constructor
	playercheckpointtype ( )
	{
		 soundloopcheckpointcountdown = 0;
		 a = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 stored = 0;
	}
	// End of Constructor

};

//  Slider menus and panels
struct slidersmenutype
{
	int tabpage;
	cstr title_s;
	int itemcount;
	int ttop;
	int tleft;
	int tclick;
	int tallowdragging;
	int tdrag;
	int thighlight;
	int titlemargin;
	int leftmargin;
	int panelheight;
	int readonly;
	int customimage;
	int customimagetype;
	int customimagesubtype;
	int minmaxbuttonlit;
	int minimised;
	bool bFrozenPanelFromLastCycle;
	bool bPermitMovementEvenIfOverlap;

	// Constructor
	slidersmenutype ( )
	{
		 bPermitMovementEvenIfOverlap = false;
		 bFrozenPanelFromLastCycle = false;
		 minimised = 0;
		 minmaxbuttonlit = 0;
		 customimagesubtype = 0;
		 customimagetype = 0;
		 customimage = 0;
		 readonly = 0;
		 panelheight = 0;
		 leftmargin = 0;
		 titlemargin = 0;
		 thighlight = 0;
		 tdrag = 0;
		 tallowdragging = 0;
		 tclick = 0;
		 tleft = 0;
		 ttop = 0;
		 itemcount = 0;
		 title_s = "";
		 tabpage = 0;
	}
	// End of Constructor

};

struct slidersmenuvaluetype
{
	cstr name_s;
	int value;
	cstr value_s;
	int readmodeindex;
	int expanddetect;
	int gadgettype;
	int gadgettypevalue;
	int useCustomRange;
	int valueMin;
	int valueMax;

	// Constructor
	slidersmenuvaluetype ( )
	{
		 valueMax = 0;
		 valueMin = 0;
		 useCustomRange = 0;
		 gadgettypevalue = 0;
		 gadgettype = 0;
		 expanddetect = 0;
		 readmodeindex = 0;
		 value_s = "";
		 value = 0;
		 name_s = "";
	}
	// End of Constructor

};

struct slidersmenunamestype
{
	int aidrilldown;
	int performance;
	int visuals;
	int camera;
	int sky;
	int weapon;
	int player;
	int graphicoptions;
	int shaderoptions;
	int qualitypanel;
	int worldpanel;
	int yesnopanel;
	int posteffects;

	// Constructor
	slidersmenunamestype ( )
	{
		 posteffects = 0;
		 yesnopanel = 0;
		 worldpanel = 0;
		 qualitypanel = 0;
		 shaderoptions = 0;
		 graphicoptions = 0;
		 player = 0;
		 weapon = 0;
		 sky = 0;
		 camera = 0;
		 visuals = 0;
		 performance = 0;
		 aidrilldown = 0;
	}
	// End of Constructor

};

//  Infini-Light Data Structure
struct colrgbtype
{
	int r;
	int g;
	int b;

	// Constructor
	colrgbtype ( )
	{
		 b = 0;
		 g = 0;
		 r = 0;
	}
	// End of Constructor

};

struct infinilighttype
{
	int used;
	int type;
	float x;
	float y;
	float z;
	float range;
	int id;
	float dist;
	colrgbtype colrgb;
	int islit;
	int e;
	float distfromcam_f;
	float intensity_f;

	// Constructor
	infinilighttype ( )
	{
		 intensity_f = 0.0f;
		 distfromcam_f = 0.0f;
		 e = 0;
		 islit = 0;
		 dist = 0.0f;
		 id = 0;
		 range = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 type = 0;
		 used = 0;
	}
	// End of Constructor

};

//  Shadow Lights Data Structure
struct shadowlighttype
{
	float x_f;
	float y_f;
	float z_f;
	float range_f;

	// Constructor
	shadowlighttype ( )
	{
		 range_f = 0.0f;
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
	}
	// End of Constructor

};


//  Data structure for Bit-Fragments
struct bitdetailstype
{
	int max;
	cstr file_s;
	int collisionmode;
	int fragmentobjstart;
	int choice;
	float sizex;
	float sizey;
	float sizez;

	// Constructor
	bitdetailstype ( )
	{
		 sizez = 0.0f;
		 sizey = 0.0f;
		 sizex = 0.0f;
		 choice = 0;
		 fragmentobjstart = 0;
		 collisionmode = 0;
		 file_s = "";
		 max = 0;
	}
	// End of Constructor

};

struct bitoffsettype
{
	float x_f;
	float y_f;
	float z_f;

	// Constructor
	bitoffsettype ( )
	{
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
	}
	// End of Constructor

};


//  Explosion Data Structure
struct explodetype
{
	int obj;
	int explode;
	int explodesound;
	float dist_f;
	float smoke_f;
	float fade_f;
	float x;
	float y;
	float z;

	// Constructor
	explodetype ( )
	{
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 fade_f = 0.0f;
		 smoke_f = 0.0f;
		 dist_f = 0.0f;
		 explodesound = 0;
		 explode = 0;
		 obj = 0;
	}
	// End of Constructor

};


//  GUI Visual Settings
struct guivisualsettingstype
{
	int updateflag;
	int ambience;
	int ambienceoverride;
	int shadows;
	int shadowstrength;
	int shadowshades;
	int dynamiclightcount;
	int ambiencescripted;

	// Constructor
	guivisualsettingstype ( )
	{
		 ambiencescripted = 0;
		 dynamiclightcount = 0;
		 shadowshades = 0;
		 shadowstrength = 0;
		 shadows = 0;
		 ambienceoverride = 0;
		 ambience = 0;
		 updateflag = 0;
	}
	// End of Constructor

};


//  HUD structure
struct hudtype
{
	int uservarislocal;
	int uservar;
	cstr name_s;
	int maintype;
	int typemode;
	float posx;
	float posy;
	float posz;
	int sizex;
	int sizey;
	int sizez;
	int red;
	int green;
	int blue;
	int fadeout;
	int baseanim;
	int image;
	cstr font_s;
	int fontsize;
	cstr text_s;
	int hide;
	float hidedelay;
	int highlighted;
	int width;
	int height;
	cstr imagefile_s;
	int scalex;
	int scaley;
	int update;
	int alpha;
	int layer;

	// Constructor
	hudtype ( )
	{
		 layer = 0;
		 alpha = 0;
		 update = 0;
		 scaley = 0;
		 scalex = 0;
		 imagefile_s = "";
		 height = 0;
		 width = 0;
		 highlighted = 0;
		 hidedelay = 0.0f;
		 hide = 0;
		 text_s = "";
		 fontsize = 0;
		 font_s = "";
		 image = 0;
		 baseanim = 0;
		 fadeout = 0;
		 blue = 0;
		 green = 0;
		 red = 0;
		 sizez = 0;
		 sizey = 0;
		 sizex = 0;
		 posz = 0.0f;
		 posy = 0.0f;
		 posx = 0.0f;
		 typemode = 0;
		 maintype = 0;
		 name_s = "";
		 uservar = 0;
		 uservarislocal = 0;
	}
	// End of Constructor

};


//  LightRay Types
struct FrameTargetData
{
	int Object;
	int Effect;
	cstr Teq_s;

	// Constructor
	FrameTargetData ( )
	{
		 Teq_s = "";
		 Effect = 0;
		 Object = 0;
	}
	// End of Constructor

};


struct RenderTargetData
{
	int Camera;
	int Image;
	int Object;
	int Effect;
	cstr Teq_s;

	// Constructor
	RenderTargetData ( )
	{
		 Teq_s = "";
		 Effect = 0;
		 Object = 0;
		 Image = 0;
		 Camera = 0;
	}
	// End of Constructor

};


//  Respawn array for arena game
struct respawntype
{
	int e;
	float x;
	float y;
	float z;
	float ry;

	// Constructor
	respawntype ( )
	{
		 ry = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 e = 0;
	}
	// End of Constructor

};





//Editor Structure Types




//  Input Control System Data Structure
struct inputsystemtype
{
	int activemouse;
	int residualidecount;
	int xmouse;
	int ymouse;
	int zmouse;
	int zmouselast;
	int xmousemove;
	int ymousemove;
	int wheelmousemove;
	int mclick;
	int mclickreleasestate;
	int picksystemused;
	int lockdown;
	cstr k_s;
	int mmx;
	int mmy;
	int atrest;
	float atrestx;
	float atresty;
	float localx_f;
	float localy_f;
	float originallocalx_f;
	float originallocaly_f;	
	float localcurrentterrainheight_f;
	float dragoffsetx_f;
	float dragoffsety_f;
	int kscancode;
	int keypress;
	int keypressallowshift;
	int keycontrol;
	int keyalt;
	int keyreturn;
	int keyshift;
	int keyleft;
	int keyright;
	int keyup;
	int keydown;
	int keyspace;
	int constructselection;
	int dorotation;
	int domirror;
	int doflip;
	int doload;
	int dosave;
	int doopen;
	int donew;
	int donewflat;
	int dosaveas;
	int dosaveandrun;
	int domodepaint;
	int domodeselect;
	int domodeart;
	int domodeterrain;
	int domodeentity;
	int domodemarker;
	int domodewaypoint;
	int domodewaypointcreate;
	int dowaypointview;
	int doentityrotate;
	int dozoomin;
	int dozoomout;
	int doscrollleft;
	int doscrollright;
	int doscrollup;
	int doscrolldown;
	int domapresize;
	int dogroundmode;
	int doundo;
	int doredo;
	int dozoomview;
	int doautozoomview;
	int dozoomviewmovex;
	int dozoomviewmovey;
	int dozoomviewmovez;
	int dozoomviewrotatex;
	int dozoomviewrotatey;
	int dozoomviewrotatez;
	int dosinglelayer;
	int dopickmapid;
	int doartresize;
	int undoredoevent;
	int undokeypress;
	int tselcontrol;
	int tselcut;
	int tselcopy;
	int tselfloor;
	int tselpaste;
	int tseldelete;
	int tselwipe;
	int willmakethirdperson;
	int rubberbandmode;
	int spacekeynotreleased;
	float rubberbandx;
	float rubberbandy;

	// Constructor
	inputsystemtype ( )
	{
		rubberbandy = 0;
		rubberbandx = 0;
		spacekeynotreleased = 0;
		rubberbandmode = 0;
		willmakethirdperson = 0;
		tselwipe = 0;
		tseldelete = 0;
		tselpaste = 0;
		tselfloor = 0;
		tselcopy = 0;
		tselcut = 0;
		tselcontrol = 0;
		undokeypress = 0;
		undoredoevent = 0;
		doartresize = 0;
		dopickmapid = 0;
		dosinglelayer = 0;
		dozoomviewrotatez = 0;
		dozoomviewrotatey = 0;
		dozoomviewrotatex = 0;
		dozoomviewmovez = 0;
		dozoomviewmovey = 0;
		dozoomviewmovex = 0;
		doautozoomview = 0;
		dozoomview = 0;
		doredo = 0;
		doundo = 0;
		dogroundmode = 0;
		domapresize = 0;
		doscrolldown = 0;
		doscrollup = 0;
		doscrollright = 0;
		doscrollleft = 0;
		dozoomout = 0;
		dozoomin = 0;
		doentityrotate = 0;
		dowaypointview = 0;
		domodewaypointcreate = 0;
		domodewaypoint = 0;
		domodeentity = 0;
		domodemarker = 0;
		domodeterrain = 0;
		domodeart = 0;
		domodeselect = 0;
		domodepaint = 0;
		dosaveandrun = 0;
		dosaveas = 0;
		donewflat = 0;
		donew = 0;
		doopen = 0;
		dosave = 0;
		doload = 0;
		doflip = 0;
		domirror = 0;
		dorotation = 0;
		constructselection = 0;
		keyspace = 0;
		keydown = 0;
		keyup = 0;
		keyright = 0;
		keyleft = 0;
		keyshift = 0;
		keyreturn = 0;
		keyalt = 0;
		keycontrol = 0;
		keypressallowshift = 0;
		keypress = 0;
		kscancode = 0;
		dragoffsety_f = 0.0f;
		dragoffsetx_f = 0.0f;
		localcurrentterrainheight_f = 0.0f;
		localy_f = 0.0f;
		localx_f = 0.0f;
		atresty = 0.0f;
		atrestx = 0.0f;
		atrest = 0;
		mmy = 0;
		mmx = 0;
		k_s = "";
		lockdown = 0;
		picksystemused = 0;
		mclickreleasestate = 0;
		mclick = 0;
		wheelmousemove = 0;
		ymousemove = 0;
		xmousemove = 0;
		zmouselast = 0;
		zmouse = 0;
		ymouse = 0;
		xmouse = 0;
		residualidecount = 0;
		activemouse = 0;
	}
	// End of Constructor

};

//  UndoRedo list data structure
struct undoredotype
{
	int event;
	int x;
	int y;
	int z;
	int a;
	int b;

	// Constructor
	undoredotype ( )
	{
		 b = 0;
		 a = 0;
		 z = 0;
		 y = 0;
		 x = 0;
		 event = 0;
	}
	// End of Constructor

};


//  Level handling
struct leveltype
{
	cstr fpm_s;
	cstr fpi_s;

	// Constructor
	leveltype ( )
	{
		 fpi_s = "";
		 fpm_s = "";
	}
	// End of Constructor

};


//  Free flight camera mode
struct editorfreeflightcameratype
{
	float x_f;
	float y_f;
	float z_f;
	float angx_f;
	float angy_f;

	// Constructor
	editorfreeflightcameratype ( )
	{
		 angy_f = 0.0f;
		 angx_f = 0.0f;
		 z_f = 0.0f;
		 y_f = 0.0f;
		 x_f = 0.0f;
	}
	// End of Constructor

};

struct editorfreeflighttype
{
	int mode;
	int storemousex;
	int storemousey;
	editorfreeflightcameratype c;
	int sused;
	editorfreeflightcameratype s;

	// Constructor
	editorfreeflighttype ( )
	{
		 sused = 0;
		 storemousey = 0;
		 storemousex = 0;
		 mode = 0;
	}
	// End of Constructor

};



//Segment Data

/*      

//  Segment Profile Data Structure
struct segmentprofileheadertype
{
	int partmax;
	int lastmeshmax;
	int preventai;

	// Constructor
	segmentprofileheadertype ( )
	{
		 preventai = 0;
		 lastmeshmax = 0;
		 partmax = 0;
	}
	// End of Constructor

};

//  V110 - 160508 - added flag to control portal generation
// `type segmentprofilevistype overlay f r wb wr wf wl owb owr owf owl ctl ctr cbr cbl octl octr ocbr ocbl endtype

struct segmentprofilevistype overlay t.f t.r wb wr wf wl owb owr owf owl ctl ctr cbr cbl octl octr ocbr ocbl portalmode endtype
{
int type;
	int mode;
	int symbol;
	int floorsizey;
	int sidesizex;
	int sidesizey;
	int sidesizez;

	// Constructor
	segmentprofilevistype overlay t.f t.r wb wr wf wl owb owr owf owl ctl ctr cbr cbl octl octr ocbr ocbl portalmode endtype ( )
	{
		 sidesizez = 0;
		 sidesizey = 0;
		 sidesizex = 0;
		 floorsizey = 0;
		 symbol = 0;
		 mode = 0;
		type = 0;
	}
	// End of Constructor

};

struct segmentprofileproperties
{
	int groundmode;
	int kindof;

	// Constructor
	segmentprofileproperties ( )
	{
		 kindof = 0;
		 groundmode = 0;
	}
	// End of Constructor

};

struct materialtype
{
	int t.index;

	// Constructor
	materialtype ( )
	{
		 t.index = 0;
	}
	// End of Constructor

};

struct segmentprofiletype
{
	int partmode;
	cstr meshname_s;
	int meshid;
	int actuallimb;
	float offx;
	float offy;
	float offz;
	float rotx;
	float roty;
	float t.rotz;
	cstr tex_s;
	cstr texd_s;
	cstr texn_s;
	cstr texs_s;
	cstr effect_s;
	int texid;
	int texdid;
	int texiid;
	int texnid;
	int texsid;
	int t.effectid;
	int transparency;
	int colmode;
	int vis;
	int blueprint;
	int properties;
	cstr csgmesh_s;
	int csgmode;
	int csgimmune;
	int lightmode;
	int multimeshmode;
	int multimeshstart;
	int t.material;
	int approxmemused;

	// Constructor
	segmentprofiletype ( )
	{
		 approxmemused = 0;
		 t.material = 0;
		 multimeshstart = 0;
		 multimeshmode = 0;
		 lightmode = 0;
		 csgimmune = 0;
		 csgmode = 0;
		 csgmesh_s = "";
		 properties = 0;
		 blueprint = 0;
		 vis = 0;
		 colmode = 0;
		 transparency = 0;
		 t.effectid = 0;
		 texsid = 0;
		 texnid = 0;
		 texiid = 0;
		 texdid = 0;
		 texid = 0;
		 effect_s = "";
		 texs_s = "";
		 texn_s = "";
		 texd_s = "";
		 tex_s = "";
		 t.rotz = 0.0f;
		 roty = 0.0f;
		 rotx = 0.0f;
		 offz = 0.0f;
		 offy = 0.0f;
		 offx = 0.0f;
		 actuallimb = 0;
		 meshid = 0;
		 meshname_s = "";
		 partmode = 0;
	}
	// End of Constructor

};

*/    

// 
//  New Types For Reloaded
// 

//  Real Time Light Mapping
struct objmetatype
{
	int active;
	int lightmapped;
	int blocking;
	int lightmappedbefore;
	int memused;

	// Constructor
	objmetatype ( )
	{
		 memused = 0;
		 lightmappedbefore = 0;
		 blocking = 0;
		 lightmapped = 0;
		 active = 0;
	}
	// End of Constructor

};

struct realtimelightmappingtype
{
	int busy;
	int liofinterest;
	int objofinterest;
	float objofinterestx;
	float objofinteresty;
	float objofinterestz;
	float oldlightx_f;
	float oldlighty_f;
	float oldlightz_f;
	float oldlightrange_f;
	int includeoldlightstart;
	int lmintervalat;
	int lminterval;
	//int liofinterest;
	int lasttimer;
	int cascadecount;
	int carryonfromo;
	int carryonfromo1;
	int carryonfromo2;
	int carryonfromli;
	int lmstat;
	DWORD lmcol1;
	DWORD lmcol2;

	// Constructor
	realtimelightmappingtype ( )
	{
		 lmcol2 = 0;
		 lmcol1 = 0;
		 lmstat = 0;
		 carryonfromli = 0;
		 carryonfromo2 = 0;
		 carryonfromo1 = 0;
		 carryonfromo = 0;
		 cascadecount = 0;
		 lasttimer = 0;
		 lminterval = 0;
		 lmintervalat = 0;
		 includeoldlightstart = 0;
		 oldlightrange_f = 0.0f;
		 oldlightz_f = 0.0f;
		 oldlighty_f = 0.0f;
		 oldlightx_f = 0.0f;
		 objofinterestz = 0.0f;
		 objofinteresty = 0.0f;
		 objofinterestx = 0.0f;
		 objofinterest = 0;
		 liofinterest = 0;
		 busy = 0;
	}
	// End of Constructor

};

//  Object Of Interest Type and Queue
struct objofinteresttype
{
	int obj;
	float objx;
	float objy;
	float objz;

	// Constructor
	objofinteresttype ( )
	{
		 objz = 0.0f;
		 objy = 0.0f;
		 objx = 0.0f;
		 obj = 0;
	}
	// End of Constructor

};


// 
//  BITMAP FONT Typres
// 

//  Bitmap Font Image Start
struct bitmapfonttype
{
	float x1;
	float y1;
	float x2;
	float y2;
	float w;
	float h;

	// Constructor
	bitmapfonttype ( )
	{
		 h = 0.0f;
		 w = 0.0f;
		 y2 = 0.0f;
		 x2 = 0.0f;
		 y1 = 0.0f;
		 x1 = 0.0f;
	}
	// End of Constructor

};

// 
//  Real-Time Effect Parameter Index (HANDLES)
// 

struct effectparamterraintype
{
	DWORD g_lights_data;
	DWORD g_lights_pos0;
	DWORD g_lights_atten0;
	DWORD g_lights_diffuse0;
	DWORD g_lights_pos1;
	DWORD g_lights_atten1;
	DWORD g_lights_diffuse1;
	DWORD g_lights_pos2;
	DWORD g_lights_atten2;
	DWORD g_lights_diffuse2;
	DWORD SpotFlashPos;
	DWORD SpotFlashColor;

	// Constructor
	effectparamterraintype ( )
	{
		 SpotFlashColor = 0;
		 SpotFlashPos = 0;
		 g_lights_diffuse2 = 0;
		 g_lights_atten2 = 0;
		 g_lights_pos2 = 0;
		 g_lights_diffuse1 = 0;
		 g_lights_atten1 = 0;
		 g_lights_pos1 = 0;
		 g_lights_diffuse0 = 0;
		 g_lights_atten0 = 0;
		 g_lights_pos0 = 0;
		 g_lights_data = 0;
	}
	// End of Constructor

};

struct effectparamvegetationtype
{
	DWORD g_lights_data;
	DWORD g_lights_pos0;
	DWORD g_lights_atten0;
	DWORD g_lights_diffuse0;
	DWORD g_lights_pos1;
	DWORD g_lights_atten1;
	DWORD g_lights_diffuse1;
	DWORD g_lights_pos2;
	DWORD g_lights_atten2;
	DWORD g_lights_diffuse2;
	DWORD SpotFlashPos;
	DWORD SpotFlashColor;

	// Constructor
	effectparamvegetationtype ( )
	{
		 SpotFlashColor = 0;
		 SpotFlashPos = 0;
		 g_lights_diffuse2 = 0;
		 g_lights_atten2 = 0;
		 g_lights_pos2 = 0;
		 g_lights_diffuse1 = 0;
		 g_lights_atten1 = 0;
		 g_lights_pos1 = 0;
		 g_lights_diffuse0 = 0;
		 g_lights_atten0 = 0;
		 g_lights_pos0 = 0;
		 g_lights_data = 0;
	}
	// End of Constructor

};

struct effectparamarraytype
{
	DWORD g_lights_data;
	DWORD g_lights_pos0;
	DWORD g_lights_atten0;
	DWORD g_lights_diffuse0;
	DWORD g_lights_pos1;
	DWORD g_lights_atten1;
	DWORD g_lights_diffuse1;
	DWORD g_lights_pos2;
	DWORD g_lights_atten2;
	DWORD g_lights_diffuse2;
	DWORD SpotFlashPos;
	DWORD SpotFlashColor;

	// Constructor
	effectparamarraytype ( )
	{
		 SpotFlashColor = 0;
		 SpotFlashPos = 0;
		 g_lights_diffuse2 = 0;
		 g_lights_atten2 = 0;
		 g_lights_pos2 = 0;
		 g_lights_diffuse1 = 0;
		 g_lights_atten1 = 0;
		 g_lights_pos1 = 0;
		 g_lights_diffuse0 = 0;
		 g_lights_atten0 = 0;
		 g_lights_pos0 = 0;
		 g_lights_data = 0;
	}
	// End of Constructor

};

struct effectparamskytype
{
	DWORD HudFogDist;
	DWORD HudFogColor;

	// Constructor
	effectparamskytype ( )
	{
		 HudFogColor = 0;
		 HudFogDist = 0;
	}
	// End of Constructor

};

struct effectparamskyscrolltype
{
	DWORD SkyScrollValues;
	DWORD HudFogDist;
	DWORD HudFogColor;

	// Constructor
	effectparamskyscrolltype ( )
	{
		 HudFogColor = 0;
		 HudFogDist = 0;
		 SkyScrollValues = 0;
	}
	// End of Constructor

};

struct effectparampostprocesstype
{
	DWORD LightDir;
	DWORD AlphaAngle;
	DWORD LightRayLength;
	DWORD LightRayQuality;
	DWORD LightRayDecay;
	DWORD ScreenColor0;
	DWORD ScreenColor2;
	DWORD ScreenColor4;

	// Constructor
	effectparampostprocesstype ( )
	{
		 ScreenColor4 = 0;
		 ScreenColor2 = 0;
		 ScreenColor0 = 0;
		 LightRayDecay = 0;
		 LightRayQuality = 0;
		 LightRayLength = 0;
		 AlphaAngle = 0;
		 LightDir = 0;
	}
	// End of Constructor

};

struct effectparamtype
{
	effectparamterraintype terrain;
	effectparamvegetationtype vegetation;
	effectparamskytype sky;
	effectparamskyscrolltype skyscroll;
	effectparamskyscrolltype water;
	effectparampostprocesstype postprocess;

	// Constructor
	effectparamtype ( )
	{
	}
	// End of Constructor

};

// 
//  TERRAIN Types
// 

struct terraintype
{
	int superflat;
	float ts_f;
	int objectstartindex;
	int imagestartindex;
	int effectstartindex;
	int TerrainID;
	int terrainobjectindex;
	int terrainshaderindex;
	//float terrainlevel_f;
	float waterliney_f;
	float waterlineyadjustforclip_f;
	float WaterCamY_f;
	int vegstatistic;
	int terrain_seed;
	int terrain_range;
	int randomiseterrain;
	int generateterrainshadows;
	int generatewaterlinemask;
	float lightrange_f;
	float ldirx_f;
	float ldiry_f;
	float ldirz_f;
	float ldirnx_f;
	float ldirny_f;
	float ldirnz_f;
	float TerrainSize_f;
	//float zoom_f;
	//float camx_f;
	//float camz_f;
	//float playerx_f;
	//float playerz_f;
	//float playera_f;
	int terrainpaintermode;
	int lastterrainpaintermode;
	float RADIUS_f;
	float X_f;
	float Y_f;
	int lastxmouse;
	int lastymouse;
	float aspect_f;
	float zoom_f;
	float camx_f;
	float camz_f;
	float mmz_f;
	float camheightatcoord_f;
	float camheight_f;
	float camheightlimit_f;
	float AMOUNT_f;
	float terrainlevel_f;
	float terrainlevelt_f;
	int terrainregionupdate;
	int terrainregionx1;
	int terrainregionx2;
	int terrainregionz1;
	int terrainregionz2;
	//int terraintriggercheapshadowrefresh;
	int terrainquickupdate;
	int terrainquickx1;
	int terrainquickx2;
	int terrainquickz1;
	int terrainquickz2;
	int grassregionupdate;
	int grassupdateafterterrain;
	int grassregionx1;
	int grassregionx2;
	int grassregionz1;
	int grassregionz2;
	int lastgrassupdatex1;
	int lastgrassupdatex2;
	int lastgrassupdatez1;
	int lastgrassupdatez2;
	int lastmc;
	float lastpaintx_f;
	float lastpaintz_f;
	int lastregx1;
	int lastregz1;
	int needanotherveg;
	int gameplaycamera;
	float playerx_f;
	float playery_f;
	float playerz_f;
	float playerax_f;
	float playeray_f;
	float playeraz_f;
	float speed_f;
	int atx;
	int atz;
	float sundirectionx_f;
	float sundirectiony_f;
	float sundirectionz_f;
	float sunstrength_f;
	float sunrotation_f;
	float suncolorr_f;
	float suncolorg_f;
	float suncolorb_f;
	float floorcolorr_f;
	float floorcolorg_f;
	float floorcolorb_f;
	float sunskyscrollspeedx_f;
	float sunskyscrollspeedz_f;
	float sunskyscrollx_f;
	float sunskyscrollz_f;
	int TerrainLODOBJStart;
	int TerrainLODOBJFinish;
	int terrainpainteroneshot;
	float shapeA_f;
	float shapeWidth_f;
	float shapeLong_f;
	float shapestartX_f;
	float shapestartZ_f;
	float shapelineX1_f;
	float shapelineZ1_f;
	float shapelineX2_f;
	float shapelineZ2_f;
	float shapeStep_f;
	float shapeincX_f;
	float shapeincZ_f;
	float stpX_f;
	float stpZ_f;
	float shapeHeight_f;
	int dirtyterrain;
	int dirtyx1;
	int dirtyz1;
	int dirtyx2;
	int dirtyz2;
	float adjaboveground_f;
	int heightsmodified;
	int triggerobstaclerefresh;
	int grassmemblock;
	int vegetationshaderindex;
	int vegetationgridsize;
	int generatedsupertexture;
	float entityeditmodecursorR;
	float entityeditmodecursorG;
	int rubberbandrampflag;
	float rubberbandrampx;
	float rubberbandrampy;
	float rubberbandrampradius;
	int reflsizer;
	cstr skyshader_s;
	cstr skyscrollshader_s;
	//int iTerrainPBRMode;
	//int iForceTerrainVegShaderUpdate;

	// Constructor
	terraintype ( )
	{
		 //iForceTerrainVegShaderUpdate = 0;
		 //iTerrainPBRMode = 0;
		 skyscrollshader_s = "";
		 skyshader_s = "";
		 reflsizer = 0;
		 rubberbandrampradius = 0.0f;
		 rubberbandrampy = 0.0f;
		 rubberbandrampx = 0.0f;
		 rubberbandrampflag = 0;
		 entityeditmodecursorG = 0.0f;
		 entityeditmodecursorR = 0.0f;
		 generatedsupertexture = 0;
		 vegetationgridsize = 0;
		 vegetationshaderindex = 0;
		 grassmemblock = 0;
		 triggerobstaclerefresh = 0;
		 heightsmodified = 0;
		 adjaboveground_f = 0.0f;
		 dirtyz2 = 0;
		 dirtyx2 = 0;
		 dirtyz1 = 0;
		 dirtyx1 = 0;
		 dirtyterrain = 0;
		 shapeHeight_f = 0.0f;
		 stpZ_f = 0.0f;
		 stpX_f = 0.0f;
		 shapeincZ_f = 0.0f;
		 shapeincX_f = 0.0f;
		 shapeStep_f = 0.0f;
		 shapelineZ2_f = 0.0f;
		 shapelineX2_f = 0.0f;
		 shapelineZ1_f = 0.0f;
		 shapelineX1_f = 0.0f;
		 shapestartZ_f = 0.0f;
		 shapestartX_f = 0.0f;
		 shapeLong_f = 0.0f;
		 shapeWidth_f = 0.0f;
		 shapeA_f = 0.0f;
		 terrainpainteroneshot = 0;
		 TerrainLODOBJFinish = 0;
		 TerrainLODOBJStart = 0;
		 sunskyscrollz_f = 0.0f;
		 sunskyscrollx_f = 0.0f;
		 sunskyscrollspeedz_f = 0.0f;
		 sunskyscrollspeedx_f = 0.0f;
		 floorcolorb_f = 0.0f;
		 floorcolorg_f = 0.0f;
		 floorcolorr_f = 0.0f;
		 suncolorb_f = 0.0f;
		 suncolorg_f = 0.0f;
		 suncolorr_f = 0.0f;
		 sunrotation_f = 0.0f;
		 sunstrength_f = 0.0f;
		 sundirectionz_f = 0.0f;
		 sundirectiony_f = 0.0f;
		 sundirectionx_f = 0.0f;
		 atz = 0;
		 atx = 0;
		 speed_f = 0.0f;
		 playerax_f = 0.0f;
		 playeray_f = 0.0f;
		 playeraz_f = 0.0f;
		 playerz_f = 0.0f;
		 playery_f = 0.0f;
		 playerx_f = 0.0f;
		 gameplaycamera = 0;
		 needanotherveg = 0;
		 lastregz1 = 0;
		 lastregx1 = 0;
		 lastpaintz_f = 0.0f;
		 lastpaintx_f = 0.0f;
		 lastmc = 0;
		 lastgrassupdatez2 = 0;
		 lastgrassupdatez1 = 0;
		 lastgrassupdatex2 = 0;
		 lastgrassupdatex1 = 0;
		 grassregionz2 = 0;
		 grassregionz1 = 0;
		 grassregionx2 = 0;
		 grassregionx1 = 0;
		 grassupdateafterterrain = 0;
		 grassregionupdate = 0;
		 terrainquickz2 = 0;
		 terrainquickz1 = 0;
		 terrainquickx2 = 0;
		 terrainquickx1 = 0;
		 terrainquickupdate = 0;
		 //terraintriggercheapshadowrefresh = 0;
		 terrainregionz2 = 0;
		 terrainregionz1 = 0;
		 terrainregionx2 = 0;
		 terrainregionx1 = 0;
		 terrainregionupdate = 0;
		 terrainlevelt_f = 0.0f;
		 terrainlevel_f = 0.0f;
		 AMOUNT_f = 0.0f;
		 camheightlimit_f = 0.0f;
		 camheight_f = 0.0f;
		 camheightatcoord_f = 0.0f;
		 mmz_f = 0.0f;
		 camz_f = 0.0f;
		 camx_f = 0.0f;
		 zoom_f = 0.0f;
		 aspect_f = 0.0f;
		 lastymouse = 0;
		 lastxmouse = 0;
		 Y_f = 0.0f;
		 X_f = 0.0f;
		 RADIUS_f = 0.0f;
		 terrainpaintermode = 0;
		 TerrainSize_f = 0.0f;
		 ldirnz_f = 0.0f;
		 ldirny_f = 0.0f;
		 ldirnx_f = 0.0f;
		 ldirz_f = 0.0f;
		 ldiry_f = 0.0f;
		 ldirx_f = 0.0f;
		 lightrange_f = 0.0f;
		 generatewaterlinemask = 0;
		 generateterrainshadows = 0;
		 randomiseterrain = 0;
		 terrain_range = 0;
		 terrain_seed = 0;
		 vegstatistic = 0;
		 WaterCamY_f = 0.0f;
		 waterlineyadjustforclip_f = 0.0f;
		 waterliney_f = 0.0f;
		 terrainshaderindex = 0;
		 terrainobjectindex = 0;
		 TerrainID = 0;
		 effectstartindex = 0;
		 imagestartindex = 0;
		 objectstartindex = 0;
		 ts_f = 0.0f;
		 superflat = 0;
	}
	// End of Constructor

};


#define terrain_chunk_size 1024
#define terrain_rock_max 1000

// `Fast veg

#define terrain_veg_areawidth 300

//  Sky Data structure (night/day/weather/etc)
struct skytype
{
	int changingsky;
	int daynightprogress;
	float currenthour_f;
	float alpha1_f;
	float alpha2_f;

	// Constructor
	skytype ( )
	{
		 alpha2_f = 0.0f;
		 alpha1_f = 0.0f;
		 currenthour_f = 0.0f;
		 daynightprogress = 0;
		 changingsky = 0;
	}
	// End of Constructor

};


//GridEdit


//  Data structure to hold gridedit settings (move globals to this)
struct gridedittype
{
	int autoflatten;
	int entityspraymode;
	int entitysprayrange;

	// Constructor
	gridedittype ( )
	{
		 entitysprayrange = 0;
		 entityspraymode = 0;
		 autoflatten = 0;
	}
	// End of Constructor

};


//AI & Animation


//  Types
struct aisystemtype
{
	int on;
	int generateobs;
	int processlogic;
	int processplayerlogic;
	int showprompts;
	int usingdebugobjects;
	int playercontainerid;
	int playerducking;
	int containerpathmax;
	float defaulteyeheight_f;
	float defaulteyehalfheight_f;
	float currenteyeheight_f;
	int debugentitymesh;
	int debugentitymesh2;
	int debugentityworkobj;
	int debugentityworkobj2;
	int usingphysicsforai;
	int objectstartindex;
	int imagestartindex;
	int effectstartindex;
	int soundstartindex;
	int obs;
	int terrainobsnum;
	int cumilativepauses;
	int obstacleradius;

	// Constructor
	aisystemtype ( )
	{
		 cumilativepauses = 0;
		 terrainobsnum = 0;
		 obs = 0;
		 soundstartindex = 0;
		 effectstartindex = 0;
		 imagestartindex = 0;
		 objectstartindex = 0;
		 usingphysicsforai = 0;
		 debugentityworkobj2 = 0;
		 debugentityworkobj = 0;
		 debugentitymesh2 = 0;
		 debugentitymesh = 0;
		 currenteyeheight_f = 0.0f;
		 defaulteyehalfheight_f = 0.0f;
		 defaulteyeheight_f = 0.0f;
		 containerpathmax = 0;
		 playerducking = 0;
		 playercontainerid = 0;
		 usingdebugobjects = 0;
		 showprompts = 0;
		 processplayerlogic = 0;
		 processlogic = 0;
		 generateobs = 0;
		 on = 0;
	}
	// End of Constructor

};

//  Charater Animation Transition Data structure
struct smoothanimtype
{
	int transition;
	float st;
	float fn;
	int rev;
	int playflag;
	int playstarted;

	// Constructor
	smoothanimtype ( )
	{
		 playstarted = 0;
		 playflag = 0;
		 rev = 0;
		 fn = 0.0f;
		 st = 0.0f;
		 transition = 0;
	}
	// End of Constructor

};


//  Character Sequence Datatype
struct charseqtype
{
	int mode;
	int trigger;
	int loopback;
	float a_f;
	float b_f;
	float c_f;
	float angle_f;
	float speed_f;
	float vertspeed_f;
	float speedinmiddle_f;
	int freeze;
	float advancecap_f;
	int alignpos;

	// Constructor
	charseqtype ( )
	{
		 alignpos = 0;
		 advancecap_f = 0.0f;
		 freeze = 0;
		 speedinmiddle_f = 0.0f;
		 vertspeed_f = 0.0f;
		 speed_f = 0.0f;
		 angle_f = 0.0f;
		 c_f = 0.0f;
		 b_f = 0.0f;
		 a_f = 0.0f;
		 loopback = 0;
		 trigger = 0;
		 mode = 0;
	}
	// End of Constructor

};


//  Charatcer Sequence Control Flags
//  1 - moving (0-not,1-fore,2-back,3-left,4-right,5-runfore,6-rolldown,7-runstrafeleft,8-runstraferight,9-sidestepleft,10-sidestepright,11-stepbackleft,12-stepbackright)
//  2 - ducking (0-not,1-down,2-up,3-rollup)
//  3 - spotactioning (0-none,1-reloading,2-wave,3-toss,4-converseA,5-converseB,6-pointfore,7-pointback,8-pointleft,9-pointright,10-climbstairs,11-decendstairs)
//  4 - leaping (0-none,1-leap over sandbag)
//  5 - impacting (0-none,1-fore,2-back,3-left,4-right,5-generic)
//  6 - alerted (0-calm,1-combat)
//  7 - meleeing (0-none,1-punch,2-kick)
//  8 - covering (0-none,1-coverleft,2-coverright,3-peekleft,4-peekright)
struct charanimcontrolstype
{
	int moving;
	int ducking;
	int spotactioning;
	int leaping;
	int impacting;
	int alerted;
	int meleeing;
	int covering;

	// Constructor
	charanimcontrolstype ( )
	{
		 covering = 0;
		 meleeing = 0;
		 alerted = 0;
		 impacting = 0;
		 leaping = 0;
		 spotactioning = 0;
		 ducking = 0;
		 moving = 0;
	}
	// End of Constructor

};


//  Character Sequence State (will be part of entity data structure as store for per-character states)
struct charanimstatetype
{
	int obj;
	int e;
	int originale;
	int playcsi;
	int limbomanualmode;
	int rocketstyle;
	int weapstyle;
	int alerted;
	int lastmoving;
	int lastcovering;
	float moveangle_f;
	int updatemoveangle;
	int runmode;
	int strafemode;
	int ammoinclipmax;
	int ammoinclip;
	int firesoundindex;
	DWORD firesoundstarted;
	DWORD firesoundexpiry;
	float firerateaccumilator;
	int punchorkickimpact;
	int freezeanyrotation;
	int freezeallmovement;
	int advancedcapactive;
	float advanced_f;
	int destinationdirection;
	float destinationheight_f;
	int state;
	int previousstate;
	float realheadangley_f;
	float lastwasteangley_f;
	int notmovingcount;
	int sameascount;
	float sameas_f;
	float climbladderangle_f;
	int climbmode;
	float climbheight_f;
	float distancetotarget_f;
	int cantrulyfire;
	int cantrulysee;
	int healthlevel;
	float currentangle_f;
	float warmupwalk_f;
	float animationspeed_f;
	int disablephysicsupdate;
	int outofrange;
	int timetofadeout;
	float fadeoutvalue_f;
	int aiobjectexists;
	int visiondelay;
	DWORD visiondelaylastime;
	float smoothoverridedest_f;
	float smoothoverridedestframe_f;
	float smoothoverrideqstart_f;
	float smoothoverrideqmiddle_f;
	float smoothoverrideqfinish_f;
	int usingcharacterposedat;

	// Constructor
	charanimstatetype ( )
	{
		 usingcharacterposedat = 0;
		 smoothoverrideqfinish_f = 0.0f;
		 smoothoverrideqmiddle_f = 0.0f;
		 smoothoverrideqstart_f = 0.0f;
		 smoothoverridedestframe_f = 0.0f;
		 smoothoverridedest_f = 0.0f;
		 visiondelaylastime = 0;
		 visiondelay = 0;
		 aiobjectexists = 0;
		 fadeoutvalue_f = 0.0f;
		 timetofadeout = 0;
		 outofrange = 0;
		 disablephysicsupdate = 0;
		 animationspeed_f = 0.0f;
		 warmupwalk_f = 0.0f;
		 currentangle_f = 0.0f;
		 healthlevel = 0;
		 cantrulysee = 0;
		 cantrulyfire = 0;
		 distancetotarget_f = 0.0f;
		 climbheight_f = 0.0f;
		 climbmode = 0;
		 climbladderangle_f = 0.0f;
		 sameas_f = 0.0f;
		 sameascount = 0;
		 notmovingcount = 0;
		 lastwasteangley_f = 0.0f;
		 realheadangley_f = 0.0f;
		 previousstate = 0;
		 state = 0;
		 destinationheight_f = 0.0f;
		 destinationdirection = 0;
		 advanced_f = 0.0f;
		 advancedcapactive = 0;
		 freezeallmovement = 0;
		 freezeanyrotation = 0;
		 punchorkickimpact = 0;
		 firerateaccumilator = 0.0f;
		 firesoundexpiry = 0;
		 firesoundstarted = 0;
		 firesoundindex = 0;
		 ammoinclip = 0;
		 ammoinclipmax = 0;
		 strafemode = 0;
		 runmode = 0;
		 updatemoveangle = 0;
		 moveangle_f = 0.0f;
		 lastcovering = 0;
		 lastmoving = 0;
		 alerted = 0;
		 weapstyle = 0;
		 rocketstyle = 0;
		 limbomanualmode = 0;
		 playcsi = 0;
		 originale = 0;
		 e = 0;
		 obj = 0;
	}
	// End of Constructor

};

#define CHARANIMSTATE_PATROL 1
#define CHARANIMSTATE_COMBAT 2
#define CHARANIMSTATE_DEAD 99


//Weapon Systems


struct weaponstype
{
	int objectstartindex;
	int imagestartindex;
	int soundstartindex;
	int effectstartindex;
	int gunpress;

	// Constructor
	weaponstype ( )
	{
		 gunpress = 0;
		 effectstartindex = 0;
		 soundstartindex = 0;
		 imagestartindex = 0;
		 objectstartindex = 0;
	}
	// End of Constructor

};


//  +0 -


//Physics types

struct physicscollisiontreetype
{
	int entType;
	float posx_f;
	float posy_f;
	float posz_f;
	float sizex_f;
	float sizey_f;
	float sizez_f;
	float ang_f;
	float dist_f;

	// Constructor
	physicscollisiontreetype ( )
	{
		 dist_f = 0.0f;
		 ang_f = 0.0f;
		 sizez_f = 0.0f;
		 sizey_f = 0.0f;
		 sizex_f = 0.0f;
		 posz_f = 0.0f;
		 posy_f = 0.0f;
		 posx_f = 0.0f;
		 entType = 0;
	}
	// End of Constructor

};

#define PHYSICSMAXCOLLISIONTREES 100

//  Third person data structure
struct thirdpersoncontroltype
{
	int enabled;
	int enabledstorefortestgameedit;
	int enabledstorefortestgameeditgunid;
	int startmarkere;
	int charactere;
	int characterindex;
	int cameralocked;
	float cameradistance;
	float cameraheight;
	float camerafocus;
	float cameraspeed;
	int camerashoulder;
	int camerafollow;
	int camerareticle;
	float livecameradistance;
	float livecameraheight;
	float livecamerafocus;
	float livecamerashoulder;
	float storecamposx;
	float storecamposy;
	float storecamposz;
	int shotfired;
	int meleestruck;

	// Constructor
	thirdpersoncontroltype ( )
	{
		meleestruck = 0;
		shotfired = 0;
		storecamposz = 0.0f;
		storecamposy = 0.0f;
		storecamposx = 0.0f;
		livecamerashoulder = 0.0f;
		livecamerafocus = 0.0f;
		livecameraheight = 0.0f;
		livecameradistance = 0.0f;
		camerareticle = 0;
		camerafollow = 0;
		camerashoulder = 0;
		cameraspeed = 0.0f;
		camerafocus = 0.0f;
		cameraheight = 0.0f;
		cameradistance = 0.0f;
		cameralocked = 0;
		characterindex = 0;
		charactere = 0;
		startmarkere = 0;
		enabledstorefortestgameeditgunid = 0;
		enabledstorefortestgameedit = 0;
		enabled = 0;
	}
	// End of Constructor

};

//  Player Globals and Movement Type (for PHYSICS SYSTEM)
struct playercontroltype
{
	int gameloopinitflag;
	int soundstartindex;
	int startlives;
	int startstrength;
	int starthasweapon;
	int starthasweaponqty;
	int startviolent;
	float speedratio_f;
	int hurtfall;

	DWORD ressurectionceasefire;
	int regenrate;
	int regenspeed;
	int regendelay;
	DWORD regentime;
	DWORD regentick;
	float topspeed_f;
	float accel_f;
	float maxspeed_f;
	float basespeed_f;
	float speed_f;
	float movey_f;
	float jumpmax_f;
	int jumpmode;
	float jumpmodecanaffectvelocitycountdown_f;
	float pushangle_f;
	float pushforce_f;
	float camerashake_f;
	float flinchx_f;
	float flinchy_f;
	float flinchz_f;
	float flinchcurrentx_f;
	float flinchcurrenty_f;
	float flinchcurrentz_f;
	int movement;
	int lastmovement;
	float floory_f;
	float wobble_f;
	float wobblespeed_f;
	float wobbleheight_f;
	float cx_f;
	float cy_f;
	float cz_f;
	int camcollisionsmooth;
	float camcurrentdistance;
	int camdofullraycheck;
	int camrightmousemode;
	float lastgoodcx_f;
	float lastgoodcy_f;
	float lastgoodcz_f;
	int footfalltype;
	float footfallpace_f;
	long long footfallcount;
	float ripplecount_f;
	int lastfootfallsound;
	int isrunning;
	DWORD isrunningtime;
	int usingrun;
	int gravityactive;
	int gravityactivepress;
	int lockatheight;
	int lockatheightpress;
	int controlheight;
	float controlheightcooldown;
	int controlheightcooldownmode;
	int controlheightpress;
	float storeoldheight;
	float storemovey;
	int deadtime;
	int plrhitfloormaterial;
	int underwater;
	DWORD timesincelastgrunt;
	float leanoverangle_f;
	float leanover_f;
	float gravity_f;
	float fallspeed_f;
	float climbangle_f;
	int drowntimestamp;
	int swimtimestamp;
	int inwaterstate;
	int heartbeatTimeStamp;
	float redDeathFog_f;
	int jetobjtouse;
	int jetpackcollected;
	int jetpackanimation;
	int jetpackhidden;
	int jetpackeffect;
	int jetpackmode;
	float jetpackthrust_f;
	float jetpackfuel_f;
	int jetpackparticleemitterindex;
	int disablemusicreset;
	thirdpersoncontroltype thirdperson;
	float finalcameraanglex_f;
	float finalcameraangley_f;
	float finalcameraanglez_f;
	int canrun;

	// Constructor
	playercontroltype ( )
	{
		 canrun = 1;
		 gameloopinitflag = 0;
		 finalcameraanglez_f = 0.0f;
		 finalcameraangley_f = 0.0f;
		 finalcameraanglex_f = 0.0f;
		 disablemusicreset = 0;
		 jetpackparticleemitterindex = 0;
		 jetpackfuel_f = 0.0f;
		 jetpackthrust_f = 0.0f;
		 jetpackmode = 0;
		 jetpackeffect = 0;
		 jetpackhidden = 0;
		 jetpackanimation = 0;
		 jetpackcollected = 0;
		 jetobjtouse = 0;
		 redDeathFog_f = 0.0f;
		 heartbeatTimeStamp = 0;
		 inwaterstate = 0;
		 swimtimestamp = 0;
		 drowntimestamp = 0;
		 climbangle_f = 0.0f;
		 fallspeed_f = 0.0f;
		 gravity_f = 0.0f;
		 leanover_f = 0.0f;
		 leanoverangle_f = 0.0f;
		 timesincelastgrunt = 0;
		 underwater = 0;
		 plrhitfloormaterial = 0;
		 deadtime = 0;
		 storemovey = 0.0f;
		 storeoldheight = 0.0f;
		 controlheightpress = 0;
		 controlheight = 0;
		 lockatheightpress = 0;
		 lockatheight = 0;
		 gravityactivepress = 0;
		 gravityactive = 0;
		 usingrun = 0;
		 isrunning = 0;
		 lastfootfallsound = 0;
		 ripplecount_f = 0.0f;
		 footfallpace_f = 0.0f;
		 footfalltype = 0;
		 lastgoodcz_f = 0.0f;
		 lastgoodcy_f = 0.0f;
		 lastgoodcx_f = 0.0f;
		 camrightmousemode = 0;
		 camdofullraycheck = 0;
		 camcurrentdistance = 0.0f;
		 camcollisionsmooth = 0;
		 cz_f = 0.0f;
		 cy_f = 0.0f;
		 cx_f = 0.0f;
		 wobbleheight_f = 0.0f;
		 wobblespeed_f = 0.0f;
		 wobble_f = 0.0f;
		 floory_f = 0.0f;
		 lastmovement = 0;
		 movement = 0;
		 flinchcurrentz_f = 0.0f;
		 flinchcurrenty_f = 0.0f;
		 flinchcurrentx_f = 0.0f;
		 flinchz_f = 0.0f;
		 flinchy_f = 0.0f;
		 flinchx_f = 0.0f;
		 camerashake_f = 0.0f;
		 pushforce_f = 0.0f;
		 pushangle_f = 0.0f;
		 jumpmodecanaffectvelocitycountdown_f = 0.0f;
		 jumpmode = 0;
		 jumpmax_f = 0.0f;
		 movey_f = 0.0f;
		 speed_f = 0.0f;
		 basespeed_f = 0.0f;
		 maxspeed_f = 0.0f;
		 accel_f = 0.0f;
		 topspeed_f = 0.0f;
		 regentick = 0;
		 regentime = 0;
		 regendelay = 0;
		 regenspeed = 0;
		 regenrate = 0;
		 ressurectionceasefire = 0;
		 hurtfall = 0;
		 speedratio_f = 0.0f;
		 startviolent = 0;
		 starthasweaponqty = 0;
		 starthasweapon = 0;
		 startstrength = 0;
		 startlives = 0;
		 soundstartindex = 0;
	}
};

//  control of player light modes
struct playerlighttype
{
	int mode;
	int modeindex;
	float spotflash;
	float spotfadeout_f;
	float spotflashx_f;
	float spotflashy_f;
	float spotflashz_f;
	float spotlightr_f;
	float spotlightg_f;
	float spotlightb_f;
	float flashlightcontrol_f;

	// Constructor
	playerlighttype ( )
	{
		 flashlightcontrol_f = 0.0f;
		 spotlightb_f = 0.0f;
		 spotlightg_f = 0.0f;
		 spotlightr_f = 0.0f;
		 spotflashz_f = 0.0f;
		 spotflashy_f = 0.0f;
		 spotflashx_f = 0.0f;
		 spotfadeout_f = 0.0f;
		 spotflash = 0.0f;
		 modeindex = 0;
		 mode = 0;
	}
	// End of Constructor

};


//Music system types and constants


struct musicsystemtype
{
	int percentageVolume;
	int fadetime;
	int default;

	// Constructor
	musicsystemtype ( )
	{
		 default = 0;
		 fadetime = 0;
		 percentageVolume = 0;
	}
	// End of Constructor

};


struct musictracktype
{
	int state;
	int previousVolume;
	int targetVolume;
	int currentVolume;
	int interval;
	int length;
	int startstamp;
	int fadestamp;
	int fadetime;
	int cuetrack;
	int cuefade;
	int playtimestamp;
	int playtime;
	int playtimefade;

	// Constructor
	musictracktype ( )
	{
		 playtimefade = 0;
		 playtime = 0;
		 playtimestamp = 0;
		 cuefade = 0;
		 cuetrack = 0;
		 fadetime = 0;
		 fadestamp = 0;
		 startstamp = 0;
		 length = 0;
		 interval = 0;
		 currentVolume = 0;
		 targetVolume = 0;
		 previousVolume = 0;
		 state = 0;
	}
	// End of Constructor

};

#define MUSICSYSTEM_MAXTRACKS 32
#define MUSICSYSTEM_MINVOLUME 70
#define MUSICSYSTEM_MAXVOLUME 100
#define MUSICSYSTEM_TRANSITIONTIMEBUFFER 20



//Audio Volume
struct taudiovolume
{
	int music;
	int sound;
	float musicFloat;
	float soundFloat;

	// Constructor
	taudiovolume ( )
	{
		 soundFloat = 0.0f;
		 musicFloat = 0.0f;
		 sound = 0;
		 music = 0;
	}
	// End of Constructor

};


//Character sound
#define CHARACTERSOUND_MAX_BANK 30
#define CHARACTERSOUND_MAX_BANK_MAX_SOUNDS 4
#define CHARACTERSOUND_STACK_SIZE 5
#define ON_AGGRO 0
#define ON_ALERT 1
#define ON_DEATH 2
#define ON_HURT 3
#define ON_HURT_PLAYER 4
#define ON_IDLE 5
#define ON_INTERACT 6

#define CHARACTERSOUND_SIZE 7


struct rubble
{
float x;
float y;
float z;
int scale;
int obj;
int used;
int delay;
int life;
int lifetime;
int damage;
int sound;
int soundplayed;
int volume;
int physicscreated;

	// Constructor
	rubble ( )
	{
		physicscreated = 0;
		volume = 0;
		soundplayed = 0;
		sound = 0;
		damage = 0;
		lifetime = 0;
		life = 0;
		delay = 0;
		used = 0;
		obj = 0;
		scale = 0;
		z = 0.0f;
		y = 0.0f;
		x = 0.0f;
	}
	// End of Constructor

};


struct parts
{
float x;
float y;
float z;
int etype;
float vx;
float vy;
float vz;
float vxvar_f;
float vyvar_f;
float vzvar_f;
int rotate;
int used;
int life;
int obj;
int lasttime;
int anispeed;
int lastanitime;
int nextframe;
int activein;
int activetime;
int scale;
int size;
int damage;
int playforloops;
int actionperformed;
float camshake;
int effect;
float fadein;
int fademax;
int fadedir;
float fadeout;
float alpha;
float oldalpha;
int time;
int activeframe;
int physicscreated;

	// Constructor
	parts ( )
	{
		physicscreated = 0;
		activeframe = 0;
		time = 0;
		oldalpha = 0.0f;
		alpha = 0.0f;
		fadeout = 0.0f;
		fadedir = 0;
		fademax = 0;
		fadein = 0.0f;
		effect = 0;
		camshake = 0.0f;
		actionperformed = 0;
		playforloops = 0;
		damage = 0;
		size = 0;
		scale = 0;
		activetime = 0;
		activein = 0;
		nextframe = 0;
		lastanitime = 0;
		anispeed = 0;
		lasttime = 0;
		obj = 0;
		life = 0;
		used = 0;
		rotate = 0;
		vzvar_f = 0.0f;
		vyvar_f = 0.0f;
		vxvar_f = 0.0f;
		vz = 0.0f;
		vy = 0.0f;
		vx = 0.0f;
		etype = 0;
		z = 0.0f;
		y = 0.0f;
		x = 0.0f;
	}
	// End of Constructor

};

//  Ravey Particles
struct travey_particle_emitter
{
	int id;
	int inUse;
	int hasLife;
	int emitterLife;
	int emitterLifePassed;

	int zclashfixcounter;

	int parentObject;
	int parentLimb;
	float xPos;
	float yPos;
	float zPos;
	float xSpeed;
	float ySpeed;
	float zSpeed;

	int isAnObjectEmitter;
	int imageNumber;
	int objectNumber;

	int isAnimated;
	float animationSpeed;
	int isLooping;
	int frameCount;
	int startFrame;
	int endFrame;
	float frameDivide;
	float frameMulti;

	int startsOffRandomAngle;

	float offsetMinX;
	float offsetMinY;
	float offsetMinZ;

	float offsetMaxX;
	float offsetMaxY;
	float offsetMaxZ;

	float scaleStartMin;
	float scaleStartMax;

	float scaleEndMin;
	float scaleEndMax;

	float movementSpeedMinX;
	float movementSpeedMinY;
	float movementSpeedMinZ;

	float movementSpeedMaxX;
	float movementSpeedMaxY;
	float movementSpeedMaxZ;

	float rotateSpeedMinX;
	float rotateSpeedMinY;
	float rotateSpeedMinZ;

	float rotateSpeedMaxX;
	float rotateSpeedMaxY;
	float rotateSpeedMaxZ;

	float startGravity;
	float endGravity;

	float lifeMin;
	float lifeMax;

	float alphaStartMin;
	float alphaStartMax;

	float alphaEndMin;
	float alphaEndMax;

	int frequency;
	float timePassed;


	// Constructor
	travey_particle_emitter ( )
	{
		 timePassed = 0.0f;
		 frequency = 0;
		 alphaEndMax = 0.0f;
		 alphaEndMin = 0.0f;
		 alphaStartMax = 0.0f;
		 alphaStartMin = 0.0f;
		 lifeMax = 0.0f;
		 lifeMin = 0.0f;
		 endGravity = 0.0f;
		 startGravity = 0.0f;
		 rotateSpeedMaxZ = 0.0f;
		 rotateSpeedMaxY = 0.0f;
		 rotateSpeedMaxX = 0.0f;
		 rotateSpeedMinZ = 0.0f;
		 rotateSpeedMinY = 0.0f;
		 rotateSpeedMinX = 0.0f;
		 movementSpeedMaxZ = 0.0f;
		 movementSpeedMaxY = 0.0f;
		 movementSpeedMaxX = 0.0f;
		 movementSpeedMinZ = 0.0f;
		 movementSpeedMinY = 0.0f;
		 movementSpeedMinX = 0.0f;
		 scaleEndMax = 0.0f;
		 scaleEndMin = 0.0f;
		 scaleStartMax = 0.0f;
		 scaleStartMin = 0.0f;
		 offsetMaxZ = 0.0f;
		 offsetMaxY = 0.0f;
		 offsetMaxX = 0.0f;
		 offsetMinZ = 0.0f;
		 offsetMinY = 0.0f;
		 offsetMinX = 0.0f;
		 startsOffRandomAngle = 0;
		 frameMulti = 0.0f;
		 frameDivide = 0.0f;
		 endFrame = 0;
		 startFrame = 0;
		 frameCount = 0;
		 isLooping = 0;
		 animationSpeed = 0.0f;
		 isAnimated = 0;
		 objectNumber = 0;
		 imageNumber = 0;
		 isAnObjectEmitter = 0;
		 zSpeed = 0.0f;
		 ySpeed = 0.0f;
		 xSpeed = 0.0f;
		 zPos = 0.0f;
		 yPos = 0.0f;
		 xPos = 0.0f;
		 parentLimb = 0;
		 parentObject = 0;
		 zclashfixcounter = 0;
		 emitterLifePassed = 0;
		 emitterLife = 0;
		 hasLife = 0;
		 inUse = 0;
		 id = 0;
	}
	// End of Constructor

};

struct travey_particle
{
	int inUse;
	int emitterID;
	float x;
	float y;
	float z;
	float moveSpeedX;
	float moveSpeedY;
	float moveSpeedZ;
	float rotateSpeedX;
	float rotateSpeedY;
	float rotateSpeedZ;
	float rotz;
	float alpha;

	int isAnimated;
	int isLooping;
	int previousFrame;
	float frame;
	float frameDivide;
	int startFrame;
	int endFrame;
	float animationSpeed;
	float frameMulti;

	float startGravity;
	float endGravity;

	float scaleStart;
	float scaleEnd;

	float movementSpeedX;
	float movementSpeedY;
	float movementSpeedZ;

	float life;
	float timePassed;

	float alphaStart;
	float alphaEnd;

	// Constructor
	travey_particle ( )
	{
		 alphaEnd = 0.0f;
		 alphaStart = 0.0f;
		 timePassed = 0.0f;
		 life = 0.0f;
		 movementSpeedZ = 0.0f;
		 movementSpeedY = 0.0f;
		 movementSpeedX = 0.0f;
		 scaleEnd = 0.0f;
		 scaleStart = 0.0f;
		 endGravity = 0.0f;
		 startGravity = 0.0f;
		 frameMulti = 0.0f;
		 animationSpeed = 0.0f;
		 endFrame = 0;
		 startFrame = 0;
		 frameDivide = 0.0f;
		 frame = 0.0f;
		 previousFrame = 0;
		 isLooping = 0;
		 isAnimated = 0;
		 alpha = 0.0f;
		 rotz = 0.0f;
		 rotateSpeedZ = 0.0f;
		 rotateSpeedY = 0.0f;
		 rotateSpeedX = 0.0f;
		 moveSpeedZ = 0.0f;
		 moveSpeedY = 0.0f;
		 moveSpeedX = 0.0f;
		 z = 0.0f;
		 y = 0.0f;
		 x = 0.0f;
		 emitterID = 0;
		 inUse = 0;
	}
};

#define AICORPSETIME 20000

/*
//  AI constants
#define AICONDNEVER 1
#define AICONDALWAYS 2
#define AICONDSTATE 3
#define AICONDRANDOM 4
#define AICONDHEALTH 5
#define AICONDHEALTHLESS 6
#define AICONDQUANTITY 7
#define AICONDSPEED 8
#define AICONDRATEOFFIRE 9
#define AICONDHEALTHGREATER 10
#define AICONDVAREQUAL 11
#define AICONDVARNOTEQUAL 12
#define AICONDVARGREATER 13
#define AICONDVARLESS 14
#define AICONDPLRHEALTHLESS 21
#define AICONDPLRHEALTHGREATER 22
#define AICONDLEVELEQUAL 31
#define AICONDLEVELNOTEQUAL 32
#define AICONDASSOCIATED 51
#define AICONDSHADERVARIABLE 71
#define AICONDSHADERVARIABLELESS 72
#define AICONDSHADERVARIABLEGREATER 73
#define AICONDPLRDISTWITHIN 101
#define AICONDPLRDISTFURTHER 102
#define AICONDPLRHIGHER 103
#define AICONDANYWITHIN 104
#define AICONDANYFURTHER 105
#define AICONDPLRELEVWITHIN 106
#define AICONDPLRELEVFURTHER 107
#define AICONDPLRFACING 108
#define AICONDPLRNOTFACING 109
#define AICONDPLRUNDERWATER 110
#define AICONDPLRALIVE 111
#define AICONDUNDERWATER 120
#define AICONDSHOTDAMAGE 121
#define AICONDIFWEAPON 122
#define AICONDSHOTDAMAGETYPE 123
#define AICONDCANTAKE 131
#define AICONDPLRCANBESEEN 151
#define AICONDPLRCANNOTBESEEN 152
#define AICONDPLRHASKEY 153
#define AICONDPLRUSINGACTION 154
#define AICONDACTIVATED 155
#define AICONDPLRWITHINZONE 156
#define AICONDENTITYWITHINZONE 157
#define AICONDPLRINGUNSIGHT 158
#define AICONDNEARACTIVATABLE 159
#define AICONDNEWWEAPONCANBESEEN 161
#define AICONDNOISEHEARD 171
#define AICONDANYWITHINZONE 191
#define AICONDANYKEYWITHINZONE 192
#define AICONDRAYCAST 201
#define AICONDRAYCASTUP 202
#define AICONDRAYCASTBACK 203
#define AICONDNORAYCASTUP 204
#define AICONDFRAMEATEND 301
#define AICONDFRAMEATSTART 302
#define AICONDFRAMEWITHIN 303
#define AICONDFRAMEBEYOND 304
#define AICONDANIMATIONOVER 305
#define AICONDREACHTARGET 351
#define AICONDLOSETARGET 352
//  Hockeykid - 250210 - Dark AI conditions
#define AICONDDARKAISTRAFEL 353
#define AICONDDARKAISTRAFER 354
#define AICONDDARKAIMOVINGF 355
#define AICONDDARKAIIDLE 356
#define AICONDDARKAIMOVINGB 357
#define AICONDDARKAICANSHOOT 358
#define AICONDDARKAISTRAFELA 359
#define AICONDAIDUCKING 360
#define AICONDAIHEARDSOUND 361
#define AICONDAIACTION 362
#define AICONDAIISAI 363
#define AICONDAITEAM 364
#define AICONDAIATPOINT 365
#define AICONDAICALLED 367
#define AICONDAITARGETDISTWITHIN 368
#define AICONDAITARGETDISTFURTHER 369
#define AICONDAIATCOVER 370
#define AICONDAICALLEDBYPLR 371
#define AICONDAIHASTARGET 372
#define AICONDDARKAIRUNNINGF 373
//  End Dark AI
#define AICONDHEADANGLEGREATER 401
#define AICONDHEADANGLELESS 402
#define AICONDSOUNDFINISHED 451
#define AICONDALPHAFADEEQUAL 471
#define AICONDWAYPOINTSTATE 501
#define AICONDIFMARKER 551
#define AICONDIFPLRTRAIL 552
#define AICONDHUDSELECTIONMADE 801
#define AICONDTIMERGREATER 811
#define AICONDETIMERGREATER 812
#define AICONDESCAPEKEYPRESSED 821
#define AICONDHUDEDITDONE 822
#define AICONDHUDHAVENAME 823
#define AICONDSCANCODEKEYPRESSED 824
//  AirMod - New Conditions >>>
#define AICONDPLRBLOCKING 825
#define AICONDINVIEW 826
#define AICONDPICKOBJECT 827
#define AICONDVELOCITY 828
//  AirMod - Done <<<
#define AICONDKEYPRESSED 829
#define AICONDHASWEAPON 830
//  V118 - 110810 - knxrb - Hud Layers
#define AICONDHUDSELECTED 831
//  V118 - 110810 - knxrb - Setup.ini Variable Conditions
#define AICONDSETUPSHADER 832
#define AICONDSETUPTEXQUALITY 833
#define AICONDSETUPMOUSESENS 834
#define AICONDSETUPASPECTRATIO 835
#define AICONDSETUPPOSTPROCESSING 836
#define AICONDSETUPWIDTH 837
#define AICONDSETUPHEIGHT 838
#define AICONDSETUPDEPTH 839
#define AICONDSETUPANTIALIAS 840
#define AICONDSETUPSHADOWS 841
//  V118 - 130810 - knxrb - Invert Mouse
#define AICONDSETUPMOUSEINVERT 842
//  V118 - 120810 - knxrb - 'mousestate=X' command
#define AICONDMOUSESTATE 843
//  V118 - 140810 - knxrb - Disable Particles
#define AICONDSETUPPARTICLESUSED 844
//  V118 - 160810 - knxrb - Auto Resolution
#define AICONDSETUPAUTORES 845
//  GUI-X9 (knxrb) >>>
#define AICONDHUDMOUSEDOWN 846
#define AICONDHUDMOUSEUP 847
#define AICONDHUDMOUSEOVER 848
#define AICONDLOADVISIBLE 849
#define AICONDSAVEVISIBLE 850
#define AICONDSTOPWATCHGREATER 851
#define AICONDSTOPWATCHLESS 852
#define AICONDSTOPWATCHRUNNING 853
#define AICONDSETUPVAREQUAL 854
#define AICONDCURSORIMAGE 855
#define AICONDCHOICEVALUEEQUAL 856
#define AICONDSLIDERVALUEEQUAL 857
//  GUI-X9 (knxrb) <<<

//  v118 - 110511 - cinematic hands - (terry cox) >>>>
// `test to see if player is prevented from selecting a gun

#define AICONDPREVENTEDSELECTINGGUN 858
//  cinematic hands <<<<

//  Scene commander conditions
#define AICONDMOUSECONDITION 870
#define AICONDAIRGREATER 871
#define AICONDAIRLESSER 872
#define AICONDAIREQUAL 873
#define AICONDSAMEFLOORASPLR 874
#define AICONDPLRFLOOREQUAL 875
#define AICONDENTFLOOREQUAL 876
#define AICONDISIMMUNE 877
#define AICONDWEAPONSGREATER 878
#define AICONDWEAPONSLESSER 879
#define AICONDWEAPONSEQUAL 880
#define AICONDWEAPONINSLOT 881
#define AICONDCURRENTWEAPON 882
#define AICONDPLRRUNNING 883
#define AICONDPLRCROUCHING 884
#define AICONDZOOMED 885
#define AICONDPLRUSINGRELOAD 886
#define AICONDPLRONGROUND 887
#define AICONDPLRJUMPING 888
#define AICONDFIREMODE 889
#define AICONDONRADAR 890
#define AICONDLASTFIRED 891
#define AICONDRADARGREATER 892
#define AICONDRADAREQUAL 893
#define AICONDENTITYPLRGREATER 894
#define AICONDENTITYPLRLESSER 895
#define AICONDFLOORHIGHER 896
#define AICONDFLOORLOWER 897
#define AICONDFLASHING 898
#define AICONDSPAWNSLEFT 899
#define AICONDSPAWNSGREATER 900
#define AICONDSHOTBY 901
#define AICONDNOTZOOMED 902
#define AICONDPLRNOTRUNNING 903
#define AICONDPLRNOTCROUCHING 904
#define AICONDPLRNOTJUMPING 905
#define AICONDPLRNOTUSINGRELOAD 906
#define AICONDNOTONRADAR 907
#define AICONDPLRCARRYING 908
#define AICONDPLRNOTCARRYING 906
#define AICONDLASTDAMAGETIME 907
#define AICONDCURRENTJAMMED 908
#define AICONDENTITYISIMMUNE 909
#define AICONDENTITYISNOTIMMUNE 910
#define AICONDPLRWEAPONIDLE 911
#define AICONDPLRWEAPONNOTIDLE 912
#define AICONDOBJECTRANGELESS 913
#define AICONDOBJECTRANGEGREATER 914
#define AICONDSOUNDPLAYING 915
#define AICONDSOUNDNOTPLAYING 916
#define AICONDWATEREQUAL 917
#define AICONDWATERLESSER 918
#define AICONDWATERGREATER 919
#define AICONDWATERISON 920
#define AICONDSHOTBYAMMO 921
#define AICONDPLRFIRED 923
#define AICONDPLRFIREDGREATER 924
#define AICONDPLRFIREDLESSER 925
#define AICONDSTATEGREATER 926
#define AICONDSTATELESSER 927
#define AICONDLMBGREATER 928
#define AICONDLMBLESSER 929
#define AICONDRMBGREATER 930
#define AICONDRMBLESSER 931

#define AICONDLAST 932

#define AIACTNONE 0
#define AIACTDESTROY 1
#define AIACTSUSPEND 2
#define AIACTFLOORLOGIC 11
#define AIACTNOGRAVITY 12
#define AIACTGLOBALVAR 31
#define AIACTLOCALVAR 32
#define AIACTSETVAR 33
#define AIACTINCVAR 34
#define AIACTDECVAR 35
#define AIACTDIMVAR 36
#define AIACTADDVAR 37
#define AIACTSUBVAR 38
#define AIACTMULVAR 39
#define AIACTDIVVAR 40
#define AIACTMODVAR 41
#define AIACTWRAPVAR 42
#define AIACTDIMLOCALVAR 43
#define AIACTRUNFPIDEFAULT 51
#define AIACTRUNFPI 52
#define AIACTWIN 61
#define AIACTSELECTSHADERVARIABLE 71
#define AIACTSETSHADERVARIABLE 72
#define AIACTINCSHADERVARIABLE 73
#define AIACTDECSHADERVARIABLE 74
#define AIACTSTATE 101
#define AIACTINCSTATE 102
#define AIACTMOVEUP 201
#define AIACTMOVEFORE 202
#define AIACTMOVEBACK 203
#define AIACTFREEZE 204
#define AIACTRUNFORE 205
#define AIACTROTATEY 226
#define AIACTROTATETOPLR 227
#define AIACTROTATEIY 228
#define AIACTNOROTATE 229
#define AIACTRESETHEAD 231
#define AIACTROTATEHEAD 232
#define AIACTROTATEHEADRANDOM 233
#define AIACTFORCEBACK 251
#define AIACTFORCEFORE 252
#define AIACTFORCELEFT 253
#define AIACTFORCERIGHT 254
#define AIACTFORCEBOUNCE 255
#define AIACTSPINRATE 281
#define AIACTFLOATRATE 282
#define AIACTSETFRAME 301
#define AIACTINCFRAME 302
#define AIACTDECFRAME 303
#define AIACTANIMATE 304
#define AIACTADVFRAME 305
#define AIACTANIMATIONNORMAL 306
#define AIACTANIMATIONREVERSE 307
#define AIACTSETTARGET 351
#define AIACTROTATETOTARGET 352
#define AIACTLOOKATTARGET 353
#define AIACTMOVETOTARGET 354
#define AIACTCOLLECTTARGET 355
#define AIACTCHOOSESTRAFE 356
#define AIACTSTRAFE 357
#define AIACTPIVOTRANDOM 358
#define AIACTLOOKATPLR 359
#define AIACTSETTARGETNAME 360
#define AIACTRAGDOLL 370
#define AIACTCARRYALL 391
#define AIACTPLRASS 401
#define AIACTPLRNOASS 402
#define AIACTPLRMOVEUP 403
#define AIACTPLRMOVEDOWN 404
#define AIACTPLRMOVEEAST 405
#define AIACTPLRMOVEWEST 406
#define AIACTPLRMOVENORTH 407
#define AIACTPLRMOVESOUTH 408
#define AIACTPLRMOVETO 409
#define AIACTPLRMOVEIFUSED 410
#define AIACTPLRFREEZE 411
#define AIACTPLRDISABLE 412
#define AIACTACTIVATEIFUSED 421
#define AIACTACTIVATEIFUSEDNEAR 422
#define AIACTACTIVATETARGET 423
#define AIACTACTIVATE 424
#define AIACTACTIVATEALLINZONE 425
#define AIACTPLRADDHEALTH 431
#define AIACTPLRSUBHEALTH 432
#define AIACTPLRSETHEALTH 433
#define AIACTADDHEALTH 441
#define AIACTSUBHEALTH 442
#define AIACTSETHEALTH 443
#define AIACTSOUND 451
#define AIACT3DSOUND 452
#define AIACTLOOPSOUND 453
#define AIACTSTOPSOUND 454
#define AIACTTALK 455
#define AIACTTALKORDERED 456
#define AIACTTALKRANDOM 457
#define AIACTALTTEXTURE 461
#define AIACTSETALPHAFADE 471
#define AIACTINCALPHAFADE 472
#define AIACTDECALPHAFADE 473
#define AIACTRUNDECAL 481
#define AIACTSHAPEDECAL 482
#define AIACTTRIGGERFORCE 491
#define AIACTWAYPOINTSTART 501
#define AIACTWAYPOINTSTOP 502
#define AIACTWAYPOINTREVERSE 503
#define AIACTWAYPOINTNEXT 504
#define AIACTWAYPOINTPREV 505
#define AIACTWAYPOINTRANDOM 506
#define AIACTDROPMARKER 551
#define AIACTNEXTMARKER 552
#define AIACTRESETMARKERS 553
#define AIACTFOLLOWPLR 554
#define AIACTPLRTAKE 601
#define AIACTPLRDROP 602
#define AIACTPLRDROPCURRENT 603
#define AIACTSHOOTPLR 701
#define AIACTUSEWEAPON 711
#define AIACTRELOADWEAPON 712
#define AIACTCOLOFF 751
#define AIACTCOLON 752
#define AIACTETIMERSTART 753
#define AIACTSPAWNON 761
#define AIACTSPAWNOFF 762
#define AIACTHEADSHOT 771
#define AIACTHEADSHOTDAMAGE 772
#define AIACTAMBIENCE 821
#define AIACTAMBIENCERED 822
#define AIACTAMBIENCEGREEN 823
#define AIACTAMBIENCEBLUE 824
#define AIACTFOG 831
#define AIACTFOGRED 832
#define AIACTFOGGREEN 833
#define AIACTFOGBLUE 834
// `markblosser - add fogstart,fogend commands

#define AIACTFOGSTART 835
#define AIACTFOGEND 836
// `markblosser - set post process effect

#define AIACTSETPOSTEFFECT 837
#define AIACTSKY 841
#define AIACTSKYSCROLL 842
#define AIACTNEWJUMPHEIGHT 901
#define AIACTBACKDROP 1001
#define AIACTMUSIC 1011
#define AIACTMUSICVOLUME 1012
#define AIACTSOUNDSCALE 1013
#define AIACTMUSICOVERRIDE 1014
#define AIACTVIDEO 1021
#define AIACTAISETVIEWRANGE 1022
#define AIACTWEBLINK 1031
#define AIACTEXPLODE 1032
#define AIACTLIGHTON 1051
#define AIACTLIGHTOFF 1052
#define AIACTLIGHTRED 1053
#define AIACTLIGHTGREEN 1054
#define AIACTLIGHTBLUE 1055
#define AIACTLIGHTRANGE 1056
#define AIACTLIGHTINTENSITY 1057
//  Hockeykid - 250210 - Dark AI constants
#define AIACTADDAITEAM 1058
#define AIACTAIALLYFOLLOW 1059
#define AIACTAISETTARGET 1061
#define AIACTAIMOVERANDOM 1062
#define AIACTAIREMOVE 1063
#define AIACTAIMOVEMENT 1065
#define AIACTAIACTIVE 1066
#define AIACTTOGGLEACTIVE 1067
#define AIACTAIACTION 1070
#define AIACTAIFOLLOWPLR 1071
#define AIACTAIMOVETOSOUND 1072
#define AIACTAIMOVEAWAYFROMSOUND 1073
#define AIACTAIADDPOINT 1074
#define AIACTAIGOTOPOINT 1075
#define AIACTAICALLTEAM 1076
#define AIACTAIRESPONDTOCALL 1077
#define AIACTAIMOVETOCOVER 1078
#define AIACTAIMOVETOTARGET 1079
#define AIACTAISTOP 1080
#define AIACTAIUSEMELEE 1081
#define AIACTAISETMELEEDAMAGE 1082
#define AIACTAIROTATETOTARGET 1083
#define AIACTAIROTATETOSOUND 1084
#define AIACTAIPLRCALLTEAM 1085
#define AIACTAISETSPEED 1086
#define AIACTAIRESPONDTOPLAYER 1087
#define AIACTAITOGLLEATTACK 1088
#define AIACTAICLEARTARGET 1089
#define AIACTAIENABLEFULLAIM 1090
#define AIACTAIEYELEVEL 1091
#define AIACTAIADDALLY 1092
#define AIACTAIADDENEMY 1093
#define AIACTAIADDNEUTRAL 1094
#define AIACTAIAUTOFACTIONOFF 1095
#define AIACTAIROTATEY 1097
#define AIACTAILOOKAROUND 1098
//  end Dark AI constants
#define AIACTNEXTLEVEL 1096
#define AIACTHUDUSERVAR 1099
#define AIACTHUDRESET 1100
#define AIACTHUDX 1101
#define AIACTHUDY 1102
#define AIACTHUDZ 1103
#define AIACTHUDSIZEX 1104
#define AIACTHUDSIZEY 1105
#define AIACTHUDSIZEZ 1106
#define AIACTHUDRED 1107
#define AIACTHUDGREEN 1108
#define AIACTHUDBLUE 1109
#define AIACTHUDIMAGE 1110
#define AIACTHUDIMAGEFINE 1111
#define AIACTHUDFONT 1112
#define AIACTHUDSIZE 1113
#define AIACTHUDTEXT 1114
#define AIACTHUDTYPE 1115
#define AIACTHUDHIDE 1116
#define AIACTHUDSHOW 1117
#define AIACTHUDUNSHOW 1118
#define AIACTHUDNAME 1119
#define AIACTHUDANIM 1120
#define AIACTHUDFADEOUT 1121
#define AIACTHUDMAKE 1199
#define AIACTNEWGAME 1201
#define AIACTLOADGAME 1202
#define AIACTSAVEGAME 1203
#define AIACTCONTINUEGAME 1204
#define AIACTQUITGAME 1205
#define AIACTPAUSEGAME 1206
#define AIACTRESUMEGAME 1207
#define AIACTHOSTGAME 1208
#define AIACTJOINGAME 1209
#define AIACTREPEATGAME 1210
#define AIACTTIMERSTART 1211
#define AIACTQUICKLOADGAME 1212
#define AIACTQUICKSAVEGAME 1213
//  AirMod - New Actions >>>
#define AIACTSETIFUSED 1214
#define AIACTSETUSEKEY 1215
#define AIACTRESETPLRWEAPONS 1216
#define AIACTBACKDROPVID 1217
#define AIACTBLOODSPURT 1218
#define AIACTNOBULLETCOL 1219
#define AIACTALTAMMO 1220
#define AIACTBLOODSPLASH 1221
#define AIACTCAMSHAKE 1222
#define AIACTCAMFOV 1223
#define AIACTCAMFOVINC 1224
#define AIACTHIDEWEAPON 1225
#define AIACTSHOWLASTWEAPON 1227
#define AIACTCAMPOINTOBJECT 1228
//  AirMod - Done <<<
#define AIACTSETUPDYNAMICSHADOWS 1229
#define AIACTSETUPUSEEFFECTS 1230
#define AIACTSETUPDIVIDETEXTURESIZE 1231
#define AIACTSETUPMOUSESENSITIVITY 1232
#define AIACTSETUPASPECTRATIO 1233
#define AIACTSETUPPOSTPROCESSING 1234
#define AIACTSETUPWIDTH 1235
#define AIACTSETUPHEIGHT 1236
#define AIACTSETUPDEPTH 1237
#define AIACTSETUPANTIALIAS 1238
#define AIACTRESET 1239
#define AIACTSAVESETUP 1240
#define AIACTPASSTOSETUP 1241
#define AIACTFPGCRAWTEXTR 1251
#define AIACTFPGCRAWTEXTG 1252
#define AIACTFPGCRAWTEXTB 1253
#define AIACTFPGCRAWTEXTX 1254
#define AIACTFPGCRAWTEXTY 1255
#define AIACTFPGCRAWTEXTSIZE 1256
#define AIACTFPGCRAWTEXTFONT 1257
#define AIACTFPGCRAWTEXTOFF 1258
#define AIACTFPGCRAWTEXT 1259
//  V118 - 110810 - knxrb - Hud Layers
#define AIACTHUDLAYER 1260
//  V118 - 130810 - knxrb - Mouse Invert (Options Screen)
#define AIACTSETUPMOUSEINVERT 1261
//  V118 - 140810 - knxrb - Disable Particles (Options Screen)
#define AIACTSETUPPARTICLESUSED 1262
//  V118 - 160810 - knxrb - Hud - sethudx Command
#define AIACTSETHUDXPOS 1263
//  V118 - 160810 - knxrb - Auto Resolution
#define AIACTSETUPAUTORES 1264
//  GUI-X9 (knxrb) >>>
#define AIACTLOADIMAGE 1265
#define AIACTMAKEHUD 1266
#define AIACTSETHUDX 1267
#define AIACTSETHUDY 1268
#define AIACTSETHUDW 1269
#define AIACTSETHUDH 1270
#define AIACTSETHUDCOLOUR 1271
#define AIACTSETHUDCOLOR 1272
#define AIACTSETHUDALPHA 1273
#define AIACTSETHUDNIMAGE 1274
#define AIACTSETHUDOIMAGE 1275
#define AIACTMAKECHECKBOX 1276
#define AIACTSETCHECKBOXX 1277
#define AIACTSETCHECKBOXY 1278
#define AIACTSETCHECKBOXW 1279
#define AIACTSETCHECKBOXH 1280
#define AIACTSETCHECKBOXCOLOUR 1281
#define AIACTSETCHECKBOXCOLOR 1282
#define AIACTSETCHECKBOXALPHA 1283
#define AIACTSETCHECKBOXNIMAGE 1284
#define AIACTSETCHECKBOXCIMAGE 1285
#define AIACTSETCHECKBOXSTATE 1286
#define AIACTMAKESLIDER 1287
#define AIACTSETSLIDERX 1288
#define AIACTSETSLIDERY 1289
#define AIACTSETSLIDERW 1290
#define AIACTSETSLIDERH 1291
#define AIACTSETSLIDERCOLOUR 1292
#define AIACTSETSLIDERCOLOR 1293
#define AIACTSETSLIDERALPHA 1294
#define AIACTSETSLIDERVALUE 1295
#define AIACTMAKECHOICE 1296
#define AIACTSETCHOICEX 1297
#define AIACTSETCHOICEY 1298
#define AIACTSETCHOICEW 1299
#define AIACTSETCHOICEH 1300
#define AIACTSETCHOICECOLOUR 1301
#define AIACTSETCHOICECOLOR 1302
#define AIACTSETCHOICEALPHA 1303
#define AIACTADDCHOICESTATE 1304
#define AIACTSETCHOICESTATE 1305
#define AIACTSETCURSOR 1306
#define AIACTHIDEALL 1307
#define AIACTREPLACEIMAGE 1308
#define AIACTSHOWHUD 1309
#define AIACTHIDEHUD 1310
#define AIACTSHOWCURSOR 1311
#define AIACTHIDECURSOR 1312
#define AIACTCLEARGUI 1313
#define AIACTMAKESTOPWATCH 1314
#define AIACTSTARTSTOPWATCH 1315
#define AIACTSTOPSTOPWATCH 1316
#define AIACTMAKESETUPVAR 1317
#define AIACTSETSETUPVARVALUE 1318
#define AIACTSETSETUPVARLINE 1319
#define AIACTREADSETUPVARVALUE 1320
#define AIACTSAVESETUPVARS 1321
#define AIACTSETHUDCLICKABLE 1322
#define AIACTSETSETUPTOCOMP 1323
#define AIACTSHOWCHOICE 1324
#define AIACTHIDECHOICE 1325
#define AIACTSHOWSLIDER 1326
#define AIACTHIDESLIDER 1327
#define AIACTSHOWCHECKBOX 1328
#define AIACTHIDECHECKBOX 1329
#define AIACTSETHUDNUMERIC 1330
#define AIACTSETHUDVALUE 1331
//  GUI-X9 (knxrb) <<<
//  Water commands
#define AIACTWATERHEIGHT 1332
#define AIACTWATER 1333
#define AIACTWATERFOGDIST 1334
#define AIACTWATERSPEED 1335
#define AIACTWATERFOGRED 1336
#define AIACTWATERFOGGREEN 1337
#define AIACTWATERFOGBLUE 1338
#define AIACTWATERRED 1339
#define AIACTWATERGREEN 1340
#define AIACTWATERBLUE 1341
#define AIACTWATERHEIGHTOFZONE 1342
//  Scene Commander water quality
#define AIACTWATERFLEC 1343
//  GUI-X9 (knxrb) >>>
#define AIACTEYEHURTTIME 1442
#define AIACTFADERSPEED 1443
#define AIACTUSEGUIX9 1444
//  GUI-X9 (knxrb) <<<
#define AIACTHIDESHADOW 1445
//  LightRay Addition
//  LightRayMod
#define AIACTLRAYSET 1447
#define AIACTLRAYACT 1448
#define AIACTLRBLOOMACT 1449
#define AIACTLRDEBUGDEACTIVE 1450
//  GUI-X9 (knxrb) >>>
#define AIACTFORCEGUISUB 1451
//  GUI-X9 (knxrb) <<<

//  v118 - 110511 - cinematic hands (terry cox) >>>>
// `commands that are needed for cinematic hands

// `the first two are optional - the last two are required

#define AIACTPREVENTPLAYERSELECTINGGUN 1452
#define AIACTDISABLEFIREBUTTON 1453
#define AIACTSTARTCINEMATICHANDS 1454
#define AIACTGETNEWWEAPON 1455
#define AIACTREMOVECURRENTWEAPON 1456
#define AIACTDISABLEBLOCKINGBUTTON 1457
//  cinematic hands <<<<
//  set entity speed - (markblosser)
#define AIACTSETENTITYSPEED 1458
//  set animate command speed - (markblosser)
#define AIACTSETANIMATESPEED 1459

//  Scene Commander actions
#define AIACTWOBBLE 1500
#define AIACTTILTON 1501
#define AIACTTILTSPEED 1502
#define AIACTTILTBOUNCE 1503
#define AIACTUNSKIP 1504
#define AIACTPLRACTION 1505
#define AIACTFORCEMOVE 1506
#define AIACTEMITFORCE 1507
#define AIACTFORCEDAMAGEON 1508
#define AIACTCROSSHAIRON 1509
#define AIACTWEAPONTOSLOT 1510
#define AIACTARMON 1511
#define AIACTARMADD 1512
#define AIACTARMDEC 1513
#define AIACTARMX 1514
#define AIACTARMY 1515
#define AIACTSETARM 1516
#define AIACTAIRON 1517
#define AIACTADDAIR 1518
#define AIACTAIRTIME 1519
#define AIACTDROWNTIME 1520
#define AIACTAIRMAX 1521
#define AIACTSETAIR 1522
#define AIACTSETAIRX 1523
#define AIACTSETAIRY 1524
#define AIACTINSTANTDROWN 1525
#define AIACTIMMUNE 1526
#define AIACTARROWON 1527
#define AIACTJUMPON 1528
#define AIACTCROUCHON 1529
#define AIACTPEEKON 1530
#define AIACTWALKON 1531
#define AIACTRUNON 1532
#define AIACTFORCEDAMAGE 1533
#define AIACTCOMPASSON 1534
#define AIACTCOMPASSOFF 1535
#define AIACTCOMPASSX 1536
#define AIACTCOMPASSY 1537
#define AIACTCOMPASSSPIN 1538
#define AIACTNEEDLESPIN 1539
#define AIACTRADAR 1540
#define AIACTRADARX 1541
#define AIACTRADARY 1542
#define AIACTRADARROTATE 1543
#define AIACTRADARRANGE 1544
#define AIACTSPEEDMOD 1545
#define AIACTADDFPGCRAWTEXT 1546
#define AIACTSETOBJECTIVE 1547
#define AIACTSETOBJECTIVEX 1548
#define AIACTSETOBJECTIVEY 1549
#define AIACTOBJECTIVEMODE 1550
#define AIACTSETVARRND 1551
#define AIACTRANDOMIZE 1552
#define AIACTMAXSLOTS 1553
#define AIACTEDAMMULT 1554
#define AIACTPLRSTRENGTH 1555
#define AIACTCULLMODE 1556
#define AIACTCULLRANGE 1557
#define AIACTHIDEOBJ 1558
#define AIACTSHOWOBJ 1559
#define AIACTCULLMODI 1560
#define AIACTFOOTFALL 1561
#define AIACTSWAPALT 1562
#define AIACTROTATEPLRX 1563
#define AIACTROTATEPLRY 1564
#define AIACTENTROTX 1565
#define AIACTENTROTY 1566
#define AIACTENTROTZ 1567
#define AIACTMOVEPLRX 1568
#define AIACTMOVEPLRY 1569
#define AIACTMOVEPLRZ 1570
#define AIACTSCALE 1571
#define AIACTSCALELIMB 1572
#define AIACTHOLSTER 1573
#define AIACTNOAIR 1574
#define AIACTSETDROWNDAMAGE 1575
#define AIACTPLRPICKON 1576
#define AIACTPLRPICKRANGE 1577
#define AIACTFLASH 1578
#define AIACTFLASHRED 1579
#define AIACTFLASHGREEN 1580
#define AIACTFLASHBLUE 1581
#define AIACTFLASHRANGE 1582
#define AIACTLOGICBURST 1583
#define AIACTPLRDAM 1584
#define AIACTDESTROYIN 1585
#define AIACTSETLISTKEY 1586
#define AIACTRAWTEXTVAR 1587
#define AIACTSIN 1588
#define AIACTCOS 1589
#define AIACTSCALEHUDX 1590
#define AIACTSCALEHUDY 1591
#define AIACTCHANGEHUDALPHA 1592
#define AIACTWATERFLOW 1593
#define AIACTWATERCURRENT 1594
#define AIACTREMOVEWEAPON 1595
#define AIACTGIVEWEAPON 1596
#define AIACTENTITYSETIMMUNE 1597
#define AIACTRESETONRELOAD 1598
#define AIACTENTITYCAM 1599
#define AIACTPLAYERCAM 1600
#define AIACTLASTCAM 1601
#define AIACTCUSTGUNANIM 1602
#define AIACTUSEENTROT 1603
#define AIACTSETCAMOFFSETX 1604
#define AIACTSETCAMOFFSETY 1605
#define AIACTSETCAMOFFSETZ 1606
#define AIACTSETCAMROTX 1607
#define AIACTSETCAMROTY 1608
#define AIACTSETCAMROTZ 1609
#define AIACTHIDELIMB 1610
#define AIACTSHOWLIMB 1611
#define AIACTPLAYPRESET 1612
#define AIACTLOCKEMPLACEMENT 1613
#define AIACTFREEEMPLACEMENT 1614
#define AIACTCULLIMMOBILE 1615
#define AIACTPLRACC 1616
#define AIACTENTITYACC 1617
#define AIACTDEBUGCURSOR 1618
#define AIACTDEBUGVAR 1619
#define AIACTDEBUGTEXT 1620
#define AIACTWIREFRAME 1621
#define AIACTPLAYEROFFSETON 1622
#define AIACTPLAYEROFFSETX 1623
#define AIACTPLAYEROFFSETY 1624
#define AIACTPLAYEROFFSETZ 1625
#define AIACTMUTATE 1626
#define AIACTOBJECTRANGE 1627
#define AIACTLINKTOPLAYER 1628
#define AIACTFREEFROMPLAYER 1629
#define AIACTOBJMET 1630
#define AIACTALWAYSRUN 1631
#define AIACTLINKENTITYROTX 1632
#define AIACTLINKENTITYROTY 1633
#define AIACTLINKENTITYROTZ 1634
#define AIACTPLROFFSETANGLEX 1635
#define AIACTPLROFFSETANGLEY 1636
#define AIACTPLROFFSETANGLEZ 1637
#define AIACTENTITYOFFSETANGLEX 1638
#define AIACTENTITYOFFSETANGLEY 1639
#define AIACTENTTIYOFFSETANGLEZ 1640
#define AIACTSHOWWEAPON 1641
#define AIACTNOSELECT 1642
#define AIACTDELAYTIMER 1643
#define AIACTCREATESPLASHSOUND 1644
#define AIACTTARGETLINK 1645
#define AIACTSHOWFLAK 1646
#define AIACTMAKEVIDEOTEXTURE 1647
#define AIACTUSEVIDEOTEXTURE 1648
#define AIACTDELETEVIDEOTEXTURE 1649
#define AIACTPAUSEVIDEOTEXTURE 1650
#define AIACTRESUMEVIDEOTEXTURE 1651
#define AIACTVIDEOTEXTURESPEED 1652
#define AIACTVIDEOTEXTUREVOLUME 1653
#define AIACTSTOPVIDEOTEXTURE 1654
#define AIACTSETBULLETCOL 1655
#define AIACTSYNCRATE 1656
#define AIACTFLASHLIGHT 1657
#define AIACTFLASHLIGHTRED 1658
#define AIACTFLASHLIGHTGREEN 1659
#define AIACTFLASHLIGHTBLUE 1660
#define AIACTFLASHLIGHTRANGE 1661
#define AIACTUSEFADE 1662
#define AIACTSHOWAIR 1663
#define AIACTDELAYDESTROY 1664
#define AIACTLAST 1665
*/

//Interface Constants Data


//  PROPERTIES
#define ENTITY_SETUP                           0
#define ENTITY_WINDOW_TITLE                    4
#define ENTITY_ADD_GROUP                       8
#define ENTITY_DELETE_GROUP                   12
#define ENTITY_SET_GROUP_INDEX                16
#define ENTITY_SET_CONTROL_INDEX              20
#define ENTITY_DELETE_CONTROL                 24
#define ENTITY_ADD_EDIT_BOX                   28
#define ENTITY_ADD_LIST_BOX                   32
#define ENTITY_ADD_COLOR_PICKER               36
#define ENTITY_ADD_FILE_PICKER                40
#define ENTITY_ADD_FONT_PICKER                44
#define ENTITY_SET_CONTROL_NAME               48
#define ENTITY_SET_CONTROL_CONTENTS           52
#define ENTITY_SET_CONTROL_DESCRIPTION        56
#define ENTITY_SET_CONTROL_STATE              60
#define ENTITY_GET_CONTROL_NAME               64
#define ENTITY_GET_CONTROL_CONTENTS           68
#define ENTITY_GET_CONTROL_DESCRIPTION        72
#define ENTITY_ADD_ITEM_TO_LIST_BOX           76
#define ENTITY_CLEAR_LIST_BOX                 80
#define ENTITY_CONTROL_MODIFIED               92
#define ENTITY_CONTROL_MODIFIED_GROUP         96
#define ENTITY_CONTROL_MODIFIED_INDEX         100
#define ENTITY_CONTROL_MODIFIED_RESET         104
#define STRING_A                              1000
#define STRING_B                              1256
#define STRING_C                              1512
#define STRING_D                              1768

//  BUILDGAME
#define BUILD_GAME_SETUP                      0
#define BUILD_GAME_SET_TITLE                  4
#define BUILD_GAME_BUTTON_OK                  8
#define BUILD_GAME_BUTTON_CANCEL              12
#define BUILD_GAME_BUTTON_HELP                16
#define BUILD_GAME_BUTTON_CLOSE               20
#define BUILD_GAME_CLOSE                      24
#define BUILD_GAME_TIMER                      28
#define SET_EDIT_ITEM                         40
#define SET_EDIT_TEXT                         44
#define GET_EDIT_TEXT                         48
#define BUTTON_CLICKED                        52
#define BROWSE_DISPLAY                        56
#define BROWSE_FILE_SELECTED                  60
#define SET_LIST_ITEM                         64
#define ADD_LIST_ITEM                         68
#define GET_LIST_ITEM                         72
#define SET_LIST_INDEX                        76
#define DELETE_LIST_ITEM                      116
#define GET_LIST_SELECTION                    120
#define LIST_SELECTION                        124
#define LIST_INSERT_ITEM                      128
#define LIST_INSERT_POSITION                  132
#define LIST_CLICKED                          140
#define LIST_CONTROL                          144
#define LIST_CLEAR                            152
#define LIST_SELECT_ITEM                      156
#define LIST_SELECT_ITEM_INDEX                160
#define SET_COMBO_ITEM                        80
#define ADD_COMBO_ITEM                        84
#define GET_COMBO_ITEM                        88
#define SET_COMBO_INDEX                       92
#define SET_RADIO_ITEM                        96
#define GET_RADIO_ITEM                        100
#define SET_RADIO_STATE                       148
#define SET_PROGRESS_ITEM                     108
#define SET_PROGRESS_POSITION                 112
#define BUILD_GAME_PROJECT_WEAPONS_DISPLAY    136


#endif