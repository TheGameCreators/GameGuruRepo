//
// Video Animation Code
//

// Includes
#include "..\..\..\..\GameGuru\Include\preprocessor-flags.h"

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
#include "CObjectsC.h"

// Theoraplayer for Classic, Windows Media Foundation for Windows 10+
#define WMFVIDEO
#ifdef WMFVIDEO
 #define INITGUID
 #include <Guiddef.h>
 #include <mfapi.h>
 #include <mfidl.h>
 #include <Shlwapi.h>
 #include <strmif.h>
 #include "qedit.h"
 #pragma comment(lib,"Strmiids.lib")
#else
 #include <clipffmpeg/clipffmpeg.h>
 #include <theoraplayer/FrameQueue.h>
 #include <theoraplayer/Manager.h>
 #include <theoraplayer/MemoryDataSource.h>
 #include <theoraplayer/theoraplayer.h>
 #include <theoraplayer/VideoClip.h>
 #include <theoraplayer/VideoFrame.h>
#endif

// Structures
#define ANIMATIONMAX 33
struct
{
	char						AnimFile[256];
	#ifdef WMFVIDEO
	 int						pMediaClip;
	#else
	 theoraplayer::VideoClip*	pMediaClip;
	#endif
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

#ifdef WMFVIDEO
///IVideoWindow* m_pVWMoniker = 0;
///IMediaControl* m_pMCMoniker = 0;
ICaptureGraphBuilder2* m_pCaptureMoniker = 0; 
IFilterGraph2* m_pFilterGraph = 0;
HWND g_hVideoButtonOK = 0;
HWND g_hVideoButtonCancel = 0;
bool g_bIsFullscreen = false;
bool g_bMFPlatExists = false;
IMFMediaSession *g_pVideoSession = NULL;
IMFMediaSource *g_pVideoSource = NULL;
IMFAudioStreamVolume *g_pVideoVolume = NULL;
IMFPresentationClock *g_pVideoClock = NULL;
int g_iVideoImageID = 0;
BOOL g_bVideoPlaying = FALSE;
char sVideoPath[MAX_PATH];
float fVideoDuration = 0;
int iVideoWidth = -1;
int iVideoHeight = -1;
float fVideoPos = -1;
float fVideoVolume = 100.0f;
volatile unsigned char *pVideoFrame = 0;
//cSpinLock cVideoLock;
volatile int iVideoChanged = 0;
class SampleHandler : public ISampleGrabberCB
{
	public:
		SampleHandler() {}
		virtual ~SampleHandler() {}

		STDMETHOD(QueryInterface)(REFIID InterfaceIdentifier, VOID** ppvObject) throw() { return E_NOINTERFACE; } 
		STDMETHOD_(ULONG, AddRef)() throw() { return 2; } 
		STDMETHOD_(ULONG, Release)() throw() { return 1; } 

		STDMETHODIMP SampleCB( double SampleTime, IMediaSample *pSample ) { return S_OK; }

		STDMETHODIMP BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) 
		{
			if ( BufferLen == 0 ) return S_OK;
			if ( !pVideoFrame ) return S_OK;

			//cAutoSLock autolock( &cVideoLock );
			memcpy( (void*)pVideoFrame, pBuffer, BufferLen );
			iVideoChanged = 1;

			return S_OK;
		}
};
SampleHandler *pVideoCallback = 0;
// Media Foundation sample grabber
class SampleGrabberCB : public IMFSampleGrabberSinkCallback 
{
	long m_cRef;

	SampleGrabberCB() : m_cRef(1) {}

	public:
		static HRESULT CreateInstance(SampleGrabberCB **ppCB) { *ppCB = new SampleGrabberCB(); return S_OK; }

		// IUnknown methods
		STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
		{
			static const QITAB qit[] = 
			{
				QITABENT(SampleGrabberCB, IMFSampleGrabberSinkCallback),
				QITABENT(SampleGrabberCB, IMFClockStateSink),
				{ 0 }
			};
			return QISearch(this, qit, iid, ppv);
		}
		STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
		STDMETHODIMP_(ULONG) Release()
		{
			ULONG cRef = InterlockedDecrement(&m_cRef);
			if (cRef == 0) delete this;
			return cRef;
		}

		// IMFClockStateSink methods
		STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset) { return S_OK; }
		STDMETHODIMP OnClockStop(MFTIME hnsSystemTime) { return S_OK; }
		STDMETHODIMP OnClockPause(MFTIME hnsSystemTime) { return S_OK; }
		STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime) { return S_OK; }
		STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate) { return S_OK; }

		// IMFSampleGrabberSinkCallback methods
		STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock) { return S_OK; }
		STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
			LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
			DWORD dwSampleSize)
		{
			if ( dwSampleSize == 0 ) return S_OK;
			if ( !pVideoFrame ) return S_OK;

			//cAutoSLock autolock( &cVideoLock );
			memcpy( (void*)pVideoFrame, pSampleBuffer, dwSampleSize );
			iVideoChanged = 1;

			return S_OK;
		}
		STDMETHODIMP OnShutdown() { return S_OK; }
};
SampleGrabberCB *g_pVideoSampleGrabber = 0;
ISampleGrabber *pVideoSampleGrabber = 0; // yep, pretty confusing!

IGraphBuilder *g_pVideoGraphBuilder;
IMediaControl *g_pVideoMediaControl = 0;
IMediaSeeking *g_pVideoMediaSeeking = 0;
IMediaEvent *g_pVideoMediaEventEx = 0;
IBasicAudio *g_pVideoBasicAudio = 0;
IBaseFilter *g_pVideoGrabberF = 0;
IBaseFilter *g_pVideoNullF = 0;

#else
theoraplayer::OutputMode theoraOutputMode = theoraplayer::FORMAT_UNDEFINED;
#endif

// Globals
float fLastTimeVideoManagerUpdated = 0.0f;

