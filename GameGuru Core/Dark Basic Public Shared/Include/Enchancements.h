// Dave - made header files since the infamous Michael Johnson didnt bother

////////////////////////////////////////////////////////////////////
// EAX /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

//#define DARKSDK	__declspec ( dllexport )
#define DARKSDK	
#define DWORD unsigned long

/*
DARKSDK void  PerformCheckListForEAXEffects ( void );
DARKSDK void  ApplyPresetEAXEffect          ( int iSound, int iEffect );

DARKSDK void  SetEAXRoom                    ( int iSound, int   iValue );
DARKSDK void  SetEAXRoomHF                  ( int iSound, int   iValue );
DARKSDK void  SetEAXRoomRolloff             ( int iSound, float fValue );
DARKSDK void  SetEAXDecayTime               ( int iSound, float fValue );
DARKSDK void  SetEAXDecayHFRatio            ( int iSound, float fValue );
DARKSDK void  SetEAXReflections             ( int iSound, int   iValue );
DARKSDK void  SetEAXReflectionsDelay        ( int iSound, float fValue );
DARKSDK void  SetEAXReverb                  ( int iSound, int   iValue );
DARKSDK void  SetEAXReverbDelay             ( int iSound, float fValue );
DARKSDK void  SetEAXEnvironment             ( int iSound, int   iValue );
DARKSDK void  SetEAXEnvironmentSize         ( int iSound, float fValue );
DARKSDK void  SetEAXEnvironmentDiffusion    ( int iSound, float fValue );
DARKSDK void  SetEAXAirAbsorption           ( int iSound, float fValue );
DARKSDK void  SetEAXFlags                   ( int iSound, int   iValue );

DARKSDK void  SetEAXScaleDecayTime          ( int iSound, int   iMode  );
DARKSDK void  SetEAXScaleReflections        ( int iSound, int   iMode  );
DARKSDK void  SetEAXScaleReflectionsDelay   ( int iSound, int   iMode  );
DARKSDK void  SetEAXScaleReverb             ( int iSound, int   iMode  );
DARKSDK void  SetEAXScaleReverbDelay        ( int iSound, int   iMode  );
DARKSDK void  SetEAXClipDecayHF             ( int iSound, int   iMode  );

DARKSDK void  SetEAXSourceDirect            ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceDirectHF          ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceRoom              ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceRoomHF            ( int iSound, float fValue );
DARKSDK void  SetEAXSourceRolloff           ( int iSound, float fValue );
DARKSDK void  SetEAXSourceOutside           ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceAbsorption        ( int iSound, float fValue );
DARKSDK void  SetEAXSourceFlags             ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceObstruction       ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceObstructionLF     ( int iSound, float fValue );
DARKSDK void  SetEAXSourceOcclusion         ( int iSound, int   iValue );
DARKSDK void  SetEAXSourceOcclusionLF       ( int iSound, float fValue );
DARKSDK void  SetEAXSourceOcclusionRoom     ( int iSound, float fValue );

DARKSDK void  SetEAXSourceAffectDirectHF    ( int iSound, int   iMode  );
DARKSDK void  SetEAXSourceAffectRoom        ( int iSound, int   iMode  );
DARKSDK void  SetEAXSourceAffectRoomHF      ( int iSound, int   iMode  );

DARKSDK void  SetEAXConeOrientationX        ( int iSound, float fValue );
DARKSDK void  SetEAXConeOrientationY        ( int iSound, float fValue );
DARKSDK void  SetEAXConeOrientationZ        ( int iSound, float fValue );
DARKSDK void  SetEAXConeInsideAngle         ( int iSound, int   iValue );
DARKSDK void  SetEAXConeOutsideAngle        ( int iSound, int   iValue );
DARKSDK void  SetEAXConeOutsideVolume       ( int iSound, int   iValue );
DARKSDK void  SetEAXMinDistance             ( int iSound, float fValue );
DARKSDK void  SetEAXMaxDistance             ( int iSound, float fValue );

DARKSDK int   GetEAX                        ( void );

DARKSDK int   GetEAXRoom                    ( int iSound );
DARKSDK int   GetEAXRoomHF                  ( int iSound );
DARKSDK DWORD GetEAXRoomRolloff             ( int iSound );
DARKSDK DWORD GetEAXDecayTime               ( int iSound );
DARKSDK DWORD GetEAXDecayHFRatio            ( int iSound );
DARKSDK int   GetEAXReflections             ( int iSound );
DARKSDK DWORD GetEAXReflectionsDelay        ( int iSound );
DARKSDK int   GetEAXReverb                  ( int iSound );
DARKSDK DWORD GetEAXReverbDelay             ( int iSound );
DARKSDK int   GetEAXEnvironment             ( int iSound );
DARKSDK DWORD GetEAXEnvironmentSize         ( int iSound );
DARKSDK DWORD GetEAXEnvironmentDiffusion    ( int iSound );
DARKSDK DWORD GetEAXAirAbsorption           ( int iSound );
DARKSDK int   GetEAXFlags                   ( int iSound );

DARKSDK int   GetEAXSourceDirect            ( int iSound );
DARKSDK int   GetEAXSourceDirectHF          ( int iSound );
DARKSDK int   GetEAXSourceRoom              ( int iSound );
DARKSDK DWORD GetEAXSourceRoomHF            ( int iSound );
DARKSDK DWORD GetEAXSourceRolloff           ( int iSound );
DARKSDK int   GetEAXSourceOutside           ( int iSound );
DARKSDK DWORD GetEAXSourceAbsorption        ( int iSound );
DARKSDK int   GetEAXSourceFlags             ( int iSound );
DARKSDK int   GetEAXSourceObstruction       ( int iSound );
DARKSDK DWORD GetEAXSourceObstructionLF     ( int iSound );
DARKSDK int   GetEAXSourceOcclusion         ( int iSound );
DARKSDK DWORD GetEAXSourceOcclusionLF       ( int iSound );
DARKSDK DWORD GetEAXSourceOcclusionRoom     ( int iSound );

DARKSDK DWORD GetEAXConeOrientationX        ( int iSound );
DARKSDK DWORD GetEAXConeOrientationY        ( int iSound );
DARKSDK DWORD GetEAXConeOrientationZ        ( int iSound );
DARKSDK int   GetEAXConeInsideAngle         ( int iSound );
DARKSDK int   GetEAXConeOutsideAngle        ( int iSound );
DARKSDK int   GetEAXConeOutsideVolume       ( int iSound );
DARKSDK DWORD GetEAXMinDistance             ( int iSound );
DARKSDK DWORD GetEAXMaxDistance             ( int iSound );

		void  GetData                       ( int iSound );
		void  ClearData					    ( void );
		void  SetProperty				    ( int iSound, DSPROPERTY_EAX_LISTENERPROPERTY property, int iType, int iValue, float fValue, eEAXFlags flags, DSPROPERTY_EAX_BUFFERPROPERTY );
		void* GetEAXData                    ( int iSound, int iData );
		void* GetConeData                   ( int iSound, int iData );*/

