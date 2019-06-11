#define _USING_V110_SDK71_
#include "globstruct.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "mmsystem.h"

#include "canimation.h"
#include ".\..\error\cerror.h"
#include "ImageSupport.h"
#include "CGfxC.h"
#include "CImageC.h"
#include "CSpritesC.h"

#include <clipffmpeg/clipffmpeg.h>
#include <theoraplayer/FrameQueue.h>
#include <theoraplayer/Manager.h>
#include <theoraplayer/MemoryDataSource.h>
#include <theoraplayer/theoraplayer.h>
#include <theoraplayer/VideoClip.h>
#include <theoraplayer/VideoFrame.h>

// Structures
#define ANIMATIONMAX 33
struct
{
	char						AnimFile[256];
	theoraplayer::VideoClip*	pMediaClip;
	GGFORMAT					TextureFormat;
	LPGGSURFACE					pTexture;
	LPGGSHADERRESOURCEVIEW		pTextureRef;
	float						ClipU;
	float						ClipV;
	RECT						StreamRect;
	int							x1;
	int							y1;
	int							x2;
	int							y2;
	RECT						WantRect;
	bool						bStreamingNow;
	bool						loop;
	int							iOutputToImage;
	int							precacheframes;
	bool						loaded;
	int							videodelayedload;
	//LPGGTEXTURE				pOutputToTexture;
} Anim[ANIMATIONMAX];

struct ANIMATIONTYPE
{
	bool						active;
	bool						playing;
	bool						paused;
	bool						looped;
	int							volume;
	int							speed;
};
ANIMATIONTYPE animation [ ANIMATIONMAX ];
theoraplayer::OutputMode theoraOutputMode = theoraplayer::FORMAT_UNDEFINED;
float fLastTimeVideoManagerUpdated = 0.0f;

// Externals
extern float					timeGetSecond(void);
extern int						GetBitDepthFromFormat(GGFORMAT Format);
extern GlobStruct*				g_pGlob;
extern LPGG						m_pDX;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
//extern PTR_FuncCreateStr					g_pCreateDeleteStringFunction;
//DBPRO_GLOBAL bool							g_bDoNotLockTextureAtThisTime = false;

#ifdef DX11

void AnimationConstructor ( void )
{
	// Initialise the Theora Player manager (.ogv)
	theoraplayer::init(1);
	theoraOutputMode = theoraplayer::FORMAT_BGRX;
	theoraplayer::manager->setWorkerThreadCount(1);

	// Provide MP4 support (.mp4)
	clipffmpeg::init();

	// Clear Arrays
	ZeroMemory(&Anim, sizeof(Anim));
	ZeroMemory(&animation, sizeof(animation));
}

int potCeil(int value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	++value;
	return value;
}

void AnimationRefreshGRAFIX ( int iMode )
{
}

DARKSDK void AnimationDestructor ( void )
{
	// Free All Animations
	for(int AnimIndex=0; AnimIndex<ANIMATIONMAX; AnimIndex++)
	{
		DB_FreeAnimation(AnimIndex);
		ZeroMemory(&Anim[AnimIndex], sizeof(Anim[AnimIndex]));
		ZeroMemory(&animation[AnimIndex], sizeof(animation[AnimIndex]));
	}

	// Free Theora Player manager
	clipffmpeg::destroy();
	theoraplayer::destroy();
}

BOOL CoreLoadAnimation( int AnimIndex, char* Filename, int precacheframes)
{
	// Vars
    HRESULT hr = S_OK;

	// load in an OGV video file (streaming)
	// PE: precache of 16 at 1080p = 95 MB , if you have setup 19 zones with video , these are preloaded and add 1.76 GB ... not good ...
	Anim[AnimIndex].pMediaClip = theoraplayer::manager->createVideoClip(Filename, theoraOutputMode, precacheframes );
	if ( Anim[AnimIndex].pMediaClip )
	{
		// can cause video to loop
		Anim[AnimIndex].pMediaClip->setAutoRestart(false);
		Anim[AnimIndex].pMediaClip->stop();

		// Get size of video
		float fWidth = Anim[AnimIndex].pMediaClip->getWidth();
		float fHeight = Anim[AnimIndex].pMediaClip->getHeight();
		float tTexWidthPOT = potCeil(fWidth);
		float tTexHeightPOT = potCeil(fHeight);

		// Determine DX11 format from THEORA Video Format
		DXGI_FORMAT chooseTextureFormat = DXGI_FORMAT_UNKNOWN;
		switch ( Anim[AnimIndex].pMediaClip->getOutputMode() )
		{
			case theoraplayer::FORMAT_BGRX : chooseTextureFormat = DXGI_FORMAT_B8G8R8X8_UNORM; break;
		}
		Anim[AnimIndex].TextureFormat = chooseTextureFormat;

		// Create Texture for video to be written to
		SAFE_RELEASE(Anim[AnimIndex].pTextureRef);
		SAFE_RELEASE(Anim[AnimIndex].pTexture);
		D3D11_TEXTURE2D_DESC VideoTextureDesc = { tTexWidthPOT, tTexHeightPOT, 1, 1, chooseTextureFormat, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, 0 };
		m_pD3D->CreateTexture2D( &VideoTextureDesc, NULL, &Anim[AnimIndex].pTexture );
		Anim[AnimIndex].ClipU = fWidth / tTexWidthPOT;
		Anim[AnimIndex].ClipV = fHeight / tTexHeightPOT;

		// Create resource view of above texture
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = chooseTextureFormat;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1; //ORG:-1; PE: we dont use any miplevels for video.
		m_pD3D->CreateShaderResourceView(Anim[AnimIndex].pTexture, &shaderResourceViewDesc, &Anim[AnimIndex].pTextureRef);

		// Get Size of Animation
		Anim[AnimIndex].StreamRect.top=0;
		Anim[AnimIndex].StreamRect.left=0;
		Anim[AnimIndex].StreamRect.right=fWidth;
		Anim[AnimIndex].StreamRect.bottom=fHeight;

		// Complete
		return TRUE;
	}
	return FALSE;
}

BOOL DB_LoadAnimationCore(int AnimIndex, char* Filename, int precacheframes)
{
	// Attempt to load video
	if (CoreLoadAnimation(AnimIndex, Filename, precacheframes))
	{
		// File loaded, store name for re-play
		strcpy(Anim[AnimIndex].AnimFile, Filename);

		// Use default anim size
		Anim[AnimIndex].iOutputToImage = 0;
		Anim[AnimIndex].x1 = 0;
		Anim[AnimIndex].y1 = 0;
		Anim[AnimIndex].x2 = 0;
		Anim[AnimIndex].y2 = 0;
		Anim[AnimIndex].WantRect.left = 0;
		Anim[AnimIndex].WantRect.top = 0;
		Anim[AnimIndex].WantRect.right = 0;
		Anim[AnimIndex].WantRect.bottom = 0;
		Anim[AnimIndex].bStreamingNow = false;
		Anim[AnimIndex].precacheframes = precacheframes;
		Anim[AnimIndex].loaded = true;

		// Complete
		return TRUE;
	}
}

BOOL DB_LoadAnimation(int AnimIndex, char* Filename, int precacheframes , int videodelayedload)
{
	if (videodelayedload == 1) {
		// File loaded, store name for re-play
		if ( Anim[AnimIndex].loaded == true) DB_FreeAnimation(AnimIndex);

		strcpy(Anim[AnimIndex].AnimFile, Filename);

		// Use default anim size
		Anim[AnimIndex].iOutputToImage = 0;
		Anim[AnimIndex].x1 = 0;
		Anim[AnimIndex].y1 = 0;
		Anim[AnimIndex].x2 = 0;
		Anim[AnimIndex].y2 = 0;
		Anim[AnimIndex].WantRect.left = 0;
		Anim[AnimIndex].WantRect.top = 0;
		Anim[AnimIndex].WantRect.right = 0;
		Anim[AnimIndex].WantRect.bottom = 0;
		Anim[AnimIndex].bStreamingNow = false;
		Anim[AnimIndex].precacheframes = precacheframes;
		Anim[AnimIndex].loaded = false;
		Anim[AnimIndex].videodelayedload = videodelayedload;

		// Complete
		return TRUE;
	}
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, Filename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename, precacheframes);
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
	return bRes;
}

BOOL DB_LoadAnimation_Delayed(int AnimIndex)
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, Anim[AnimIndex].AnimFile );
	g_pGlob->UpdateFilenameFromVirtualTable((DWORD)VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt((DWORD)VirtualFilename);
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename, Anim[AnimIndex].precacheframes);
	g_pGlob->Encrypt((DWORD)VirtualFilename);
	return bRes;
}

DARKSDK BOOL DB_FreeAnimation(int AnimIndex)
{
	// Shut down the graph
	if(Anim[AnimIndex].pMediaClip)
	{
		// Release video
        Anim[AnimIndex].pMediaClip->stop();
		theoraplayer::manager->destroyVideoClip(Anim[AnimIndex].pMediaClip);
		Anim[AnimIndex].pMediaClip = NULL;

		// Release resources used by video
		SAFE_RELEASE ( Anim[AnimIndex].pTextureRef );
		SAFE_RELEASE ( Anim[AnimIndex].pTexture );
		//SAFE_RELEASE ( Anim[AnimIndex].pOutputToTexture );

		if (Anim[AnimIndex].videodelayedload == 1)
		{
			Anim[AnimIndex].loaded = false;
		}
	}

	// Complete
	return TRUE;
}

