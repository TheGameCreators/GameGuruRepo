#include "CreateObject.h"
#include "CObjectsC.h"

void BT_Intern_Error(int);

// This is the function that creates everything.
sObject* BT_Intern_CreateBlankObject(DWORD ObjectId, DWORD FrameCount)
{
    CreateNewObject(ObjectId,"BlitzTerrain", FrameCount);
	sObject* Object = GetObjectData(ObjectId);
	return Object;
}

void BT_Intern_SetupMesh(sMesh* Mesh, DWORD VertexCount, DWORD IndexCount, DWORD Fvf)
{
	SetupMeshFVFData( Mesh, Fvf, VertexCount, IndexCount, false );
    Mesh->iPrimitiveType = GGPT_TRIANGLELIST;
    Mesh->iDrawVertexCount = VertexCount;
    Mesh->iDrawPrimitives = (IndexCount ? IndexCount : VertexCount)  / 3;
	return;
}

void BT_Intern_FinnishObject(unsigned long ObjectID)
{
    SetNewObjectFinalProperties(ObjectID, 0.0f);
	TextureObject(ObjectID,0);
	return;
}