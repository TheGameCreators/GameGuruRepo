#ifndef LEETHREAD_H
#define LEETHREAD_H

#include "Thread.h"
#include "Path.h"
#include "Container.h"
#include "PathFinderAdvanced.h"

#define DEGTORAD 0.01745329252f

class LeeThread : public Thread
{

public:

	LeeThread( );
	~LeeThread( );
	void SetupData ( void );
	void BeginWork ( int iAIObj, float fX, float fZ, float fFDX, float fFDY, float fFDZ, Container* pContainer, int iDestContID );
	unsigned int Run( );
	void EndWork ( void );

	void StopRunning ( void );
	bool IsReadyToRun ( void );

	int GetWorkInProgress ( void ) { return m_iAIBeingWorkedOn; }
	int GetWorkComplete ( void ) { if ( m_iAIWorkComplete > 0 ) return 1; else return 0; }
	Path GetNewPath ( void ) { return m_cNewPath; }
	void GetNewFinalDest ( float* pfFDX, float* pfFDY, float* pfFDZ ); 

private:

	// input data
	int m_iAIBeingWorkedOn;
	float m_fGetX;
	float m_fGetZ;
	float m_fFinalDestX;
	float m_fFinalDestY;
	float m_fFinalDestZ;
	Container* m_pContainer;
	int m_iDestContainer;

	// output result
	int m_iAIWorkComplete;
	Path m_cNewPath;

	// loop
	bool m_bReadyToRun;
	bool m_bKeepRunning;
};

#endif