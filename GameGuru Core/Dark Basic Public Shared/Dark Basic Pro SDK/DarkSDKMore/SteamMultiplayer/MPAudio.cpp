#include "stdafx.h"
#include "MPAudio.h"

IXAudio2* pXAudio2 = NULL;
IXAudio2MasteringVoice* pMasteringVoice = NULL;

std::map<HGAMEVOICECHANNEL, CVoiceContext* > MapVoiceChannel;
uint32 unVoiceChannelCount = 0;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) if ( 0 != ( x ) ) { ( x )->Release(); x = 0; }
#endif

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

void StartVoiceChat()
{
	// for XAudio2
	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	// initialize XAudio2 interface
	if( FAILED( XAudio2Create( &pXAudio2, 0 ) ) )
	{
		::MessageBoxA( NULL, "Failed to init XAudio2 engine (grab the latest \"Direct X End-User Runtime Web Installer\" )", "Fatal error", MB_OK | MB_ICONERROR );
		return;
	}

	// Create a mastering voice
	if( FAILED( pXAudio2->CreateMasteringVoice( &pMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, VOICE_OUTPUT_SAMPLE_RATE ) ) )
	{
		::MessageBoxA( NULL, "Failed to create mastering voice", "Fatal error", MB_OK | MB_ICONERROR );
		return;
	}
}

void EndVoiceChat()
{
	// All XAudio2 interfaces are released when the engine is destroyed, but being tidy
	if ( pMasteringVoice )
	{
		pMasteringVoice->DestroyVoice();
		pMasteringVoice = NULL;
	}

	SAFE_RELEASE( pXAudio2 );
}

HGAMEVOICECHANNEL HCreateVoiceChannel()
{
	if ( !pXAudio2 )
		return 0;

	unVoiceChannelCount++;
	CVoiceContext* pVoiceContext = new CVoiceContext;

	// The format we sample voice in.
	WAVEFORMATEX voicesampleformat =
	{
		WAVE_FORMAT_PCM,		// wFormatTag
		1,						// nChannels
		VOICE_OUTPUT_SAMPLE_RATE,// nSamplesPerSec
		VOICE_OUTPUT_SAMPLE_RATE*BYTES_PER_SAMPLE, // nAvgBytesPerSec
		2,						// nBlockAlign
		8*BYTES_PER_SAMPLE,		// wBitsPerSample
		sizeof(WAVEFORMATEX)	// cbSize
	};

	if( FAILED( pXAudio2->CreateSourceVoice( &pVoiceContext->m_pSourceVoice, &voicesampleformat , 0, 1.0f, pVoiceContext ) ) )
	{
		delete pVoiceContext;
		return 0; // failed
	}

	pVoiceContext->m_pSourceVoice->Start( 0, 0 );

	MapVoiceChannel[unVoiceChannelCount] = pVoiceContext;

	return unVoiceChannelCount;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DestroyVoiceChannel( HGAMEVOICECHANNEL hChannel )
{
	std::map<HGAMEVOICECHANNEL, CVoiceContext* >::iterator iter;
	iter = MapVoiceChannel.find( hChannel );
	if ( iter != MapVoiceChannel.end() )
	{
		CVoiceContext* pVoiceContext = iter->second;
		XAUDIO2_VOICE_STATE state;

		for(;;)
		{
			pVoiceContext->m_pSourceVoice->GetState( &state );
			if( !state.BuffersQueued )
				break;

			WaitForSingleObject( pVoiceContext->m_hBufferEndEvent, INFINITE );
		}

		pVoiceContext->m_pSourceVoice->Stop( 0 );
		pVoiceContext->m_pSourceVoice->DestroyVoice();

		delete pVoiceContext;

		MapVoiceChannel.erase( iter );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool AddVoiceData( HGAMEVOICECHANNEL hChannel, const uint8 *pVoiceData, uint32 uLength )
{
	std::map<HGAMEVOICECHANNEL, CVoiceContext* >::iterator iter;
	iter = MapVoiceChannel.find( hChannel );
	if ( iter == MapVoiceChannel.end() )
		return false; // channel not found
	
	CVoiceContext* pVoiceContext = iter->second;

	//
	// At this point we have a buffer full of audio and enough room to submit it, so
	// let's submit it and get another read request going.
	
	uint8 *pBuffer = (uint8 *) malloc( uLength );
	memcpy( pBuffer, pVoiceData, uLength );

	XAUDIO2_BUFFER buf = {0};
	buf.AudioBytes = uLength;
	buf.pAudioData = pBuffer;
	buf.pContext = pBuffer; // the buffer is freed again in CVoiceContext::OnBufferEnd

	if ( FAILED( pVoiceContext->m_pSourceVoice->SubmitSourceBuffer( &buf ) ) )
	{
		free ( pBuffer );
		return false;
	}

	return true;
}