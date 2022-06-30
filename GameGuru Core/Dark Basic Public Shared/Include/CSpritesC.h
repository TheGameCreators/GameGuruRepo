#ifndef _CSPRITES_H_
#define _CSPRITES_H_

//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#include ".\..\Dark Basic Pro SDK\Shared\Sprites\cspritemanagerc.h"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// DEFINES ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//#ifndef DARKSDK_COMPILE
//	#define DARKSDK __declspec ( dllexport )
//	#define DBPRO_GLOBAL 
//#else
//	#define DARKSDK static
//	#define DBPRO_GLOBAL static
//#endif
//#define SAFE_DELETE( p )       { if ( p ) { delete ( p );       ( p ) = NULL; } }
//#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
//#define SAFE_DELETE_ARRAY( p ) { if ( p ) { delete [ ] ( p );   ( p ) = NULL; } }
#undef DARKSDK
#define DARKSDK

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK void SpritesConstructor		( void );
DARKSDK void SpritesDestructor			( void );
DARKSDK void SetSpritesErrorHandler	( LPVOID pErrorHandlerPtr );
DARKSDK void SpritesPassCoreData		( LPVOID pGlobPtr );
DARKSDK void SpritesRefreshGRAFIX			( int iMode );
DARKSDK void UpdateSprites				( void );
DARKSDK void UpdateAllSprites(void);

#ifdef DARKSDK_COMPILE
	void ConstructorSprites		( HINSTANCE hSetup, HINSTANCE hImage );
	void DestructorSprites		( void );
	void SetErrorHandlerSprites	( LPVOID pErrorHandlerPtr );
	void PassCoreDataSprites		( LPVOID pGlobPtr );
	void RefreshGRAFIXSprites		( int iMode );
	void UpdateSprites			( void );
	void UpdateAllSpritesSprites(void);
#endif

DARKSDK bool UpdateSpritesPtr ( int iID );
DARKSDK void SetVertexDataForSprite( int iX, int iY );
DARKSDK void SaveSpritesBack      ( void );								// backsave
DARKSDK void RestoreSpritesBack      ( void );								// backsave
//DARKSDK void SetErrorHandler ( LPVOID pErrorHandlerPtr );
DARKSDK void GetSpritesDiffuse ( int iID, int* piRed, int* piGreen, int* piBlue );	// get the diffuse of a sprite
DARKSDK void PasteImage ( int iImageID, int iX, int iY, float fU, float fV );
DARKSDK void PasteImageRaw ( LPGGTEXTUREREF	lpTextureView, int iWidth, int iHeight, int iX, int iY, float fU, float fV, int iTransparent );
DARKSDK void PasteImage ( int iImageID, int iX, int iY, float fU, float fV, int iTransparent );
DARKSDK void PasteTextureToRect ( LPGGTEXTUREREF pTexture, float fU, float fV, RECT Rect );

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// COMMAND SET FUNCTIONS //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// dbv1 commands
DARKSDK void SetSprite ( int iID, int iBacksave, int iTransparent );
DARKSDK void Sprite ( int iID, float fX, float fY, int iImage );
DARKSDK void Sprite ( int iID, int iX, int iY, int iImage );		// create a sprite
DARKSDK void PasteSprite ( int iID, int iX, int iY );					// paste sprite on the screen
DARKSDK void PasteSprite ( int iID, int iX, int iY, int iDrawImmediately );
DARKSDK void ResetSpriteBatcher ( void );
DARKSDK void DrawSpriteBatcher ( void );
DARKSDK void SizeSprite ( int iID, int iXSize, int iYSize );			// set the size of the sprite
DARKSDK void ScaleSprite   ( int iID, float fScale );						// scale a sprite
DARKSDK void StretchSprite ( int iID, int iXStretch, int iYStretch );		// stretch the sprite
DARKSDK void MirrorSprite  ( int iID );									// mirror a sprite
DARKSDK void FlipSprite   ( int iID );									// flip a sprite
DARKSDK void OffsetSprite  ( int iID, int iXOffset, int iYOffset );		// offset sprite
DARKSDK void HideSprite           ( int iID );							// hide sprite
DARKSDK void ShowSprite           ( int iID );							// show sprite
DARKSDK void HideAllSprites ( void );								// hide all sprites
DARKSDK void ShowAllSprites ( void );								// show all sprites
DARKSDK void DisableAllSprites ( void );								// disable all sprites
DARKSDK void EnableAllSprites ( void );								// enable all sprites
DARKSDK void DeleteSprite         ( int iID );							// delete sprite

// dbpro commands
DARKSDK void MoveSprite  ( int iID, float velocity );						// rotate the sprite
DARKSDK void RotateSprite  ( int iID, float fRotate );						// rotate the sprite
DARKSDK void SetSpriteImage ( int iID, int iImage );						// set the image that the sprite uses
DARKSDK void SetSpriteAlpha   ( int iID, int iValue );					// set the alpha level of a sprite from 0 - 255
DARKSDK void SetSpriteDiffuse ( int iID, int iR, int iG, int iB );		// set the diffuse colours
DARKSDK void PlaySprite                 ( int iID, int iStart, int iEnd, int iDelay );					// play an animation
DARKSDK void SetSpriteFrameEx             ( int iID, int iFrame );
DARKSDK void SetSpriteFrame             ( int iID, int iFrame );											// set the current frame of animation
DARKSDK void SetSpriteTextureCoordinates ( int iID, int iVertex, float tu, float tv );					// set texture coordinates
DARKSDK void CreateAnimatedSprite ( int iID, char* szImage, int iWidth, int iHeight, int iImageID );	// create an animated sprite from 1 image
DARKSDK void CloneSprite  ( int iID, int iDestinationID );				// clones a sprite
DARKSDK void SetSpritePriority ( int iID, int iPriority );