DARKSDK BOOL DB_PlayAnimationToScreen(int AnimIndex, int set, int x, int y, int x2, int y2, bool bPlayFromScratch)
{
	//PE: only load when we need video.
	if ( Anim[AnimIndex].videodelayedload == 1)
	{
		if (Anim[AnimIndex].loaded == false) {
			if (!DB_LoadAnimation_Delayed(AnimIndex)) {
				return FALSE;
			}
			Anim[AnimIndex].loaded = false;
		}
	}

	// Set new animation placements if so
	if(set==1)
	{
		Anim[AnimIndex].x1 = x;
		Anim[AnimIndex].y1 = y;
		Anim[AnimIndex].x2 = x2;
		Anim[AnimIndex].y2 = y2;
	}
	else
	{
		x = Anim[AnimIndex].x1;
		y = Anim[AnimIndex].y1;
		x2 = Anim[AnimIndex].x2;
		y2 = Anim[AnimIndex].y2;
	}

	// Must have animation to play
	if(Anim[AnimIndex].pMediaClip==NULL)
		return FALSE;

	// Output Size
	if(x2==0 && y2==0)
	{
		Anim[AnimIndex].WantRect.left	= x;
		Anim[AnimIndex].WantRect.top	= y;
		Anim[AnimIndex].WantRect.right	= x + Anim[AnimIndex].StreamRect.right;
		Anim[AnimIndex].WantRect.bottom	= y + Anim[AnimIndex].StreamRect.bottom;
	}
	else
	{
		Anim[AnimIndex].WantRect.left	= x;
		Anim[AnimIndex].WantRect.top	= y;
		Anim[AnimIndex].WantRect.right	= x2;
		Anim[AnimIndex].WantRect.bottom	= y2;
	}

	// Set State to Run
	Anim[AnimIndex].bStreamingNow=true;
	//Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
	Anim[AnimIndex].pMediaClip->play();

	// Complete
	return TRUE;
}

DARKSDK BOOL DB_ResizeAnimation(int AnimIndex, int x1, int y1, int x2, int y2)
{
	Anim[AnimIndex].x1 = x1;
	Anim[AnimIndex].y1 = y1;
	Anim[AnimIndex].x2 = x2;
	Anim[AnimIndex].y2 = y2;
	Anim[AnimIndex].WantRect.left	= x1;
	Anim[AnimIndex].WantRect.top	= y1;
	Anim[AnimIndex].WantRect.right	= x2;
	Anim[AnimIndex].WantRect.bottom	= y2;
	return TRUE;
}

BOOL DB_StopAnimation(int AnimIndex)
{
	// If currently not playing this animation, nothing to stop
	if(Anim[AnimIndex].pMediaClip==NULL)
		return FALSE;

	// stop animation
	Anim[AnimIndex].bStreamingNow=false;
	Anim[AnimIndex].pMediaClip->stop();

	if (Anim[AnimIndex].videodelayedload == 1) {
		//Free video.
		animation[AnimIndex].playing = false;
		DB_FreeAnimation(AnimIndex);
	}

	// Complete
	return TRUE;
}

DARKSDK BOOL DB_LoopAnimationOn(int AnimIndex)
{
	Anim[AnimIndex].loop=true;
	return TRUE;
}

DARKSDK BOOL DB_LoopAnimationOff(int AnimIndex)
{
	Anim[AnimIndex].loop=false;
	return TRUE;
}

