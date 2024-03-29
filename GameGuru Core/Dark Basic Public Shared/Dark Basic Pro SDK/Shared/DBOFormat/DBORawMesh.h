//
// DBORawMesh Functions Header
//

#ifndef _DBORAWMESH_H_
#define _DBORAWMESH_H_

#include "DBOFormat.h"

#include "preprocessor-flags.h"
#include "global.h"

// Mesh Load Functions

DARKSDK bool	LoadRawMesh			( LPSTR pFilename, sMesh** ppMesh );
DARKSDK bool	SaveRawMesh			( LPSTR pFilename, sMesh* pMesh );
DARKSDK bool	DeleteRawMesh		( sMesh* pMesh );

#endif _DBORAWMESH_H_