////////////////////////////////////////////////////////////////////
// FILE BLOCK //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK void CreateFileBlock					( int iID, char* szFilename );
DARKSDK void DeleteFileBlock					( int iID );

// this actually does nothing now, just returns out as the passwork is set in
DARKSDK void SetFileBlockKey					( int iID, char* szKey );
//DARKSDK void SetFileBlockCompression			( int iID, int iLevel );

DARKSDK void AddFileToBlock						( int iID, char* dwFile );
//DARKSDK void AddObjectToBlock					( int iID, int iObject, DWORD dwFilename );
//DARKSDK void AddImageToBlock					( int iID, int iImage, DWORD dwFilename );
//DARKSDK void AddBitmapToBlock                   ( int iID, int iBitmap, DWORD dwFilename );

DARKSDK void SaveFileBlock						( int iID );
DARKSDK void OpenFileBlock						( char* szFile, int iID );
DARKSDK void OpenFileBlock						( char* szFile, int iID, char* szKey );
DARKSDK void OpenFileBlockNoPw					( char* szFile, int iID, char* szKey );
DARKSDK void CloseFileBlock						( int iID );

//DARKSDK void RemoveFileFromBlock				( int iID, DWORD dwFile );

DARKSDK void PerformCheckListForFileBlockData	( int iID );
DARKSDK void ExtractFileFromBlock               ( int iID, char* szFilename, char* szPath );