DARKSDK void UpdateAllAnimation(void)
{
	// an update call to the video manager
	bool bSkipFirstCycle = false;
	if ( fLastTimeVideoManagerUpdated == 0.0f ) bSkipFirstCycle = true;
	float fTimeNow = timeGetSecond();
	float fDeltaTime = fTimeNow - fLastTimeVideoManagerUpdated;
	fLastTimeVideoManagerUpdated = fTimeNow;
	if ( bSkipFirstCycle == false ) theoraplayer::manager->update(fDeltaTime);

	// Monitor all Animations
	for(int AnimIndex=0; AnimIndex<ANIMATIONMAX; AnimIndex++)
	{
		if(Anim[AnimIndex].pMediaClip)
		{
			// only if playing
			if ( animation[AnimIndex].playing == true )
			{
				// possibly need to wait until video frames buffered in cache before we display anything
				//if (!started)
				//{
				//	// let's wait until the system caches up a few frames on startup
				//	if (clip->getReadyFramesCount() < clip->getPrecachedFramesCount() * 0.5f)
				//	{
				//		started = false;
				//	}
				//	started = true;
				//}

				// pull next available frame
				theoraplayer::VideoFrame* frame = Anim[AnimIndex].pMediaClip->fetchNextFrame();
				if (frame != NULL)
				{
					// Copy data from video to video texture
					GGSURFACE_DESC destdesc;
					LPGGSURFACE pTextureInterface = NULL;
					Anim[AnimIndex].pTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
					pTextureInterface->GetDesc(&destdesc);
					SAFE_RELEASE ( pTextureInterface );
					DWORD dwDescBPP = GetBitDepthFromFormat ( destdesc.Format ) / 8;
					ID3D11Texture2D* pTempDestStageTexture = NULL;
					D3D11_TEXTURE2D_DESC StagedDestDesc = { destdesc.Width, destdesc.Height, 1, 1, destdesc.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE, 0 };
					m_pD3D->CreateTexture2D( &StagedDestDesc, NULL, &pTempDestStageTexture );
					if ( pTempDestStageTexture )
					{
						// lock for writing staging texture
						GGLOCKED_RECT grafixlr;
						if(SUCCEEDED(m_pImmediateContext->Map(pTempDestStageTexture, 0, D3D11_MAP_WRITE, 0, &grafixlr)))
						{
							// get raw data from frame and copy it directly to video dest stage texture
							unsigned char* pSrc = frame->getBuffer();
							unsigned char* pPtr = (unsigned char*)grafixlr.pData;
							DWORD dwVideoWidth = Anim[AnimIndex].pMediaClip->getWidth();
							DWORD dwVideoHeight = Anim[AnimIndex].pMediaClip->getHeight();
							DWORD dwDataWidth = dwVideoWidth * dwDescBPP;
							for(DWORD y=0; y<dwVideoHeight; y++)
							{
								memcpy(pPtr, pSrc, dwDataWidth);
								pPtr+=grafixlr.RowPitch;
								pSrc+=dwDataWidth;
							}

							/* format conversion during writing from video data to video texture
							for ( int iY=RegionRect.top; iY<RegionRect.bottom; iY++)
							{
								int yadd = (int)iY*grafixlr.RowPitch;
								int animyadd = (int)fAnimY*animd3dlr.RowPitch;
								for ( int iX=RegionRect.left; iX<(int)RegionRect.right; iX++)
								{
									// Get source pixel and write to dest
									LPSTR pRead = (LPSTR)animd3dlr.pData+(int)fAnimX*dwSrcBPP+(animyadd);

									if ( dwDescBPP==2 )
									{
										DWORD dwPxl = *(WORD*)pRead;
										if(dwSrcBPP==4)
										{
											// convert 8888 to 1555
											dwPxl = *(DWORD*)pRead;
											int red =	(int)(((dwPxl & (255<<16)) >> 16) / 8.3);
											int green = (int)(((dwPxl & (255<<8) ) >> 8)  / 8.3);
											int blue =	(int)(( dwPxl &  255     )        / 8.3);
											if(red>31) red=31;
											if(green>31) green=31;
											if(blue>31) blue=31;
											dwPxl = (1<<15)+(red<<10)+(green<<5)+(blue);
										}
										LPSTR pWrite = (LPSTR)grafixlr.pData+(int)iX*dwDescBPP+(yadd);
										*(WORD*)pWrite = (WORD)dwPxl;
									}
									if ( dwDescBPP==4 )
									{
										DWORD dwPxl = *(DWORD*)pRead;
										LPSTR pWrite = (LPSTR)grafixlr.pData+(int)iX*dwDescBPP+(yadd);
										*(DWORD*)pWrite = dwPxl;
									}

									// Advance source vector
									fAnimX += fXBit;
								}

								// Advance source vector
								fAnimY += fYBit;
								fAnimX = 0.0f;
							}
							*/

							// release temp destination stage texture lock
							m_pImmediateContext->Unmap(pTempDestStageTexture, 0);

							// and finally copy the staged dest texture to the real video texture
							D3D11_BOX rc = { 0, 0, 0, (LONG)(destdesc.Width), (LONG)(destdesc.Height), 1 }; 
							m_pImmediateContext->CopySubresourceRegion(Anim[AnimIndex].pTexture, 0, 0, 0, 0, pTempDestStageTexture, 0, &rc);
						}

						// performance can be improved if this is created once during video playback
						SAFE_RELEASE ( pTempDestStageTexture );
					}

					// pop this frame so can get the next one
					Anim[AnimIndex].pMediaClip->popFrame();
				}

				// Handle Animations that loop
				if(Anim[AnimIndex].bStreamingNow==true)
				{
					// If Looping, repeat run when it ends
					if ( Anim[AnimIndex].pMediaClip->getTimePosition() >= Anim[AnimIndex].pMediaClip->getDuration() )
					{
						if(Anim[AnimIndex].loop==true)
						{
							//if(Anim[AnimIndex].pMediaPosition)
							//{
							//	Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
							//}
							Anim[AnimIndex].pMediaClip->stop();
							Anim[AnimIndex].pMediaClip->play();
						}
						else
						{
							Anim[AnimIndex].pMediaClip->stop();
							animation[AnimIndex].playing=false;
						}
					}
				}

				// To image or current render target
				if ( Anim[AnimIndex].iOutputToImage == 0 )
				{
					// Update sample in backbuffer (reverse Y as anim data is reversed)
					float myClipV = Anim[AnimIndex].ClipV;
					PasteTextureToRect( Anim[AnimIndex].pTextureRef, Anim[AnimIndex].ClipU, myClipV, Anim[AnimIndex].WantRect );
				}
				else
				{
					/* no video to texture for now - but can be easily restored
					// Get destination size
					GGSURFACE_DESC destdesc;
					LPGGSURFACE pTextureInterface = NULL;
					Anim[AnimIndex].pOutputToTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
					pTextureInterface->GetDesc(&destdesc);
					SAFE_RELEASE ( pTextureInterface );
					DWORD dwDescBPP = GetBitDepthFromFormat ( destdesc.Format ) / 8;

					// Get source size
					GGSURFACE_DESC srcdesc;
					pTextureInterface = NULL;
					Anim[AnimIndex].pTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
					pTextureInterface->GetDesc(&srcdesc);
					SAFE_RELEASE ( pTextureInterface );
					DWORD dwSrcBPP = GetBitDepthFromFormat ( srcdesc.Format ) / 8;

					// Get region to draw animation to
					RECT RegionRect = Anim[AnimIndex].WantRect;

					// Work out texture/image ratio
					int tImageWidth = ImageWidth ( Anim[AnimIndex].iOutputToImage );
					int tImageHeight = ImageHeight ( Anim[AnimIndex].iOutputToImage );
					float ratioX = (float)destdesc.Width / (float)tImageWidth;
					float ratioY = (float)destdesc.Height / (float)tImageHeight;

					// Scale region to any texture stretching
					RegionRect.top = (int)((float)RegionRect.top * ratioY);
					RegionRect.bottom = (int)((float)RegionRect.bottom * ratioY);
					RegionRect.left = (int)((float)RegionRect.left * ratioX);
					RegionRect.right = (int)((float)RegionRect.right * ratioX);

					// Final Region dimension
					DWORD RegionWidth = RegionRect.right - RegionRect.left;
					DWORD RegionHeight = RegionRect.bottom - RegionRect.top;

					// if image texutre smaller than dest area, reduce dest area
					DWORD dwWidth = Anim[AnimIndex].StreamRect.right;
					DWORD dwHeight = Anim[AnimIndex].StreamRect.bottom;
					if(destdesc.Width<dwWidth) dwWidth=destdesc.Width;
					if(destdesc.Height<dwHeight) dwHeight=destdesc.Height;
					float fAnimX=0.0f;
					float fAnimY=0.0f;
					float fXBit=(float)Anim[AnimIndex].StreamRect.right/(float)RegionWidth;
					float fYBit=(float)Anim[AnimIndex].StreamRect.bottom/(float)RegionHeight;

					// Sort out anim source
					ID3D11Texture2D* pTempSysMemTexture = NULL;
					D3D11_TEXTURE2D_DESC StagedDesc = { srcdesc.Width, srcdesc.Height, 1, 1, srcdesc.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0 };
					m_pD3D->CreateTexture2D( &StagedDesc, NULL, &pTempSysMemTexture );
					if ( pTempSysMemTexture )
					{
						// and copy texture image to it
						D3D11_BOX rc = { 0, 0, 0, (LONG)(srcdesc.Width), (LONG)(srcdesc.Height), 1 }; 
						m_pImmediateContext->CopySubresourceRegion(pTempSysMemTexture, 0, 0, 0, 0, Anim[AnimIndex].pTexture, 0, &rc);

						// lock for reading staging texture
						GGLOCKED_RECT animd3dlr;
						if(SUCCEEDED(m_pImmediateContext->Map(pTempSysMemTexture, 0, D3D11_MAP_READ, 0, &animd3dlr)))
						{
							ID3D11Texture2D* pTempDestStageTexture = NULL;
							D3D11_TEXTURE2D_DESC StagedDestDesc = { destdesc.Width, destdesc.Height, 1, 1, destdesc.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE, 0 };
							m_pD3D->CreateTexture2D( &StagedDestDesc, NULL, &pTempDestStageTexture );
							if ( pTempDestStageTexture )
							{
								// lock for writing staging texture
								GGLOCKED_RECT grafixlr;
								if(SUCCEEDED(m_pImmediateContext->Map(pTempDestStageTexture, 0, D3D11_MAP_WRITE, 0, &grafixlr)))
								{
									for ( int iY=RegionRect.top; iY<RegionRect.bottom; iY++)
									{
										int yadd = (int)iY*grafixlr.RowPitch;
										int animyadd = (int)fAnimY*animd3dlr.RowPitch;
										for ( int iX=RegionRect.left; iX<(int)RegionRect.right; iX++)
										{
											// Get source pixel and write to dest
											LPSTR pRead = (LPSTR)animd3dlr.pData+(int)fAnimX*dwSrcBPP+(animyadd);

											if ( dwDescBPP==2 )
											{
												DWORD dwPxl = *(WORD*)pRead;
												if(dwSrcBPP==4)
												{
													// convert 8888 to 1555
													dwPxl = *(DWORD*)pRead;
													int red =	(int)(((dwPxl & (255<<16)) >> 16) / 8.3);
													int green = (int)(((dwPxl & (255<<8) ) >> 8)  / 8.3);
													int blue =	(int)(( dwPxl &  255     )        / 8.3);
													if(red>31) red=31;
													if(green>31) green=31;
													if(blue>31) blue=31;
													dwPxl = (1<<15)+(red<<10)+(green<<5)+(blue);
												}
												LPSTR pWrite = (LPSTR)grafixlr.pData+(int)iX*dwDescBPP+(yadd);
												*(WORD*)pWrite = (WORD)dwPxl;
											}
											if ( dwDescBPP==4 )
											{
												DWORD dwPxl = *(DWORD*)pRead;
												LPSTR pWrite = (LPSTR)grafixlr.pData+(int)iX*dwDescBPP+(yadd);
												*(DWORD*)pWrite = dwPxl;
											}

											// Advance source vector
											fAnimX += fXBit;
										}

										// Advance source vector
										fAnimY += fYBit;
										fAnimX = 0.0f;
									}

									// release temp destination stage texture lock
									m_pImmediateContext->Unmap(pTempDestStageTexture, 0);

									// and finally copy the staged dest texture to the real output texture
									D3D11_BOX rc = { 0, 0, 0, (LONG)(destdesc.Width), (LONG)(destdesc.Height), 1 }; 
									m_pImmediateContext->CopySubresourceRegion(Anim[AnimIndex].pOutputToTexture, 0, 0, 0, 0, pTempDestStageTexture, 0, &rc);
								}
							}

							// release temp stage texture lock
							m_pImmediateContext->Unmap(pTempSysMemTexture, 0);
						}
	
						// free temp system surface
						SAFE_RELEASE(pTempSysMemTexture);
					}
					*/
				}
			}
		}
	}
}

//
// Actual function calls used by GameGuru below
//

extern void timestampactivity(int i, char* desc_s);

