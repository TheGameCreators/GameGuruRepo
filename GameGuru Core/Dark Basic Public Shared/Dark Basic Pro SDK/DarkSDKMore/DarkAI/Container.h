#ifndef H_CONTAINER
#define H_CONTAINER

class PathFinderAdvanced;

class Container
{

private:

	int iID;
	bool bActive;

public:

	Container *pNextContainer;
	PathFinderAdvanced		*pPathFinder;

	Container ( int id );
	~Container ( );

	int GetID ( );

	void Activate ( );
	void DeActivate ( );
	bool IsActive ( );
};

#endif