/*DARKSDK void LoadObjectFromBlock				( int iID, DWORD dwFile, int iObject );
DARKSDK void LoadImageFromBlock					( int iID, DWORD dwFile, int iImage );
DARKSDK void LoadBitmapFromBlock				( int iID, DWORD dwFile, int iBitmap );*/

//DARKSDK void LoadSoundFromBlock					( int iID, DWORD dwFile, int iSound );
//DARKSDK void LoadFileFromBlock					( int iID, DWORD dwFile, int iFile );
//DARKSDK void LoadMemblockFromBlock				( int iID, DWORD dwFile, int iMemblock );
//DARKSDK void LoadAnimationFromBlock				( int iID, DWORD dwFile, int iAnimation );

/*DARKSDK int GetFileBlockExists					( int iID );
DARKSDK int GetFileBlockSize					( int iID );
DARKSDK int GetFileBlockCount					( int iID );
DARKSDK int GetFileBlockDataExists				( int iID, DWORD dwFile );
DARKSDK int GetFileBlockKey						( int iID );
DARKSDK int GetFileBlockCompression				( int iID );*/

void SetupFileBlocks    ( void );
void DestroyFileBlocks  ( void );
void SetTempDirectory   ( void );
void RestoreDirectory   ( void );

/*void AddOrObtainResourceFromBlock ( int iID, int iIndex, DWORD dwFilename, int iType, int iMode );
void Encryption                   ( int iID, int iMode );*/

//void CheckData ( int iID );

////////////////////////////////////////////////////////////////////
// CPU USAGE ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK int   GetProcessorCount				( void );
DARKSDK DWORD GetProcessorVendor			( DWORD dwReturn, int iID );
DARKSDK int   GetProcessorTypeID			( int iID );
DARKSDK int   GetProcessorFamilyID			( int iID );
DARKSDK int   GetProcessorModelID			( int iID );
DARKSDK int   GetProcessorSteppingID		( int iID );
DARKSDK DWORD GetProcessorBrand				( DWORD dwReturn, int iID );
DARKSDK DWORD GetProcessorPlatform			( DWORD dwReturn, int iID );
DARKSDK DWORD GetProcessorName				( DWORD dwReturn, int iID );
DARKSDK int   GetProcessorFrequency			( int iID );
DARKSDK void  GetProcessorOnChipFeatures    ( int iID );
DARKSDK void  GetProcessorPowerManagement   ( int iID );
DARKSDK void  GetProcessorSupportedFeatures ( int iID );

DARKSDK int   GetProcessorUsage             ( int iID );

char* GetOS ( void );

////////////////////////////////////////////////////////////////////
// OS DIALOG ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK DWORD OpenFileDialog ( DWORD dwReturn, DWORD dwDir, DWORD dwFilter, DWORD dwTitle );
DARKSDK DWORD SaveFileDialog ( DWORD dwReturn, DWORD dwDir, DWORD dwFilter, DWORD dwTitle );