DARKSDK void LoadAnimation( LPSTR pFilename, int animindex , int precacheframes , int videodelayedload)
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		// Delete before proceeding to load..
		if(animation[animindex].active==true)
		{
			DB_FreeAnimation(animindex);
			animation[animindex].active=false;
		}

		if(DB_LoadAnimation(animindex, pFilename, precacheframes, videodelayedload))
		{
			animation[animindex].active=true;
			animation[animindex].playing=false;
			animation[animindex].paused=false;
			animation[animindex].looped=false;
			animation[animindex].volume=100;
			animation[animindex].speed=100;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMLOADFAILED);

		char mdebug[1024];
		sprintf(mdebug, "LoadAnimation: %s", pFilename);
		timestampactivity(0, mdebug);

	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void DeleteAnimation( int animindex )
{
	if( Anim[animindex].videodelayedload == 1 && Anim[animindex].loaded == false) return;

	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_FreeAnimation(animindex);
			animation[animindex].active=false;
			if (Anim[animindex].videodelayedload == 1 ) Anim[animindex].loaded = false;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 0, 0, 0, 0, 0, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void StopAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if (Anim[animindex].videodelayedload == 1 && Anim[animindex].loaded == false ) return;
			if(DB_StopAnimation(animindex))
			{
				animation[animindex].playing=false;
				animation[animindex].looped=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMNOTPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimation( int animindex, int x1, int y1, int x2, int y2 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 1, x1, y1, x2, y2, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlaceAnimation( int animindex, int x1, int y1, int x2, int y2)
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_ResizeAnimation(animindex, x1, y1, x2, y2);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

//
// Command Expressions Functions
//

DARKSDK int AnimationExist( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
			return 1;
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationPlaying( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(animation[animindex].playing==true)
				return 1;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int DB_GetAnimationLength ( int AnimIndex )
{
	REFTIME length = 0;
	//HRESULT hRes = Anim[AnimIndex].pMediaPosition->get_Duration(&length);
	return (int)length;
}

#else

struct
{
#ifndef _DEBUG
	char						AnimFile[256];
	IGraphBuilder*				pGraph;
	IMediaControl*				pMediaControl;
	IMediaEvent*				pEvent;
	IBasicVideo*				pBasicVideo;
	IBasicAudio*				pBasicAudio;
	IMediaPosition*				pMediaPosition;

	// mike - 130808 - media seeking for extra video information
	IMediaSeeking*				pMediaSeeking;

	IDvdGraphBuilder*			pDVDGraph;
	IDvdInfo2*					pDVDInfo;
	IDvdControl2*				pDVDControl;
	IVideoWindow*				pVideo;

	IBaseFilter*				pSplitter;
	IBaseFilter*				pDSound;

	CTextureRenderer*			TextureRenderer;
	GGFORMAT					TextureFormat;
	LPGGTEXTURE					pTexture;
	float						ClipU;
	float						ClipV;

	RECT						WantRect;
	RECT						StreamRect;
	RECT						StreamStore;

	bool						bStreamingNow;
	bool						loop;
	int							x1;
	int							y1;
	int							x2;
	int							y2;

	int							iOutputToImage;
	LPGGTEXTURE			pOutputToTexture;
#endif
} Anim[ANIMATIONMAX];

struct ANIMATIONTYPE
{
	bool						active;
	bool						playing;
	bool						paused;
	bool						looped;
	int							volume;
	int							speed;
};

extern GlobStruct*							g_pGlob;
extern PTR_FuncCreateStr					g_pCreateDeleteStringFunction;
extern LPGGDEVICE							m_pD3D;
DBPRO_GLOBAL bool							g_bDoNotLockTextureAtThisTime = false;
DBPRO_GLOBAL ANIMATIONTYPE					animation [ ANIMATIONMAX ];

DARKSDK void AnimationConstructor ( void )
{
	// Clear Arrays
	ZeroMemory(&Anim, sizeof(Anim));
	ZeroMemory(&animation, sizeof(animation));
}

DARKSDK void AnimationDestructor ( void )
{
	// Free All Animations
	for(int AnimIndex=0; AnimIndex<ANIMATIONMAX; AnimIndex++)
	{
		DB_FreeAnimation(AnimIndex);
		ZeroMemory(&Anim[AnimIndex], sizeof(Anim[AnimIndex]));
		ZeroMemory(&animation[AnimIndex], sizeof(animation[AnimIndex]));
	}
}

DARKSDK void AnimationSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void AnimationPassCoreData( LPVOID pGlobPtr )
{
	g_pGlob = (GlobStruct*)pGlobPtr;
	g_pCreateDeleteStringFunction = g_pGlob->CreateDeleteString;
}

DARKSDK void AnimationRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		AnimationDestructor();
	}
	if(iMode==1)
	{
		AnimationConstructor();
	}
}

DARKSDK void PreventTextureLock ( bool bDoNotLock )
{
	g_bDoNotLockTextureAtThisTime=bDoNotLock;
}

#ifndef _DEBUG
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Texture Renderer"), pUnk, phr)
{
#ifndef _DEBUG
    // Store and AddRef the texture for our use.
    *phr = S_OK;

	// Clear Texture Ptr
	m_pTexture=NULL;

	// Cleat Buffer
	m_dwBitmapSize = 0;
	m_pSampleBitmap = NULL;
#endif
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
#ifndef _DEBUG
	// Free associated texture
	SAFE_RELEASE(m_pTexture);

	// Free Buffer
	SAFE_DELETE(m_pSampleBitmap);
#endif
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
#ifndef _DEBUG
    VIDEOINFO *pvi;
    
    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }
    
    // Only accept RGB24
    pvi = (VIDEOINFO *)pmt->Format();
    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
    {
        hr = S_OK;
    }
#endif  
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr;
	#ifndef _DEBUG
    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();
    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

    // Create the texture that maps to this media type
	#ifdef DX11
	#else
    if( FAILED( hr = D3DXCreateTexture(m_pD3D,
                    m_lVidWidth, m_lVidHeight,
                    1, 0, 
                    GGFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexture ) ) )
    {
        return hr;
    }
    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = m_pTexture->GetLevelDesc( 0, &ddsd ) ) )
	{
        return hr;
    }
    m_TextureFormat = ddsd.Format;
    m_ClipU = (float)((double)m_lVidWidth/(double)ddsd.Width);
    m_ClipV = (float)((double)m_lVidHeight/(double)ddsd.Height);
	if(m_ClipU>1.0f) m_ClipU=1.0f;
	if(m_ClipV>1.0f) m_ClipV=1.0f;
    if (m_TextureFormat != GGFMT_A8R8G8B8 &&
        m_TextureFormat != GGFMT_A1R5G5B5)
	{
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
	#endif
	#endif
    return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
#ifndef _DEBUG
    // Get the video bitmap buffer
    BYTE  *pBmpBuffer=NULL;
    pSample->GetPointer( &pBmpBuffer );

	// Copy Bitmap For Now (cannot go right to texture - might be rendering)
	if(m_pSampleBitmap==NULL)
	{
		m_dwBitmapSize = m_lVidHeight * m_lVidPitch;
		m_pSampleBitmap = new char[m_dwBitmapSize];
	}

	// Copy Bitmap For Now (cannot go right to texture - might be rendering)
	if(pBmpBuffer)
	{
		// Copy animation sample in reverse
		memcpy(m_pSampleBitmap, pBmpBuffer, m_dwBitmapSize);
	}
#endif
    return S_OK;
}

HRESULT CTextureRenderer::CopyBufferToTexture( void )
{
	#ifndef _DEBUG
    BYTE  *pBmpBuffer, *pTxtBuffer;     // Bitmap buffer, texture buffer
    LONG  lTxtPitch;                // Pitch of bitmap, texture
    
	// Must be rendering, so do not lock texture now...
	if(g_bDoNotLockTextureAtThisTime==true)
		return S_OK;

    // Get the video bitmap from buffer (if any)
    pBmpBuffer = (BYTE*)m_pSampleBitmap;
	if(pBmpBuffer==NULL)
		return S_OK;

	// some cards have small textures (less than the video size)
	#ifdef DX11
	#else
    D3DSURFACE_DESC ddsd;
    if ( FAILED( m_pTexture->GetLevelDesc( 0, &ddsd ) ) ) return S_OK;
	int iRealWidth = ddsd.Width;
	int iRealHeight = ddsd.Height;

	// copy only as much video exists
	bool bNeedToStretchToFitTexture=false;
	if(m_lVidWidth>iRealWidth) { bNeedToStretchToFitTexture=true; }
	if(m_lVidHeight>iRealHeight) { bNeedToStretchToFitTexture=true; }
	if(m_lVidWidth<iRealWidth) { iRealWidth=m_lVidWidth; }
	if(m_lVidHeight<iRealHeight) { iRealHeight=m_lVidHeight; }

	// advance to end
	pBmpBuffer += (m_lVidHeight * m_lVidPitch) - m_lVidPitch;
	BYTE* pBmpBufferMain = pBmpBuffer;
    
	// Lock the Texture
    GGLOCKED_RECT d3dlr;
    if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, 0 )))
        return E_FAIL;
    
    // Get the texture buffer & pitch
    pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    lTxtPitch = d3dlr.Pitch;
    
    // Copy the bits    
	if(bNeedToStretchToFitTexture==true)
	{
		float fXBit = (float)m_lVidWidth/(float)iRealWidth;
		float fYBit = (float)m_lVidHeight/(float)iRealHeight;

		if (m_TextureFormat == GGFMT_A8R8G8B8)
		{
			float fY = 0;
			for(int y = 0; y < iRealHeight; y++ )
			{
				float fX = 0;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				pBmpBuffer = (BYTE*)pBmpBufferMain - (int)(fY*m_lVidPitch);
				BYTE* pBmpBufferOld = pBmpBuffer;
				for (int x = 0; x < iRealWidth; x++)
				{
					pTxtBuffer[0] = pBmpBuffer[0];
					pTxtBuffer[1] = pBmpBuffer[1];
					pTxtBuffer[2] = pBmpBuffer[2];
					pTxtBuffer[3] = 0xff;

					pTxtBuffer += 4;
					pBmpBuffer = pBmpBufferOld + (((int)fX)*3);
					fX+=fXBit;
				}
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
				fY+=fYBit;
			}
		}

		if (m_TextureFormat == GGFMT_A1R5G5B5)
		{
			float fY = 0;
			for(int y = 0; y < iRealHeight; y++ )
			{
				float fX = 0;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				pBmpBuffer = (BYTE*)pBmpBufferMain - (int)(fY*m_lVidPitch);
				BYTE* pBmpBufferOld = pBmpBuffer;
				for (int x = 0; x < iRealWidth; x++)
				{
					*(WORD *)pTxtBuffer = (WORD)
						(0x8000 +
						((pBmpBuffer[2] & 0xF8) << 7) +
						((pBmpBuffer[1] & 0xF8) << 2) +
						(pBmpBuffer[0] >> 3));
					pTxtBuffer += 2;
					pBmpBuffer = pBmpBufferOld + (((int)fX)*3);
					fX+=fXBit;
				}
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
				fY+=fYBit;
			}
		}
	}
	else
	{
		// OPTIMIZATION OPPORTUNITY: Use a video and texture
		// format that allows a simpler copy than this one.
		if (m_TextureFormat == GGFMT_A8R8G8B8) {
			for(int y = 0; y < iRealHeight; y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < iRealWidth; x++) {
					pTxtBuffer[0] = pBmpBuffer[0];
					pTxtBuffer[1] = pBmpBuffer[1];
					pTxtBuffer[2] = pBmpBuffer[2];
					pTxtBuffer[3] = 0xff;
					pTxtBuffer += 4;
					pBmpBuffer += 3;
				}
				pBmpBuffer = pBmpBufferOld - m_lVidPitch;
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
			}
		}

		if (m_TextureFormat == GGFMT_A1R5G5B5) {
			for(int y = 0; y < iRealHeight; y++ ) {
				BYTE *pBmpBufferOld = pBmpBuffer;
				BYTE *pTxtBufferOld = pTxtBuffer;   
				for (int x = 0; x < iRealWidth; x++)
				{
					*(WORD *)pTxtBuffer = (WORD)
						(0x8000 +
						((pBmpBuffer[2] & 0xF8) << 7) +
						((pBmpBuffer[1] & 0xF8) << 2) +
						(pBmpBuffer[0] >> 3));
					pTxtBuffer += 2;
					pBmpBuffer += 3;
				}
				pBmpBuffer = pBmpBufferOld - m_lVidPitch;
				pTxtBuffer = pTxtBufferOld + lTxtPitch;
			}
		}
	}

    // Unlock the Texture
    if (FAILED(m_pTexture->UnlockRect(0)))
        return E_FAIL;
	#endif
	#endif

    return S_OK;
}