// Externals
extern float					timeGetSecond(void);
extern int						GetBitDepthFromFormat(GGFORMAT Format);
extern GlobStruct*				g_pGlob;
extern LPGG						m_pDX;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;

#ifdef DX11

// Utility Functions

HRESULT IsPinConnected(IPin *pPin, BOOL *pResult)
{
	IPin *pTmp = NULL;
	HRESULT hr = pPin->ConnectedTo(&pTmp);
	if (SUCCEEDED(hr))
	{
		*pResult = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		// The pin is not connected. This is not an error for our purposes.
		*pResult = FALSE;
		hr = S_OK;
	}

	SAFE_RELEASE(pTmp);
	return hr;
}

HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
	PIN_DIRECTION pinDir;
	HRESULT hr = pPin->QueryDirection(&pinDir);
	if (SUCCEEDED(hr))
	{
		*pResult = (pinDir == dir);
	}
	return hr;
}

HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, BOOL bShouldBeConnected, BOOL *pResult)
{
	BOOL bMatch = FALSE;
	BOOL bIsConnected = FALSE;

	HRESULT hr = IsPinConnected(pPin, &bIsConnected);
	if (SUCCEEDED(hr))
	{
		if (bIsConnected == bShouldBeConnected)
		{
			hr = IsPinDirection(pPin, direction, &bMatch);
		}
	}

	if (SUCCEEDED(hr))
	{
		*pResult = bMatch;
	}
	return hr;
}

HRESULT FindUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	IEnumPins *pEnum = NULL;
	IPin *pPin = NULL;
	BOOL bFound = FALSE;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pPin);
		SAFE_RELEASE(pEnum);
		return hr;
	}

	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = MatchPin(pPin, PinDir, FALSE, &bFound);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pPin);
			SAFE_RELEASE(pEnum);
			return hr;
		}
		if (bFound)
		{
			*ppPin = pPin;
			(*ppPin)->AddRef();
			break;
		}
		SAFE_RELEASE(pPin);
	}

	if (!bFound)
	{
		hr = VFW_E_NOT_FOUND;
	}

	SAFE_RELEASE(pPin);
	SAFE_RELEASE(pEnum);
	return hr;
}

HRESULT ConnectFilters( IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest )
{
	IPin *pIn = NULL;
	        
	// Find an input pin on the downstream filter.
	HRESULT hr = FindUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pIn->Release();
	}
	return hr;
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
	IPin *pOut = NULL;

	// Find an output pin on the first filter.
	HRESULT hr = FindUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (SUCCEEDED(hr))
	{
		hr = ConnectFilters(pGraph, pOut, pDest);
		pOut->Release();
	}
	return hr;
}

