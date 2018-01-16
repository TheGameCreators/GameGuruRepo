//
// Collision Functions Implementation
//

// Includes
#include "collision.h"
#include "..\\CommonC.h"

// Global Collision Data
float *pool = NULL;		// Triangle buffer holding vertices
CollisionTrace *trace;	// Holds the collision information

// Functions

void Collision::Init ( void )
{
	// prepare collision flags
	float temp[3]={0,0,0};
	colEnableGravity(false);
	colEnableBackCulling(false);
	colEnableResponse(true);
	colSetUnitsPerMeter(200.0f);
	colSetGravity((float*)temp);
	///colSetTrianglePool(g_pUniverse->m_dwCollisionPoolIndex/9, g_pUniverse->m_pCollisionPool, 0);
}

BOOL Collision::World ( GGVECTOR3 starto, GGVECTOR3 endo, GGVECTOR3 eRadius, float fScale )
{
	// used to store final collision feedback
	bool bCollisionDetected = false;

	// U75 - 150610 - reset so this value remains fresh and reflective of current material after each collision call
	g_DBPROCollisionResult.dwArbitaryValue = 0;

	// shrink volume to ColE space
	GGVECTOR3 start, end;
	start.x = starto.x / eRadius.x;
	start.y = starto.y / eRadius.y;
	start.z = starto.z / eRadius.z;
	end.x = endo.x / eRadius.x;
	end.y = endo.y / eRadius.y;
	end.z = endo.z / eRadius.z;

	// in addition to radius from pre-reduced ColE data, have a scale value for crouch!
	float radius[3]={1,fScale,1};
	colSetRadius((float*)radius);

	// find collision
	float point[3];
	float vel[3];
	point[0]=start.x;point[1]=start.y;point[2]=start.z;
	vel[0]=end.x-start.x;
	vel[1]=end.y-start.y;
	vel[2]=end.z-start.z;
	trace=collideAndSlide(point, vel);

	// fill collision feedback data to result
	if ( trace->foundCollision )
	{
		// collision was detected
		bCollisionDetected = true;

		// expand volume out of ColE space
		GGVECTOR3 finalPos = GGVECTOR3 ( trace->finalPosition[0], trace->finalPosition[1], trace->finalPosition[2] );
		start.x *= eRadius.x;
		start.y *= eRadius.y;
		start.z *= eRadius.z;
		finalPos.x *= eRadius.x;
		finalPos.y *= eRadius.y;
		finalPos.z *= eRadius.z;

		// basic collision feedback
		g_DBPROCollisionResult.vecPos = finalPos;
		g_DBPROCollisionResult.vecDifference = finalPos - start;

		// leecatch - 220506 - u62 - catch a QNAN assign, by using the old camera positions
		if ( g_DBPROCollisionResult.vecDifference.x<=0.0f || g_DBPROCollisionResult.vecDifference.x>0.0f )
		{
			// a valid difference value returned
		}
		else
		{
			// otherwise a NAN produced - useful for debugging
			int leefounderror=1;
		}

		g_DBPROCollisionResult.iPolysChecked = 0;
		if ( trace->triangleindex>=0 ) g_DBPROCollisionResult.dwArbitaryValue = 0;
		g_DBPROCollisionResult.bCollidedAgainstFloor = trace->collidedagainstfloor;
	}

	// return if collision was made
	return bCollisionDetected;
}