//
// Internal Animation Functions
//
#endif

DARKSDK BOOL CoreLoadAnimation( int AnimIndex, char* Filename, bool bFromDVD )
{
#ifndef _DEBUG
#ifndef DARKSDK_COMPILE
    HRESULT hr = S_OK;
    CComPtr<IBaseFilter>    pFTR;           // Texture Renderer Filter
    CComPtr<IPin>           pFTRPinIn;      // Texture Renderer Input Pin
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   

	// DVD or Animfile
	if ( bFromDVD )
	{
		// Create an instance of the DVD Graph Builder object.
		HRESULT hr;
		hr = CoCreateInstance(CLSID_DvdGraphBuilder,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IDvdGraphBuilder,
							  reinterpret_cast<void**>(&Anim[AnimIndex].pDVDGraph));

		if(Anim[AnimIndex].pDVDGraph==NULL)
			return FALSE;

		// Build the DVD filter graph.
		AM_DVD_RENDERSTATUS    buildStatus;
		hr = Anim[AnimIndex].pDVDGraph->RenderDvdVideoVolume(NULL, AM_DVD_HWDEC_PREFER, &buildStatus);
		if (FAILED(hr)) // total failure
		{
			// If there is no DVD decoder, give a user-friendly message
			int iErrCode = 0;
			switch ( hr )
			{
				case S_FALSE  : iErrCode = 0;				break;
				case VFW_E_DVD_DECNOTENOUGH : iErrCode = 1;	break;
				case VFW_E_DVD_RENDERFAIL : iErrCode = 2;	break;
			}
			return FALSE;
		}
		if (S_FALSE == hr) // partial failure
		{
			// Carry on..
		}

		// Get the pointers to the DVD Navigator interfaces.
		hr = Anim[AnimIndex].pDVDGraph->GetDvdInterface(IID_IDvdInfo2, reinterpret_cast<void**>(&Anim[AnimIndex].pDVDInfo));
		if(Anim[AnimIndex].pDVDInfo==NULL) return FALSE;
		hr = Anim[AnimIndex].pDVDGraph->GetDvdInterface(IID_IDvdControl2, reinterpret_cast<void**>(&Anim[AnimIndex].pDVDControl));
		if(Anim[AnimIndex].pDVDControl==NULL) return FALSE;

		// Get a pointer to the filter graph manager.
		hr = Anim[AnimIndex].pDVDGraph->GetFiltergraph(&Anim[AnimIndex].pGraph);
		if(Anim[AnimIndex].pGraph==NULL) return FALSE;

		// Use the graph builder pointer again to get the IVideoWindow interface,
		// used to set the window style and message-handling behavior of the video renderer filter.
		hr = Anim[AnimIndex].pDVDGraph->GetDvdInterface(IID_IVideoWindow, reinterpret_cast<void**>(&Anim[AnimIndex].pVideo));

		// set the window inside the main app
		RECT grc;
		GetClientRect(g_pGlob->hWnd, &grc);
		Anim[AnimIndex].pVideo->put_Owner((OAHWND)g_pGlob->hWnd);
		Anim[AnimIndex].pVideo->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
		Anim[AnimIndex].pVideo->SetWindowPosition(0, 0, grc.right, grc.bottom);

		// Special mode which leaves an area of the primary surface alone
		g_pGlob->iNoDrawLeft = 0;
		g_pGlob->iNoDrawTop = 0;
		g_pGlob->iNoDrawRight = grc.right;
		g_pGlob->iNoDrawBottom = grc.bottom;
	}
	else
	{
		// Creates a FilterGraph
		CoCreateInstance(	CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
							IID_IGraphBuilder, (void **)&Anim[AnimIndex].pGraph);

		// Create the Texture Renderer object
		Anim[AnimIndex].TextureRenderer = new CTextureRenderer(NULL, &hr);
		if (FAILED(hr)) return FALSE;

		// Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
		pFTR = Anim[AnimIndex].TextureRenderer;
		if (FAILED(hr = Anim[AnimIndex].pGraph->AddFilter(pFTR, L"TEXTURERENDERER")))
			return FALSE;

		// Construct WideCharacter Filename
		DWORD dwLength = strlen(Filename)+1;
		LPOLESTR pWideStr = (LPOLESTR)new WORD[dwLength];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Filename, -1, pWideStr, dwLength*2);

		// Construct entire graph from file
		hr = Anim[AnimIndex].pGraph->RenderFile(pWideStr, NULL);
		SAFE_DELETE(pWideStr);
		if (FAILED(hr)) return FALSE;
	}

	// Get Media Control and Media Event interfaces
	Anim[AnimIndex].pGraph->QueryInterface(IID_IMediaControl, (void **)&Anim[AnimIndex].pMediaControl);
	Anim[AnimIndex].pGraph->QueryInterface(IID_IMediaEvent, (void **)&Anim[AnimIndex].pEvent);
	Anim[AnimIndex].pGraph->QueryInterface(IID_IBasicVideo, (void **)&Anim[AnimIndex].pBasicVideo);
	Anim[AnimIndex].pGraph->QueryInterface(IID_IBasicAudio, (void **)&Anim[AnimIndex].pBasicAudio);
	Anim[AnimIndex].pGraph->QueryInterface(IID_IMediaPosition, (void **)&Anim[AnimIndex].pMediaPosition);
	Anim[AnimIndex].pGraph->QueryInterface(IID_IMediaSeeking, (void **)&Anim[AnimIndex].pMediaSeeking);
	
	if(Anim[AnimIndex].pMediaControl && Anim[AnimIndex].pEvent)
	{
		// Get Texture Of Video
		long lWidth, lHeight;
		if(Anim[AnimIndex].TextureRenderer)
		{
			Anim[AnimIndex].TextureFormat = Anim[AnimIndex].TextureRenderer->m_TextureFormat;
			Anim[AnimIndex].pTexture = Anim[AnimIndex].TextureRenderer->m_pTexture;
			Anim[AnimIndex].ClipU = Anim[AnimIndex].TextureRenderer->m_ClipU;
			Anim[AnimIndex].ClipV = Anim[AnimIndex].TextureRenderer->m_ClipV;

			// Get size of video
			lWidth=Anim[AnimIndex].TextureRenderer->m_lVidWidth;
			lHeight=Anim[AnimIndex].TextureRenderer->m_lVidHeight;
		}
		else
		{
			// No texture with this mode
			Anim[AnimIndex].TextureFormat = GGFMT_R8G8B8;
			Anim[AnimIndex].pTexture = NULL;
			Anim[AnimIndex].ClipU = 0.0f;
			Anim[AnimIndex].ClipV = 0.0f;

			// Get width and height of video
			RECT grc;
			GetClientRect(g_pGlob->hWnd, &grc);
			lWidth=grc.right;
			lHeight=grc.bottom;
		}

		// Get Size of Animation
		Anim[AnimIndex].StreamRect.top=0;
		Anim[AnimIndex].StreamRect.left=0;
		Anim[AnimIndex].StreamRect.right=lWidth;
		Anim[AnimIndex].StreamRect.bottom=lHeight;

		// Complete
		return TRUE;
	}

	#endif
	#endif

	return FALSE;
}