HRESULT FindUnconnectedFilterPin(IBaseFilter *pSrc, IPin **pOut)
{
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	pSrc->EnumPins( &pEnum );
	pEnum->Reset();
	PIN_INFO info = {0};

	// first look for an unconnected pin
	while ( pEnum->Next( 1, &pPin, NULL ) == S_OK )
	{
		// only interested in output pins
		pPin->QueryPinInfo( &info );
		if ( info.dir == PINDIR_INPUT ) 
		{
			pPin->Release();
			continue;
		}

		// check if it is connected
		IPin *pOther = 0;
		if ( FAILED(pPin->ConnectedTo( &pOther )) )
		{
			*pOut = pPin;
			pEnum->Release();
			return S_OK;
		}
		else
		{
			pOther->Release();
			pPin->Release();
		}
	}

	pEnum->Reset();

	while ( pEnum->Next( 1, &pPin, NULL ) == S_OK )
	{
		// only interested in output pins
		pPin->QueryPinInfo( &info );
		if ( info.dir == PINDIR_INPUT ) 
		{
			pPin->Release();
			continue;
		}

		// find the next filter and check its pins
		IPin *pOther = 0;
		if ( SUCCEEDED(pPin->ConnectedTo( &pOther )) )
		{
			pOther->QueryPinInfo( &info );
			pOther->Release();
								
			if ( FindUnconnectedFilterPin( info.pFilter, pOut ) == S_OK ) 
			{
				pPin->Release();
				pEnum->Release();
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();

	return VFW_E_NOT_FOUND;
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

#ifdef WMFVIDEO
void StopVideo()
{
	if ( g_bMFPlatExists )
	{
		if ( !g_pVideoSession ) return;
		g_pVideoSession->Stop();
		fVideoPos = 0;
		g_bVideoPlaying = FALSE;
	}
	else
	{
		if ( !g_pVideoMediaControl ) return;
		long long pos = 0;
		HRESULT hr = g_pVideoMediaSeeking->SetPositions( &pos, AM_SEEKING_AbsolutePositioning, 0, AM_SEEKING_NoPositioning );
		g_pVideoMediaControl->Stop();
	}
}

void DeleteVideo()
{
	StopVideo();

	// DirectShow
	if ( g_pVideoMediaControl ) g_pVideoMediaControl->Release();
	if ( g_pVideoMediaSeeking ) g_pVideoMediaSeeking->Release();
	if ( g_pVideoMediaEventEx ) g_pVideoMediaEventEx->Release();
	if ( g_pVideoBasicAudio ) g_pVideoBasicAudio->Release();
	if ( g_pVideoGraphBuilder ) g_pVideoGraphBuilder->Release();
	if ( pVideoSampleGrabber ) pVideoSampleGrabber->Release();
	if ( g_pVideoGrabberF ) g_pVideoGrabberF->Release();
	if ( g_pVideoNullF ) g_pVideoNullF->Release();
	if ( pVideoCallback ) delete pVideoCallback;

	g_pVideoMediaControl = 0;
	g_pVideoMediaSeeking = 0;
	g_pVideoMediaEventEx = 0;
	g_pVideoBasicAudio = 0;
	g_pVideoGraphBuilder = 0;
	pVideoSampleGrabber = 0;
	g_pVideoGrabberF = 0;
	g_pVideoNullF = 0;
	pVideoCallback = 0;

	// Media Foundation
	if ( g_pVideoClock ) g_pVideoClock->Release();
	if ( g_pVideoSampleGrabber ) g_pVideoSampleGrabber->Release();
	if ( g_pVideoSource ) { g_pVideoSource->Shutdown(); g_pVideoSource->Release(); }
	if ( g_pVideoSession ) { g_pVideoSession->Shutdown(); g_pVideoSession->Release(); }
	if ( g_pVideoVolume ) g_pVideoVolume->Release();
	
	g_pVideoClock = 0;
	g_pVideoSession = 0;
	g_pVideoSource = 0;
	g_pVideoSampleGrabber = 0;
	g_pVideoVolume = 0;

	g_iVideoImageID = 0; // deleted by m_pVideoTexture below

	strcpy ( sVideoPath, "");
	fVideoDuration = 0;
	iVideoWidth = -1;
	iVideoHeight = -1;
	fVideoPos = -1;
	fVideoVolume = 100;
	if ( pVideoFrame ) { delete [] pVideoFrame; pVideoFrame = 0; }
	iVideoChanged = 0;
}

int LoadVideo( const char *szFilename )
{
	if ( g_bMFPlatExists )
	{
		if ( g_pVideoSession ) DeleteVideo();

		strcpy ( sVideoPath, szFilename );

		IMFSourceResolver* pSourceResolver = NULL;
		IMFPresentationDescriptor *pPD = NULL;
		IMFMediaTypeHandler *pHandler = NULL;
		DWORD cStreams = 0;
		UINT64 duration;
		BOOL bFound = FALSE;
		BOOL fSelected = FALSE;
		GUID majorType;
		IMFStreamDescriptor *pSD = NULL;
		IMFMediaType *pType2 = NULL;
		UINT32 iWidth, iHeight;

		HRESULT hr = MFCreateSourceResolver(&pSourceResolver);
		if ( FAILED(hr) ) { Error1( "Failed to create source resolver" ); return 0; }

		MF_OBJECT_TYPE ObjectType;
		IUnknown* pUnknownSource = NULL;
		int iLength = MultiByteToWideChar( CP_UTF8, 0, sVideoPath, -1, 0, 0 );
		WCHAR *wstr = new WCHAR[ iLength + 1 ];
		MultiByteToWideChar( CP_UTF8, 0, sVideoPath, -1, wstr, iLength+1 );
		hr = pSourceResolver->CreateObjectFromURL(wstr, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pUnknownSource);
		delete [] wstr;
		if ( FAILED(hr) ) { Error1( "Failed to create object from URL" ); goto failed; }

		hr = pUnknownSource->QueryInterface(IID_PPV_ARGS(&g_pVideoSource));
		pUnknownSource->Release();
		if ( FAILED(hr) ) { Error1( "Failed to query source interface" ); goto failed; }

		hr = g_pVideoSource->CreatePresentationDescriptor(&pPD);
		if ( FAILED(hr) ) { Error1( "Failed to create presentation description" ); goto failed; }
		hr = pPD->GetStreamDescriptorCount(&cStreams);
		if ( FAILED(hr) ) { Error1( "Failed to get stream description count" ); goto failed; }

		hr = pPD->GetUINT64(MF_PD_DURATION, (UINT64*)&duration);
		if ( FAILED(hr) ) { Error1( "Failed to get video duration" ); goto failed; }
		fVideoDuration = duration / 10000000.0f;

		for (DWORD i = 0; i < cStreams; i++)
		{
			hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
			if ( FAILED(hr) ) { Error1( "Failed to get stream descriptor by index" ); goto failed; }
			hr = pSD->GetMediaTypeHandler(&pHandler);
			if ( FAILED(hr) ) { Error1( "Failed to get media type handler" ); goto failed; }
			hr = pHandler->GetMajorType(&majorType);
			if ( FAILED(hr) ) { Error1( "Failed to get media major type" ); goto failed; }

			if (majorType == MFMediaType_Video && fSelected)
			{
				hr = pHandler->GetCurrentMediaType( &pType2 );
				if ( FAILED(hr) ) { Error1( "Failed to get media type" ); goto failed; }

				MFGetAttributeSize( pType2, MF_MT_FRAME_SIZE, &iWidth, &iHeight );
				iVideoWidth = iWidth;
				iVideoHeight = iHeight;

				SAFE_RELEASE( pType2 )
				bFound = TRUE;
				break;
			}

			SAFE_RELEASE( pSD )
			SAFE_RELEASE( pHandler )
		}

		SAFE_RELEASE( pType2 )
		SAFE_RELEASE( pHandler )
		SAFE_RELEASE( pSD )
		SAFE_RELEASE( pPD )
		SAFE_RELEASE( pSourceResolver )
		if ( g_pVideoSource ) g_pVideoSource->Shutdown();
		SAFE_RELEASE( g_pVideoSource )
		goto success;

	failed:
		SAFE_RELEASE( pType2 )
		SAFE_RELEASE( pHandler )
		SAFE_RELEASE( pSD )
		SAFE_RELEASE( pPD )
		SAFE_RELEASE( pSourceResolver )
		if ( g_pVideoSource ) g_pVideoSource->Shutdown();
		SAFE_RELEASE( g_pVideoSource )
		return 0;

	success:
		
		if ( !bFound ) Error1( "Failed to get video width and height" );
	}

	fVideoPos = -1;
	return 1;
}

void HandleVideoEvents()
{
	if ( !g_pVideoMediaEventEx ) return;

	HRESULT h;
	long evCode;
	LONG_PTR param1, param2;
	while ( !FAILED(h = g_pVideoMediaEventEx->GetEvent(&evCode, &param1, &param2, 0)) )
	{
		if ( evCode == EC_COMPLETE )
		{
			StopVideo();
		}
		g_pVideoMediaEventEx->FreeEventParams( evCode, param1, param2 );
	}
}

bool VideoUpdate()
{
	if ( g_bMFPlatExists && g_pVideoSession )
	{
		IMFMediaEvent *pEvent;
		while( g_pVideoSession->GetEvent( MF_EVENT_FLAG_NO_WAIT, &pEvent ) == S_OK )
		{
			MediaEventType eType;
			pEvent->GetType( &eType );
			if ( eType == MEEndOfPresentation )
			{
				StopVideo();
			}
			pEvent->Release();
		}
	}

	if ( g_bMFPlatExists && !g_pVideoSampleGrabber ) return false;
	if ( !g_bMFPlatExists && !pVideoSampleGrabber ) return false;
	if ( !pVideoFrame ) return false;
	if ( !iVideoChanged ) return false;
	return true;
}

void SetVideoVolume( float volume )
{
	fVideoVolume = volume;
	if ( g_bMFPlatExists && g_pVideoVolume != 0 )
	{
		if ( !g_pVideoVolume ) return;
		
		volume /= 100.0f;
		UINT32 count = 0;
		g_pVideoVolume->GetChannelCount( &count );
		for( uint32_t i = 0; i < count; i++ )
		{
			g_pVideoVolume->SetChannelVolume( i, volume );
		}
	}
}

void PlayVideoToImage( uint32_t imageID )
{
	if ( strlen(sVideoPath) == 0 ) return;
	if( g_bMFPlatExists )
	{
		if ( !g_pVideoSession )
		{
			IMFActivate *pSinkActivate = NULL;
			IMFTopology *pTopology = NULL;
			IMFMediaType *pType = NULL;
			IUnknown* pUnknownSource = NULL;
			IMFSourceResolver* pSourceResolver = NULL;
			MF_OBJECT_TYPE ObjectType;
			IMFPresentationDescriptor *pPD = NULL;
			IMFStreamDescriptor *pSD = NULL;
			IMFMediaTypeHandler *pHandler = NULL;
			IMFTopologyNode *pNode1 = NULL;
			IMFTopologyNode *pNode2 = NULL;
			DWORD cStreams = 0;
			bool bVideoDone = false;
			bool bAudioDone = false;
			BOOL fSelected = FALSE;
			GUID majorType;
			IMFActivate *pAudioActivate = NULL;

			HRESULT hr = MFCreateMediaType(&pType);
			if ( FAILED(hr) ) { Error1( "Failed to create media type" ); goto failed; }
			hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if ( FAILED(hr) ) { Error1( "Failed to set media major type" ); goto failed; }
			hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2); // Using an RGB format generates an exception when decoding H264 video, for some reason
			if ( FAILED(hr) ) { Error1( "Failed to set media minor type" ); goto failed; }


			hr = SampleGrabberCB::CreateInstance(&g_pVideoSampleGrabber);
			if ( FAILED(hr) ) { Error1( "Failed to create sample grabber" ); goto failed; }
			hr = MFCreateSampleGrabberSinkActivate(pType, g_pVideoSampleGrabber, &pSinkActivate);
			if ( FAILED(hr) ) { Error1( "Failed to create sample grabber 2" ); goto failed; }

			hr = MFCreateMediaSession(NULL, &g_pVideoSession);
			if ( FAILED(hr) ) { Error1( "Failed to create media session" ); goto failed; }

			hr = MFCreateSourceResolver(&pSourceResolver);
			if ( FAILED(hr) ) { Error1( "Failed to create source resolver" ); goto failed; }
			
			int iLength = MultiByteToWideChar( CP_UTF8, 0, sVideoPath, -1, 0, 0 );
			WCHAR *wstr = new WCHAR[ iLength + 1 ];
			MultiByteToWideChar( CP_UTF8, 0, sVideoPath, -1, wstr, iLength+1 );
			hr = pSourceResolver->CreateObjectFromURL(wstr, MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, NULL, &ObjectType, &pUnknownSource);
			delete [] wstr;
			if ( FAILED(hr) ) { Error1( "Failed to create load video from file" ); goto failed; }

			hr = pUnknownSource->QueryInterface(IID_PPV_ARGS(&g_pVideoSource));
			if ( FAILED(hr) ) { Error1( "Failed to query source interface" ); goto failed; }

			// Create Topology
			hr = MFCreateTopology(&pTopology);
			if ( FAILED(hr) ) { Error1( "Failed to create topology" ); goto failed; }
			hr = g_pVideoSource->CreatePresentationDescriptor(&pPD);
			if ( FAILED(hr) ) { Error1( "Failed to create presentation description" ); goto failed; }
			hr = pPD->GetStreamDescriptorCount(&cStreams);
			if ( FAILED(hr) ) { Error1( "Failed to get stream description count" ); goto failed; }

			for (DWORD i = 0; i < cStreams; i++)
			{
				hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
				if ( FAILED(hr) ) { Error1( "Failed to get stream descriptor by index" ); goto failed; }
				hr = pSD->GetMediaTypeHandler(&pHandler);
				if ( FAILED(hr) ) { Error1( "Failed to get media type handler" ); goto failed; }
				hr = pHandler->GetMajorType(&majorType);
				if ( FAILED(hr) ) { Error1( "Failed to get media major type" ); goto failed; }

				if (majorType == MFMediaType_Video && fSelected && !bVideoDone)
				{
					hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode1);
					if ( FAILED(hr) ) { Error1( "Failed to create source node" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_SOURCE, g_pVideoSource);
					if ( FAILED(hr) ) { Error1( "Failed to set node source" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
					if ( FAILED(hr) ) { Error1( "Failed to set node presentation descriptor" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
					if ( FAILED(hr) ) { Error1( "Failed to set node stream descriptor" ); goto failed; }
					hr = pTopology->AddNode(pNode1);
					if ( FAILED(hr) ) { Error1( "Failed to add topology node" ); goto failed; }

					hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode2);
					if ( FAILED(hr) ) { Error1( "Failed to create output node" ); goto failed; }
					hr = pNode2->SetObject(pSinkActivate);
					if ( FAILED(hr) ) { Error1( "Failed to set node sink" ); goto failed; }
					hr = pNode2->SetUINT32(MF_TOPONODE_STREAMID, 0);
					if ( FAILED(hr) ) { Error1( "Failed to set node2 stream ID" ); goto failed; }
					hr = pNode2->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
					if ( FAILED(hr) ) { Error1( "Failed to set node shutdown mode" ); goto failed; }
					hr = pTopology->AddNode(pNode2);
					if ( FAILED(hr) ) { Error1( "Failed to add output node" ); goto failed; }

					hr = pNode1->ConnectOutput(0, pNode2, 0);
					if ( FAILED(hr) ) { Error1( "Failed to connect nodes" ); goto failed; }
					
					bVideoDone = true;
				}
				else if (majorType == MFMediaType_Audio && fSelected && !bAudioDone)
				{
					hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode1);
					if ( FAILED(hr) ) { Error1( "Failed to create audio source node" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_SOURCE, g_pVideoSource);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node source" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node presentation descriptor" ); goto failed; }
					hr = pNode1->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node stream descriptor" ); goto failed; }
					hr = pTopology->AddNode(pNode1);
					if ( FAILED(hr) ) { Error1( "Failed to add audio topology node" ); goto failed; }

					hr = MFCreateAudioRendererActivate( &pAudioActivate );
					if ( FAILED(hr) ) { Error1( "Failed to create audio output node" ); goto failed; }

					hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode2);
					if ( FAILED(hr) ) { Error1( "Failed to create output node" ); goto failed; }
					hr = pNode2->SetObject(pAudioActivate);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node sink" ); goto failed; }
					hr = pNode2->SetUINT32(MF_TOPONODE_STREAMID, 0);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node2 stream ID" ); goto failed; }
					hr = pNode2->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
					if ( FAILED(hr) ) { Error1( "Failed to set audio node shutdown mode" ); goto failed; }
					hr = pTopology->AddNode(pNode2);
					if ( FAILED(hr) ) { Error1( "Failed to add audio output node" ); goto failed; }

					hr = pNode1->ConnectOutput(0, pNode2, 0);
					if ( FAILED(hr) ) { Error1( "Failed to connect audio nodes" ); goto failed; }

					bAudioDone = true;
				}
				else
				{
					hr = pPD->DeselectStream(i);
					if ( FAILED(hr) ) { Error1( "Failed to deselect stream" ); goto failed; }
				}

				SAFE_RELEASE( pNode1 )
				SAFE_RELEASE( pNode2 )
				SAFE_RELEASE( pSD )
				SAFE_RELEASE( pHandler )
			}
			
			hr = g_pVideoSession->SetTopology( 0, pTopology );
			if ( FAILED(hr) ) { Error1( "Failed to set session topology" ); goto failed; }

			//LB: set time out if no sound card, the loop would never end
			DWORD dwTimeElapsed = timeGetTime() + 5000; // 5 seconds

			// must wait for topology to finish setting before we can get the volume interface
			///float startTime = Timer();
			IMFMediaEvent *pEvent = 0;
			do
			{
				Sleep( 100 );
				///if ( agk::Timer() - startTime > 4 )
				///{
				///	Error1( "Failed to load video, format may not be supported" );
				///	goto failed;
				///}
				hr = g_pVideoSession->GetEvent( MF_EVENT_FLAG_NO_WAIT, &pEvent );
				if ( hr == S_OK )
				{
					MediaEventType eType;
					pEvent->GetType( &eType );
					pEvent->Release();
					if ( eType == MESessionTopologyStatus ) break;
				}
				if (timeGetTime() > dwTimeElapsed) break;
			} while( hr == S_OK || hr == (0xC00D3E80L) ); // MF_E_NO_EVENTS_AVAILABLE

			if ( hr != S_OK )
			{
				Error1( "Failed to check video session topology" );
				goto failed;
			}
			
			hr = MFGetService( g_pVideoSession, MR_STREAM_VOLUME_SERVICE, IID_PPV_ARGS(&g_pVideoVolume) );
			if ( FAILED(hr) ) 
			{ 
				//Error1( "Failed to get video volume interface" ); soft error 
				g_pVideoVolume = 0; 
			}
						
			SAFE_RELEASE( pNode2 )
			SAFE_RELEASE( pNode1 )
			SAFE_RELEASE( pHandler )
			SAFE_RELEASE( pSD )
			SAFE_RELEASE( pPD )
			SAFE_RELEASE( pUnknownSource )
			SAFE_RELEASE( pSourceResolver )
			SAFE_RELEASE( pSinkActivate )
			SAFE_RELEASE( pTopology )
			SAFE_RELEASE( pType )
			SAFE_RELEASE( pAudioActivate )
			goto success;

		failed:
			SAFE_RELEASE( g_pVideoVolume )
			SAFE_RELEASE( pNode2 )
			SAFE_RELEASE( pNode1 )
			SAFE_RELEASE( pHandler )
			SAFE_RELEASE( pSD )
			SAFE_RELEASE( pPD )
			SAFE_RELEASE( pUnknownSource )
			SAFE_RELEASE( pSourceResolver )
			SAFE_RELEASE( pSinkActivate )
			SAFE_RELEASE( pTopology )
			SAFE_RELEASE( pType )
			SAFE_RELEASE( pAudioActivate )
			if ( g_pVideoSource ) g_pVideoSource->Shutdown();
			SAFE_RELEASE( g_pVideoSource );
			if ( g_pVideoSession ) g_pVideoSession->Shutdown();
			SAFE_RELEASE( g_pVideoSession )
			return;

		success:
		
			pVideoFrame = new unsigned char[ iVideoWidth*iVideoHeight*4 ];

			if ( g_pVideoVolume != 0 )
				SetVideoVolume( fVideoVolume );

			g_pVideoClock = 0;
			IMFClock *pClock = 0;
			hr = g_pVideoSession->GetClock(&pClock);
			if ( SUCCEEDED(hr) && pClock )
			{
				hr = pClock->QueryInterface(IID_PPV_ARGS(&g_pVideoClock));
				pClock->Release();
			}
		}

		PROPVARIANT var;
		PropVariantInit(&var); 
		var.vt = VT_EMPTY;

		if ( fVideoPos > fVideoDuration ) fVideoPos = 0;
		if ( fVideoPos >= 0 ) 
		{
			long long pos = (long long) (fVideoPos * 10000000);
			var.vt = VT_I8;
			var.hVal.QuadPart = pos;
			fVideoPos = -1;
		}

		HRESULT hr = g_pVideoSession->Start( NULL, &var );
		if ( FAILED(hr) ) { Error1( "Failed to start session" ); return; }
		g_bVideoPlaying = TRUE;
	}
}

void PauseVideo()
{
	if ( g_bMFPlatExists )
	{
		if ( !g_pVideoSession ) return;
		g_pVideoSession->Pause();
		g_bVideoPlaying = FALSE;
	}
}

int GetVideoPlaying()
{
	if ( g_bMFPlatExists )
	{
		if ( !g_pVideoSession ) return 0;
		return g_bVideoPlaying != 0 ? 1 : 0;
	}
}

float GetVideoPosition()
{
	if ( g_bMFPlatExists )
	{
		//LB: if no video or at end, show position as at end (so loops that wait to end of video can exit)
		if ( !g_pVideoSession ) return fVideoDuration;
		if ( !g_pVideoClock ) return fVideoDuration;

		MFTIME currTime;
		g_pVideoClock->GetTime( &currTime );
		float fPos = currTime / 10000000.0f;
		return fPos;
	}
}

float GetVideoDuration()
{
	return fVideoDuration;
}

float GetVideoWidth()
{
	return (float) iVideoWidth;
}

float GetVideoHeight()
{
	return (float) iVideoHeight;
}

void SetVideoPosition( float seconds )
{
	if ( g_bMFPlatExists )
	{
		if ( !g_pVideoSession ) fVideoPos = seconds;
		else if ( !GetVideoPlaying() ) 
		{
			if ( seconds >= fVideoDuration ) fVideoPos = 0;
			else fVideoPos = seconds;
		}
		else
		{
			if ( seconds >= fVideoDuration ) StopVideo();
			else
			{
				PROPVARIANT var; PropVariantInit(&var); var.vt = VT_EMPTY;

				long long pos = (long long) (seconds * 10000000);
				var.vt = VT_I8;
				var.hVal.QuadPart = pos;
				fVideoPos = -1;
			
				HRESULT hr = g_pVideoSession->Start( NULL, &var );
				if ( FAILED(hr) ) { Error1( "Failed to set video position" ); return; }
			}
		}
	}
}
#endif

// Main Functions

void AnimationConstructor ( void )
{
	// Initialise the video manager
	#ifdef WMFVIDEO
	g_bMFPlatExists = false;
	HMODULE hMFPlat = LoadLibrary( "mfplat.dll" );
	if ( hMFPlat ) 
	{
		FreeLibrary( hMFPlat );
		g_bMFPlatExists = true;
	}
	if ( g_bMFPlatExists ) MFStartup( MF_VERSION );
	#else
	theoraplayer::init(1);
	theoraOutputMode = theoraplayer::FORMAT_BGRX;
	theoraplayer::manager->setWorkerThreadCount(1);
	// Provide MP4 support (.mp4)
	clipffmpeg::init();
	#endif

	// Clear Arrays
	ZeroMemory(&Anim, sizeof(Anim));
	ZeroMemory(&animation, sizeof(animation));
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
	#ifdef WMFVIDEO
	if ( g_bMFPlatExists ) 
	{
		if ( g_pVideoSession ) DeleteVideo();
		MFShutdown();
	}
	#else
	clipffmpeg::destroy();
	theoraplayer::destroy();
	#endif
}

BOOL CoreLoadAnimation( int AnimIndex, char* Filename, int precacheframes)
{
	// Vars
    HRESULT hr = S_OK;

	// load in an OGV video file (streaming)
	#ifdef WMFVIDEO
	 if(!LoadVideo(Filename))
		 Anim[AnimIndex].pMediaClip = 0;
	 else
		 Anim[AnimIndex].pMediaClip = 1;
	#else
	 // PE: precache of 16 at 1080p = 95 MB , if you have setup 19 zones with video , these are preloaded and add 1.76 GB ... not good ...
	 Anim[AnimIndex].pMediaClip = theoraplayer::manager->createVideoClip(Filename, theoraOutputMode, precacheframes );
	#endif
	if ( Anim[AnimIndex].pMediaClip )
	{
		#ifdef WMFVIDEO
		// WMF
		float fWidth = iVideoWidth;
		float fHeight = iVideoHeight;
		float tTexWidthPOT = potCeil(fWidth);
		float tTexHeightPOT = potCeil(fHeight);
		DXGI_FORMAT chooseTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		Anim[AnimIndex].TextureFormat = chooseTextureFormat;
		#else
		// THEORA 
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
		#endif

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
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
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
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename, precacheframes);
	g_pGlob->Encrypt( VirtualFilename );
	return bRes;
}

BOOL DB_LoadAnimation_Delayed(int AnimIndex)
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, Anim[AnimIndex].AnimFile );
	//g_pGlob->UpdateFilenameFromVirtualTable(VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt(VirtualFilename);
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename, Anim[AnimIndex].precacheframes);
	g_pGlob->Encrypt(VirtualFilename);
	return bRes;
}

