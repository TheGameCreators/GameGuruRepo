#include "LeeThread.h"

// prototypes
int waypoint_getmax(void);
int waypoint_ispointinzoneex( int iWPIndex, float fX, float fY, float fZ, int iStyleFloorZoneOnly );

LeeThread::LeeThread( )
{
}

LeeThread::~LeeThread( )
{
}

void LeeThread::SetupData( void )
{
	// reset work thread flags
	m_iAIBeingWorkedOn = -1;
	m_iAIWorkComplete = -1;
	m_bReadyToRun = false;
	m_bKeepRunning = true;
}

void LeeThread::BeginWork( int iAIObj, float fX, float fZ, float fFDX, float fFDY, float fFDZ, Container* pContainer, int iDestContID )
{
	m_fGetX = fX;
	m_fGetZ = fZ;
	m_fFinalDestX = fFDX;
	m_fFinalDestY = fFDY;
	m_fFinalDestZ = fFDZ;
	m_pContainer = pContainer;
	m_iDestContainer = iDestContID;
	m_iAIBeingWorkedOn = iAIObj;
}

unsigned int LeeThread::Run( )
{
	// wait for the trigger to do some path finding work
	m_bReadyToRun = false;
	while ( m_bKeepRunning == true )
	{
		if ( m_iAIBeingWorkedOn != -1 && m_iAIWorkComplete == -1 )
		{
			// local values
			float fPathStartCostLimit = -1.0f;

			// prepare level obstacles for new path calculation
			Path cNewPath;
			m_pContainer->pPathFinder->ActivateAllWaypoints ( );
			m_pContainer->pPathFinder->CalculatePath ( m_fGetX, m_fGetZ, m_fFinalDestX, m_fFinalDestZ, &cNewPath, fPathStartCostLimit, m_iDestContainer );

			// if no path to finaldest, work from finaldest back to known reachable point
			if ( cNewPath.CountPoints() == 0 )
			{
				// okay, so final dest not pathable, and no direct line to barrier
				// so find closest node from container, and set that as finaldest
				int iThisContainer = m_pContainer->GetID(); 
				m_iDestContainer = iThisContainer; // container ID can be changed below
				float fBestDistance = 999999.0f;
				float fBestX = 0.0f;
				float fBestZ = 0.0f;
				float fDX = m_fGetX - m_fFinalDestX;
				float fDZ = m_fGetZ - m_fFinalDestZ;
				float fDD = sqrt ( fabs(fDX*fDX) + fabs(fDZ*fDZ) );
				float fDDInc = fDD / 30.0f;
				int iDistCount = (int)fDDInc;
				for ( int iDist = 1; iDist <= 30; iDist++ )
				{
					for ( int iAng = 0; iAng < 360; iAng+=45 )
					{
						// this is where I want the AI to try to get to
						float fTryX = m_fFinalDestX + (sin(iAng*DEGTORAD)*(iDist*fDDInc));
						float fTryZ = m_fFinalDestZ + (cos(iAng*DEGTORAD)*(iDist*fDDInc));
						int waypointmax = waypoint_getmax();
						for ( int waypointindex = 1; waypointindex <= waypointmax; waypointindex++ )
						{
							int tokay = waypoint_ispointinzoneex ( waypointindex, fTryX, m_fFinalDestY, fTryZ, 1 );
							if ( tokay == 1 ) 
							{
								m_iDestContainer = waypointindex;
								fBestDistance = 0;
								fBestX = fTryX;
								fBestZ = fTryZ;
								iDist = 31;
								iAng = 361;
								break;
							}
						}
					}
				}
				if ( fBestDistance != 999999.0f )
				{
					// best is within waypoint zone, but outside AI obstacle zone (margin added to AI waypoint system)
					// so project away from player to ensure we get inside
					float fDX = fBestX - m_fFinalDestX;
					float fDZ = fBestZ - m_fFinalDestZ;
					float fDD = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
					fDX = (fDX/fDD)*(fDD+30.0f);
					fDZ = (fDZ/fDD)*(fDD+30.0f);
					fBestX = m_fFinalDestX + fDX;
					fBestZ = m_fFinalDestZ + fDZ;

					// now assign final pos
					if ( m_iDestContainer != iThisContainer )
					{
						// found closer position in another container, so just go there
						m_fFinalDestX = fBestX;
						m_fFinalDestZ = fBestZ;
					}
					else
					{
						// same container, so dont get too close to edge when move nearer target
						float fPushX = fBestX - m_fFinalDestX;
						float fPushZ = fBestZ - m_fFinalDestZ;
						float fPushDD = sqrt ( fabs(fPushX*fPushX) + fabs(fPushZ*fPushZ) );
						fPushX /= fPushDD;
						fPushZ /= fPushDD;
						m_fFinalDestX = fBestX + (fPushX*2.0f);
						m_fFinalDestZ = fBestZ + (fPushZ*2.0f);
					}
				}
				else
				{
					if ( m_pContainer->GetID() == 0 )
					{
						float fResult = m_pContainer->pPathFinder->FindClosestPolygon ( m_fGetX, m_fGetZ, m_fFinalDestX, m_fFinalDestZ );
						if ( fResult >= 0.1f ) 
						{
							m_iDestContainer = m_pContainer->GetID(); 
							m_fFinalDestX = m_fGetX + ( m_fFinalDestX - m_fGetX )*fResult;
							m_fFinalDestZ = m_fGetZ + ( m_fFinalDestZ - m_fGetZ )*fResult;
						}
					}
				}

				// chart new path to this permimeter position
				cNewPath.Clear ( );
				m_pContainer->pPathFinder->ActivateAllWaypoints ( );
				m_pContainer->pPathFinder->CalculatePath ( m_fGetX, m_fGetZ, m_fFinalDestX, m_fFinalDestZ, &cNewPath, fPathStartCostLimit, m_iDestContainer );
				if ( cNewPath.CountPoints( ) == 2 )
				{
					// if path has zero length, reduce to a single node
					if ( fabs ( m_cNewPath.GetPoint( 0 ).x - cNewPath.GetPoint( 1 ).x ) < 5.0f && fabs ( cNewPath.GetPoint( 0 ).y - cNewPath.GetPoint( 1 ).y ) < 5.0f )
						cNewPath.RemoveLast();
				}
			}

			// pass cNewPath back as result of this thread work
			m_cNewPath = cNewPath;
			m_iAIWorkComplete = m_iAIBeingWorkedOn;
		}
		else
		{
			//PE: This thread takes 70% of CPU time used even if it has nothing to do , so sleep a little.
			Sleep(1);
		}
	}
	m_bReadyToRun = true;
	return 0;
}

void LeeThread::StopRunning ( void )
{
	m_bKeepRunning = false;
}

bool LeeThread::IsReadyToRun ( void )
{
	return m_bReadyToRun;
}

void LeeThread::EndWork ( void )
{
	// remember the LeeThread is still running in a VERY TIGHT loop above
	m_iAIBeingWorkedOn = -1; // do this first to wipe out old job
	m_iAIWorkComplete = -1; // and only then can we declare the job done
}

void LeeThread::GetNewFinalDest ( float* pfFDX, float* pfFDY, float* pfFDZ )
{
	*pfFDX = m_fFinalDestX;
	*pfFDY = m_fFinalDestY;
	*pfFDZ = m_fFinalDestZ;
}