DARKSDK BOOL DB_LoadAnimationCore(int AnimIndex, char* Filename)
{
#ifndef _DEBUG
	// Attempt to load AVI
	if (CoreLoadAnimation(AnimIndex, Filename, false))
	{
		// File loaded, store name for re-play
		strcpy(Anim[AnimIndex].AnimFile, Filename);

		// Use default anim size
		Anim[AnimIndex].x1 = 0;
		Anim[AnimIndex].y1 = 0;
		Anim[AnimIndex].x2 = 0;
		Anim[AnimIndex].y2 = 0;
		Anim[AnimIndex].WantRect.left = 0;
		Anim[AnimIndex].WantRect.top = 0;
		Anim[AnimIndex].WantRect.right = 0;
		Anim[AnimIndex].WantRect.bottom = 0;
		Anim[AnimIndex].bStreamingNow = false;

		// Complete
		return TRUE;
	}
	else
#endif
		return FALSE;
}

DARKSDK BOOL DB_LoadAnimation(int AnimIndex, char* Filename)
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, Filename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename);
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
	return bRes;
}

DARKSDK int DB_GetAnimationLength ( int AnimIndex )
{
#ifndef _DEBUG
	REFTIME length;
	HRESULT hRes = Anim[AnimIndex].pMediaPosition->get_Duration(&length);
	return (int)length;
#else
	return 0;
#endif
}

DARKSDK BOOL DB_LoadDVDAnimation(int AnimIndex)
{
#ifndef _DEBUG
	// Attempt to load AVI
	if(CoreLoadAnimation(AnimIndex, NULL, true))
	{
		// Use default anim size
		Anim[AnimIndex].x1 = 0;
		Anim[AnimIndex].y1 = 0;
		Anim[AnimIndex].x2 = 0;
		Anim[AnimIndex].y2 = 0;
		Anim[AnimIndex].WantRect.left = 0;
		Anim[AnimIndex].WantRect.top = 0;
		Anim[AnimIndex].WantRect.right = 0;
		Anim[AnimIndex].WantRect.bottom = 0;
		Anim[AnimIndex].bStreamingNow=false;

		// Complete
		return TRUE;
	}
	else
#endif
		return FALSE;
}

DARKSDK BOOL DB_FreeAnimation(int AnimIndex)
{
#ifndef _DEBUG
	// Shut down the graph
	if(Anim[AnimIndex].pMediaControl)
        Anim[AnimIndex].pMediaControl->Stop();

	// Restore Window if used
	if(Anim[AnimIndex].pVideo)
	{
		Anim[AnimIndex].pVideo->put_Visible(OAFALSE);
		Anim[AnimIndex].pVideo->put_Owner(NULL);   

		// restore special nodraw mode
		g_pGlob->iNoDrawLeft = 0;
		g_pGlob->iNoDrawTop = 0;
		g_pGlob->iNoDrawRight = 0;
		g_pGlob->iNoDrawBottom = 0;
	}
	
	// Release all interfaces
    SAFE_RELEASE(Anim[AnimIndex].pMediaControl);
	SAFE_RELEASE(Anim[AnimIndex].pEvent);
	SAFE_RELEASE(Anim[AnimIndex].pMediaPosition);
	SAFE_RELEASE(Anim[AnimIndex].pBasicAudio);
	SAFE_RELEASE(Anim[AnimIndex].pBasicVideo);
	SAFE_RELEASE(Anim[AnimIndex].pGraph);

	SAFE_RELEASE(Anim[AnimIndex].pSplitter);
	SAFE_RELEASE(Anim[AnimIndex].pDSound);

	// Release DVD specific interfaces
	SAFE_RELEASE(Anim[AnimIndex].pDVDGraph);
	SAFE_RELEASE(Anim[AnimIndex].pDVDInfo);
	SAFE_RELEASE(Anim[AnimIndex].pDVDControl);
	SAFE_RELEASE(Anim[AnimIndex].pVideo);
    SAFE_RELEASE(Anim[AnimIndex].pMediaSeeking);
    Anim[AnimIndex].pTexture = NULL;
    Anim[AnimIndex].pOutputToTexture = NULL;

	// Complete
#endif
	return TRUE;
}

DARKSDK void ClipAnimationPlace(int AnimIndex)
{
#ifndef _DEBUG
	int x = Anim[AnimIndex].WantRect.left;
	int y = Anim[AnimIndex].WantRect.top;
	int width = Anim[AnimIndex].WantRect.right - x;
	int height = Anim[AnimIndex].WantRect.bottom - y;

	// destination area on screen
	RECT drect;
	drect.left=x;
	drect.top=y;
	drect.right=drect.left+width;
	drect.bottom=drect.top+height;

	// No forced clipping to client area (maybe should be surface area)
	RECT clip;
	GetClientRect(g_pGlob->hWnd, &clip);

	// stretchfactor
	float actualwidth = (float)Anim[AnimIndex].StreamStore.right - Anim[AnimIndex].StreamStore.left;
	float actualheight = (float)Anim[AnimIndex].StreamStore.bottom - Anim[AnimIndex].StreamStore.top;
	float blitwidth = (float)width;
	float blitheight = (float)height;

	// clipped (source clipped also)
	Anim[AnimIndex].StreamRect = Anim[AnimIndex].StreamStore;
	if(drect.left<clip.left)
	{
		int inward = (int)((actualwidth/blitwidth) * abs(drect.left-clip.left));
		Anim[AnimIndex].StreamRect.left+=inward;
		drect.left=clip.left;
	}
	if(drect.top<clip.top)
	{
		int inward = (int)((actualheight/blitheight) * abs(drect.top-clip.top));
		Anim[AnimIndex].StreamRect.top+=inward;
		drect.top=clip.top;
	}
	if(drect.right>clip.right)
	{
		int inward = (int)((actualwidth/blitwidth) * (drect.right-clip.right));
		Anim[AnimIndex].StreamRect.right-=inward;
		drect.right=clip.right;
	}
	if(drect.bottom>clip.bottom)
	{
		int inward = (int)((actualheight/blitheight) * (drect.bottom-clip.bottom));
		Anim[AnimIndex].StreamRect.bottom-=inward;
		drect.bottom=clip.bottom;
	}

	// Re-assign dest rect for streaming
	Anim[AnimIndex].WantRect = drect;
#endif
}

DARKSDK BOOL RestartAnimation(int AnimIndex)
{
#ifndef _DEBUG
	if(Anim[AnimIndex].pMediaControl)
	{
		Anim[AnimIndex].pMediaControl->Stop();
		Anim[AnimIndex].pMediaControl->Run();
		return TRUE;
	}
	else
#endif
		return FALSE;
}

DARKSDK BOOL DB_PlayAnimationToScreen(int AnimIndex, int set, int x, int y, int x2, int y2, bool bPlayFromScratch)
{
#ifndef _DEBUG
	// Set new animation placements if so
	if(set==1)
	{
		Anim[AnimIndex].x1 = x;
		Anim[AnimIndex].y1 = y;
		Anim[AnimIndex].x2 = x2;
		Anim[AnimIndex].y2 = y2;
	}
	else
	{
		x = Anim[AnimIndex].x1;
		y = Anim[AnimIndex].y1;
		x2 = Anim[AnimIndex].x2;
		y2 = Anim[AnimIndex].y2;
	}

	// Must have animation to play
	if(Anim[AnimIndex].pMediaControl==NULL)
		return FALSE;

	// Output Size
	if(x2==0 && y2==0)
	{
		Anim[AnimIndex].WantRect.left	= x;
		Anim[AnimIndex].WantRect.top	= y;
		Anim[AnimIndex].WantRect.right	= x + Anim[AnimIndex].StreamRect.right;
		Anim[AnimIndex].WantRect.bottom	= y + Anim[AnimIndex].StreamRect.bottom;
	}
	else
	{
		Anim[AnimIndex].WantRect.left	= x;
		Anim[AnimIndex].WantRect.top	= y;
		Anim[AnimIndex].WantRect.right	= x2;
		Anim[AnimIndex].WantRect.bottom	= y2;
	}

	// Set State to Run
	Anim[AnimIndex].bStreamingNow=true;
	Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
	Anim[AnimIndex].pMediaControl->Run();

	// Complete
#endif
	return TRUE;
}

__int64 dbGetAnimationPosition ( int AnimIndex )
{
	__int64 currentPosition = 0;
#ifndef _DEBUG	
	if ( Anim[AnimIndex].pMediaSeeking )
		Anim[AnimIndex].pMediaSeeking->GetCurrentPosition ( &currentPosition );
#endif

	return currentPosition;
}

__int64 dbGetAnimationStopPosition ( int AnimIndex )
{
	__int64 stopPosition = 0;
#ifndef _DEBUG
	if ( Anim[AnimIndex].pMediaSeeking )
		Anim[AnimIndex].pMediaSeeking->GetStopPosition ( &stopPosition );
#endif
	return stopPosition;
}

#ifndef _DEBUG
IMediaSeeking* dbGetAnimationMediaSeeking ( int AnimIndex )
{
	if ( Anim[AnimIndex].pMediaSeeking )
		return Anim[AnimIndex].pMediaSeeking;

	return NULL;
}
#endif

