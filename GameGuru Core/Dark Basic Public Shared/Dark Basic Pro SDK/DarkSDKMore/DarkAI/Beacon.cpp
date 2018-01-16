
#include "Beacon.h"
#include "Container.h"
#include "DBPro Functions.h"

int Beacon::iObjID = 0;
float Beacon::fObjHeight = 0.0f;

Beacon::Beacon ( )
{
	iType = -1;
	fLife = 0.0f;
	iExtraInfo = 0;
	fSoundSize = 0;
	vecPos.x = 0;
	vecPos.y = 0;
	vecPos.z = 0;
	pContainer = 0;
	pNextBeacon = 0;
}

Beacon::Beacon ( float fNewLife )
{
	iType = -1;
	fLife = fNewLife;
	iExtraInfo = 0;
	fSoundSize = 0;
	vecPos.x = 0;
	vecPos.y = 0;
	vecPos.z = 0;
	pContainer = 0;
	pNextBeacon = 0;
}

Beacon::~Beacon ( ) { }

void Beacon::SetPosition ( float fX, float fY, float fZ )
{
	vecPos.x = fX;
	vecPos.y = fY;
	vecPos.z = fZ;
}

void Beacon::SetSound ( int iUrgency, float fSize, Container *pInContainer )
{
	iType = 0;
	iExtraInfo = iUrgency;
	fSoundSize = fSize;
	pContainer = pInContainer;
}

void Beacon::SetAlert ( int iInfo )
{
	iType = 10;
	iExtraInfo = iInfo;
}

void Beacon::SetOther ( int iType, int iInfo )
{
	iType = 11;
	iExtraInfo = iInfo;
}

bool Beacon::IsOld ( float fTimeDelta )
{
	fLife -= fTimeDelta;

	return fLife <= 0;
}

bool Beacon::IsSound ( )
{
	return ( iType >= 0 && iType <=2 );
}

bool Beacon::IsAlert ( )
{
	return ( iType == 10 );
}

bool Beacon::IsOther ( )
{
	return ( iType == 11 );
}

float Beacon::GetSoundSize ( )
{
	return fSoundSize;
}

Container* Beacon::GetContainer ( )
{
	return pContainer;
}

void Beacon::DebugAddLimb ( int iLimb, int iMesh )
{
	AddLimb ( iObjID, iLimb, iMesh );
	OffsetLimb ( iObjID, iLimb, vecPos.x, 0.0f, vecPos.z );
}