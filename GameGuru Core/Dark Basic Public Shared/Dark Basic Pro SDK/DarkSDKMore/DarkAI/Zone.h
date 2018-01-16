#ifndef H_ZONE
#define H_ZONE

class Entity;
class Hero;
class Container;

class Zone
{

private:
	
	struct ZoneMember
	{
		Entity *pEntity;
		ZoneMember *pNextMember;

		ZoneMember ( ) { pEntity = 0; pNextMember = 0; }
		~ZoneMember ( ) { }
	};
	
	int iID;
	float fMinx, fMinz, fMaxx, fMaxz;
	Container *pContainer;

	ZoneMember *pMemberList;

public:

	Zone *pNextZone;

	Zone ( );
	Zone ( int id, float minx, float minz, float maxx, float maxz, Container *pInContainer );
	~Zone ( );

	int GetID ( );
	Container *GetContainer ( );

	void AddEntity ( Entity *pEntity );
	void RemoveEntity ( Entity *Entity ); 

	bool InZone ( float x, float z, Container *pEntityContainer );
	void NotifyEnter ( Entity *pEntityIn );
	void NotifyEnter ( Hero *pHeroIn );
	
	void NotifyLeave ( Entity *pEntityIOut );
	void NotifyLeave ( Hero *pHeroOut );
};

#endif