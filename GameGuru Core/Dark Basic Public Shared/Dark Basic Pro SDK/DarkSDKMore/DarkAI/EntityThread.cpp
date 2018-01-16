#include "EntityThread.h"
#include "EntityBuffer.h"
#include "Entity.h"

EntityThread::EntityThread( )
{
	bTerminate = false;
	bIsRunning = false;
	
	hStartEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( !hStartEvent )
	{
		char str [ 256 ];
		sprintf_s( str, 256, "Error creating start event: %d",GetLastError( ) );
		MessageBox( NULL, str, "AI Error", 0 );
		exit(-1);
	}

	hDoneEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
	if ( !hDoneEvent )
	{
		char str [ 256 ];
		sprintf_s( str, 256, "Error creating done event: %d",GetLastError( ) );
		MessageBox( NULL, str, "AI Error", 0 );
		exit(-1);
	}
	
}

EntityThread::~EntityThread( )
{
	CloseHandle( hStartEvent );
	CloseHandle( hDoneEvent );
}

void EntityThread::SetupData( EntityBuffer *pBuffer, float time )
{
	pEntityBuffer = pBuffer;
	fTimeDelta = time;
}

void EntityThread::WaitOnThread( )
{
	WaitForSingleObject( hDoneEvent, INFINITE );
}

void EntityThread::ResumeThread( )
{
	if ( !bIsRunning ) Start( );

	ResetEvent( hDoneEvent );
	SetEvent( hStartEvent );
}

unsigned int EntityThread::Run( )
{
	bIsRunning = true;

	while( !bTerminate )
	{
		int result = WaitForSingleObject( hStartEvent, INFINITE );
		if ( result == WAIT_FAILED ) 
		{
			char str[64];
			sprintf_s( str, 64, "Failed to get start event, error: %d", GetLastError( ) );
			MessageBox( NULL, str, "Error", 0 );
			exit(-1);
		}
				
		if ( !pEntityBuffer ) 
		{
			SetEvent( hDoneEvent );
			continue;
		}
		Entity *pEntity = pEntityBuffer->GetNextEntity( );

		while ( pEntity )
		{
			if ( pEntity->GetActive( ) )
			{
				pEntity->UpdateTargets ( fTimeDelta );
				pEntity->UpdateState ( );
				pEntity->UpdateMovement ( fTimeDelta );
			}

			pEntity = pEntityBuffer->GetNextEntity( );
		}

		SetEvent( hDoneEvent );
	}

	bIsRunning = false;

	return 0;
}