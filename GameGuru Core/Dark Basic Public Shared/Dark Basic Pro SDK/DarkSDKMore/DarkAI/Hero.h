#ifndef H_HERO
#define H_HERO

#include ".\..\..\Shared\DBOFormat\DBOData.h"

class Team;
class Zone;
class Container;

//Hero - Human controlled component
class Hero
{

private:
	
	bool bIsDucking;
	Container *pContainer;

	float fHeight;

public:
	
	bool bActive;
	int iID;
	//int iTeam;			//may share a team with enities
	Team *pTeam;
	float fRadius;

	sObject* pObject;

	float fPosX, fPosY, fPosZ;
	float fAngY;

	GGVECTOR3 vecLastPos;
	
	Hero *pNextHero;	//Linked List
	
	Hero ( );
	Hero ( int id, sObject *pObj );
	~Hero ( );

	void SetRadius ( float fNewRadius );
	float GetRadius ( );

	void SetContainer ( Container *pNewContainer );
	Container *GetContainer ( );

	bool Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide );
	
	int GetID ( );

	float GetX ( );
	float GetY ( );
	float GetZ ( );
	float GetAngleY ( );
	float GetHeight( );

	bool GetIsDucking ( );

	void SetDucking ( bool bDuck );
	void SetPosition ( float x, float y, float z );
	void SetTeam ( Team *pNewTeam );

	void SetAngleY( float angY );
	void SetHeight( float height );

	void CheckZones ( Zone *pZoneList );
	void Update ( float fTimeDelta );

};

#endif