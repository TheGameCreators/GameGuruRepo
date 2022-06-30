#include "DBPro Functions.h"

int dbFreeObject ( )
{
	int i = 65536;

	do {
		i--;
	} while ( i > 0 && ObjectExist ( i ) == 1 );

	return i;
}

int dbFreeMesh ( )
{
	int i = 65536;

	do {
		i--;
	} while ( i > 0 && GetMeshExist ( i ) == 1 );

	return i;
}

int dbMakeEdgeMesh ( )
{
	int iTempObj = dbFreeObject ( );
	if ( iTempObj == 0 ) return 0;
	int iTempMesh = dbFreeMesh ( );
	if ( iTempMesh == 0 ) return 0;

	MakeObjectPlane ( iTempObj, 0.3f, 1.0f, 1 );
	XRotateObject ( iTempObj, 90.0f );
	FixObjectPivot ( iTempObj );
	MakeMeshFromObject ( iTempMesh, iTempObj );
	DeleteObject ( iTempObj );

	return iTempMesh;
}

int dbMakePointMesh ( )
{
	int iTempObj = dbFreeObject ( );
	if ( iTempObj == 0 ) return 0;
	int iTempMesh = dbFreeMesh ( );
	if ( iTempMesh == 0 ) return 0;

	MakeObjectSphere ( iTempObj, 2.0f, 2, 4 );
	MakeMeshFromObject ( iTempMesh, iTempObj );
	DeleteObject ( iTempObj );

	return iTempMesh;
}

void dbCombineLimbs ( int iObjID )
{
	int iTempMesh = dbFreeMesh ( );

	MakeMeshFromObject ( iTempMesh, iObjID );
	DeleteObject ( iObjID );
	MakeObject ( iObjID, iTempMesh, 0 );
	SetObjectMask ( iObjID, 1 );
	DeleteMesh ( iTempMesh );
}