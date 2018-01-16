#ifndef H_DBPRO_FUNC
#define H_DBPRO_FUNC

#include ".\..\..\Shared\DBOFormat\DBOData.h"

#include "CObjectsC.h"
#include "CMemblocks.h"

int dbFreeObject ( );
int dbFreeMesh ( );
int dbMakeEdgeMesh ( );
int dbMakePointMesh ( );
void dbCombineLimbs ( int iObjID );

#endif