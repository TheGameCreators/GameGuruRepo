#ifndef __MPAUDIO__
#define __MPAUDIO__

#include <map>

// Typedef for voice channels
typedef int HGAMEVOICECHANNEL;

#define VOICE_OUTPUT_SAMPLE_RATE			11000	// real sample rate is 11025 but for XAudio2 it must be a multiple of XAUDIO2_QUANTUM_DENOMINATOR
#define VOICE_OUTPUT_SAMPLE_RATE_IDEAL		11025
#define BYTES_PER_SAMPLE					2

// Voice chat functions
HGAMEVOICECHANNEL HCreateVoiceChannel();
void DestroyVoiceChannel( HGAMEVOICECHANNEL hChannel );
bool AddVoiceData( HGAMEVOICECHANNEL hChannel, const uint8 *pVoiceData, uint32 uLength );

void StartVoiceChat();
void EndVoiceChat();

//===================

class CVoiceContext : public IXAudio2VoiceCallback
{
public:
	CVoiceContext() : m_hBufferEndEvent( CreateEvent( NULL, FALSE, FALSE, NULL ) )
	{
		m_pSourceVoice = NULL;
	}
	virtual ~CVoiceContext()
	{
		CloseHandle( m_hBufferEndEvent );
	}

	STDMETHOD_( void, OnVoiceProcessingPassStart)()
	{
	}

	STDMETHOD_( void, OnVoiceProcessingPassStart )( UINT32 )
	{
	}
	STDMETHOD_( void, OnVoiceProcessingPassEnd )()
	{
	}
	STDMETHOD_( void, OnStreamEnd )()
	{
	}
	STDMETHOD_( void, OnBufferStart )( void* )
	{
	}
	STDMETHOD_( void, OnBufferEnd )( void* pContext )
	{
		free( pContext ); // free the sound buffer
		SetEvent( m_hBufferEndEvent );
	}
	STDMETHOD_( void, OnLoopEnd )( void* )
	{
	}
	STDMETHOD_( void, OnVoiceError )( void*, HRESULT )
	{
	}

	HANDLE m_hBufferEndEvent;
	IXAudio2SourceVoice* m_pSourceVoice;
};

//===================

extern IXAudio2* pXAudio2;
extern IXAudio2MasteringVoice* pMasteringVoice;

extern std::map<HGAMEVOICECHANNEL, CVoiceContext* > MapVoiceChannel;
extern uint32 unVoiceChannelCount;

#endif __MPAUDIO__

/*	
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
uh uh uh
ohnn ohnn
*/
