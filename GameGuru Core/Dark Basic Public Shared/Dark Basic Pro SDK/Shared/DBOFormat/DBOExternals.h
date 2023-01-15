// DBOExternals Functions Header
#ifndef _DBOEXTERNALS_H_
#define _DBOEXTERNALS_H_

#include "preprocessor-flags.h"

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include "directx-macros.h"
#include "global.h"
#include "globstruct.h"
#include "..\\error\\cerror.h"

extern LPGGDEVICE			m_pD3D;
extern GlobStruct*			g_pGlob;

#endif _DBOEXTERNALS_H_

DARKSDK void AnglesFromMatrix ( GGMATRIX* pmatMatrix, GGVECTOR3* pVecAngles );
extern void ScalesFromMatrix ( GGMATRIX* pmatMatrix, GGVECTOR3* pVecScales );
extern void DirectionsFromMatrix ( GGMATRIX* pmatMatrix, GGVECTOR3* pVecDir );