DARKSDK void SetSpriteResizeMode ( int iMode );							// mike - 041005 - special option to resize sprites automatically
DARKSDK void SetSpriteFilterMode ( int iMode );
DARKSDK void ResizeSprite ( int iMode, float fOffsetX, float fScaleX );

DARKSDK int  SpritePriority ( int iID );							// mike - 041005 - return the priority of the sprite
DARKSDK int  SpriteExist     ( int iID );								// does sprite exist?
DARKSDK int  SpriteX         ( int iID );								// get x pos
DARKSDK int  SpriteY         ( int iID );								// get y pos
DARKSDK int  SpriteImage     ( int iID );								// get image number
DARKSDK int	 GetSpriteImage	 ( int iID );
DARKSDK int  SpriteWidth     ( int iID );								// get width
DARKSDK int  SpriteHeight    ( int iID );								// get height
DARKSDK int  SpriteScaleX    ( int iID );								// get x scale
DARKSDK int  SpriteScaleY    ( int iID );								// get y scale
DARKSDK int  SpriteMirrored  ( int iID );								// is mirrored?
DARKSDK int  SpriteFlipped   ( int iID );								// is flipped
DARKSDK int  SpriteOffsetX   ( int iID );								// get x offset
DARKSDK int  SpriteOffsetY   ( int iID );								// get y offset
DARKSDK int  SpriteHit       ( int iID, int iTarget );					// has sprite hit another?
DARKSDK int  SpriteCollision ( int iID, int iTarget );					// is sprite overlapping?

DARKSDK float SpriteAngle	  ( int iID );								// get the alpha of a sprite
DARKSDK int  SpriteAlpha     ( int iID );								// get the alpha of a sprite
DARKSDK int  SpriteRed       ( int iID );								// get the red colour
DARKSDK int  SpriteGreen     ( int iID );								// get the green colour
DARKSDK int  SpriteBlue      ( int iID );								// get the blue colour
DARKSDK int  SpriteFrame     ( int iID );								// get current animation frame
DARKSDK int  SpriteVisible   ( int iID );								

#ifdef DARKSDK_COMPILE
		void	dbSetSprite						( int iID, int iBacksave, int iTransparent );
		void	dbSprite						( int iID, int iX, int iY, int iImage );
		void	dbPasteSprite					( int iID, int iX, int iY );	
		void	dbSizeSprite					( int iID, int iXSize, int iYSize );
		void	dbScaleSprite					( int iID, float fScale );			
		void	dbStretchSprite					( int iID, int iXStretch, int iYStretch );
		void	dbMirrorSprite					( int iID );							
		void	dbFlipSprite					( int iID );							
		void	dbOffsetSprite					( int iID, int iXOffset, int iYOffset );	
		void	dbHideSprite					( int iID );						
		void	dbShowSprite					( int iID );						
		void	dbHideAllSprites				( void );							
		void	dbShowAllSprites				( void );							
		void	dbDeleteSprite					( int iID );						

		void	dbMoveSprite					( int iID, float velocity );					
		void	dbRotateSprite					( int iID, float fRotate );				
		void	dbSetSpriteImage				( int iID, int iImage );					
		void	dbSetSpriteAlpha				( int iID, int iValue );				
		void	dbSetSpriteDiffuse 				( int iID, int iR, int iG, int iB );	
		void	dbPlaySprite					( int iID, int iStart, int iEnd, int iDelay );			
		void	dbSetSpriteFrame				( int iID, int iFrame );									
		void	dbSetSpriteTextureCoordinates	( int iID, int iVertex, float tu, float tv );			
		void	dbCreateAnimatedSprite			( int iID, char* szImage, int iWidth, int iHeight, int iImageID );
		void	dbCloneSprite					( int iID, int iDestinationID );			
		void	dbSetSpritePriority				( int iID, int iPriority );

		int		dbSpriteExist					( int iID );
		int		dbSpriteX         				( int iID );
		int		dbSpriteY         				( int iID );
		int		dbSpriteImage     				( int iID );
		int		dbSpriteWidth     				( int iID );
		int		dbSpriteHeight    				( int iID );
		int		dbSpriteScaleX    				( int iID );
		int		dbSpriteScaleY    				( int iID );
		int		dbSpriteMirrored  				( int iID );
		int		dbSpriteFlipped   				( int iID );
		int		dbSpriteOffsetX   				( int iID );
		int		dbSpriteOffsetY   				( int iID );
		int		dbSpriteHit       				( int iID, int iTarget );
		int		dbSpriteCollision 				( int iID, int iTarget );

		float	dbSpriteAngle	  				( int iID );							
		int		dbSpriteAlpha     				( int iID );							
		int		dbSpriteRed       				( int iID );							
		int		dbSpriteGreen     				( int iID );							
		int		dbSpriteBlue      				( int iID );							
		int		dbSpriteFrame     				( int iID );							
		int		dbSpriteVisible   				( int iID );

		void	dbPasteImage					( int iImageID, int iX, int iY, float fU, float fV );
		void	dbPasteImageEx					( int iImageID, int iX, int iY, float fU, float fV, int iTransparent );
		void	dbPasteTextureToRect			( LPGGTEXTURE pTexture, float fU, float fV, RECT Rect );

		void	dbSaveSpriteBack				( void );
		void	dbRestoreSpriteBack				( void );

		// lee - 300706 - GDK fixes
		void	dbScaleSprite					( int iID, int iScale );			
		void	dbSetSpriteTextureCoord			( int iID, int iVertex, float tu, float tv );			

#endif

//////////////////////////////////////////////////////////////////////////////////

#endif _CSPRITES_H_