////////////////////////////////////////////////////////////////////
// OS DISPLAY //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// lee - 270715 - duplicated in SETUP DLL
//DARKSDK int GetDisplayWidth       ( void );
//DARKSDK int GetDisplayHeight      ( void );
//DARKSDK int GetDisplayDepth       ( void );
//DARKSDK int GetDisplayRefreshRate ( void );

////////////////////////////////////////////////////////////////////
// OS HARDDRIVE ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK int   GetDriveCount				( void );
DARKSDK int   GetDriveCylinderCount		( int iID );
DARKSDK int   GetDriveTrackCount		( int iID );
DARKSDK int   GetDriveSectorsPerTrack	( int iID );
DARKSDK int   GetDriveBytesPerSector	( int iID );
DARKSDK int   GetDriveTotalSize    	    ( int iID, int iReturn );
DARKSDK int   GetDriveUsedSpace			( int iID, int iReturn );
DARKSDK int   GetDriveFreeSpace			( int iID, int iReturn );
DARKSDK int   GetDriveFileLengthSupport ( int iID );
DARKSDK DWORD GetDriveSerial			( DWORD dwReturn, int iID );
DARKSDK DWORD GetDriveSerial			( DWORD dwReturn, int iID, int iUniqueCode );
DARKSDK DWORD GetDriveFileSystem		( DWORD dwReturn, int iID );

DARKSDK int   GetCDCount  ( void );
DARKSDK DWORD GetCDLetter ( DWORD dwReturn, int iID );

void  HardDriveSetup ( void );

////////////////////////////////////////////////////////////////////
// OS INTERNET /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK DWORD GetWebClient ( void );
DARKSDK DWORD GetIPAddress ( void );

////////////////////////////////////////////////////////////////////
// OS MEMORY ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK int GetInstalledMemory   ( int iReturn );
DARKSDK int GetMemoryAvailable   ( int iReturn );
DARKSDK int GetMemoryPercentUsed ( void );
DARKSDK int GetMemoryPercentFree ( void );

////////////////////////////////////////////////////////////////////
// OS COMPUTER NAMES ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK DWORD GetComputerNameEx ( DWORD dwReturn );
DARKSDK DWORD GetUserNameEx     ( DWORD dwReturn );

////////////////////////////////////////////////////////////////////
// OS Speech ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DARKSDK int  GetVoiceAPI		( void );
DARKSDK void GetVoiceList		( void );
DARKSDK void SetVoice			( int iID );
DARKSDK void SetVoiceSpeed		( int iSpeed );
DARKSDK void SetVoiceVolume		( int iVolume );
DARKSDK void SendVoiceMessage	( DWORD dwMessage );
DARKSDK void SendVoiceMessageXML( DWORD dwMessage );
DARKSDK int  GetVoiceSpeed		( void );
DARKSDK int  GetVoiceVolume		( void );

		void VoiceSetup			( void );
		void VoiceDestroy		( void );
		void SetupVoices        ( int iMode, int iID );
		void SendVoiceMessage   ( DWORD dwMessage, int iOption );

//
// FILE MAPPING
//

DARKSDK void  CreateFileMap    ( int iID, LPSTR dwName, DWORD dwSize );
DARKSDK void  OpenFileMap      ( int iID, LPSTR dwName );
DARKSDK void  CloseFileMap     ( int iID );
DARKSDK void  DestroyFileMap   ( int iID );

DARKSDK DWORD GetFileMapDWORD  ( int iID, DWORD dwOffset );
DARKSDK LPSTR GetFileMapString ( int iID, DWORD dwOffset );
DARKSDK DWORD GetFileMapFloat  ( int iID, DWORD dwOffset );

DARKSDK void  SetFileMapDWORD  ( int iID, DWORD dwOffset, DWORD dwValue  );
DARKSDK void  SetFileMapString ( int iID, DWORD dwOffset, LPSTR dwString );
DARKSDK void  SetFileMapFloat  ( int iID, DWORD dwOffset, float fValue );

DARKSDK void  SetEventAndWait  ( int iID );
