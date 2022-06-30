#include "cThread.h"
#include <Windows.h>
#include <wininet.h>
#include <process.h>   

unsigned int cThread::EntryPoint( void *pParams )
{
	if ( !pParams ) return 0;
	cThread* pThis = (cThread*) pParams;
	unsigned int result = pThis->Run();
	pThis->Finally();
	return result;
}

// when a thread stops for whatever reason this function shuld be called
void cThread::Finally()
{
	m_bRunning = false;
}

cThread::cThread( )
{
	pThread = 0;
	iThreadID = 0;
	m_bRunning = false;
	m_bTerminate = false;
	m_pStop = 0;
	PlatformInit(); 
}

 cThread::~cThread( )
{
	// tell it to stop
	Stop();

	// give some time?
	//Join();
	//Sleep( 100 );

	// force it to stop
	//PlatformTerminate();
	PlatformCleanUp();
}

bool cThread::IsRunning()
{
	return m_bRunning;
}

// starts the thread and calls Run() (if not already running)
void cThread::Start( )
{
	if ( m_bRunning ) return;
	m_bTerminate = false;
	m_bRunning = true;
	PlatformStart();
}

// tells the thread to stop and returns immediately, the thread is not guaranteed to stop
 void cThread::Stop()
{
	m_bTerminate = true;
	PlatformStop();
}

// sleep for a specified time but will wake early if the thread is told to stop, check m_bTerminate after this command
void cThread::SleepSafe( unsigned int milliseconds )
{
	PlatformSleepSafe( milliseconds );
}

// waits for the thread to stop then returns, can wait forever. If thread is already stopped or 
// not yet started returns immediately
void cThread::Join( )
{
	if ( !IsRunning() ) return;
	PlatformJoin();
}


void cThread::PlatformInit( )
{
	m_pStop = CreateEvent( NULL, FALSE, FALSE, NULL );
}

void cThread::PlatformStart( )
{
	ResetEvent( m_pStop );
	pThread = (HANDLE)_beginthreadex( NULL, 0, EntryPoint, (void*) this, 0, &iThreadID );
}

void cThread::PlatformStop( )
{
	SetEvent( m_pStop );
}


void cThread::PlatformCleanUp( )
{
	if ( m_pStop ) CloseHandle( m_pStop );
}

void cThread::PlatformJoin( )
{
	WaitForSingleObject( pThread, INFINITE );
}

void cThread::PlatformSleepSafe( UINT milliseconds )
{
	WaitForSingleObject( m_pStop, milliseconds );
}