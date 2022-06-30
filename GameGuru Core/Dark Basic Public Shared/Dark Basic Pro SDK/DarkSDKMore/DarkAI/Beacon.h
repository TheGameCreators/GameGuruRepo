#ifndef H_BEACON
#define H_BEACON

#include ".\..\..\Shared\DBOFormat\DBOData.h"

class Container;

class Beacon
{
private:
	int iType;				//the type of beacon ( unexplained noise, radio command, ... )
	float fLife;			//how long till it disappears (in seconds)

public:
	static int iObjID;
	static float fObjHeight;

	GGVECTOR3 vecPos;		//a position associated with this beacon (e.g. noise pos)
	int iExtraInfo;			//additional infomation to share with entities receiving the beacon
	float fSoundSize;		//volume or radius of sound
	Container* pContainer;	//the container this beacon belongs to, if any
	Beacon *pNextBeacon;	//linked list

	Beacon ( );
	Beacon ( float fNewLife );
	~Beacon ( );

	void SetPosition ( float fX, float fY, float fZ );
	void SetSound ( int iUrgency, float fSize, Container *pInContainer );
	void SetHiding ( int iTeam );
	void SetAlert ( int iInfo );
	void SetOther ( int iType, int iInfo );

	bool IsOld ( float fTimeDelta );
	bool IsSound ( );
	bool IsAlert ( );
	bool IsOther ( );

	float GetSoundSize ( );
	Container* GetContainer( );

	void DebugAddLimb ( int iLimb, int iMesh );
};

#endif