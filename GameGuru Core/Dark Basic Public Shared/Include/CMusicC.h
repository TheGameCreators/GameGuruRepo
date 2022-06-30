#ifndef _CMUSIC_H_
#define _CMUSIC_H_

#include <wtypes.h>

#undef DARKSDK
#define DARKSDK

DARKSDK void MusicConstructor		( void );
DARKSDK void MusicDestructor		( void );
DARKSDK void SetErrorHandler		( LPVOID pErrorHandlerPtr );
DARKSDK void PassMusicCoreData		( LPVOID pGlobPtr );
DARKSDK void UpdateMusic			( void );
DARKSDK void MusicRefreshD3D		( int iMode );

DARKSDK bool UpdateMusicPtr			( int iID );
	
// loading commands
DARKSDK void LoadMusic				( LPSTR szFilename, int iID );			// load midi file
DARKSDK void LoadCDMusic			( int iTrackNumber, int iID );			// load music file
DARKSDK void DeleteMusic			( int iID );								// delete music

// play commands
DARKSDK void PlayMusic				( int iID );								// play specified music
DARKSDK void LoopMusic				( int iID );								// loop the music
DARKSDK void StopMusic				( int iID );								// stop the music
DARKSDK void PauseMusic				( int iID );								// pause the music
DARKSDK void ResumeMusic			( int iID );								// resume playing the music

// property commands
DARKSDK void SetMusicVolume			( int iID, int iVolume );				// set music volume
DARKSDK void SetMusicSpeed			( int iID, int iSpeed );

// get information commands
DARKSDK int  GetMusicExist			( int iID );								// does music exist?
DARKSDK int  GetMusicPlaying		( int iID );								// is music playing?
DARKSDK int  GetMusicLooping		( int iID );								// is music looping?
DARKSDK int  GetMusicPaused			( int iID );								// is music paused?
DARKSDK int  GetMusicVolume			( int iID );
DARKSDK int  GetMusicSpeed			( int iID );

DARKSDK int  GetNumberOfCDTracks	( void );								// get number of cd tracks

#endif _CMUSIC_H_