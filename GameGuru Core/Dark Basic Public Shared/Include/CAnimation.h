#ifndef _CANIMATION_H_
#define _CANIMATION_H_

#define _LIB

#include "windows.h"
#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>
#include "directx-macros.h"
#include <streams.h>

#ifdef DX11
#else
 struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;
#endif

// old animation method
/*
#ifndef _DEBUG
class CTextureRenderer : public CBaseVideoRenderer
{
	public:
		CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
		virtual ~CTextureRenderer();

	public:
		HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
		HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
		HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample
		HRESULT CopyBufferToTexture();						// Update texture now
    
		LONG					m_lVidWidth;		// Video width
		LONG					m_lVidHeight;		// Video Height
		LONG					m_lVidPitch;		// Video Pitch

		GGFORMAT				m_TextureFormat;	// hold Texture Format
		LPGGTEXTURE		m_pTexture;			// hold Texture Ptr
		float					m_ClipU;			// holds UV of texture clip
		float					m_ClipV;			// holds UV of texture clip

		DWORD					m_dwBitmapSize;
		LPSTR					m_pSampleBitmap;
		bool					m_bSampleBeingUsed;
};
#endif
*/

#undef DARKSDK
#define DARKSDK

DARKSDK void AnimationConstructor					( void );
DARKSDK void AnimationDestructor						( void );
DARKSDK void AnimationSetErrorHandler				( LPVOID pErrorHandlerPtr );
DARKSDK void AnimationPassCoreData					( LPVOID pGlobPtr );
DARKSDK void AnimationRefreshGRAFIX						( int iMode );
DARKSDK void PreventTextureLock				( bool bDoNotLock );
DARKSDK void UpdateAllAnimation				( void );
ID3D11ShaderResourceView* GetAnimPointerView(int AnimIndex);
LPGGSURFACE GetAnimPointerTexture(int AnimIndex);
void SetRenderAnimToImage(int AnimIndex, bool active);
int GetAnimWidth(int AnimIndex);
int GetAnimHeight(int AnimIndex);
void PauseAnim(int AnimIndex);
void ResumeAnim(int AnimIndex);
bool GetAnimDone(int AnimIndex);
float GetAnimPercentDone(int AnimIndex);
void RestartAnim(int AnimIndex);
float GetAnimU(int AnimIndex);
float GetAnimV(int AnimIndex);
DARKSDK BOOL DB_FreeAnimation				( int AnimIndex );

DARKSDK bool LoadAnimation					( LPSTR pFilename, int iIndex, int precacheframes, int videodelayedload, int iSilentMode );
DARKSDK int GetAnimationLength				( int animindex );
DARKSDK void PlayAnimation					( int iIndex );
DARKSDK void DeleteAnimation				( int animindex );
DARKSDK void PlayAnimation					( int animindex );
DARKSDK void StopAnimation					( int animindex );
DARKSDK void PauseAnimation					( int animindex );
DARKSDK void ResumeAnimation				( int animindex );
DARKSDK void LoopAnimation					( int animindex );
DARKSDK void PlayAnimation					( int animindex, int x1, int y1 );
DARKSDK void PlayAnimation					( int animindex, int x1, int y1, int x2, int y2 );
DARKSDK void PlayAnimationToImage			( int animindex, int imageindex, int x1, int y1, int x2, int y2 );
DARKSDK void PlayAnimationToBitmap			( int animindex, int bitmapindex, int x1, int y1, int x2, int y2 );
DARKSDK void LoopAnimationToBitmap			( int animindex, int bitmapindex, int x1, int y1, int x2, int y2 );
DARKSDK void PlaceAnimation					( int animindex, int x1, int y1, int x2, int y2);
DARKSDK void OverrideTextureWithAnimation	( int animindex, int objectnumber );

DARKSDK int  AnimationExist					( int animindex );
DARKSDK int  AnimationPlaying				( int animindex );
DARKSDK int  AnimationPaused				( int animindex );
DARKSDK int  AnimationLooping				( int animindex );
DARKSDK int  AnimationPositionX				( int animindex );
DARKSDK int  AnimationPositionY				( int animindex );
DARKSDK int  AnimationWidth					( int animindex );
DARKSDK int  AnimationHeight				( int animindex );

DARKSDK void LoadDVDAnimation				( int animindex );
DARKSDK void SetAnimationVolume				( int animindex, int ivolume );
DARKSDK void SetAnimationSpeed				( int animindex, int ispeed );
DARKSDK void SetDVDChapter					( int animindex, int iTitle, int iChapterNumber );

DARKSDK int  AnimationVolume				( int animindex );
DARKSDK int  AnimationSpeed					( int animindex );
DARKSDK int  TotalDVDChapters				( int animindex, int iTitle );

#endif _CANIMATION_H_