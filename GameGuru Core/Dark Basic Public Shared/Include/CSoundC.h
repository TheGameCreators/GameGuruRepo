#ifndef _CSOUND_H_
#define _CSOUND_H_

#include "directx-macros.h"

#include ".\..\Dark Basic Pro SDK\Shared\Sound\csoundmanagerc.h"
#include ".\..\Dark Basic Pro SDK\Shared\Sound\csounddatac.h"

#define DARKSDK

#undef PlaySound

DARKSDK void SoundConstructor		( void );
DARKSDK void SoundDestructor			( void );
DARKSDK void SetSoundErrorHandler	( LPVOID pErrorHandlerPtr );
DARKSDK void PassSoundCoreData		( LPVOID pGlobPtr );
DARKSDK void SoundRefreshGRAFIX			( int iMode );
DARKSDK float GetSoundPosition			(int iID);
DARKSDK void UpdateSound				( void );

DARKSDK bool UpdateSoundPtr ( int iID );
DARKSDK sSoundData* GetSound( int iID );

// loading commands
DARKSDK void LoadSound			   ( LPSTR szFilename, int iID );			// load sound file
DARKSDK void LoadSound			   ( LPSTR szFilename, int iID, int iFlag );
DARKSDK void LoadSound			   ( LPSTR szFilename, int iID, int iFlag, int iSilentFail );
DARKSDK void CloneSound			   ( int iDestination, int iSource );		// clone sound id
DARKSDK void DeleteSound		   ( int iID );								// delete sound

// playing commands
DARKSDK void PlaySound			   ( int iID );								// play sound
DARKSDK void PlaySoundOffset	   ( int iID, int iOffset );				// play sound at offset position
DARKSDK void LoopSound			   ( int iID );								// loop sound
DARKSDK void LoopSound			   ( int iID, int iStart );
DARKSDK void LoopSound			   ( int iID, int iStart, int iEnd );
DARKSDK void LoopSound			   ( int iID, int iStart, int iEnd, int iInitialPos );

DARKSDK void StopSound			   ( int iID );								// stop sound
DARKSDK void ResumeSound		   ( int iID );								// resume sound
DARKSDK void PauseSound			   ( int iID );								// pause sound

// sound properties commands
DARKSDK void SetSoundPan           ( int iID, int iPan );					// set sound pan
DARKSDK void SetSoundSpeed		   ( int iID, int iFrequency );				// set sound speed / frequency
DARKSDK void SetSoundVolume		   ( int iID, int iVolume );				// set sound volume

// recording commands
DARKSDK void RecordSound           ( int iID );								// record sound
DARKSDK void RecordSound		   ( int iID, int iCaptureDuration );
DARKSDK void StopRecordingSound    ( void );								// stop recording session
DARKSDK void SaveSound             ( LPSTR szFilename, int iID );			// save sound to disk

// 3D sound commands
DARKSDK void Load3DSound           ( LPSTR szFilename, int iID );			// load a sound in 3D space
DARKSDK void Load3DSound           ( LPSTR szFilename, int iID, int iSilentFail );
DARKSDK void PositionSound         ( int iID, float fX, float fY, float fZ );	// position sound
DARKSDK void PositionListener	   ( float fX, float fY, float fZ );	// position sound listener
DARKSDK void RotateListener		   ( float fX, float fY, float fZ );		// rotate the listening point
DARKSDK void ScaleListener		   ( float fScale );
DARKSDK void SetEAX				   ( int iEffect ) ;

// get properties commands
DARKSDK int  SoundExist         ( int iID );								// does sound exist?
DARKSDK int  SoundType          ( int iID );								// get sound type - normal or 3D
DARKSDK int  SoundPlaying       ( int iID );								// is sound playing?
DARKSDK int  SoundLooping       ( int iID );								// is sound looping?
DARKSDK int  SoundPaused        ( int iID );								// is sound paused?
DARKSDK int  SoundPan           ( int iID );								// get pan
DARKSDK int  SoundSpeed         ( int iID );								// get speed
DARKSDK int  SoundVolume        ( int iID );								// get volume

// get 3D properties commands
DARKSDK DWORD  SoundPositionXEx     ( int iID );								// get x position
DARKSDK DWORD  SoundPositionYEx     ( int iID );								// get y position
DARKSDK DWORD  tSoundPositionZEx     ( int iID );								// get z position
DARKSDK DWORD  ListenerPositionXEx  ( void );								// get listener x position
DARKSDK DWORD  ListenerPositionYEx  ( void );								// get listener y position
DARKSDK DWORD  ListenerPositionZEx  ( void );								// get listener z position
DARKSDK DWORD  ListenerAngleXEx     ( void );								// get listener x angle position
DARKSDK DWORD  ListenerAngleYEx     ( void );								// get listener y angle position
DARKSDK DWORD  ListenerAngleZEx     ( void );								// get listener z angle position

// SDK Functions

DARKSDK float  SoundPositionX     ( int iID );								// get x position
DARKSDK float  SoundPositionY     ( int iID );								// get y position
DARKSDK float  SoundPositionZ     ( int iID );								// get z position

DARKSDK void GetSoundData( int iID, DWORD* dwBitsPerSecond, DWORD* Frequency, DWORD* Duration, LPSTR* pData, DWORD* dwDataSize, bool bLockData, WAVEFORMATEX* wfx );
DARKSDK void SetSoundData( int iID, DWORD dwBitsPerSecond, DWORD Frequency, DWORD Duration, LPSTR pData, DWORD dwDataSize, WAVEFORMATEX wfx );
DARKSDK LPDIRECTSOUND8 GetSoundInterface ( void );
DARKSDK IDirectSound3DBuffer8* GetSoundBuffer ( int iID );

#endif _CMATRIX_H_

