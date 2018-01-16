#ifndef _CFDBP_H
#define _CFDBP_H

#include "windows.h"
#include "directx-macros.h"
#include "globstruct.h"
#include ".\..\..\Shared\camera\ccameradatac.h"
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include ".\..\..\Shared\error\cerror.h"

bool DBPRO_SetMeshRenderStates( sMesh* pMesh );
void DBPRO_ApplyEffect ( sMesh* pMesh, tagCameraData* m_Camera_Ptr );

#endif