DARKSDK BOOL DB_FreeAnimation(int AnimIndex)
{
	// Shut down the graph
	if(Anim[AnimIndex].pMediaClip)
	{
		// Release video
		#ifdef WMFVIDEO
		DeleteVideo();
		#else
        Anim[AnimIndex].pMediaClip->stop();
		theoraplayer::manager->destroyVideoClip(Anim[AnimIndex].pMediaClip);
		#endif
		Anim[AnimIndex].pMediaClip = 0;

		// Release resources used by video
		SAFE_RELEASE ( Anim[AnimIndex].pTextureRef );
		SAFE_RELEASE ( Anim[AnimIndex].pTexture );

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
	if(Anim[AnimIndex].pMediaClip==0)
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

	// moved this as actual start is when everything is going (i.e. frame being rendered), so moved there
	//Anim[AnimIndex].bStreamingNow=true;

	// Set State to Run
	#ifdef WMFVIDEO
	PlayVideoToImage(0);
	#else
	//Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
	Anim[AnimIndex].pMediaClip->play();
	#endif

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

DARKSDK BOOL DB_OverrideTextureWithAnimation(int AnimIndex, int ObjectNumber)
{
	TextureObjectRef ( ObjectNumber, Anim[AnimIndex].pTextureRef, Anim[AnimIndex].ClipU, Anim[AnimIndex].ClipV );
	return TRUE;
}

BOOL DB_StopAnimation(int AnimIndex)
{
	// If currently not playing this animation, nothing to stop
	if(Anim[AnimIndex].pMediaClip==0)
		return FALSE;

	// stop animation
	Anim[AnimIndex].bStreamingNow=false;
	#ifdef WMFVIDEO
	StopVideo();
	#else
	Anim[AnimIndex].pMediaClip->stop();
	#endif

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

void SetRenderAnimToImage(int AnimIndex, bool active)
{
	if (active)
		Anim[AnimIndex].iOutputToImage = 1;
	else
		Anim[AnimIndex].iOutputToImage = 0;
}

float GetAnimU(int AnimIndex)
{
	return(Anim[AnimIndex].ClipU);
}
float GetAnimV(int AnimIndex)
{
	return(Anim[AnimIndex].ClipV);
}

int GetAnimWidth(int AnimIndex)
{
	return(Anim[AnimIndex].StreamRect.right);
}
int GetAnimHeight(int AnimIndex)
{
	return(Anim[AnimIndex].StreamRect.bottom);
}

void PauseAnim(int AnimIndex)
{
	if (!Anim[AnimIndex].pMediaClip)
		return;
	#ifdef WMFVIDEO
	PauseVideo();
	#else
	Anim[AnimIndex].pMediaClip->pause();
	#endif
	animation[AnimIndex].playing = false;
}
void ResumeAnim(int AnimIndex)
{
	if (!Anim[AnimIndex].pMediaClip)
		return;
	#ifdef WMFVIDEO
	PlayVideoToImage(0);
	#else
	Anim[AnimIndex].pMediaClip->play();
	#endif
	animation[AnimIndex].playing = true;
}
void RestartAnim(int AnimIndex)
{
	if (!Anim[AnimIndex].pMediaClip)
		return;
	#ifdef WMFVIDEO
	StopVideo();
	PlayVideoToImage(0);
	#else
	Anim[AnimIndex].pMediaClip->restart();
	#endif
	animation[AnimIndex].playing = true;
}

bool GetAnimDone(int AnimIndex)
{
	if(!Anim[AnimIndex].pMediaClip)
		return true;

	#ifdef WMFVIDEO
	if (GetVideoPosition() >= GetVideoDuration())
	#else
	if( Anim[AnimIndex].pMediaClip->getTimePosition() >= Anim[AnimIndex].pMediaClip->getDuration())
	#endif
	{
		return true;
	}
	return false;
}

float GetAnimPercentDone(int AnimIndex)
{
	if (!Anim[AnimIndex].pMediaClip)
		return 0.0f;

	#ifdef WMFVIDEO
	if (GetVideoPosition() >= GetVideoDuration())
	{
		return 100.0f;
	}
	float fpos = GetVideoPosition();
	float fdur = GetVideoDuration();
	#else
	if (Anim[AnimIndex].pMediaClip->getTimePosition() >= Anim[AnimIndex].pMediaClip->getDuration())
	{
		return 100.0f;
	}
	float fpos = Anim[AnimIndex].pMediaClip->getTimePosition();
	float fdur = Anim[AnimIndex].pMediaClip->getDuration();
	#endif
	float fdone = 100.0f / (fdur/fpos);
	return fdone;
}

ID3D11ShaderResourceView* GetAnimPointerView(int AnimIndex)
{
	if (Anim[AnimIndex].pMediaClip ) 
	{
		return Anim[AnimIndex].pTextureRef;
	}
	return NULL;
}

LPGGSURFACE GetAnimPointerTexture(int AnimIndex)
{
	if (Anim[AnimIndex].pMediaClip)
	{
		return Anim[AnimIndex].pTexture;
	}
	return NULL;
}

DARKSDK void UpdateAllAnimation(void)
{
	// an update call to the video manager
	bool bSkipFirstCycle = false;
	if ( fLastTimeVideoManagerUpdated == 0.0f ) bSkipFirstCycle = true;
	float fTimeNow = timeGetSecond();
	float fDeltaTime = fTimeNow - fLastTimeVideoManagerUpdated;
	fLastTimeVideoManagerUpdated = fTimeNow;
	if (bSkipFirstCycle == false)
	{
		#ifdef WMFVIDEO
		HandleVideoEvents();
		#else
		theoraplayer::manager->update(fDeltaTime);
		#endif
	}

	// Monitor all Animations
	for(int AnimIndex=0; AnimIndex<ANIMATIONMAX; AnimIndex++)
	{
		if(Anim[AnimIndex].pMediaClip)
		{
			// only if playing
			if ( animation[AnimIndex].playing == true )
			{
				// pull next available frame
				#ifdef WMFVIDEO
				volatile unsigned char* frame = NULL;
				if ( g_bMFPlatExists ) 
					if ( VideoUpdate() == true )
						frame = pVideoFrame;
				#else
				theoraplayer::VideoFrame* frame = Anim[AnimIndex].pMediaClip->fetchNextFrame();
				#endif
				if (frame != NULL && Anim[AnimIndex].pTexture )
				{
					// NOW we are really streaming
					Anim[AnimIndex].bStreamingNow = true;

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
							#ifdef WMFVIDEO
							unsigned char* pSrc = (unsigned char*)frame;
							unsigned char* pPtr = (unsigned char*)grafixlr.pData;
							DWORD dwVideoWidth = GetVideoWidth();
							DWORD dwVideoHeight = GetVideoHeight();
							unsigned int* pVideoFrameInt = (unsigned int*)pVideoFrame;
							for ( int y = 0; y < iVideoHeight; y++ )
							{
								unsigned int* pThisLinePtr = (unsigned int*)pPtr;
								uint32_t index = y*iVideoWidth/2;
								for ( int x = 0; x < iVideoWidth/2; x++ )
								{
									unsigned int value = pVideoFrameInt[ index ];
									int iY0 = value & 0xff;
									int iU = (value >> 8) & 0xff;
									int iY1 = (value >> 16) & 0xff;
									int iV = value >> 24;
									iU -= 128;
									iV -= 128;

									// 16-235 conversion
									iY0 -= 16;
									iY0 *= 298;
									iY0 += 128;
									int iRed = (iY0 + iV*409) >> 8;
									int iGreen = (iY0 - iU*100 - iV*208) >> 8;
									int iBlue = (iY0 + iU*516) >> 8;

									if( iRed < 0 ) iRed = 0;
									if( iGreen < 0 ) iGreen = 0;
									if( iBlue < 0 ) iBlue = 0;

									if( iRed > 255 ) iRed = 255;
									if( iGreen > 255 ) iGreen = 255;
									if( iBlue > 255 ) iBlue = 255;

									*(pThisLinePtr+0) = 0xff000000 | (iBlue << 16) | (iGreen << 8) | iRed;

									iY1 -= 16;
									iY1 *= 298;
									iY1 += 128;
									int iRed2 = (iY1 + iV*409) >> 8;
									int iGreen2 = (iY1 - iU*100 - iV*208) >> 8;
									int iBlue2 = (iY1 + iU*516) >> 8;

									if( iRed2 < 0 ) iRed2 = 0;
									if( iGreen2 < 0 ) iGreen2 = 0;
									if( iBlue2 < 0 ) iBlue2 = 0;

									if( iRed2 > 255 ) iRed2 = 255;
									if( iGreen2 > 255 ) iGreen2 = 255;
									if( iBlue2 > 255 ) iBlue2 = 255;

									*(pThisLinePtr+1) = 0xff000000 | (iBlue2 << 16) | (iGreen2 << 8) | iRed2;

									index++;
									pThisLinePtr += 2;
								}
								pPtr+=grafixlr.RowPitch;
							}
							#else
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
							#endif
							
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
					#ifdef WMFVIDEO
					#else
					Anim[AnimIndex].pMediaClip->popFrame();
					#endif
				}

				// Handle Animations that loop
				if(Anim[AnimIndex].bStreamingNow==true)
				{
					// If Looping, repeat run when it ends
					#ifdef WMFVIDEO
					//char what[256];
					//sprintf(what, "%d %d", (int)GetVideoPosition(), (int)GetVideoDuration());
					//MessageBox(NULL, what, "", MB_OK);
					if ( GetVideoPosition() >= GetVideoDuration() || GetVideoPlaying() == 0)
					{
						if(Anim[AnimIndex].loop==true)
						{
							//if(Anim[AnimIndex].pMediaPosition)
							//{
							//	Anim[AnimIndex].pMediaPosition->put_CurrentPosition(0);
							//}
							StopVideo();
							PlayVideoToImage(0);
						}
						else
						{
							StopVideo();
							animation[AnimIndex].playing=false;
						}
					}
					#else
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
					#endif
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
					//PE: Use:
					//SetRenderAnimToImage(tut.bVideoID, true);
					//ID3D11ShaderResourceView* lpTexture = GetAnimPointerView(tut.bVideoID);

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

DARKSDK bool LoadAnimation( LPSTR pFilename, int animindex , int precacheframes , int videodelayedload, int iSilentMode )
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
		{
			char mdebug[1024];
			sprintf(mdebug, "LoadAnimation: %s", pFilename);
			timestampactivity(0, mdebug);
			if ( iSilentMode == 0 ) RunTimeError(RUNTIMEERROR_ANIMLOADFAILED);
			return false;
		}
	}
	else
	{
		if ( iSilentMode == 0 )  RunTimeError(RUNTIMEERROR_ANIMNUMBERILLEGAL);
		return false;
	}
	return true;
}

DARKSDK void DeleteAnimation( int animindex )
{
	if (Anim[animindex].videodelayedload == 1 && Anim[animindex].loaded == false)
	{
		//PE: Not really loaded, just set as free to use.
		animation[animindex].active = false;
		return;
	}

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

DARKSDK void OverrideTextureWithAnimation( int animindex, int objectnumber )
{
	if(animindex>=1 && animindex<ANIMATIONMAX)
	{
		DB_OverrideTextureWithAnimation(animindex, objectnumber);
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

#ifdef WMFVIDEO
DARKSDK int AnimationWidth( int animindex )
{
	return iVideoWidth;
}
DARKSDK int AnimationHeight( int animindex )
{
	return iVideoHeight;
}
#endif

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
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	BOOL bRes = DB_LoadAnimationCore(AnimIndex, VirtualFilename);
	g_pGlob->Encrypt( VirtualFilename );
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