DARKSDK BOOL DB_PlayAnimationToImage(int iImageIndex, int AnimIndex, int set, int x, int y, int x2, int y2, bool bPlayFromScratch)
{
	// Regular full play animation
	BOOL bResult = DB_PlayAnimationToScreen(AnimIndex, set, x, y, x2, y2, bPlayFromScratch);
#ifndef _DEBUG

	// Switch output to image
	Anim[AnimIndex].iOutputToImage = iImageIndex;
	if ( 1 )
	{
		// If region exceeds image, delete image
		if ( ImageExist( iImageIndex ) )
		{
			if ( x2>ImageWidth(iImageIndex) || y2>ImageHeight(iImageIndex) )
			{
					DeleteImage ( iImageIndex );
			}
		}

		if ( ImageExist( iImageIndex )==false )
		{
			// Create new image 
			Anim[AnimIndex].pOutputToTexture = MakeImageUsage ( iImageIndex, x2, y2, 0 );
		}
		else
		{
			// Get existing image texture
			Anim[AnimIndex].pOutputToTexture = GetImagePointer ( iImageIndex );
		}
	}
#endif
	// Return result
	return bResult;
}

DARKSDK BOOL DB_ResizeAnimation(int AnimIndex, int x1, int y1, int x2, int y2)
{
#ifndef _DEBUG
	Anim[AnimIndex].x1 = x1;
	Anim[AnimIndex].y1 = y1;
	Anim[AnimIndex].x2 = x2;
	Anim[AnimIndex].y2 = y2;

	Anim[AnimIndex].WantRect.left	= x1;
	Anim[AnimIndex].WantRect.top	= y1;
	Anim[AnimIndex].WantRect.right	= x2;
	Anim[AnimIndex].WantRect.bottom	= y2;
#endif
	return TRUE;
}

DARKSDK BOOL DB_GetAnimationArea(int AnimIndex, int* x, int* y, int* width, int* height)
{
#ifndef _DEBUG
	if(Anim[AnimIndex].WantRect.right>0)
	{
		*x = Anim[AnimIndex].WantRect.left;
		*y = Anim[AnimIndex].WantRect.top;
		*width = (Anim[AnimIndex].WantRect.right-Anim[AnimIndex].WantRect.left);
		*height = (Anim[AnimIndex].WantRect.bottom-Anim[AnimIndex].WantRect.top);
	}
	else
	{
		*x = 0;
		*y = 0;
		*width = Anim[AnimIndex].StreamRect.right;
		*height = Anim[AnimIndex].StreamRect.bottom;
	}
#endif
	return TRUE;
}

DARKSDK void UpdateAllAnimation(void)
{
#ifndef _DEBUG
	// Temp Var
	long evCode;
	long lParam1;
	long lParam2;

	// Free All Animations
	for(int AnimIndex=0; AnimIndex<ANIMATIONMAX; AnimIndex++)
	{
		if(Anim[AnimIndex].pMediaControl)
		{
			// Refresh texture with latest from buffer
			if(Anim[AnimIndex].pVideo==NULL)
			{
				Anim[AnimIndex].TextureRenderer->CopyBufferToTexture();
			}

			// Handle Animations that loop
			if(Anim[AnimIndex].bStreamingNow==true)
			{
				Anim[AnimIndex].pEvent->GetEvent(&evCode, &lParam1, &lParam2, 0);
				if(Anim[AnimIndex].loop==true)
				{
					// If Looping, repeat run when it ends
					if(evCode==EC_COMPLETE)
					{
						if(Anim[AnimIndex].pMediaPosition)
						{
							Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
							Anim[AnimIndex].pEvent->FreeEventParams(evCode, lParam1, lParam2);
						}
					}
				}
				else
				{
					if(evCode==EC_COMPLETE)
					{
						animation[AnimIndex].playing=false;
					}
				}
			}

			// To window or texture
			if(Anim[AnimIndex].pVideo)
			{
				// Control window size from wanted rect
				DWORD x=Anim[AnimIndex].WantRect.left;
				DWORD y=Anim[AnimIndex].WantRect.top;
				DWORD width=Anim[AnimIndex].WantRect.right-Anim[AnimIndex].WantRect.left;
				DWORD height=Anim[AnimIndex].WantRect.bottom-Anim[AnimIndex].WantRect.top;

				// Special mode which leaves an area of the primary surface alone
				g_pGlob->iNoDrawLeft = x;
				g_pGlob->iNoDrawTop = y;
				g_pGlob->iNoDrawRight = x+width;
				g_pGlob->iNoDrawBottom = y+height;

				// work out adjustment based on window size to screensize
				RECT grc;
				GetClientRect(g_pGlob->hWnd, &grc);
				float fX = (float)grc.right / (float)g_pGlob->iScreenWidth;
				float fY = (float)grc.bottom / (float)g_pGlob->iScreenHeight;

				// adjust final ccordinates
				x = (int)(x * fX);
				y = (int)(y * fY);
				width = (int)(width * fX);
				height = (int)(height * fY);

				// set window position and size
				Anim[AnimIndex].pVideo->SetWindowPosition(x,y,width,height);
			}
			else
			{
				// To image or current render target
				if(Anim[AnimIndex].iOutputToImage==0)
				{
					// Update sample in backbuffer (reverse Y as anim data is reversed)
					float myClipV = Anim[AnimIndex].ClipV;
					PasteTextureToRect( Anim[AnimIndex].pTexture, Anim[AnimIndex].ClipU, myClipV, Anim[AnimIndex].WantRect );
				}
				else
				{
					// Get Size of texture
					D3DSURFACE_DESC destdesc;
					Anim[AnimIndex].pOutputToTexture->GetLevelDesc( 0, &destdesc );
					DWORD dwDescBPP = GetBitDepthFromFormat ( destdesc.Format ) / 8;

					// Get source size
					D3DSURFACE_DESC srcdesc;
					Anim[AnimIndex].pTexture->GetLevelDesc( 0, &srcdesc );
					DWORD dwSrcBPP = GetBitDepthFromFormat ( srcdesc.Format ) / 8;

					// Get region to draw animation to
					RECT RegionRect = Anim[AnimIndex].WantRect;

					// Work out texture/image ratio
					int tImageWidth = ImageWidth ( Anim[AnimIndex].iOutputToImage );
					int tImageHeight = ImageHeight ( Anim[AnimIndex].iOutputToImage );
					float ratioX = (float)destdesc.Width / (float)tImageWidth;
					float ratioY = (float)destdesc.Height / (float)tImageHeight;

					// Scale region to any texture stretching
					RegionRect.top = (int)((float)RegionRect.top * ratioY);
					RegionRect.bottom = (int)((float)RegionRect.bottom * ratioY);
					RegionRect.left = (int)((float)RegionRect.left * ratioX);
					RegionRect.right = (int)((float)RegionRect.right * ratioX);

					// Final Region dimension
					DWORD RegionWidth = RegionRect.right - RegionRect.left;
					DWORD RegionHeight = RegionRect.bottom - RegionRect.top;

					// Sort out anim source
					GGLOCKED_RECT animd3dlr;
					Anim[AnimIndex].pTexture->LockRect( 0, &animd3dlr, NULL, D3DLOCK_READONLY );

					// if image texutre smaller than dest area ,reduce dest area
					DWORD dwWidth = Anim[AnimIndex].StreamRect.right;
					DWORD dwHeight = Anim[AnimIndex].StreamRect.bottom;
					if(destdesc.Width<dwWidth) dwWidth=destdesc.Width;
					if(destdesc.Height<dwHeight) dwHeight=destdesc.Height;

					float fAnimX=0.0f;
					float fAnimY=0.0f;
					float fXBit=(float)Anim[AnimIndex].StreamRect.right/(float)RegionWidth;
					float fYBit=(float)Anim[AnimIndex].StreamRect.bottom/(float)RegionHeight;

					// Stretch copy into image
					GGLOCKED_RECT grafixlr;
					Anim[AnimIndex].pOutputToTexture->LockRect( 0, &grafixlr, NULL, D3DLOCK_NOSYSLOCK );
					if ( grafixlr.pBits )
					{
						for ( int iY=RegionRect.top; iY<RegionRect.bottom; iY++)
						{
							int yadd = (int)iY*grafixlr.Pitch;
							int animyadd = (int)fAnimY*animd3dlr.Pitch;
							for ( int iX=RegionRect.left; iX<(int)RegionRect.right; iX++)
							{
								// Get source pixel and write to dest
								LPSTR pRead = (LPSTR)animd3dlr.pBits+(int)fAnimX*dwSrcBPP+(animyadd);

								if ( dwDescBPP==2 )
								{
									DWORD dwPxl = *(WORD*)pRead;
									if(dwSrcBPP==4)
									{
										// convert 8888 to 1555
										dwPxl = *(DWORD*)pRead;
										int red =	(int)(((dwPxl & (255<<16)) >> 16) / 8.3);
										int green = (int)(((dwPxl & (255<<8) ) >> 8)  / 8.3);
										int blue =	(int)(( dwPxl &  255     )        / 8.3);
										if(red>31) red=31;
										if(green>31) green=31;
										if(blue>31) blue=31;
										dwPxl = (1<<15)+(red<<10)+(green<<5)+(blue);
									}
									LPSTR pWrite = (LPSTR)grafixlr.pBits+(int)iX*dwDescBPP+(yadd);
									*(WORD*)pWrite = (WORD)dwPxl;
								}
								if ( dwDescBPP==4 )
								{
									DWORD dwPxl = *(DWORD*)pRead;
									LPSTR pWrite = (LPSTR)grafixlr.pBits+(int)iX*dwDescBPP+(yadd);
									*(DWORD*)pWrite = dwPxl;
								}

								// Advance source vector
								fAnimX += fXBit;
							}

							// Advance source vector
							fAnimY += fYBit;
							fAnimX = 0.0f;
						}
						Anim[AnimIndex].pOutputToTexture->UnlockRect(0);
						Anim[AnimIndex].pTexture->UnlockRect(0);
					}
				}
			}
		}
	}
#endif
}

DARKSDK BOOL DB_StopAnimation(int AnimIndex)
{
#ifndef _DEBUG
	// If currently not playing this animation, nothing to stop
	if(Anim[AnimIndex].pMediaControl==NULL)
		return FALSE;

	// stop animation
	Anim[AnimIndex].bStreamingNow=false;
	Anim[AnimIndex].pMediaControl->Stop();
#endif
	// Complete
	return TRUE;
}

DARKSDK BOOL DB_PauseAnimation(int AnimIndex)
{
#ifndef _DEBUG
	// If cannot pause
	if(Anim[AnimIndex].pMediaControl==NULL)
		return FALSE;

	// pause anim
	Anim[AnimIndex].pMediaControl->Pause();

	// Complete
#endif
	return TRUE;
}

DARKSDK BOOL DB_ResumeAnimation(int AnimIndex)
{
#ifndef _DEBUG
	// If cannot resume
	if(Anim[AnimIndex].pMediaControl==NULL)
		return FALSE;

	// resume anim
	Anim[AnimIndex].pMediaControl->Run();
	
	// Complete
#endif
	return TRUE;
}

DARKSDK BOOL DB_LoopAnimationOn(int AnimIndex)
{
#ifndef _DEBUG
	Anim[AnimIndex].loop=true;
#endif
	return TRUE;
}

DARKSDK BOOL DB_LoopAnimationOff(int AnimIndex)
{
#ifndef _DEBUG
	Anim[AnimIndex].loop=false;
#endif
	return TRUE;
}

DARKSDK BOOL DB_SetAnimationVolume(int AnimIndex, int iVolume)
{
#ifndef _DEBUG
	if(Anim[AnimIndex].pBasicAudio)
	{
		if(iVolume<0) iVolume=0;
		if(iVolume>100) iVolume=100;
		long lVolume=((100-iVolume)*-100);
		HRESULT Result = Anim[AnimIndex].pBasicAudio->put_Volume(lVolume);
		return SUCCEEDED( Result );
	}
	else
#endif
		return FALSE;
}

DARKSDK BOOL DB_SetAnimationSpeed(int AnimIndex, int iRate)
{
#ifndef _DEBUG
	if(Anim[AnimIndex].pMediaPosition)
	{
		if(iRate<0) iRate=0;
		if(iRate>100) iRate=100;
		double dRate = iRate/100.0f;
		Anim[AnimIndex].pMediaPosition->put_Rate(dRate);
		return TRUE;
	}
	else
#endif
		return FALSE;
}

//
// Commands Functions
//

DARKSDK void LoadAnimation( LPSTR pFilename, int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		// Delete before proceeding to load..
		if(animation[animindex].active==true)
		{
			DB_FreeAnimation(animindex);
			animation[animindex].active=false;
		}

		if(DB_LoadAnimation(animindex, pFilename))
		{
			animation[animindex].active=true;
			animation[animindex].playing=false;
			animation[animindex].paused=false;
			animation[animindex].looped=false;
			animation[animindex].volume=100;
			animation[animindex].speed=100;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMLOADFAILED);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK int GetAnimationLength ( int animindex )
{
	int iLengthInSeconds = 0;
	if(animindex>=1 && animindex<ANIMATIONMAX)
		if(animation[animindex].active==true)
			iLengthInSeconds = DB_GetAnimationLength(animindex);

	return iLengthInSeconds;
}

DARKSDK void DeleteAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_FreeAnimation(animindex);
			animation[animindex].active=false;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 0, 0, 0, 0, 0, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void StopAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_StopAnimation(animindex))
			{
				animation[animindex].playing=false;
				animation[animindex].looped=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMNOTPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PauseAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PauseAnimation(animindex))
			{
				animation[animindex].paused=true;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMNOTPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void ResumeAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_ResumeAnimation(animindex))
				animation[animindex].paused=false;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void LoopAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_PlayAnimationToScreen(animindex, 0, 0, 0, 0, 0, false);
			DB_LoopAnimationOn(animindex);
			animation[animindex].looped=true;
			animation[animindex].playing=true;
			animation[animindex].paused=false;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimation( int animindex, int x1, int y1 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 1, x1, y1, 0, 0, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimation( int animindex, int x1, int y1, int x2, int y2 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 1, x1, y1, x2, y2, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimationToBitmap( int animindex, int bitmapindex, int x1, int y1, int x2, int y2 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(DB_PlayAnimationToScreen(animindex, 1, x1, y1, x2, y2, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlayAnimationToImage( int animindex, int imageindex, int x1, int y1, int x2, int y2 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(imageindex<=0 || imageindex>MAXIMUMVALUE)
			{
				RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
				return;
			}

			if(DB_PlayAnimationToImage(imageindex, animindex, 1, x1, y1, x2, y2, true))
			{
				DB_LoopAnimationOff(animindex);
				animation[animindex].playing=true;
				animation[animindex].looped=false;
				animation[animindex].paused=false;
			}
			else
				RunTimeError(RUNTIMEERROR_ANIMALREADYPLAYING);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void LoopAnimationToBitmap( int animindex, int bitmapindex, int x1, int y1, int x2, int y2 )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_PlayAnimationToScreen(animindex, 1, x1, y1, x2, y2, false);
			DB_LoopAnimationOn(animindex);
			animation[animindex].looped=true;
			animation[animindex].playing=true;
			animation[animindex].paused=false;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void PlaceAnimation( int animindex, int x1, int y1, int x2, int y2)
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			DB_ResizeAnimation(animindex, x1, y1, x2, y2);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

//
// Command Expressions Functions
//

DARKSDK int AnimationExist( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
			return 1;
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationPlaying( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(animation[animindex].playing==true)
				return 1;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationPaused( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(animation[animindex].paused==true)
				return 1;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationLooping( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if(animation[animindex].looped==true)
				return 1;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationPositionX( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			int x, y, width, height;
			DB_GetAnimationArea(animindex, &x, &y, &width, &height);
			return x;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationPositionY( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			int x, y, width, height;
			DB_GetAnimationArea(animindex, &x, &y, &width, &height);
			return y;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationWidth( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			int x, y, width, height;
			DB_GetAnimationArea(animindex, &x, &y, &width, &height);
			return width;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationHeight( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			int x, y, width, height;
			DB_GetAnimationArea(animindex, &x, &y, &width, &height);
			return height;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

//
// new commands
//

DARKSDK void LoadDVDAnimation( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		// Delete before proceeding to load..
		if(animation[animindex].active==true)
		{
			DB_FreeAnimation(animindex);
			animation[animindex].active=false;
		}

		if(DB_LoadDVDAnimation(animindex))
		{
			animation[animindex].active=true;
			animation[animindex].playing=false;
			animation[animindex].paused=false;
			animation[animindex].looped=false;
			animation[animindex].volume=100;
			animation[animindex].speed=100;
		}
		else
		{
			// Silent fail - may not have DVD!
			animation[animindex].active=false;
		}
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void SetDVDChapter( int animindex, int iTitle, int iChapterNumber )
{
#ifndef _DEBUG
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if ( iTitle>=1 && iTitle<=99 )
			{
				if ( iChapterNumber>=1 && iChapterNumber<=999 )
				{
					if ( Anim[animindex].pDVDControl )
					{
						HRESULT hRes;
						// lee - 2103060 u6b4 - this seems to work, before it was a hit and miss affair!
						for ( int twiceseemstowork=0; twiceseemstowork<2; twiceseemstowork++ )
						{
							hRes = Anim[animindex].pDVDControl->Stop();
							hRes = Anim[animindex].pDVDControl->PlayChapterInTitle(
												(ULONG)iTitle,
												(ULONG)iChapterNumber,
												DVD_CMD_FLAG_Block,
												NULL );
						}
					}
				}
			}
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
#endif
}

DARKSDK void SetAnimationVolume( int animindex, int ivolume )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			animation[animindex].volume=ivolume;
			DB_SetAnimationVolume(animindex, ivolume);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

DARKSDK void SetAnimationSpeed( int animindex, int ispeed )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			animation[animindex].speed=ispeed;
			DB_SetAnimationSpeed(animindex, ispeed);
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
}

// new expresisons

DARKSDK int AnimationVolume( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			return animation[animindex].volume;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int AnimationSpeed( int animindex )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			return animation[animindex].speed;
		}
		else
			RunTimeError(RUNTIMEERROR_ANIMNOTEXIST);
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);

	return 0;
}

DARKSDK int TotalDVDChapters( int animindex, int iTitle )
{
#ifndef _DEBUG
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		if(animation[animindex].active==true)
		{
			if ( iTitle>=1 && iTitle<=99 )
			{
				if ( Anim[animindex].pDVDInfo )
				{
					ULONG iChapters=0;
					Anim[animindex].pDVDInfo->GetNumberOfChapters ( (ULONG)iTitle, &iChapters );
					return iChapters;
				}
				else
				{
					// Signals no DVD present
					return -1;
				}
			}
		}
		else
		{
			// Signals no DVD present
			return -1;
		}
	}
	else
		RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
#endif
	return 0;
}

#endif
