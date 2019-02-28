//----------------------------------------------------
//--- GAMEGURU - M-Entity
//----------------------------------------------------

#include "gameguru.h"

// Globals for blacklist string array
LPSTR* g_pBlackList = NULL;
int g_iBlackListMax = 0;
bool g_bBlackListRemovedSomeEntities = false;

// Externs
//extern int g_iFBXGeometryToggleMode;
//extern int g_iFBXGeometryCenterMesh;

// prototypes
void LoadFBX ( LPSTR szFilename, int iID );

// 
//  ENTITY COMMON CODE (not game specific)
// 

void entity_addtoselection_core ( void )
{
	//  ensure ENT$ does not contain duplicate \ symbols
	t.tnewent_s="";
	for ( t.n = 1 ; t.n<=  Len(t.ent_s.Get()); t.n++ )
	{
		t.tnewent_s=t.tnewent_s+Mid(t.ent_s.Get(),t.n);
		if (  cstr(Mid(t.ent_s.Get(),t.n)) == "\\" && cstr(Mid(t.ent_s.Get(),t.n+1)) == "\\" ) 
		{
			++t.n;
		}
	}
	t.ent_s=t.tnewent_s;

	//  Load entity from file
	t.entdir_s="entitybank\\";
	t.ent_s=Right(t.ent_s.Get(),1+(Len(t.ent_s.Get())-(Len(g.rootdir_s.Get())+Len(t.entdir_s.Get()))));

	//  Check if filename valid
	t.entnewloaded=0 ; t.entid=0;
	if (  cstr(Right(t.ent_s.Get(),4)) == ".fpe" ) 
	{
		//  Check entity exists in bank
		t.tokay=1;
		if (  g.entidmaster>0 ) 
		{
			for ( t.entid = 1 ; t.entid<=  g.entidmaster; t.entid++ )
			{
				if (  t.entitybank_s[t.entid] == t.ent_s ) {  t.tokay = 0  ; t.tfoundid = t.entid ; break; }
			}
		}
		if (  t.tokay == 1 ) 
		{
			//  Find Free entity Index
			t.freeentid=-1;
			if (  g.entidmaster>0 ) 
			{
				for ( t.entid = 1 ; t.entid <= g.entidmaster; t.entid++ )
				{
					if (  t.entityprofileheader[t.entid].desc_s == "" ) {  t.freeentid = t.entid  ; break; }
				}
			}

			//  New entity or Free One
			if (  t.freeentid == -1 ) 
			{
				++g.entidmaster ; entity_validatearraysize ( );
				t.entid=g.entidmaster;
				t.entnewloaded=1;
			}
			else
			{
				t.entid=t.freeentid;
			}

			//  Load entity
			t.entitybank_s[t.entid]=t.ent_s;
			t.entpath_s=getpath(t.ent_s.Get());
			entity_load ( );
		}
		else
		{
			//  already got, assign ID from existing
			t.entid=t.tfoundid;
		}
	}

}

void entity_addtoselection ( void )
{
	//  Load entity from file requester
	SetDir (  g.currententitydir_s.Get() );
	t.ent_s=browseropen_s(9);
	g.currententitydir_s=GetDir();
	SetDir (  g.rootdir_s.Get() );
	entity_addtoselection_core ( );
}

void entity_adduniqueentity ( bool bAllowDuplicates )
{
	//  Ensure 'entitybank\' is not part of entity filename
	t.entdir_s="entitybank\\";
	if (  cstr(Lower(Left(t.addentityfile_s.Get(),11))) == "entitybank\\" ) 
	{
		t.addentityfile_s=Right(t.addentityfile_s.Get(),Len(t.addentityfile_s.Get())-11);
	}
	if (  cstr(Lower(Left(t.addentityfile_s.Get(),8))) == "ebebank\\" ) 
	{
		t.entdir_s = "";
	}

	//  Check if entity already loaded in
	t.talreadyloaded=0;
	if ( bAllowDuplicates == false )
	{
		for ( t.t = 1 ; t.t<=  g.entidmaster; t.t++ )
		{
			if (  t.entitybank_s[t.t] == t.addentityfile_s ) {  t.talreadyloaded = 1  ; t.entid = t.t; }
		}
	}
	if (  t.talreadyloaded == 0 ) 
	{
		//  Allocate one more entity item in array
		if (  g.entidmaster>g.entitybankmax-4 ) 
		{
			Dim (  t.tempentitybank_s,g.entitybankmax  );
			for ( t.t = 0 ; t.t<=  g.entitybankmax ; t.t++ ) t.tempentitybank_s[t.t]=t.entitybank_s[t.t] ; 
			++g.entitybankmax;
			UnDim (  t.entitybank_s );
			Dim (  t.entitybank_s,g.entitybankmax  );
			for ( t.t = 0 ; t.t<= g.entitybankmax-1 ; t.t++ ) t.entitybank_s[t.t]=t.tempentitybank_s[t.t] ;
		}

		//  Add entity to bank
		++g.entidmaster ; entity_validatearraysize ( );
		t.entitybank_s[g.entidmaster]=t.addentityfile_s;

		//  Load extra entity
		t.entid=g.entidmaster;
		t.ent_s=t.entitybank_s[t.entid];
		t.entpath_s=getpath(t.ent_s.Get());
		entity_load ( );

		// 090317 - ignore ebebank new structure to avoid empty EBE icons being added to local library left list
		if ( stricmp ( t.addentityfile_s.Get(), "..\\ebebank\\_builder\\New Site.fpe" ) == NULL )
			t.talreadyloaded = 1;
	}
}

void entity_validatearraysize ( void )
{
	//  ensure enough space in entity profile arrays
	if (  g.entidmaster+32>g.entidmastermax ) 
	{
		g.entidmastermax=g.entidmaster+32;
		Dim2(  t.entitybodypart,g.entidmastermax, 100   );
		Dim2(  t.entityappendanim,g.entidmastermax, 100  );
		Dim2(  t.entityanim,g.entidmastermax, g.animmax   );
		Dim2(  t.entityfootfall,g.entidmastermax, g.footfallmax  );
		Dim (  t.entityprofileheader,g.entidmastermax   );
		Dim (  t.entityprofile,g.entidmastermax  );
		Dim2(  t.entitydecal_s,g.entidmastermax, 100  );
		Dim2(  t.entitydecal,g.entidmastermax, 100   );
		Dim2(  t.entityblood,g.entidmastermax, BLOODMAX  );
		g.entitybankmax=g.entidmastermax;
		Dim (  t.entitybank_s,g.entidmastermax  );
	}
}


//PE: GenerateD3D9ForMesh - make sure semantic is stored in old D3D9 format.
//PE: Without get fvf offset can fail , and original skin weight is not used but generated , this can give animation problems.
//PE: This is not a problem when using the importer, as it will save everything in the old D3D9 format into the dbo.
void GenerateD3D9ForMesh(sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bBinormals, BOOL bDiffuse, BOOL bBones)
{
	// get FVF details
	sOffsetMap offsetMap;
	GetFVFValueOffsetMap(pMesh->dwFVF, &offsetMap);

	// deactivate bone flag if no bones in source mesh
	if (pMesh->dwBoneCount == 0) bBones = FALSE;

	// valid mesh (no longer using DXMESH)
	if (pMesh->dwFVF > 0)
	{
		// extract vertex size from mesh
		WORD wNumBytesPerVertex = (WORD)pMesh->dwFVFSize;

		// Starting declaration
		int iDeclarationIndex = 0;
		D3D11_INPUT_ELEMENT_DESC pDeclaration[12];

		// check if mesh already has a component (and build declaration)
		BOOL bHasNormals = FALSE;
		BOOL bHasDiffuse = FALSE;
		BOOL bHasTangents = FALSE;
		BOOL bHasBinormals = FALSE;
		BOOL bHasBlendWeights = FALSE;
		BOOL bHasBlendIndices = FALSE;
		BOOL bHasSecondaryUVs = FALSE;
		if (pMesh->dwFVF & GGFVF_XYZ)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "POSITION";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = 0;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
		}
		if (pMesh->dwFVF & GGFVF_NORMAL)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "NORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasNormals = TRUE;
		}
		if (pMesh->dwFVF & GGFVF_TEX1)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasDiffuse = TRUE;
		}
		if (pMesh->dwFVF & GGFVF_DIFFUSE)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "COLOR";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasDiffuse = TRUE;
		}
		if (pMesh->dwFVF & offsetMap.dwTU[1] > 0)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 1;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasSecondaryUVs = TRUE;
		}

		if (!bHasNormals && bNormals)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "NORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 12;
		}
		if (!bHasDiffuse && bDiffuse)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "COLOR";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 4;
		}
		if (!bHasTangents && bTangents)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TANGENT";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 12;
		}
		if (!bHasBinormals && bBinormals)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "BINORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 12;
		}
		DWORD dwOffsetToWeights = wNumBytesPerVertex;
		if (!bHasBlendWeights && bBones)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 1;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 16;
		}
		DWORD dwOffsetToIndices = wNumBytesPerVertex;
		if (!bHasBlendIndices && bBones)
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 2;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex += 16;
		}

		// copy declaration into old D3D9 format (as DBO relies on this data in the binary!)
		int iDecIndex = 0;
		int iByteOffset = 0;
		for (; iDecIndex < iDeclarationIndex; iDecIndex++)
		{
			int iEntryByteSize = 0;
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "POSITION") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_POSITION;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "NORMAL") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_NORMAL;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "COLOR") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_COLOR;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT2;
				iEntryByteSize = 4;
			}
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "TANGENT") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_TANGENT;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "BINORMAL") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_BINORMAL;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if (stricmp(pDeclaration[iDecIndex].SemanticName, "TEXCOORD") == NULL)
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_TEXCOORD;
				if (pDeclaration[iDecIndex].Format == DXGI_FORMAT_R32G32B32A32_FLOAT)
				{
					pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT4;
					iEntryByteSize = 16;
				}
				else
				{
					pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT2;
					iEntryByteSize = 8;
				}
			}
			pMesh->pVertexDeclaration[iDecIndex].Stream = 0;
			pMesh->pVertexDeclaration[iDecIndex].Method = GGDECLMETHOD_DEFAULT;
			pMesh->pVertexDeclaration[iDecIndex].UsageIndex = pDeclaration[iDecIndex].SemanticIndex;
			pMesh->pVertexDeclaration[iDecIndex].Offset = iByteOffset;
			iByteOffset += iEntryByteSize;
		}
		pMesh->pVertexDeclaration[iDecIndex].Stream = 255;
	}
}


void entity_load ( void )
{
	//  Activate auto generation of mipmaps for ALL entities
	SetImageAutoMipMap (  1 );

	//  Entity Object Index
	t.entobj=g.entitybankoffset+t.entid;

	//  debug info
	t.mytimer=Timer();

	//  Load Entity profile data
	entity_loaddata ( );

	//  Only load characters for entity-local-testing
	t.desc_s=t.entityprofileheader[t.entid].desc_s;
	if (  t.scanforentitiescharactersonly == 1 ) 
	{
		if (  t.entityprofile[t.entid].ischaracter == 0 ) 
		{
			t.desc_s="";
		}
	}

	//  Special mode (from lightmapper) which only loads static entities
	//  which will speed up lightmapping process and reduce hit on system memory
	//  not excluding markers as we need some of them for lighting info
	if (  t.lightmapper.onlyloadstaticentitiesduringlightmapper == 1 ) 
	{
		if (  t.entityprofile[t.entid].ischaracter == 1 ) 
		{
			t.desc_s="";
		}
	}

	//  Only if profile data exists
	if (  t.desc_s != "" ) 
	{
		//  Load the model
		if (  t.entityprofile[t.entid].ischaractercreator  ==  0 ) 
		{
			t.tfile_s=t.entdir_s+t.entpath_s+t.entityprofile[t.entid].model_s;
		}
		else
		{
			t.tfile_s=t.entityprofile[t.entid].model_s;
		}
		deleteOutOfDateDBO(t.tfile_s.Get());
		// if .X or .FBX file specified, and DBO exists, load DBO as main model file
		int iSrcFormat = 0;
		if ( strcmp ( Lower(Right(t.tfile_s.Get(),2)) , ".x" ) == 0 ) iSrcFormat = 1;
		if ( strcmp ( Lower(Right(t.tfile_s.Get(),4)) , ".fbx" ) == 0 ) iSrcFormat = 2;
		if ( iSrcFormat > 0 ) 
		{ 
			if ( iSrcFormat == 1 )
			{
				// X File Format
				t.tdbofile_s = Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-2); t.tdbofile_s += ".dbo"; 
				if ( t.tdbofile_s != "" && FileExist(t.tdbofile_s.Get()) == 1 ) t.tfile_s = t.tdbofile_s;
			}
			if ( iSrcFormat == 2 )
			{
				// FBX File Format
				t.tdbofile_s = Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4); t.tdbofile_s += ".dbo"; 
				if ( t.tdbofile_s != "" && FileExist(t.tdbofile_s.Get()) == 1 ) t.tfile_s = t.tdbofile_s;
			}
		} 
		else 
		{
			// if .X or .FBX file NOT specified
			t.tdbofile_s = "";
			if ( strcmp ( Lower(Right(t.tfile_s.Get(),4)) , ".dbo" ) == 0  ) 
			{
				// and .DBO specified instead, check if .DBO does not exist
				if ( FileExist(t.tfile_s.Get()) == 0 )
				{
					// if not exist try .X model file (typical of model import entities that use original X file)
					// but do not copy the DBO file with it
					t.tfile_s = Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4); t.tfile_s += ".x"; 
				}
			}
		}
		// get path to original model
		char pModelPath[10248];
		strcpy ( pModelPath, "" );
		LPSTR pOrigModelFilename = t.tfile_s.Get();
		for ( int n = strlen(pOrigModelFilename) ; n > 0; n-- )
		{
			if ( pOrigModelFilename[n] == '\\' || pOrigModelFilename[n] == '/' )
			{
				strcpy ( pModelPath, pOrigModelFilename );
				pModelPath[n+1] = 0;
				break;
			}
		}
		// 070718 - if append final file exists, use that
		bool bUsingAppendAnimFileModel = false;
		cstr pAppendFinalModelFilename = t.entityappendanim[t.entid][0].filename;
		if ( strlen(pAppendFinalModelFilename.Get()) > 0 )
		{
			pAppendFinalModelFilename = cstr(pModelPath) + pAppendFinalModelFilename;
			if ( FileExist(pAppendFinalModelFilename.Get()) == 1 ) 
			{
				bUsingAppendAnimFileModel = true;
				t.tfile_s = pAppendFinalModelFilename;
				pAppendFinalModelFilename = "";
				t.tdbofile_s = "";
			}
		}
		if ( FileExist(t.tfile_s.Get()) == 0 )
		{
			t.tfile_s=t.entityprofile[t.entid].model_s;
			//  V109 BETA6 - 290408 - allow DBO creation/read if full relative path provides (i.e. meshbank\scifi\etc)
			if ( strcmp ( Lower(Right(t.tfile_s.Get(),2)) , ".x" ) == 0  ) { t.tdbofile_s = Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-2); t.tdbofile_s += ".dbo"; } else t.tdbofile_s = "";
			if ( t.tdbofile_s != "" && FileExist(t.tdbofile_s.Get()) == 1  )  t.tfile_s = t.tdbofile_s;
		}
		t.txfile_s=t.tfile_s;
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			//  if DBO version exists, use that (quicker)
			if (  FileExist(t.tdbofile_s.Get()) == 1 ) 
			{
				t.tfile_s=t.tdbofile_s;
				t.tdbofile_s="";
			}
			else
			{
				//  allowed to save DBO (once only)
			}

			//  Load entity (compile does not need the dynamic objects)
			if (  t.entobj>0 ) 
			{
				if (  ObjectExist(t.entobj) == 0 ) 
				{
					//  load entity model
					if (  t.entityprofile[t.entid].fullbounds == 1  )  SetFastBoundsCalculation (  0 );
					if ( strnicmp ( t.tfile_s.Get() + strlen(t.tfile_s.Get()) - 4, ".fbx", 4 )==NULL )
					{
						//int iStoreFBXGeometryToggleMode = g_iFBXGeometryToggleMode;
						//int iStoreFBXGeometryCenterMesh = g_iFBXGeometryCenterMesh;
						//g_iFBXGeometryToggleMode = 0;
						//g_iFBXGeometryCenterMesh = 0;
						LoadFBX ( t.tfile_s.Get(), t.entobj );
						//g_iFBXGeometryToggleMode = iStoreFBXGeometryToggleMode;
						//g_iFBXGeometryCenterMesh = iStoreFBXGeometryCenterMesh;
					}
					else
						LoadObject ( t.tfile_s.Get(), t.entobj );

					// 060718 - append animation data from other DBO files
					if ( bUsingAppendAnimFileModel == false )
					{
						if ( Len(t.tdbofile_s.Get()) == 0 )
						{
							if ( t.entityprofile[t.entid].appendanimmax > 0 )
							{
								for ( int aa = 1 ; aa <= t.entityprofile[t.entid].appendanimmax; aa++ )
								{
									cstr pAppendModelFilename = cstr(pModelPath) + t.entityappendanim[t.entid][aa].filename;
									int iStartFrame = t.entityappendanim[t.entid][aa].startframe;
									AppendObject ( (DWORD)(LPSTR)pAppendModelFilename.Get(), t.entobj, iStartFrame );
								}
							}
						}
					}

					// wipe ANY material emission colors
					SetObjectEmissive ( t.entobj, 0 );

					// prepare properties
					SetFastBoundsCalculation (  1 );
					SetObjectFilter (  t.entobj,2 );
					SetObjectCollisionOff (  t.entobj );

					//  if strictly NON-multimaterial, convert now
					if (  t.entityprofile[t.entid].onetexture == 1 ) 
					{
						if (  GetMeshExist(g.meshgeneralwork) == 1  )  DeleteMesh (  g.meshgeneralwork );
						MakeMeshFromObject (  g.meshgeneralwork,t.entobj );
						DeleteObject (  t.entobj );
						MakeObject (  t.entobj,g.meshgeneralwork,0 );
					}

					// 011215 - if specified, we can smooth the model before we use it (concrete pipe in TBE level)
					float fSmoothingAngleOrFullGenerate = t.entityprofile[t.entid].smoothangle;
					if ( fSmoothingAngleOrFullGenerate > 0 )
					{
						// 090217 - this only works on orig X files (not subsequent DBO) as they change 
						// the mesh which is then saved out (below)
						if ( Len(t.tdbofile_s.Get()) > 1 ) 
						{
							// 090216 - a special mode of over 101 will flip normals for the object (when normals are bad)
							if ( fSmoothingAngleOrFullGenerate >= 101.0f ) 
							{ 
								//SetObjectNormalsEx ( t.entobj, 1 ); // will correct objects with flipped normals
								SetObjectNormalsEx ( t.entobj, 0 ); // will generate new smooth normals for object
								fSmoothingAngleOrFullGenerate -= 101.0f;
							}

							// and if smoothing factor required, weld some of them together
							if ( fSmoothingAngleOrFullGenerate > 0.0f )
							{
								SetObjectSmoothing ( t.entobj, fSmoothingAngleOrFullGenerate );
							}
						}
					}
				}
			}

			//  loaded okay
			if (  ObjectExist(t.entobj) == 1 ) 
			{
				// 070718 - if append final model needs to be created, prefer that
				if ( strlen(pAppendFinalModelFilename.Get()) > 0 )
					if ( FileExist(pAppendFinalModelFilename.Get()) == 0 ) 
						t.tdbofile_s = pAppendFinalModelFilename;

				// Save if DBO not exist for entity (for fast loading)
				if ( Len(t.tdbofile_s.Get()) > 1 ) 
				{
					// ensure legacy compatibility (avoids new mapedito crashing build process)
					if ( FileExist(t.tdbofile_s.Get()) == 1 )  DeleteFile ( t.tdbofile_s.Get() );
					SaveObject ( t.tdbofile_s.Get(), t.entobj );
					if (  FileExist(t.tdbofile_s.Get()) == 1 ) 
					{
						DeleteObject (  t.entobj );
						LoadObject (  t.tdbofile_s.Get(),t.entobj );
						SetObjectFilter (  t.entobj,2 );
						SetObjectCollisionOff (  t.entobj );
					}
				}

		
				// 300817 - if an EBE object with no .EBE file, remove handle from entity
				if ( t.entityprofile[t.entid].isebe == 2 )
					if ( ObjectExist ( t.entobj ) == 1 )
						if ( LimbExist ( t.entobj, 0 ) == 1 )
							ChangeMesh ( t.entobj, 0, 0 );

				// Special matrix transform mode for FBX and similar models
				SetObjectRenderMatrixMode ( t.entobj, t.entityprofile[t.entid].matrixmode );

				//  XYZ=0x002 and NORMAL=0x010 and 1UV=0x100
				if (  t.entityprofile[t.entid].skipfvfconvert == 0 ) 
				{
					// lee - 300714 - seems to screw up Zombie models somehow, does it screw up rest of engine commenting it out?
					// PE: zombie problems could be the missing skin weight like below, did not test this.
					// PE: perhaps we can streamline this now , so skipfvfconvert is not needed :)

					CloneMeshToNewFormat(t.entobj, 0x002 + 0x010 + 0x100);
				}
				else {
					//PE: make sure we use the correct FVF. even when using skipfvfconvert=1
					DWORD dwRequiredFVF = 0x002 + 0x010 + 0x100;
					sObject* pObject = g_ObjectList[t.entobj];
					for (int iMeshIndex = 0; iMeshIndex<pObject->iMeshCount; iMeshIndex++)
					{
						sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
						if (pMesh->dwFVF != dwRequiredFVF)
						{
							ConvertToFVF(pMesh, dwRequiredFVF);
						}
					}
				}

				//PE: We are missing skin weight/others in old DX9 DBO setup. needed for some functions.
				//PE: prevent generation of vertex weight that screw up some animations.
				sObject* pObject = g_ObjectList[t.entobj];
				for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++)
				{
					sMesh* pMesh = pObject->ppMeshList[iMesh];
					GenerateD3D9ForMesh(pMesh, true, true, true, true, true);
				}

				// 131115 - fixes issue of some models not being able to detect with intersectall
				SetObjectDefAnim ( t.entobj, t.entityprofile[t.entid].ignoredefanim );

				//  the reverse can be used to allow transparent limbs to be rendered last
				if (  t.entityprofile[t.entid].reverseframes == 1 ) 
				{
					ReverseObjectFrames (  t.entobj );
				}

				//  main profile object adjustments
				if (  t.entityprofile[t.entid].scale != 0  )  ScaleObject (  t.entobj,t.entityprofile[t.entid].scale,t.entityprofile[t.entid].scale,t.entityprofile[t.entid].scale );

				//  Apply texture and effect to entity profile obj
				entity_loadtexturesandeffect ( );

				//  until static bonemodel scales when not animating, loop
				if (  t.entityprofile[t.entid].ischaracter == 1 ) 
				{
					//  refresh using loop stop to get correct character pose
					LoopObject (  t.entobj  ); StopObject (  t.entobj );
					//  pivot character to face right way
					RotateObject (  t.entobj,0,180,0  ); FixObjectPivot (  t.entobj );
				}

				//  if entity uses a handle, create and attach it now
				t.entityprofile[t.entid].addhandlelimb=0;
				if (  Len(t.entityprofile[t.entid].addhandle_s.Get())>1 ) 
				{
					t.thandlefile_s=t.entdir_s+t.entpath_s+"\\"+t.entityprofile[t.entid].addhandle_s;
					if (  FileExist(t.thandlefile_s.Get()) == 1 ) 
					{
						if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
						MakeObjectPlane (  g.entityworkobjectoffset,50,50 );
						RotateObject (  g.entityworkobjectoffset,90,0,0 );
						ScaleObject (  g.entityworkobjectoffset,50,50,50 );
						if (  GetMeshExist(g.entityworkobjectoffset) == 1  )  DeleteMesh (  g.entityworkobjectoffset );
						MakeMeshFromObject (  g.entityworkobjectoffset,g.entityworkobjectoffset );
						PerformCheckListForLimbs (  g.entityworkobjectoffset );
						t.entityprofile[t.entid].addhandlelimb = 1+ChecklistQuantity();
						AddLimb (  t.entobj,t.entityprofile[t.entid].addhandlelimb,g.entityworkobjectoffset );
						LinkLimb (  t.entobj,0,t.entityprofile[t.entid].addhandlelimb );
						t.tyoffset_f=(t.entityprofile[t.entid].defaultheight/((t.entityprofile[t.entid].scale+0.0)/100.0))*-1;
						OffsetLimb (  t.entobj,t.entityprofile[t.entid].addhandlelimb,0,2+t.tyoffset_f,0 );
						t.texhandleid=loadinternaltexture(t.thandlefile_s.Get());
						TextureLimb (  t.entobj,t.entityprofile[t.entid].addhandlelimb,t.texhandleid );
						SetLimbEffect (  t.entobj,t.entityprofile[t.entid].addhandlelimb,t.entityprofile[t.entid].usingeffect );
						if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
						if (  GetMeshExist(g.entityworkobjectoffset) == 1  )  DeleteMesh (  g.entityworkobjectoffset );
					}
				}

				//  Parent LOD is not enabled, we use clone and instance
				//  settings as we need per-entity element distance control

				if ( t.entityprofile[t.entid].isebe == 0 )
				{
					// only set material if not EBE, as EBE objects carry per-mesh material values
					SetObjectArbitaryValue (  t.entobj,t.entityprofile[t.entid].materialindex );
				}

				//  if entity has decals, find indexes to decals (which are already preloaded)
				t.entityprofile[t.entid].bloodscorch=0;
				if (  t.entityprofile[t.entid].decalmax>0 ) 
				{
					for ( t.tq = 0 ; t.tq<=  t.entityprofile[t.entid].decalmax-1; t.tq++ )
					{
						t.decal_s=t.entitydecal_s[t.entid][t.tq];
						if (  strcmp ( Lower(t.decal_s.Get()) , "blood" ) == 0  )  t.entityprofile[t.entid].bloodscorch = 1;
						decal_find ( );
						if (  t.decalid<0  )  t.decalid = 0;
						t.entitydecal[t.entid][t.tq]=t.decalid;
					}
				}

				//  add in character creator objects if needed
				if (  t.entityprofile[t.entid].ischaractercreator  ==  1 ) 
				{
					characterkit_loadEntityProfileObjects ( );
				}

				//  HideObject (  away )
				PositionObject (  t.entobj,100000,100000,100000 );

				//  Set radius of zero allows parent to animate even if outside of frustrum view
				if (  GetNumberOfFrames(t.entobj)>0 ) 
				{
					//  but ONLY for animating objects, do not need to run parent objects if still
					SetSphereRadius (  t.entobj,0 );
				}

			}
		}
		else
		{
			//  prevent crash when model name wrong/geometry file missing/etc
			MakeObjectSphere (  t.entobj,1 );
			PositionObject (  t.entobj,100000,100000,100000 );
		}

		//  work out if this is in the bitbob system (1 or -1 automatic (decided based on size) and precalcualte bitbob fade distances
		if (  t.entityprofile[t.entid].ischaracter == 1 && t.entityprofile[t.entid].bitbobon == -1 ) 
		{
			//  characters need to remain visible for sniping
			t.entityprofile[t.entid].bitbobon=0;
		}
		if (  t.entityprofile[t.entid].bitbobon  !=  0 && t.entityprofile[t.entid].ismarker  ==  0 ) 
		{
			t.tLargestSide_f = ObjectSizeX(t.entobj,1);
			if (  ObjectSizeY(t.entobj,1) > t.tLargestSide_f  )  t.tLargestSide_f  =  ObjectSizeY(t.entobj,1);
			if (  ObjectSizeZ(t.entobj,1) > t.tLargestSide_f  )  t.tLargestSide_f  =  ObjectSizeZ(t.entobj,1);
			if (  t.tLargestSide_f < BITBOBS_DEFAULT_MAXSIZE  )  t.entityprofile[t.entid].bitbobon  =  1;
			if (  t.entityprofile[t.entid].bitbobon  ==  1 ) 
			{
				t.entityprofile[t.entid].bitbobnear_f = t.tLargestSide_f * BITBOBS_DEFAULT_FADEIN * t.entityprofile[t.entid].bitbobdistweight_f;
				t.entityprofile[t.entid].bitbobfar_f = t.tLargestSide_f * BITBOBS_DEFAULT_FADEOUT * t.entityprofile[t.entid].bitbobdistweight_f;
			}
		}

		//  must hide parent objects
		HideObject (  t.entobj );

		//  Resolve default weapon gun ids
		if (  t.entityprofile[t.entid].isweapon_s != "" ) 
		{
			t.findgun_s=Lower(t.entityprofile[t.entid].isweapon_s.Get()) ; gun_findweaponindexbyname ( );
			t.entityprofile[t.entid].isweapon=t.foundgunid;
			if (  t.foundgunid>0  )  t.gun[t.foundgunid].activeingame = 1;
		}
		else
		{
			t.entityprofile[t.entid].isweapon=0;
		}

		//  Finding hasweapon also in createlemenents (as eleprof may have changed the weapon!)
		if (  t.entityprofile[t.entid].hasweapon_s != "" ) 
		{
			t.findgun_s=Lower(t.entityprofile[t.entid].hasweapon_s.Get()) ; gun_findweaponindexbyname ( );
			t.entityprofile[t.entid].hasweapon=t.foundgunid;
			if ( t.foundgunid>0 && t.entityprofile[t.entid].isammo == 0 )  
			{
				// make gun active in game
				t.gun[t.foundgunid].activeingame = 1;

				// 301115 - also populate profile with correct default ammo from clip if default required
				if ( t.entityprofile[t.entid].ischaracter == 1 ) 
				{
					if ( t.entityprofile[t.entid].quantity == -1  ) 
					{
						t.entityprofile[t.entid].quantity = t.gun[t.foundgunid].settings.reloadqty;
						if ( t.entityprofile[t.entid].quantity==0 )
						{
							// discover reload quantity if gun data not loaded in
							t.gunid = t.foundgunid;
							t.gun_s = t.gun[t.gunid].name_s; 
							gun_loaddata();
							t.entityprofile[t.entid].quantity = g.firemodes[t.gunid][0].settings.reloadqty;
							t.gunid = 0;
						}
					}
				}
			}
		}
		else
		{
			t.entityprofile[t.entid].hasweapon=0;
		}

		// see if we can find head automatically
		// 010818 - expanded to find mixamo_xx or bip01_xx or anything_xx
		if (  t.entityprofile[t.entid].ischaracter == 1 ) 
		{
			if (  t.entityprofile[t.entid].headlimb == -1 ) 
			{
				if (  ObjectExist(t.entobj) == 1 ) 
				{
					PerformCheckListForLimbs (  t.entobj );
					for ( t.tc = 1 ; t.tc<=  ChecklistQuantity(); t.tc++ )
					{
						cstr sChecklistFound = Lower(ChecklistString(t.tc));
						LPSTR pChecklistFound = sChecklistFound.Get();
						if ( strnicmp ( pChecklistFound + strlen(pChecklistFound) - 5, "_head", 5 ) == NULL ) 
						{
							t.entityprofile[t.entid].headlimb=t.tc-1;
							t.tc=ChecklistQuantity()+1;
						}
					}
				}
			}
			if (  t.entityprofile[t.entid].firespotlimb == -1 ) 
			{
				if (  ObjectExist(t.entobj) == 1 ) 
				{
					PerformCheckListForLimbs (  t.entobj );
					for ( t.tc = 1 ; t.tc<=  ChecklistQuantity(); t.tc++ )
					{
						if (  cstr(Lower(ChecklistString(t.tc))) == "firespot" ) 
						{
							t.entityprofile[t.entid].firespotlimb=t.tc-1;
							t.tc=ChecklistQuantity()+1;
						}
					}
				}
			}
			if (  t.entityprofile[t.entid].spine == -1 ) 
			{
				if (  ObjectExist(t.entobj) == 1 ) 
				{
					PerformCheckListForLimbs (  t.entobj );
					for ( t.tc = 1 ; t.tc<=  ChecklistQuantity(); t.tc++ )
					{
						//if (  cstr(Lower(ChecklistString(t.tc))) == "bip01_spine1" ) 
						cstr sChecklistFound = Lower(ChecklistString(t.tc));
						LPSTR pChecklistFound = sChecklistFound.Get();
						if ( strnicmp ( pChecklistFound + strlen(pChecklistFound) - 7, "_spine1", 7 ) == NULL ) 
						{
							t.entityprofile[t.entid].spine=t.tc-1;
							break;
						}
					}
				}
			}
			if (  t.entityprofile[t.entid].spine2 == -1 ) 
			{
				if (  ObjectExist(t.entobj) == 1 ) 
				{
					PerformCheckListForLimbs (  t.entobj );
					for ( t.tc = 1 ; t.tc<=  ChecklistQuantity(); t.tc++ )
					{
						//if (  cstr(Lower(ChecklistString(t.tc))) == "bip01_spine2" ) 
						cstr sChecklistFound = Lower(ChecklistString(t.tc));
						LPSTR pChecklistFound = sChecklistFound.Get();
						if ( strnicmp ( pChecklistFound + strlen(pChecklistFound) - 7, "_spine2", 7 ) == NULL ) 
						{
							t.entityprofile[t.entid].spine2=t.tc-1;
							break;
						}
					}
				}
			}
		}

		// 090217 - new feature for some characters (Fuse FBX) to have perfect foot planting
		if ( t.entityprofile[t.entid].ischaracter == 1 && t.entityprofile[t.entid].isspinetracker == 1 && t.entityprofile[t.entid].spine != -1 )
		{
			sObject* pObject = GetObjectData ( t.entobj );
			pObject->bUseSpineCenterSystem = true;
			pObject->dwSpineCenterLimbIndex = t.entityprofile[t.entid].spine;
			pObject->fSpineCenterTravelDeltaX = 0.0f;
			pObject->fSpineCenterTravelDeltaZ = 0.0f;
		}
		else
		{
			// 051115 - if character, ensure the Y offset is applied to the parent object
			if ( t.entityprofile[t.entid].ischaracter == 1 || t.entityprofile[t.entid].offyoverride != 0 ) 
			{
				// calc bounds for static entities that have hierarchy that need shifting up when offyoverride used
				int iCalculateBounds = 0;
				if ( t.entityprofile[t.entid].offyoverride != 0 ) iCalculateBounds = 1;
				OffsetLimb ( t.entobj, 0, 0.0f, t.entityprofile[t.entid].offy, 0.0f, iCalculateBounds );
			}
		}

		// 010917 - hide any firespot limb meshes
		if ( t.entityprofile[t.entid].firespotlimb > 0 )
		{
			ExcludeLimbOn ( t.entobj, t.entityprofile[t.entid].firespotlimb );
		}

		//  190115 - wipe out limb control on legacy models, not configured for rotation!
		if (  t.entityprofile[t.entid].skipfvfconvert == 1 ) 
		{
			//t.entityprofile[t.entid].firespotlimb=0; but can allow firespot to be retained
			t.entityprofile[t.entid].headlimb=0;
			t.entityprofile[t.entid].spine=0;
			t.entityprofile[t.entid].spine2=0;
		}

		//  debug info and timestamp list (if logging)
		if (  t.entobj>0 ) 
		{
			if (  ObjectExist(t.entobj) == 1 ) 
			{
				t.strwork = "" ; t.strwork = t.strwork + "Loaded "+Str(t.entid)+":"+t.ent_s + " (" + cstr(GetObjectPolygonCount(t.entobj)) + ")";
				timestampactivity(0, t.strwork.Get() );
			}
		}
	}
	else
	{

		//  debug info and timestamp list (if logging)
		t.strwork = ""; t.strwork = t.strwork + "Skipped "+Str(t.entid)+":"+t.ent_s;
		timestampactivity(0, t.strwork.Get() );
	}

	//  Decactivate auto generation of mipmaps when entity load finished
	SetImageAutoMipMap (  0 );
}

void entity_loaddata ( void )
{
    //  Protect BIN file if no FPE backup (standalone run)
	t.tprotectBINfile=0;
	t.tFPEName_s=t.entdir_s+t.ent_s;
	if (  FileExist(t.tFPEName_s.Get()) == 0 ) 
	{
		t.tprotectBINfile=1;
	}

	//  Ensure entity profile still exists
	t.entityprofileheader[t.entid].desc_s="";
	t.tprofile_s=Left(t.tFPEName_s.Get(),Len(t.tFPEName_s.Get())-4); t.tprofile_s += ".bin";
	if (  t.tprotectBINfile == 0 ) 
	{
		//  020715 - to solve BIN issue once and for all, delete them when load entity
		//  and only preserve for final standalone export (actually NO performance difference!)
		if (  FileExist(t.tprofile_s.Get()) == 1  )  DeleteAFile (  t.tprofile_s.Get() );
	}

	t.strwork = t.entdir_s+t.ent_s;
	if (  FileExist(t.strwork.Get()) == 1 || FileExist(t.tprofile_s.Get()) == 1 ) 
	{

	//  Export entity FPE file if flagged
	if (  g.gexportassets == 1 ) 
	{
		t.strwork = t.entdir_s+t.ent_s;
		t.tthumbbmpfile_s = "";	t.tthumbbmpfile_s=t.tthumbbmpfile_s + Left(t.strwork.Get(),(Len(t.entdir_s.Get())+Len(t.ent_s.Get()))-4)+".bmp";
	}

	//  Allowed to loop around if skipBIN flag set
	do {  t.skipBINloadingandtryagain=0;

	//  Check if binary version of entity profile exists (DELETE BIN AT MOMEMENT!)
	//C++ISSUE forcing non binary at the moment due to make memblock from array not implemented for new arrays
	//looks like we are delelting bins anyway, but just incase...
	if ( 1 ) //if (  FileExist(t.tprofile_s.Get()) == 0 ) 
	{
		// 061115 - reset entity anim start value (so can be filled further down)
		for ( int n = 0; n < g.animmax; n++ )
		{
			t.entityanim[t.entid][n].start = 0;
			t.entityanim[t.entid][n].finish = 0;
			t.entityanim[t.entid][n].found = 0;
		}

		//  Must be reset before parse
		t.entityprofile[t.entid].limbmax=0;
		t.entityprofile[t.entid].animmax=0;
		t.entityprofile[t.entid].appendanimmax=0; //PE: sometimes , caused endless loop, was never set anywhere.
		t.entityprofile[t.entid].footfallmax=0;
		t.entityprofile[t.entid].headlimb=-1;
		t.entityprofile[t.entid].firespotlimb=-1;
		t.entityprofile[t.entid].physics=1;
		t.entityprofile[t.entid].phyweight=100;
		t.entityprofile[t.entid].phyfriction=0;
		t.entityprofile[t.entid].hoverfactor=0;
		t.entityprofile[t.entid].phyalways=0;
		t.entityprofile[t.entid].spine=-1;
		t.entityprofile[t.entid].spine2=-1;
		t.entityprofile[t.entid].decaloffsetangle=0;
		t.entityprofile[t.entid].decaloffsetdist=0;
		t.entityprofile[t.entid].decaloffsety=0;
		t.entityprofile[t.entid].ragdoll=0;
		t.entityprofile[t.entid].nothrowscript=0;
		t.entityprofile[t.entid].canfight=1;
		t.entityprofile[t.entid].rateoffire=85;
		t.entityprofile[t.entid].transparency=0;
		t.entityprofile[t.entid].canseethrough=0;
		t.entityprofile[t.entid].cullmode=0;
		t.entityprofile[t.entid].lod1distance=0;
		t.entityprofile[t.entid].lod2distance=0;
		t.entityprofile[t.entid].bitbobon = -1;
		t.entityprofile[t.entid].bitbobdistweight_f = 1;
		t.entityprofile[t.entid].autoflatten=0;
		t.entityprofile[t.entid].headframestart=-1;
		t.entityprofile[t.entid].headframefinish=-1;
		t.entityprofile[t.entid].hairframestart=-1;
		t.entityprofile[t.entid].hairframefinish=-1;
		t.entityprofile[t.entid].hideframestart=-1;
		t.entityprofile[t.entid].hideframefinish=-1;
		t.entityprofile[t.entid].animspeed=100;
		t.entityprofile[t.entid].animstyle=0;
		t.entityprofile[t.entid].collisionscaling=100;
		t.entityprofile[t.entid].physicsobjectcount = 0;
		t.entityprofile[t.entid].ishudlayer_s="";
		t.entityprofile[t.entid].ishudlayer=0;
		t.entityprofile[t.entid].fatness=50;
		t.entityprofile[t.entid].matrixmode=0;
		t.entityprofile[t.entid].skipfvfconvert=0;
		t.entityprofile[t.entid].resetlimbmatrix=0;
		t.entityprofile[t.entid].onetexture=0;
		t.entityprofile[t.entid].usesweapstyleanims=0;
		t.entityprofile[t.entid].isviolent=1;
		t.entityprofile[t.entid].reverseframes=0;
		t.entityprofile[t.entid].fullbounds=0;
		t.entityprofile[t.entid].cpuanims=0;
		t.entityprofile[t.entid].ignoredefanim=0;
		t.entityprofile[t.entid].teamfield=0;
		t.entityprofile[t.entid].scale=100;
		t.entityprofile[t.entid].addhandle_s="";
		t.entityprofile[t.entid].addhandlelimb=0;
		t.entityprofile[t.entid].ischaractercreator=0;
		t.entityprofile[t.entid].charactercreator_s="";
		t.entityprofile[t.entid].fJumpModifier=1.0f;		
		t.entityprofile[t.entid].jumphold=0;
		t.entityprofile[t.entid].jumpresume=0;
		t.entityprofile[t.entid].jumpvaulttrim=1;
		t.entityprofile[t.entid].meleerange=80;
		t.entityprofile[t.entid].meleehitangle=30;
		t.entityprofile[t.entid].meleestrikest=0;
		t.entityprofile[t.entid].meleestrikefn=0;
		t.entityprofile[t.entid].meleedamagest=20;
		t.entityprofile[t.entid].meleedamagefn=30;
		for ( t.q = 0 ; t.q<=  100 ; t.q++ ) { t.entitybodypart[t.entid][t.q]=0 ;   }
		t.entityprofile[t.entid].usespotlighting=0;
		t.entityprofile[t.entid].lodmodifier=0;
		t.entityprofile[t.entid].isocluder=1; // can be adjusted (if notanoccluder set to 1)
		t.entityprofile[t.entid].isocludee=1;
		t.entityprofile[t.entid].specularperc=100;
		t.entityprofile[t.entid].specular=0;
		t.entityprofile[t.entid].uvscrollu=0;
		t.entityprofile[t.entid].uvscrollv=0;
		t.entityprofile[t.entid].uvscaleu=1.0f;
		t.entityprofile[t.entid].uvscalev=1.0f;
		t.entityprofile[t.entid].invertnormal=0;
		t.entityprofile[t.entid].preservetangents=0;		
		t.entityprofile[t.entid].colondeath=1;
		t.entityprofile[t.entid].parententityindex=0;
		t.entityprofile[t.entid].parentlimbindex=0;
		t.entityprofile[t.entid].quantity=-1; // FPE specifies a value or we use a single weapon clip buy default (below)
		t.entityprofile[t.entid].smoothangle=0;
		t.entityprofile[t.entid].noXZrotation=0;
		t.entityprofile[t.entid].zdepth = 1;
		t.entityprofile[t.entid].isebe = 0;
		t.entityprofile[t.entid].offyoverride = 0;
		t.entityprofile[t.entid].ischaracter = 0;
		t.entityprofile[t.entid].isspinetracker = 0;
		t.entityprofile[t.entid].phyweight=100;
		t.entityprofile[t.entid].phyfriction=100;
		t.entityprofile[t.entid].phyforcedamage=100;
		t.entityprofile[t.entid].rotatethrow=1;
		t.entityprofile[t.entid].explodedamage=100;
		t.entityprofile[t.entid].forcesimpleobstacle=0;
		t.entityprofile[t.entid].forceobstaclepolysize=30.0f;
		t.entityprofile[t.entid].forceobstaclesliceheight=14.0f;
		t.entityprofile[t.entid].forceobstaclesliceminsize=5.0f;
		t.entityprofile[t.entid].effectprofile=0;
		t.entityprofile[t.entid].ignorecsirefs=0;

		//  Starter animation counts
		t.tnewanimmax=0 ; t.entityprofile[t.entid].animmax=t.tnewanimmax;
		t.tstartofaianim=-1 ; t.entityprofile[t.entid].startofaianim=t.tstartofaianim;

		// other resets
		t.entityappendanim[t.entid][0].filename = "";
		t.entityappendanim[t.entid][0].startframe = 0;

		//  temp variable to hold which physics object we are on from the importer
		t.tPhysObjCount = 0;

		//  Load entity Data from file
		Dim (  t.data_s,400  );
		t.strwork = t.entdir_s+t.ent_s;
		LoadArray ( t.strwork.Get() ,t.data_s );
		for ( t.l = 0 ; t.l<=  399; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if (  t.line_s.Get()[0] != ';' ) 
				{

					//  take fieldname and value
					for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
					{
						if ( t.line_s.Get()[t.c] == '=' ) 
						{ 
							t.mid = t.c+1  ; break;
						}
					}
					t.field_s=cstr(Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1))));
					t.value_s=cstr(removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid)));

					//  take value 1 and 2 from value
					for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
					{
						if (  t.value_s.Get()[t.c] == ',' ) 
						{ 
							t.mid = t.c+1 ; break; 
						}
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value1_f=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value2_s=cstr(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));
					if (  Len(t.value2_s.Get())>0  )  t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;

					//  entity header
					if (  t.field_s == "desc"  )  t.entityprofileheader[t.entid].desc_s = t.value_s;

					//  entity AI
					t.tryfield_s="aiinit";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].aiinit_s = Lower(t.value_s.Get());
					t.tryfield_s="aimain";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].aimain_s = Lower(t.value_s.Get());
					t.tryfield_s="aidestroy";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].aidestroy_s = Lower(t.value_s.Get());
					t.tryfield_s="aishoot";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].aishoot_s = Lower(t.value_s.Get());

					t.tryfield_s="soundset";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].soundset_s = t.value_s;
					t.tryfield_s="soundset1";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].soundset1_s = t.value_s;
					t.tryfield_s="soundset2";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].soundset2_s = t.value_s;
					t.tryfield_s="soundset3";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].soundset3_s = t.value_s;
					t.tryfield_s="soundset4";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].soundset4_s = t.value_s;

					//  entity AI related vars
					t.tryfield_s="usekey";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].usekey_s = t.value_s;
					t.tryfield_s="ifused";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ifused_s = t.value_s;
					t.tryfield_s="ifusednear";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ifusednear_s = t.value_s;

					//  entity SPAWN
					t.tryfield_s="spawnmax";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].spawnmax = t.value1;
					t.tryfield_s="spawndelay";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].spawndelay = t.value1;
					t.tryfield_s="spawnqty";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].spawnqty = t.value1;
					//  entity orientation
					t.tryfield_s="model";
					//  if it is a character creator model, ignore this
					if (  t.entityprofile[t.entid].ischaractercreator == 0 ) 
					{
						if (  t.field_s == t.tryfield_s  ) 
							t.entityprofile[t.entid].model_s = t.value_s;
					}

					t.tryfield_s="offsetx";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].offx = t.value1;
					t.tryfield_s="offsety";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].offy = t.value1;
					t.tryfield_s="offyoverride";
					if (  t.field_s == t.tryfield_s  )  
					{
						t.entityprofile[t.entid].offyoverride = 1;
						t.entityprofile[t.entid].offy = t.value1;
					}
					t.tryfield_s="offsetz";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].offz = t.value1;
					t.tryfield_s="rotx";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].rotx = t.value1;
					t.tryfield_s="roty";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].roty = t.value1;
					t.tryfield_s="rotz";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].rotz = t.value1;
					t.tryfield_s="scale";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].scale = t.value1;
					t.tryfield_s="fixnewy";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].fixnewy = t.value1;
					t.tryfield_s="hoverfactor";
					if (  t.field_s == t.tryfield_s ) 
					{
						//  FPGC - V116 - some FPE characters use a range 0.1-0.9, must be accounted!
						if (  t.value1_f>-1.0 && t.value1_f<1.0  )  t.value1 = 1;
						t.entityprofile[t.entid].hoverfactor=t.value1;
					}
					t.tryfield_s="forwardfacing";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].forwardfacing = t.value1;
					t.tryfield_s="dontfindfloor";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].dontfindfloor = t.value1;
					t.tryfield_s="defaultheight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].defaultheight = t.value1;
					t.tryfield_s="defaultstatic";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].defaultstatic = t.value1;
					t.tryfield_s="autoflatten";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].autoflatten = t.value1;

					//  collisionmode (see GameGuru\Docs\collisionmodevalues.txt)
					//  0  ; box shape (default)
					//  1  ; polygon shape
					//  2  ; sphere shape
					//  3  ; cylinder shape
					//  9  ; convex hull reduction shape
					//  11 ; no physics
					//  12 ; no physics but can still be detected with IntersectAll command
					//  21 ; player repel feature (for characters and other beasts/zombies)
					//  40 ; collision boxes (defined in Import Model feature)
					//  41-49 ; reserved (collision polylist, sphere list, cylinder list)
					//  50 ; generate obstacle and cylinder from 1/64th up from base of model
					//  51 ; generate obstacle and cylinder from 1/32th down from base of model
					//  52 ; generate obstacle and cylinder from 8/16th up from base of model
					//  53 ; generate obstacle and cylinder from 7/16th up from base of model
					//  54 ; generate obstacle and cylinder from 6/16th up from base of model
					//  55 ; generate obstacle and cylinder from 5/16th up from base of model
					//  56 ; generate obstacle and cylinder from 4/16th up from base of model
					//  57 ; generate obstacle and cylinder from 3/16th up from base of model
					//  58 ; generate obstacle and cylinder from 2/16th up from base of model
					//  59 ; generate obstacle and cylinder from 1/16th up from base of model
					//  1000-2000 ; only one limb has collision Box Shape (1000=limb zero,1001=limb one,etc)
					//  2000-3000 ; only one limb has collision Polygons Shape (2000=limb zero,2001=limb one,etc)					
					t.tryfield_s="collisionmode";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].collisionmode = t.value1;
					t.tryfield_s="collisionscaling";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].collisionscaling = t.value1;
					t.tryfield_s="collisionoverride";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].collisionoverride = t.value1;

					// endcollision:
					// 0 - no collision for ragdoll
					// 1 - collision for ragdoll
					t.tryfield_s="endcollision";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].colondeath = t.value1;

					//  forcesimpleobstacle
					//  -1 ; absolutely no obstacle
					//  0 ; default
					//  1 ; Box (  )
					//  2 ; contour
					//  3 ; full poly scan
					t.tryfield_s="forcesimpleobstacle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].forcesimpleobstacle = t.value1;
					t.tryfield_s="forceobstaclepolysize";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].forceobstaclepolysize = t.value1;
					t.tryfield_s="forceobstaclesliceheight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].forceobstaclesliceheight = t.value1;
					t.tryfield_s="forceobstaclesliceminsize";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].forceobstaclesliceminsize = t.value1;

					t.tryfield_s="notanoccluder";
					if (  t.field_s == t.tryfield_s  )  
					{
						t.entityprofile[t.entid].notanoccluder = t.value1;
						if ( t.entityprofile[t.entid].notanoccluder == 1 ) t.entityprofile[t.entid].isocluder = 0;
					}
					t.tryfield_s="notanoccludee";
					if (  t.field_s == t.tryfield_s  )  
					{
						int notanoccludee = t.value1;
						if ( notanoccludee == 1 ) t.entityprofile[t.entid].isocludee = 0;
					}

					t.tryfield_s="skipfvfconvert";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].skipfvfconvert = t.value1;
					t.tryfield_s="matrixmode";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].matrixmode = t.value1;

					// 040116 - when lightmapper scales entities incorrectly, need this flag to correct!
					t.tryfield_s="resetlimbmatrix";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].resetlimbmatrix = t.value1;

					t.tryfield_s="reverseframes";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].reverseframes = t.value1;
					t.tryfield_s="fullbounds";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].fullbounds = t.value1;

					//  cpuanims
					//  0 ; GPU animation
					//  1 ; CPU animation (for wide scope animations that need accurate ray detection)
					//  2 ; Same as [1] but will hide any meshes that do not have animations
					t.tryfield_s="cpuanims";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].cpuanims = t.value1;
				
					// 131115 - some legacy models hold an OLD nasty frame in matCombined for each frame, and can mess up collision detection if CPUANIMS=1 also
					t.tryfield_s="ignoredefanim";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ignoredefanim = t.value1;

					//  materialindex
					//  0    = - GenericSoft
					//  1    = S Stone
					//  2    = M Metal
					//  3    = W Wood
					//  4    = G Glass
					//  5    = L Liquid Splashy Wet
					//  6    = F Flesh (Bloody Organic)
					//  7    = H Hollow Drum Metal
					//  8    = T Small High Pitch Tin
					//  9    = V Small Low Pitch Tin
					//  10   = I Silly Material
					//  11   = A Marble
					//  12   = C Cobble
					//  13   = R Gravel
					//  14   = E Soft Metal
					//  15   = O Old Stone
					//  16   = D Old Wood
					//  17   = W Shallow Water
					//  18   = U Underwater
					t.tryfield_s="materialindex";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].materialindex = t.value1;
					t.tryfield_s="debrisshape";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].debrisshapeindex = t.value1;

					//  LOD and BITBOB system
					t.tryfield_s="disablebatch";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].disablebatch = t.value1;
					t.tryfield_s="lod1distance";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].lod1distance = t.value1;
					t.tryfield_s="lod2distance";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].lod2distance = t.value1;
					// Not using bitbobon or bitbobdistweight anymore due to the occluder taking over these tasks
					/*
					t.tryfield_s="bitbobon";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].bitbobon = t.value1;
					t.tryfield_s="bitbobdistweight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].bitbobdistweight_f = t.value1_f;
					*/

					//  physics setup
					t.tryfield_s="physics";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].physics = t.value1;
					t.tryfield_s="phyweight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].phyweight = t.value1;
					t.tryfield_s="phyfriction";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].phyfriction = t.value1;
					t.tryfield_s="explodable";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].explodable = t.value1;
					t.tryfield_s="explodedamage";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].explodedamage = t.value1;
					t.tryfield_s="ragdoll";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ragdoll = t.value1;

					//  FPGC - 160511 - added NOTHROWSCRIPT to entity profile
					t.tryfield_s="nothrowscript";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].nothrowscript = t.value1;

					//  cone of sight
					t.tryfield_s="coneheight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].coneheight = t.value1;
					t.tryfield_s="coneangle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].coneangle = t.value1;
					t.tryfield_s="conerange";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].conerange = t.value1;

					//  visual info
					t.tryfield_s="onetexture";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].onetexture = t.value1;
					if (  t.entityprofile[t.entid].ischaractercreator == 0 ) 
					{
						t.tryfield_s="texturepath";
						if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].texpath_s = t.value_s;
						t.tryfield_s="textured";
						if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].texd_s = t.value_s;
						t.tryfield_s="texturealtd";
						if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].texaltd_s = t.value_s;
					}
					t.tryfield_s="effect";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].effect_s = t.value_s;

					// effectprofile:
					// 0 - default non-PBR
					// 1 - new PBR texture arrangement
					t.tryfield_s="effectprofile";
					if ( t.field_s == t.tryfield_s ) t.entityprofile[t.entid].effectprofile = t.value1;

					//  transparency modes;
					//  0 - first-phase no alpha
					//  1 - first-phase with alpha masking
					//  2 and 3 - second-phase which overlaps solid geometry
					//  4 - alpha test (only render beyond 0x000000CF alpha values)
					//  5 - water Line (  object (seperates depth sort automatically) )
					//  6 - combination of 3 and 4 (second phase render with alpha blend AND alpha test, used for fading LOD leaves)
					//  7 - very early draw phase no alpha
					t.tryfield_s="transparency";
					if (  t.field_s == t.tryfield_s  ) 
					{
						if ( t.value1 == 5 ) t.value1 = 6; // 021215 - can only ben one water line
						t.entityprofile[t.entid].transparency = t.value1;
					}
					t.tryfield_s="canseethrough";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].canseethrough = t.value1;

					// specular:
					// 0 - uses _S texture
					// 1 - uses effectbank\\reloaded\\media\\blank_none_S.dds
					// 2 - uses effectbank\\reloaded\\media\\blank_low_S.dds
					// 3 - uses effectbank\\reloaded\\media\\blank_medium_S.dds
					// 4 - uses effectbank\\reloaded\\media\\blank_high_S.dds
					t.tryfield_s="specular";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].specular = t.value1;
					// specularperc:
					// percentage 0 to 100 to modulate how much global specular gets to individual entity
					t.tryfield_s="specularperc";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].specularperc = t.value1;

					// can scale the uv data inside the shader
					t.tryfield_s="uvscroll";
					if (  t.field_s == t.tryfield_s  ) { t.entityprofile[t.entid].uvscrollu = t.value1/100.0f; t.entityprofile[t.entid].uvscrollv = t.value2/100.0f; }
					t.tryfield_s="uvscale";
					if (  t.field_s == t.tryfield_s  ) { t.entityprofile[t.entid].uvscaleu = t.value1/100.0f; t.entityprofile[t.entid].uvscalev = t.value2/100.0f; }

					// can invert the normal, or set to zero to not invert (not inverted by default)
					t.tryfield_s="invertnormal";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].invertnormal = t.value1;

					// can choose whether to generate tangent/binormal in the shader
					t.tryfield_s="preservetangents";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].preservetangents = t.value1;
					
					t.tryfield_s="zdepth";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].zdepth = t.value1;

					t.tryfield_s="cullmode";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].cullmode = t.value1;
					t.tryfield_s="reducetexture";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].reducetexture = t.value1;

					// castshadow:
					//  0 = default shadow caster mode
					// -1 = do not cast shadows or lightmap shadows
					t.tryfield_s="castshadow";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].castshadow = t.value1;
					t.tryfield_s="smoothangle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].smoothangle = t.value1;

					//  entity identity details
					t.tryfield_s="strength";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].strength = t.value1;
					t.tryfield_s="lives";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].lives = t.value1;
					t.tryfield_s="speed";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].speed = t.value1;
					t.tryfield_s="animspeed";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].animspeed = t.value1;
					t.tryfield_s="hurtfall";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].hurtfall = t.value1;

					t.tryfield_s="isimmobile";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isimmobile = t.value1;
					t.tryfield_s="isviolent";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isviolent = t.value1;
					t.tryfield_s="isobjective";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isobjective = t.value1;
					t.tryfield_s="alwaysactive";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].phyalways = t.value1;

					t.tryfield_s="ischaracter";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ischaracter = t.value1;
					t.tryfield_s="isspinetracker";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isspinetracker = t.value1;
					
					t.tryfield_s="noxzrotation";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].noXZrotation = t.value1;				
					t.tryfield_s="canfight";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].canfight = t.value1;

					t.tryfield_s="jumpmodifier";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].fJumpModifier = (float)t.value1 / 100.0f;
					t.tryfield_s="jumphold";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].jumphold = t.value1;
					t.tryfield_s="jumpresume";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].jumpresume = t.value1;
					t.tryfield_s="jumpvaulttrim";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].jumpvaulttrim = t.value1;

					t.tryfield_s="meleerange";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleerange = t.value1;
					t.tryfield_s="meleehitangle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleehitangle = t.value1;
					t.tryfield_s="meleestrikest";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleestrikest = t.value1;
					t.tryfield_s="meleestrikefn";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleestrikefn = t.value1;
					t.tryfield_s="meleedamagest";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleedamagest = t.value1;
					t.tryfield_s="meleedamagefn";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].meleedamagefn = t.value1;

					t.tryfield_s="custombiped";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].custombiped = t.value1;

					t.tryfield_s="cantakeweapon";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].cantakeweapon = t.value1;
					t.tryfield_s="isweapon";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isweapon_s = t.value_s;
					t.tryfield_s="rateoffire";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].rateoffire = t.value1;
					t.tryfield_s="ishudlayer";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ishudlayer_s = t.value_s;

					//  fpgc - same internals just sanitized  -NEED TO GO IN EDITOR TOO!
					t.tryfield_s="isequipment";
					if (  t.field_s == t.tryfield_s ) 
					{
						t.entityprofile[t.entid].isweapon_s=t.value_s;
						if (  Len(t.value_s.Get())>2 ) 
						{
							//  FPGC - 280809 - if equipment specified, this entity is ALWAYS ACTIVE (so can pickup AND DROP the item)
							t.entityprofile[t.entid].phyalways=1;
						}
					}

					t.tryfield_s="isammo";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isammo = t.value1;
					t.tryfield_s="hasweapon";
					if (  t.field_s == t.tryfield_s  )  
						t.entityprofile[t.entid].hasweapon_s = t.value_s;
					t.tryfield_s="hasequipment";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].hasweapon_s = t.value_s;
					t.tryfield_s="ishealth";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ishealth = t.value1;
					t.tryfield_s="isflak";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isflak = t.value1;
					t.tryfield_s="fatness";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].fatness = t.value1;

					//  marker extras
					//  1=player
					//  2=lights
					//  3=trigger zone
					//  4=decal particle emitter
					//  5=entity lights
					//  6=checkpoint zone
					//  7=multiplayer start
					//  8=floor zone
					//  9=cover zone
					t.tryfield_s="ismarker";
					if (  t.field_s == t.tryfield_s  )  
					{
						t.entityprofile[t.entid].ismarker = t.value1;
						if ( t.entityprofile[t.entid].ismarker > 0 && t.entityprofile[t.entid].ismarker != 2 )
						{
							// force zone and arrow markers to rise above terrain if planted there
							t.entityprofile[t.entid].offyoverride = 1;
							t.entityprofile[t.entid].offy = 2;
						}
					}
					t.tryfield_s="markerindex";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].markerindex = t.value1;
					t.tryfield_s="addhandle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].addhandle_s = t.value_s;

					// ebe builder extras
					t.tryfield_s="isebe";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].isebe = t.value1;

					//  light extras
					t.tryfield_s="lightcolor";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].light.color = t.value1;
					t.tryfield_s="lightrange";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].light.range = t.value1;
					t.tryfield_s="lightoffsetup";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].light.offsetup = t.value1;
					t.tryfield_s="lightoffsetz";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].light.offsetz = t.value1;

					// light type flags
					t.tryfield_s="usespotlighting";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].usespotlighting = t.value1;

					//  trigger extras
					t.tryfield_s="stylecolor";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].trigger.stylecolor = t.value1;

					//  extra decal offset (ideal for placing flames in torches, etc)
					t.tryfield_s="decalangle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].decaloffsetangle = t.value1;
					t.tryfield_s="decaldist";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].decaloffsetdist = t.value1/10.0;
					t.tryfield_s="decaly";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].decaloffsety = t.value1/10.0;

					//  entity body part list (20/01/11 - refeatured for V118)
					t.tryfield_s="limbmax";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].limbmax = t.value1;
					if (  t.entityprofile[t.entid].limbmax>100  )  t.entityprofile[t.entid].limbmax = 100;
					if (  t.entityprofile[t.entid].limbmax>0 ) 
					{
						for ( t.q = 0 ; t.q<=  t.entityprofile[t.entid].limbmax-1; t.q++ )
						{
							t.strwork = ""; t.strwork = t.strwork + "limb"+Str(t.q);
							t.tryfield_s=t.strwork.Get();
							if (  cstr(Lower(t.field_s.Get())) == Lower(t.tryfield_s.Get()) ) 
							{
								t.entitybodypart[t.entid][t.q]=t.value1;
							}
						}
					}

					//  determine if entity has a head, and which limbs represent it
					t.tryfield_s="headlimbs";
					if (  t.field_s == t.tryfield_s ) { t.entityprofile[t.entid].headframestart = t.value1; t.entityprofile[t.entid].headframefinish = t.value2; }

					//  determine if entity has hair, and which limbs represent it/them
					t.tryfield_s="hairlimbs";
					if (  t.field_s == t.tryfield_s ) { t.entityprofile[t.entid].hairframestart = t.value1; t.entityprofile[t.entid].hairframefinish = t.value2; }

					//  determine if entity has limbs to hide
					t.tryfield_s="hidelimbs";
					if (  t.field_s == t.tryfield_s ) { t.entityprofile[t.entid].hideframestart = t.value1; t.entityprofile[t.entid].hideframefinish = t.value2; }

					//  entity decal refs
					t.tryfield_s="decalmax";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].decalmax = t.value1;
					if (  t.entityprofile[t.entid].decalmax>0 ) 
					{
						for ( t.q = 0 ; t.q<=  t.entityprofile[t.entid].decalmax-1; t.q++ )
						{
							t.tryfield_s = cstr((cstr("decal")+Str(t.q)) );
							if (  t.field_s == t.tryfield_s  )  t.entitydecal_s[t.entid][t.q] = t.value_s;
						}
					}

					// 060718 - entity append anim system
					t.tryfield_s=cstr("appendanimfinal");
					if (  t.field_s == t.tryfield_s )
					{ 
						t.entityappendanim[t.entid][0].filename = t.value_s; 
						t.entityappendanim[t.entid][0].startframe = 0;
					}
					t.tryfield_s="appendanimmax";
					if ( t.field_s == t.tryfield_s ) 
					{
						t.entityprofile[t.entid].appendanimmax = t.value1; 
						if ( t.entityprofile[t.entid].appendanimmax > 99 ) 
							t.entityprofile[t.entid].appendanimmax = 99;
					}

					//PE: Hanging, in my case: appendanimmax=573444874 value_s=road_straight01.x
					//PE: Hang if you are unlucky and get mem that "appendanimmax" are not already set to zero.
					if ( t.entityprofile[t.entid].appendanimmax > 0 && t.entityprofile[t.entid].appendanimmax <= 99 )
					{
						for ( int aa = 1 ; aa <= t.entityprofile[t.entid].appendanimmax; aa++ )
						{
							t.tryfield_s=cstr("appendanim")+Str(aa);
							if (  t.field_s == t.tryfield_s ) { t.entityappendanim[t.entid][aa].filename = t.value_s; }
							t.tryfield_s=cstr("appendanimframe")+Str(aa);
							if (  t.field_s == t.tryfield_s ) { t.entityappendanim[t.entid][aa].startframe = t.value1; }
						}
					}

					//  entity animation sets
					t.tryfield_s="ignorecsirefs";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].ignorecsirefs = t.value1;
					t.tryfield_s="playanimineditor";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].playanimineditor = t.value1;
					t.tryfield_s="animstyle";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].animstyle = t.value1;
					t.tryfield_s="animmax";
					if (  t.field_s == t.tryfield_s ) 
					{
						t.tnewanimmax=t.value1 ; t.tstartofaianim=t.tnewanimmax;
					}
					if (  t.tnewanimmax>0 ) 
					{
						for ( t.q = 0 ; t.q<=  t.tstartofaianim-1; t.q++ )
						{
							t.tryfield_s=cstr("anim")+Str(t.q);
							if (  t.field_s == t.tryfield_s ) { t.entityanim[t.entid][t.q].start = t.value1 ; t.entityanim[t.entid][t.q].finish = t.value2 ; t.entityanim[t.entid][t.q].found = 1; }
							if (  t.entityanim[t.entid][t.q].found == 0 ) { t.entityanim[t.entid][t.q].start = -1  ; t.entityanim[t.entid][t.q].finish = -1; }
						}
					}

					// 291014 - AI system animation sets (takes field$ and value1/value2)
					if ( 1 ) // t.entityprofile[t.entid].ignorecsirefs == 0 ) // for now, still need these for THIRD PERSON which uses old CSI system
					{
						// 200918 - externalised internal AI system into scripts, but keeping for legacy support
						darkai_assignanimtofield ( );
					}

					//  V110 BETA5 - 080608 - get foot fall data (optional)
					t.tryfield_s="footfallmax";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].footfallmax = t.value1;
					if (  t.entityprofile[t.entid].footfallmax>0 ) 
					{
						for ( t.q = 0 ; t.q <= t.entityprofile[t.entid].footfallmax-1; t.q++ )
						{
							t.tryfield_s=cstr("footfall")+Str(t.q);
							if (  t.field_s == t.tryfield_s ) { t.entityfootfall[t.entid][t.q].keyframe = t.value1  ; t.entityfootfall[t.entid][t.q].soundtype = t.value2; }
						}
					}

					//  more data
					t.tryfield_s="quantity";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].quantity = t.value1;

					//  character creator

					t.tryfield_s="charactercreator";
					if (  t.field_s == t.tryfield_s ) 
					{
						t.entityprofile[t.entid].ischaractercreator=1;
						t.entityprofile[t.entid].charactercreator_s=t.value_s;
						t.tstring_s = t.value_s;
						//  "v"
						t.tnothing_s = FirstToken(t.tstring_s.Get(),":");
						//  "version number"
						t.tnothing_s = NextToken(":");
						//  body mesh
						t.tbody_s = NextToken(":");

						//  read in cci
						t.tcciloadname_s = g.fpscrootdir_s+"\\Files\\characterkit\\bodyandhead\\" + t.tbody_s + ".cci";
						t.tpath_s = "characterkit\\bodyandhead\\";
						t.tccquick = 1;
						characterkit_loadCCI ( );
						t.entityprofile[t.entid].model_s = t.tccimesh_s;
						t.entityprofile[t.entid].texpath_s = t.tpath_s;
						t.entityprofile[t.entid].texd_s = t.tccidiffuse_s;
					}

					//  physics objects from the importer
					t.tryfield_s="physicscount";
					if (  t.field_s == t.tryfield_s  )  t.entityprofile[t.entid].physicsobjectcount = t.value1;

					if (  cstr(Left(t.field_s.Get(),7)) == "physics" && t.field_s != "physicscount" && t.tPhysObjCount < MAX_ENTITY_PHYSICS_BOXES ) 
					{

						Dim (  t.tArray,10 );

							//  get rid of the quotation marks
							t.tStrip_s = t.value_s;
							t.tStrip_s = Left(t.tStrip_s.Get(), Len(t.tStrip_s.Get())-1);
							t.tStrip_s = Right(t.tStrip_s.Get(), Len(t.tStrip_s.Get())-1);

							t.tArrayMarker = 0;
							t.ttToken_s=FirstToken(t.tStrip_s.Get(),",");
							if (  t.ttToken_s  !=  "" ) 
							{
								t.tArray[t.tArrayMarker] = t.ttToken_s;
								++t.tArrayMarker;
							}
							do
							{
								t.ttToken_s=NextToken(",");
								if (  t.ttToken_s  !=  "" ) 
								{
									t.tArray[t.tArrayMarker] = t.ttToken_s;
									++t.tArrayMarker;
								}
							} while ( !(  t.ttToken_s == "" ) );

							//  Format; shapetype, sizex, sizey, sizez, offx, offy, offz, rotx, roty, rotz
							t.tPShapeType = ValF(t.tArray[0].Get());
							//  is it a box?
							if (  t.tPShapeType  ==  0 ) 
							{
								//Dave Crash fix - check we are not going out of bounds
								if ( t.entid < MAX_ENTITY_PHYSICS_BOXES*2 )
								{
									t.entityphysicsbox[t.entid][t.tPhysObjCount].SizeX = ValF(t.tArray[1].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].SizeY = ValF(t.tArray[2].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].SizeZ = ValF(t.tArray[3].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].OffX = ValF(t.tArray[4].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].OffY = ValF(t.tArray[5].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].OffZ = ValF(t.tArray[6].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].RotX = ValF(t.tArray[7].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].RotY = ValF(t.tArray[8].Get());
									t.entityphysicsbox[t.entid][t.tPhysObjCount].RotZ = ValF(t.tArray[9].Get());
								}
								++t.tPhysObjCount;
						}

						UnDim (  t.tArray );

					}

				}
			}
		}
		UnDim (  t.data_s );

		// 200918 - no longer use internal AI system, but keep for legacy compatibility
		if ( 1 ) //t.entityprofile[t.entid].ignorecsirefs == 0 ) // for now, still need these for THIRD PERSON which uses old CSI system
		{
			// if No AI anim sets, fill with hard defaults from official template character
			if (  t.entityprofile[t.entid].ischaracter == 1 ) 
			{
				// 010917 - but only if a character, so as not to force non-anim entities to use entity_anim
				for ( t.n = 1 ; t.n<=  90; t.n++ )
				{
					if (  t.n == 1 ) { t.field_s = "csi_relaxed1"  ; t.value1 = 900 ; t.value2 = 999; }
					if (  t.n == 2 ) { t.field_s = "csi_relaxed2"  ; t.value1 = 1000 ; t.value2 = 1282; }
					if (  t.n == 3 ) { t.field_s = "csi_relaxedmovefore"  ; t.value1 = 1290 ; t.value2 = 1419; }
					if (  t.n == 4 ) { t.field_s = "csi_cautious"  ; t.value1 = 900 ; t.value2 = 999; }
					if (  t.n == 5 ) { t.field_s = "csi_cautiousmovefore"  ; t.value1 = 1325 ; t.value2 = 1419; }
					if (  t.n == 6 ) { t.field_s = "csi_unarmed1"  ; t.value1 = 3000 ; t.value2 = 3100; }
					if (  t.n == 7 ) { t.field_s = "csi_unarmed2"  ; t.value1 = 3430 ; t.value2 = 3697; }
					if (  t.n == 8 ) { t.field_s = "csi_unarmedconversation"  ; t.value1 = 3110 ; t.value2 = 3420; }
					if (  t.n == 10 ) { t.field_s = "csi_unarmedexplain"  ; t.value1 = 4260 ; t.value2 = 4464; }
					if (  t.n == 11 ) { t.field_s = "csi_unarmedpointfore"  ; t.value1 = 4470 ; t.value2 = 4535; }
					if (  t.n == 12 ) { t.field_s = "csi_unarmedpointback"  ; t.value1 = 4680 ; t.value2 = 4745; }
					if (  t.n == 13 ) { t.field_s = "csi_unarmedpointleft"  ; t.value1 = 4610 ; t.value2 = 4675; }
					if (  t.n == 14 ) { t.field_s = "csi_unarmedpointright"  ; t.value1 = 4540 ; t.value2 = 4605; }
					if (  t.n == 15 ) { t.field_s = "csi_unarmedmovefore"  ; t.value1 = 3870 ; t.value2 = 3900; }
					if (  t.n == 16 ) { t.field_s = "csi_unarmedmoverun"  ; t.value1 = 3905 ; t.value2 = 3925; }
					if (  t.n == 17 ) { t.field_s = "csi_unarmedstairascend"  ; t.value1 = 5600 ; t.value2 = 5768; }
					if (  t.n == 18 ) { t.field_s = "csi_unarmedstairdecend"  ; t.value1 = 5800 ; t.value2 = 5965; }
					if (  t.n == 19 ) { t.field_s = "csi_unarmedladderascend1"  ; t.value1 = 4148 ; t.value2 = 4110; }
					if (  t.n == 20 ) { t.field_s = "csi_unarmedladderascend2"  ; t.value1 = 4148 ; t.value2 = 4255; }
					if (  t.n == 21 ) { t.field_s = "csi_unarmedladderascend3"  ; t.value1 = 4225 ; t.value2 = 4255; }
					if (  t.n == 22 ) { t.field_s = "csi_unarmedladderdecend1"  ; t.value1 = 4255 ; t.value2 = 4225; }
					if (  t.n == 23 ) { t.field_s = "csi_unarmedladderdecend2"  ; t.value1 = 4225 ; t.value2 = 4148; }
					if (  t.n == 24 ) { t.field_s = "csi_unarmeddeath"  ; t.value1 = 4800 ; t.value2 = 4958; }
					if (  t.n == 25 ) { t.field_s = "csi_unarmedimpactfore"  ; t.value1 = 4971 ; t.value2 = 5021; }
					if (  t.n == 26 ) { t.field_s = "csi_unarmedimpactback"  ; t.value1 = 5031 ; t.value2 = 5090; }
					if (  t.n == 27 ) { t.field_s = "csi_unarmedimpactleft"  ; t.value1 = 5171 ; t.value2 = 5229; }
					if (  t.n == 28 ) { t.field_s = "csi_unarmedimpactright"  ; t.value1 = 5101 ; t.value2 = 5160; }
					if (  t.n == 29 ) { t.field_s = "csi_inchair"  ; t.value1 = 3744 ; t.value2 = 3828; }
					if (  t.n == 30 ) { t.field_s = "csi_inchairsit"  ; t.value1 = 3710 ; t.value2 = 3744; }
					if (  t.n == 31 ) { t.field_s = "csi_inchairgetup"  ; t.value1 = 3828 ; t.value2 = 3862; }
					if (  t.n == 32 ) { t.field_s = "csi_swim"  ; t.value1 = 3930 ; t.value2 = 4015; }
					if (  t.n == 33 ) { t.field_s = "csi_swimmovefore"  ; t.value1 = 4030 ; t.value2 = 4072; }
					if (  t.n == 34 ) { t.field_s = "csi_stoodnormal"  ; t.value1 = 100 ; t.value2 = 205; }
					if (  t.n == 35 ) { t.field_s = "csi_stoodrocket"  ; t.value1 = 6133 ; t.value2 = 6206; }
					if (  t.n == 36 ) { t.field_s = "csi_stoodfidget1"  ; t.value1 = 100 ; t.value2 = 205; }
					if (  t.n == 37 ) { t.field_s = "csi_stoodfidget2"  ; t.value1 = 210 ; t.value2 = 318; }
					if (  t.n == 38 ) { t.field_s = "csi_stoodfidget3"  ; t.value1 = 325 ; t.value2 = 431; }
					if (  t.n == 39 ) { t.field_s = "csi_stoodfidget4"  ; t.value1 = 440 ; t.value2 = 511; }
					if (  t.n == 40 ) { t.field_s = "csi_stoodstartled"  ; t.value1 = 1425 ; t.value2 = 1465; }
					if (  t.n == 41 ) { t.field_s = "csi_stoodpunch"  ; t.value1 = 0 ; t.value2 = 0; }
					if (  t.n == 42 ) { t.field_s = "csi_stoodkick"  ; t.value1 = 5511 ; t.value2 = 5553; }
					if (  t.n == 43 ) { t.field_s = "csi_stoodmovefore"  ; t.value1 = 685 ; t.value2 = 707; }
					if (  t.n == 44 ) { t.field_s = "csi_stoodmoveback"  ; t.value1 = 710 ; t.value2 = 735; }
					if (  t.n == 45 ) { t.field_s = "csi_stoodmoveleft"  ; t.value1 = 740 ; t.value2 = 762; }
					if (  t.n == 46 ) { t.field_s = "csi_stoodmoveright"  ; t.value1 = 765 ; t.value2 = 789; }
					if (  t.n == 47 ) { t.field_s = "csi_stoodstepleft"  ; t.value1 = 610 ; t.value2 = 640; }
					if (  t.n == 48 ) { t.field_s = "csi_stoodstepright"  ; t.value1 = 645 ; t.value2 = 676; }
					if (  t.n == 49 ) { t.field_s = "csi_stoodstrafeleft"  ; t.value1 = 855 ; t.value2 = 871; }
					if (  t.n == 50 ) { t.field_s = "csi_stoodstraferight"  ; t.value1 = 875 ; t.value2 = 892; }
					//  51 see below
					//  reserved 52
					if (  t.n == 53 ) { t.field_s = "csi_stoodvault"  ; t.value1 = 0 ; t.value2 = 0; } // 220217 - these now need to come from FPE
					if (  t.n == 54 ) { t.field_s = "csi_stoodmoverun"  ; t.value1 = 795 ; t.value2 = 811; }
					if (  t.n == 55 ) { t.field_s = "csi_stoodmoverunleft"  ; t.value1 = 815 ; t.value2 = 830; }
					if (  t.n == 56 ) { t.field_s = "csi_stoodmoverunright"  ; t.value1 = 835 ; t.value2 = 850; }
					if (  t.n == 57 ) { t.field_s = "csi_stoodreload"  ; t.value1 = 515 ; t.value2 = 605; }
					if (  t.n == 58 ) { t.field_s = "csi_stoodreloadrocket"  ; t.value1 = 6233 ; t.value2 = 6315; }
					if (  t.n == 59 ) { t.field_s = "csi_stoodwave"  ; t.value1 = 1470 ; t.value2 = 1520; }
					if (  t.n == 60 ) { t.field_s = "csi_stoodtoss"  ; t.value1 = 2390 ; t.value2 = 2444; }
					if (  t.n == 61 ) { t.field_s = "csi_stoodfirerocket"  ; t.value1 = 6207 ; t.value2 = 6232; }
					if (  t.n == 62 ) { t.field_s = "csi_stoodincoverleft"  ; t.value1 = 1580 ; t.value2 = 1580; }
					if (  t.n == 63 ) { t.field_s = "csi_stoodincoverpeekleft"  ; t.value1 = 1581 ; t.value2 = 1581; }
					if (  t.n == 64 ) { t.field_s = "csi_stoodincoverthrowleft"  ; t.value1 = 2680 ; t.value2 = 2680; }
					if (  t.n == 65 ) { t.field_s = "csi_stoodincoverright"  ; t.value1 = 1525 ; t.value2 = 1525; }
					if (  t.n == 66 ) { t.field_s = "csi_stoodincoverpeekright"  ; t.value1 = 1526 ; t.value2 = 1526; }
					if (  t.n == 67 ) { t.field_s = "csi_stoodincoverthrowright"  ; t.value1 = 2570 ; t.value2 = 2570; }
					if (  t.n == 51 ) { t.field_s = "csi_stoodandturn"  ; t.value1 = 0 ; t.value2 = 0; }
					if (  t.n == 68 ) { t.field_s = "csi_crouchidlenormal1"  ; t.value1 = 1670 ; t.value2 = 1819; }
					if (  t.n == 69 ) { t.field_s = "csi_crouchidlenormal2"  ; t.value1 = 1825 ; t.value2 = 1914; }
					if (  t.n == 70 ) { t.field_s = "csi_crouchidlerocket"  ; t.value1 = 6472 ; t.value2 = 6545; }
					if (  t.n == 71 ) { t.field_s = "csi_crouchdown"  ; t.value1 = 1630 ; t.value2 = 1646; }
					if (  t.n == 72 ) { t.field_s = "csi_crouchdownrocket"  ; t.value1 = 6316 ; t.value2 = 6356; }
					if (  t.n == 73 ) { t.field_s = "csi_crouchrolldown"  ; t.value1 = 2160 ; t.value2 = 2216; }
					if (  t.n == 74 ) { t.field_s = "csi_crouchrollup"  ; t.value1 = 2225 ; t.value2 = 2281; }
					if (  t.n == 75 ) { t.field_s = "csi_crouchmovefore"  ; t.value1 = 2075 ; t.value2 = 2102; }
					if (  t.n == 76 ) { t.field_s = "csi_crouchmoveback"  ; t.value1 = 2102 ; t.value2 = 2131; }
					if (  t.n == 77 ) { t.field_s = "csi_crouchmoveleft"  ; t.value1 = 2015 ; t.value2 = 2043; }
					if (  t.n == 78 ) { t.field_s = "csi_crouchmoveright"  ; t.value1 = 2043 ; t.value2 = 2072; }
					if (  t.n == 79 ) { t.field_s = "csi_crouchmoverun"  ; t.value1 = 2135 ; t.value2 = 2153; }
					if (  t.n == 80 ) { t.field_s = "csi_crouchreload"  ; t.value1 = 1920 ; t.value2 = 2010; }
					if (  t.n == 81 ) { t.field_s = "csi_crouchreloadrocket"  ; t.value1 = 6380 ; t.value2 = 6471; }
					if (  t.n == 82 ) { t.field_s = "csi_crouchwave"  ; t.value1 = 2460 ; t.value2 = 2510; }
					if (  t.n == 83 ) { t.field_s = "csi_crouchtoss"  ; t.value1 = 2520 ; t.value2 = 2555; }
					if (  t.n == 84 ) { t.field_s = "csi_crouchfirerocket"  ; t.value1 = 6357 ; t.value2 = 6379; }
					if (  t.n == 85 ) { t.field_s = "csi_crouchimpactfore"  ; t.value1 = 5240 ; t.value2 = 5277; }
					if (  t.n == 86 ) { t.field_s = "csi_crouchimpactback"  ; t.value1 = 5290 ; t.value2 = 5339; }
					if (  t.n == 87 ) { t.field_s = "csi_crouchimpactleft"  ; t.value1 = 5409 ; t.value2 = 5466; }
					if (  t.n == 88 ) { t.field_s = "csi_crouchimpactright"  ; t.value1 = 5350 ; t.value2 = 5395; }
					if (  t.n == 89 ) { t.field_s = "csi_crouchgetup"  ; t.value1 = 1646 ; t.value2 = 1663; }
					if (  t.n == 90 ) { t.field_s = "csi_crouchgetuprocket"  ; t.value1 = 6573 ; t.value2 = 6607; }
					darkai_assignanimtofield ( );
				}
			}
		}

		//  Finish animation quantities
		t.entityprofile[t.entid].animmax=t.tnewanimmax;
		t.entityprofile[t.entid].startofaianim=t.tstartofaianim;

		//  Localisation must change desc to local name
		if (  t.entityprofileheader[t.entid].desc_s != "" ) 
		{
			if (  cstr(Left(t.entityprofileheader[t.entid].desc_s.Get(),1)) != "%" ) 
			{
				t.tflocalfilename_s=cstr("languagebank\\")+g.language_s+"\\textfiles\\library\\"+t.entdir_s+t.ent_s;
				t.tflocalfilename_s=cstr(Left(t.tflocalfilename_s.Get(),Len(t.tflocalfilename_s.Get())-4))+cstr(".loc");
				if (  FileExist(t.tflocalfilename_s.Get()) == 1 ) 
				{
					Dim (  t.tflocal_s,1  );
					LoadArray (  t.tflocalfilename_s.Get() ,t.tflocal_s );
					t.entityprofileheader[t.entid].desc_s=t.tflocal_s[0];
					UnDim (  t.tflocal_s );
				}
			}
		}

		//  Translate entity references inside entity profile (token translations)
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%key" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[472];
		}
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%light" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[473];
		}
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%remote door" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[474];
		}
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%teleporter in" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[615];
		}
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%teleporter out" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[616];
		}
		if (  cstr(Lower(t.entityprofileheader[t.entid].desc_s.Get())) == "%lift" ) 
		{
			t.entityprofileheader[t.entid].desc_s=t.strarr_s[617];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].usekey_s.Get())) == "%key" ) 
		{
			t.entityprofile[t.entid].usekey_s=t.strarr_s[472];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].ifused_s.Get())) == "%light" ) 
		{
			t.entityprofile[t.entid].ifused_s=t.strarr_s[473];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].ifused_s.Get())) == "%remote door" ) 
		{
			t.entityprofile[t.entid].ifused_s=t.strarr_s[474];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].ifused_s.Get())) == "%teleporter in" ) 
		{
			t.entityprofile[t.entid].ifused_s=t.strarr_s[615];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].ifused_s.Get())) == "%teleporter out" ) 
		{
			t.entityprofile[t.entid].ifused_s=t.strarr_s[616];
		}
		if (  cstr(Lower(t.entityprofile[t.entid].ifused_s.Get())) == "%lift" ) 
		{
			t.entityprofile[t.entid].ifused_s=t.strarr_s[617];
		}

		//  All profile defaults
		if (  t.entityprofile[t.entid].ismarker != 1 ) 
		{
			if (  t.entityprofile[t.entid].lives<1  )  t.entityprofile[t.entid].lives = 1;
		}
		if (  t.entityprofile[t.entid].speed == 0  )  t.entityprofile[t.entid].speed = 100;
		if (  t.entityprofile[t.entid].hurtfall == 0  )  t.entityprofile[t.entid].hurtfall = 100;

		//  Physics Data Defaults
		if (  t.entityprofile[t.entid].ismarker == 0 ) 
		{
			//  default physics settings (weight and friction done during object load (we need the obj size!)
			//  health packs have no physics by default for A compatibility
			if (  t.entityprofile[t.entid].ishealth != 0 ) 
			{
				t.entityprofile[t.entid].physics=0;
			}
		}
		else
		{
			t.entityprofile[t.entid].physics=0;
		}

		//  LOD System Defaults
		if ( t.entityprofile[t.entid].lod1distance > 0 && t.entityprofile[t.entid].lod2distance == 0 )
		{
			t.entityprofile[t.entid].lod2distance = t.entityprofile[t.entid].lod1distance;
		}

		//  Spawn defaults
		t.entityprofile[t.entid].spawnatstart=1;
		t.entityprofile[t.entid].spawndelayrandom=0;
		t.entityprofile[t.entid].spawnqtyrandom=0;
		t.entityprofile[t.entid].spawnvel=0;
		t.entityprofile[t.entid].spawnvelrandom=0;
		t.entityprofile[t.entid].spawnangle=0;
		t.entityprofile[t.entid].spawnanglerandom=0;
		t.entityprofile[t.entid].spawnlife=0;
		if (  t.entityprofile[t.entid].spawnmax>0 ) 
		{
			t.entityprofile[t.entid].spawnupto=t.entityprofile[t.entid].spawnmax;
			t.entityprofile[t.entid].spawnafterdelay=1;
			if (  t.entityprofile[t.entid].ischaracter == 1 ) 
			{
				t.entityprofile[t.entid].spawnwhendead=1;
			}
			else
			{
				t.entityprofile[t.entid].spawnwhendead=0;
			}
		}
		else
		{
			t.entityprofile[t.entid].spawnupto=0;
			t.entityprofile[t.entid].spawnafterdelay=0;
			t.entityprofile[t.entid].spawnwhendead=0;
		}

		//  Fix scale for FPE
		if (  t.entityprofile[t.entid].scale == 0 ) 
		{
			t.entityprofile[t.entid].scale=100;
		}

		// 010917 - if shader effect is a decal, auto switch zdepth flag (shader no longer does this in DX11)
		LPSTR pEffectMatch = "effectbank\\reloaded\\decal";
		if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pEffectMatch, strlen(pEffectMatch) ) == NULL ) 
		{
			t.entityprofile[t.entid].zdepth = 0;
		}

		// 261117 - intercept and replace any legacy shaders with new PBR ones if game visuals using RealtimePBR (3) mode
		if ( g.gpbroverride == 1 )
		{
			int iReplaceMode = 0;
			LPSTR pTryMatch = "effectbank\\reloaded\\entity_basic.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 1;
			pTryMatch = "effectbank\\reloaded\\character_static.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 1;
			pTryMatch = "effectbank\\reloaded\\entity_anim.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
			pTryMatch = "effectbank\\reloaded\\character_basic.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
			pTryMatch = "effectbank\\reloaded\\character_transparency.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
			pTryMatch = "effectbank\\reloaded\\tree_basic.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 3;			
			pTryMatch = "effectbank\\reloaded\\treea_basic.fx";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 4;			
			if ( iReplaceMode > 0 )
			{
				if ( iReplaceMode == 1 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\apbr_basic.fx";
				if ( iReplaceMode == 2 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\apbr_anim.fx";
				if ( iReplaceMode == 3 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\apbr_tree.fx";
				if ( iReplaceMode == 4 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\apbr_treea.fx";
			}
		}
		else
		{
			// 120418 - conversely, if PBR override not active, and have new PBR asset entities that still 
			// have old DNS textures, switch them back to classic non-PBR (this allows new PBR assets to 
			// replace older legacy assets but still allow backwards compatibility for users who want the
			// old shaders and old textures to remain in effect using PBR override of zero)
			char pEntityItemPath[1024];
			strcpy ( pEntityItemPath, t.ent_s.Get() );
			int n = 0;
			for ( n = strlen(pEntityItemPath)-1; n > 0; n-- )
			{
				if ( pEntityItemPath[n] == '\\' || pEntityItemPath[n] == '/' )
				{
					pEntityItemPath[n+1] = 0;
					break;
				}
			}
			if ( n <= 0 ) strcpy ( pEntityItemPath, "" );
			char pJustTextureName[1024];
			strcpy ( pJustTextureName, t.entityprofile[t.entid].texd_s.Get() );
			if ( strlen ( pJustTextureName ) > 4 )
			{
				pJustTextureName[strlen(pJustTextureName)-4]=0;
				if ( stricmp ( pJustTextureName+strlen(pJustTextureName)-6, "_color" ) == NULL )
				{
					pJustTextureName[strlen(pJustTextureName)-6]=0;
					strcat ( pJustTextureName, "_D" );
				}
				strcat ( pJustTextureName, ".png" );
			}
			char pReplaceWithDNS[1024];
			strcpy ( pReplaceWithDNS, pEntityItemPath );
			strcat ( pReplaceWithDNS, pJustTextureName );
			bool bReplacePBRWithNonPBRDNS = false;
			LPSTR pPBREffectMatch = "effectbank\\reloaded\\apbr";
			if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pPBREffectMatch, strlen(pPBREffectMatch) ) == NULL ) 
			{
				// entity effect specifies PBR, do we have the DNS files available
				if ( strlen ( pJustTextureName ) > 4 )
				{
					cstr pFindDNSFile = t.entdir_s + pReplaceWithDNS;
					if ( FileExist ( pFindDNSFile.Get() ) == 0 )
					{
						pReplaceWithDNS[strlen(pReplaceWithDNS)-4]=0;
						strcat ( pReplaceWithDNS, ".dds" );
						pFindDNSFile = t.entdir_s + pReplaceWithDNS;
						if ( FileExist ( pFindDNSFile.Get() ) == 0 )
						{
							pReplaceWithDNS[strlen(pReplaceWithDNS)-4]=0;
							strcat ( pReplaceWithDNS, ".jpg" );
							pFindDNSFile = t.entdir_s + pReplaceWithDNS;
							if ( FileExist ( pFindDNSFile.Get() ) == 1 )
							{
								bReplacePBRWithNonPBRDNS = true;
							}
						}
						else
						{
							bReplacePBRWithNonPBRDNS = true;
						}
					}
					else
					{
						bReplacePBRWithNonPBRDNS = true;
					}
				}
				else
				{
					// no texture specified, but can still switch to classic shaders (legacy behavior)
					bReplacePBRWithNonPBRDNS = true;
				}
			}
			if ( bReplacePBRWithNonPBRDNS == true )
			{
				// replace the shader used
				int iReplaceMode = 0;
				LPSTR pTryMatch = "effectbank\\reloaded\\apbr_basic.fx";
				if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 1;
				pTryMatch = "effectbank\\reloaded\\apbr_anim.fx";
				if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
				pTryMatch = "effectbank\\reloaded\\apbr_tree.fx";
				if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 3;
				pTryMatch = "effectbank\\reloaded\\apbr_treea.fx";
				if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 4;
				if ( iReplaceMode > 0 )
				{
					if ( iReplaceMode == 1 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\entity_basic.fx";
					if ( iReplaceMode == 2 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\character_basic.fx";
					if ( iReplaceMode == 3 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\tree_basic.fx";
					if ( iReplaceMode == 4 ) t.entityprofile[t.entid].effect_s = "effectbank\\reloaded\\treea_basic.fx";
				}

				// replace the texture specified (from _color to _D)
				t.entityprofile[t.entid].texd_s = pJustTextureName;
			}
		}

		// if effect shader starts with APBR, auto shift effectprofile from zero to one
		LPSTR pPBREffectMatch = "effectbank\\reloaded\\apbr";
		if ( strnicmp ( t.entityprofile[t.entid].effect_s.Get(), pPBREffectMatch, strlen(pPBREffectMatch) ) == NULL ) 
		{
			if ( t.entityprofile[t.entid].effectprofile == 0 )
				t.entityprofile[t.entid].effectprofile = 1;
		}
	}

	//  Can loop back if skipBIN flag set
	if (  t.skipBINloadingandtryagain == 1 ) 
	{
		timestampactivity(0,cstr(cstr(Str(t.tprotectBINfile))+" Entity BIN File Out Of Date: "+t.tprofile_s).Get());
		if (  t.game.gameisexe == 1 ) 
		{
			t.skipBINloadingandtryagain=0;
		}
		else
		{
			if (  t.tprotectBINfile == 0 ) 
			{
				if (  FileExist(t.tprofile_s.Get()) == 1  )  DeleteAFile (  t.tprofile_s.Get() );
			}
			else
			{
				t.skipBINloadingandtryagain=0;
			}
		}
	}

	} while ( !(  t.skipBINloadingandtryagain == 0 ) );

	//  new field as we now have pure lights and entity lights
	if (  t.entityprofile[t.entid].ismarker == 2 || t.entityprofile[t.entid].ismarker == 5 ) 
	{
		if (  t.entityprofile[t.entid].ismarker == 5  )  t.entityprofile[t.entid].ismarker = 0;
		t.entityprofile[t.entid].islightmarker=1;
		//  FPGC - 300310 - entitylights always active as they may control a dynamic light and possibly decal-particle(mode7)
		t.entityprofile[t.entid].phyalways=1;
	}
	else
	{
		t.entityprofile[t.entid].islightmarker=0;
	}

	//  FPGC - 100610 - all FPGC characters are ALWAYS ACTIVE for full speed logic (more predictable)
	if (  t.entityprofile[t.entid].ischaracter == 1 && g.fpgcgenre == 0 ) 
	{
		t.entityprofile[t.entid].phyalways=1;
		//  FPGC - 110610 - and ALL are invincible
		t.entityprofile[t.entid].strength=0;
	}

	//  fileexistelse
	}
	else
	{
		//  File not exist, provide debug information (only if file specified (old entities can be renamed and still hang around inside FPMs)
		if (  Len( cstr(t.entdir_s+t.ent_s).Get() )>Len("entitybank\\") ) 
		{
			debugfilename( cstr(t.entdir_s+t.ent_s).Get(),t.tprofile_s.Get() );
		}
	}

	//  V109 BETA5 - 250408 - flag material is being used
	if (  t.entityprofile[t.entid].materialindex>0 ) 
	{
		t.mi=t.entityprofile[t.entid].materialindex-1;
		t.material[t.mi].usedinlevel=1;
	}
	if (  t.entityprofile[t.entid].debrisshapeindex>0 ) 
	{
		t.di=t.entityprofile[t.entid].debrisshapeindex;
		t.debrisshapeindexused[t.di]=1;
	}

	// if flagged as EBE, attempt to load any EBE cube data
	if ( t.entityprofile[t.entid].isebe != 0 )
	{
		cstr sEBEFile = cstr(Left(t.tFPEName_s.Get(),strlen(t.tFPEName_s.Get())-4)) + cstr(".ebe");
		if ( FileExist ( sEBEFile.Get() ) ) 
		{
			// load EBE data into entityID
			ebe_load_ebefile ( sEBEFile, t.entid );
		}
		else
		{
			// 300817 - EBE has had .ebe file removed to make it regular entity, so remove handle limb
			if ( t.entityprofile[t.entid].isebe == 1 ) 
			{
				t.entityprofile[t.entid].isebe = 2;
			}
		}
	}
}

void entity_loadvideoid ( void )
{
	t.tvideoid=0;
	t.text_s = Lower(Right(t.tvideofile_s.Get(),4));
	if ( t.text_s == ".ogv" || t.text_s == ".mp4" ) 
	{
		t.tvideoid=32;
		for ( t.tt = 1 ; t.tt<=  32; t.tt++ )
		{
			if (  AnimationExist(t.tt) == 0 ) { t.tvideoid = t.tt  ; break; }
		}
		LoadAnimation ( t.tvideofile_s.Get(), t.tvideoid );
	}
}

void entity_loadactivesoundsandvideo ( void )
{
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entid>0 ) 
		{
			if (  t.entityelement[t.e].active == 1 ) 
			{
				if (  t.entityelement[t.e].soundset == 0 ) 
				{
					t.tvideofile_s=t.entityelement[t.e].eleprof.soundset_s ; entity_loadvideoid ( );
					if (  t.tvideoid>0 ) 
						t.entityelement[t.e].soundset=t.tvideoid*-1;
					else
						t.entityelement[t.e].soundset=loadinternalsoundcore(t.entityelement[t.e].eleprof.soundset_s.Get(),1);
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
				}
				if (  t.entityelement[t.e].soundset1 == 0 ) 
				{
					t.tvideofile_s=t.entityelement[t.e].eleprof.soundset1_s ; entity_loadvideoid ( );
					if (  t.tvideoid>0 ) 
						t.entityelement[t.e].soundset1=t.tvideoid*-1;
					else
						t.entityelement[t.e].soundset1=loadinternalsoundcore(t.entityelement[t.e].eleprof.soundset1_s.Get(),1);
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
				}
				if (  t.entityelement[t.e].soundset2 == 0 ) 
				{
					t.entityelement[t.e].soundset2=loadinternalsoundcore(t.entityelement[t.e].eleprof.soundset2_s.Get(),1);
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
				}
				if (  t.entityelement[t.e].soundset3 == 0 ) 
				{
					t.entityelement[t.e].soundset3=loadinternalsoundcore(t.entityelement[t.e].eleprof.soundset3_s.Get(),1);
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
				}
				if (  t.entityelement[t.e].soundset4 == 0 ) 
				{
					t.entityelement[t.e].soundset4=loadinternalsoundcore(t.entityelement[t.e].eleprof.soundset4_s.Get(),1);
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
				}
			}
		}
	}

return;

}

void entity_fillgrideleproffromprofile ( void )
{
	//  Name
	t.grideleprof.name_s=t.entityprofileheader[t.entid].desc_s;

	//  AI values
	t.grideleprof.aiinit_s=t.entityprofile[t.entid].aiinit_s;
	t.grideleprof.aimain_s=t.entityprofile[t.entid].aimain_s;
	t.grideleprof.aidestroy_s=t.entityprofile[t.entid].aidestroy_s;
	t.grideleprof.aishoot_s=t.entityprofile[t.entid].aishoot_s;

	//  AI use vars
	t.grideleprof.usekey_s=t.entityprofile[t.entid].usekey_s;
	t.grideleprof.ifused_s=t.entityprofile[t.entid].ifused_s;
	t.grideleprof.ifusednear_s=t.entityprofile[t.entid].ifusednear_s;

	//  Spawn
	t.grideleprof.spawnatstart=t.entityprofile[t.entid].spawnatstart;
	t.grideleprof.spawnmax=t.entityprofile[t.entid].spawnmax;
	t.grideleprof.spawndelay=t.entityprofile[t.entid].spawndelay;
	t.grideleprof.spawnqty=t.entityprofile[t.entid].spawnqty;
	t.grideleprof.spawnupto=t.entityprofile[t.entid].spawnupto;
	t.grideleprof.spawnafterdelay=t.entityprofile[t.entid].spawnafterdelay;
	t.grideleprof.spawnwhendead=t.entityprofile[t.entid].spawnwhendead;
	t.grideleprof.spawndelayrandom=t.entityprofile[t.entid].spawndelayrandom;
	t.grideleprof.spawnqtyrandom=t.entityprofile[t.entid].spawnqtyrandom;
	t.grideleprof.spawnvel=t.entityprofile[t.entid].spawnvel;
	t.grideleprof.spawnvelrandom=t.entityprofile[t.entid].spawnvelrandom;
	t.grideleprof.spawnangle=t.entityprofile[t.entid].spawnangle;
	t.grideleprof.spawnanglerandom=t.entityprofile[t.entid].spawnanglerandom;
	t.grideleprof.spawnlife=t.entityprofile[t.entid].spawnlife;

	//  Scale, Cone
	t.grideleprof.scale=t.entityprofile[t.entid].scale;
	t.grideleprof.coneheight=t.entityprofile[t.entid].coneheight;
	t.grideleprof.coneangle=t.entityprofile[t.entid].coneangle;
	t.grideleprof.conerange=t.entityprofile[t.entid].conerange;

	//  Texture and Effect Data
	t.grideleprof.uniqueelement=0;
	t.grideleprof.texd_s=t.entityprofile[t.entid].texd_s;
	t.grideleprof.texaltd_s=t.entityprofile[t.entid].texaltd_s;
	t.grideleprof.effect_s=t.entityprofile[t.entid].effect_s;
	t.grideleprof.transparency=t.entityprofile[t.entid].transparency;
	t.grideleprof.castshadow=t.entityprofile[t.entid].castshadow;
	t.grideleprof.reducetexture=t.entityprofile[t.entid].reducetexture;

	//  Strength and Quantity
	t.grideleprof.strength=t.entityprofile[t.entid].strength;
	t.grideleprof.lives=t.entityprofile[t.entid].lives;
	t.grideleprof.isimmobile=t.entityprofile[t.entid].isimmobile;
	t.grideleprof.lodmodifier=t.entityprofile[t.entid].lodmodifier;
	t.grideleprof.isocluder=t.entityprofile[t.entid].isocluder;
	t.grideleprof.isocludee=t.entityprofile[t.entid].isocludee;
	t.grideleprof.specularperc=t.entityprofile[t.entid].specularperc;
	t.grideleprof.colondeath=t.entityprofile[t.entid].colondeath;
	t.grideleprof.parententityindex=t.entityprofile[t.entid].parententityindex;
	t.grideleprof.parentlimbindex=t.entityprofile[t.entid].parentlimbindex;
	t.grideleprof.isviolent=t.entityprofile[t.entid].isviolent;
	t.grideleprof.cantakeweapon=t.entityprofile[t.entid].cantakeweapon;
	t.grideleprof.hasweapon_s=t.entityprofile[t.entid].hasweapon_s;
	t.grideleprof.quantity=t.entityprofile[t.entid].quantity;
	t.grideleprof.isobjective=t.entityprofile[t.entid].isobjective;
	t.grideleprof.hurtfall=t.entityprofile[t.entid].hurtfall;
	t.grideleprof.speed=t.entityprofile[t.entid].speed;
	t.grideleprof.animspeed=t.entityprofile[t.entid].animspeed;

	//  Decal and Sound Name
	t.grideleprof.basedecal_s=t.entitydecal_s[t.entid][0];
	t.grideleprof.soundset_s=t.entityprofile[t.entid].soundset_s;
	t.grideleprof.soundset1_s=t.entityprofile[t.entid].soundset1_s;
	t.grideleprof.soundset2_s=t.entityprofile[t.entid].soundset2_s;
	t.grideleprof.soundset3_s=t.entityprofile[t.entid].soundset3_s;
	t.grideleprof.soundset4_s=t.entityprofile[t.entid].soundset4_s;

	//  FPGC - 310710 - decal particle settings
	t.particlefile_s=t.grideleprof.basedecal_s;
	decal_getparticlefile ( );
	t.grideleprof.particleoverride=1;
	t.grideleprof.particle=g.gotparticle;

	//  Marker Data
	t.grideleprof.markerindex=t.entityprofile[t.entid].markerindex;
	t.grideleprof.light=t.entityprofile[t.entid].light;
	t.grideleprof.trigger=t.entityprofile[t.entid].trigger;
	t.grideleprof.usespotlighting=t.entityprofile[t.entid].usespotlighting;

	//  Data Extracted From GUN and FLAK
	t.tgunid_s=t.entityprofile[t.entid].isweapon_s;
	entity_getgunidandflakid ( );
	t.grideleprof.rateoffire=t.entityprofile[t.entid].rateoffire;
	t.grideleprof.weaponisammo=0;
	if (  t.tgunid>0 ) 
	{
		t.grideleprof.accuracy=g.firemodes[t.tgunid][0].settings.accuracy;
		t.grideleprof.reloadqty=g.firemodes[t.tgunid][0].settings.reloadqty;
		t.grideleprof.fireiterations=g.firemodes[t.tgunid][0].settings.iterate;
		t.grideleprof.usespotlighting=g.firemodes[t.tgunid][0].settings.usespotlighting;
		if (  t.tflakid == 0 ) 
		{
			t.grideleprof.damage=g.firemodes[t.tgunid][0].settings.damage;
			t.grideleprof.range=g.firemodes[t.tgunid][0].settings.range;
			t.grideleprof.dropoff=g.firemodes[t.tgunid][0].settings.dropoff;
		}
		else
		{
			t.grideleprof.damage=0;
			t.grideleprof.lifespan=0;
			t.grideleprof.throwspeed=0;
			t.grideleprof.throwangle=0;
			t.grideleprof.bounceqty=0;
			t.grideleprof.explodeonhit=0;
			t.grideleprof.weaponisammo=t.tflakid;
		}
	}

	//  Physics Data
	t.grideleprof.physics=t.entityprofile[t.entid].physics;
	t.grideleprof.phyalways=t.entityprofile[t.entid].phyalways;
	t.grideleprof.phyweight=t.entityprofile[t.entid].phyweight;
	t.grideleprof.phyfriction=t.entityprofile[t.entid].phyfriction;
	t.grideleprof.phyforcedamage=t.entityprofile[t.entid].phyforcedamage;
	t.grideleprof.rotatethrow=t.entityprofile[t.entid].rotatethrow;
	t.grideleprof.explodable=t.entityprofile[t.entid].explodable;
	t.grideleprof.explodedamage=t.entityprofile[t.entid].explodedamage;
	t.grideleprof.teamfield=t.entityprofile[t.entid].teamfield;

	// 301115 - data extracted from neighbors (LOD Modifiers are shared across all parent copies)
	int iThisBankIndex = t.entid;
	if ( t.entityprofile[iThisBankIndex].addhandlelimb==0 )
	{
		for ( int e=1; e<=g.entityelementlist; e++ )
		{
			if ( t.entityelement[e].bankindex==iThisBankIndex )
			{
				t.grideleprof.lodmodifier = t.entityelement[e].eleprof.lodmodifier;
				break;
			}
		}
	}
}

void entity_updatetextureandeffectfromeleprof ( void )
{

	//  Texture and Effect (use entityprofile loader)
	//t.storeentdefaults as entityprofiletype;
	t.storeentdefaults=t.entityprofile[t.entid];
	t.entityprofile[t.entid].texd_s=t.entityelement[t.e].eleprof.texd_s;
	t.entityprofile[t.entid].texaltd_s=t.entityelement[t.e].eleprof.texaltd_s;
	t.entityprofile[t.entid].texdid=t.entityelement[t.e].eleprof.texdid;
	t.entityprofile[t.entid].texaltdid=t.entityelement[t.e].eleprof.texaltdid;
	t.entityprofile[t.entid].effect_s=t.entityelement[t.e].eleprof.effect_s;
	t.entityprofile[t.entid].usingeffect=t.entityelement[t.e].eleprof.usingeffect;
	t.entityprofile[t.entid].texnid=t.entityelement[t.e].eleprof.texnid;
	t.entityprofile[t.entid].texsid=t.entityelement[t.e].eleprof.texsid;
	t.entityprofile[t.entid].texidmax=t.entityelement[t.e].eleprof.texidmax;
	t.entityprofile[t.entid].transparency=t.entityelement[t.e].eleprof.transparency;
	t.entityprofile[t.entid].reducetexture=t.entityelement[t.e].eleprof.reducetexture;
	entity_loadtexturesandeffect ( );
	t.entityelement[t.e].eleprof.texd_s=t.entityprofile[t.entid].texd_s;
	t.entityelement[t.e].eleprof.texaltd_s=t.entityprofile[t.entid].texaltd_s;
	t.entityelement[t.e].eleprof.texdid=t.entityprofile[t.entid].texdid;
	t.entityelement[t.e].eleprof.texaltdid=t.entityprofile[t.entid].texaltdid;
	t.entityelement[t.e].eleprof.effect_s=t.entityprofile[t.entid].effect_s;
	t.entityelement[t.e].eleprof.usingeffect=t.entityprofile[t.entid].usingeffect;
	t.entityelement[t.e].eleprof.texnid=t.entityprofile[t.entid].texnid;
	t.entityelement[t.e].eleprof.texsid=t.entityprofile[t.entid].texsid;
	t.entityelement[t.e].eleprof.texidmax=t.entityprofile[t.entid].texidmax;
	t.entityelement[t.e].eleprof.transparency=t.entityprofile[t.entid].transparency;
	t.entityelement[t.e].eleprof.reducetexture=t.entityprofile[t.entid].reducetexture;
	t.entityprofile[t.entid]=t.storeentdefaults;

return;

}

void entity_updatetextureandeffectfromgrideleprof ( void )
{

	//  Texture and Effect (use entityprofile loader)
	//t.storeentdefaults as entityprofiletype;
	t.storeentdefaults=t.entityprofile[t.entid];
	t.entityprofile[t.entid].texd_s=t.grideleprof.texd_s;
	t.entityprofile[t.entid].texaltd_s=t.grideleprof.texaltd_s;
	t.entityprofile[t.entid].texdid=t.grideleprof.texdid;
	t.entityprofile[t.entid].texaltdid=t.grideleprof.texaltdid;
	t.entityprofile[t.entid].effect_s=t.grideleprof.effect_s;
	t.entityprofile[t.entid].usingeffect=t.grideleprof.usingeffect;
	t.entityprofile[t.entid].texnid=t.grideleprof.texnid;
	t.entityprofile[t.entid].texsid=t.grideleprof.texsid;
	t.entityprofile[t.entid].texidmax=t.grideleprof.texidmax;
	t.entityprofile[t.entid].transparency=t.grideleprof.transparency;
	t.entityprofile[t.entid].reducetexture=t.grideleprof.reducetexture;
	entity_loadtexturesandeffect ( );
	t.grideleprof.texd_s=t.entityprofile[t.entid].texd_s;
	t.grideleprof.texaltd_s=t.entityprofile[t.entid].texaltd_s;
	t.grideleprof.texdid=t.entityprofile[t.entid].texdid;
	t.grideleprof.texaltdid=t.entityprofile[t.entid].texaltdid;
	t.grideleprof.effect_s=t.entityprofile[t.entid].effect_s;
	t.grideleprof.usingeffect=t.entityprofile[t.entid].usingeffect;
	t.grideleprof.texnid=t.entityprofile[t.entid].texnid;
	t.grideleprof.texsid=t.entityprofile[t.entid].texsid;
	t.grideleprof.texidmax=t.entityprofile[t.entid].texidmax;
	t.grideleprof.transparency=t.entityprofile[t.entid].transparency;
	t.grideleprof.reducetexture=t.entityprofile[t.entid].reducetexture;
	t.entityprofile[t.entid]=t.storeentdefaults;
}

void entity_getgunidandflakid ( void )
{
	//  Use Weapon Name to get GUNID and FLAKID
	if (  t.tgunid_s != "" ) 
	{
		//  get gun
		t.findgun_s=Lower(t.tgunid_s.Get());
		gun_findweaponindexbyname ( );
		t.tgunid=t.foundgunid;
		//  no flak - old system
		t.tflakid=0;
	}
	else
	{
		t.tgunid=0 ; t.tflakid=0;
	}
}

void entity_loadtexturesandeffect ( void )
{
	//  If entity object not exist, reset var
	if (  t.entobj>0 ) 
	{
		if (  ObjectExist(t.entobj) == 0  )  t.entobj = 0;
	}

	//  Only characters need a higher quality texture, rest use divide standard settings
	t.tfullorhalfdivide=0;
	if (  t.segobjusedformapeditor == 0 ) 
	{
		if (  t.entityprofile[t.entid].ischaracter == 1 ) 
		{
			t.tfullorhalfdivide=2;
		}
		else
		{
			if (  t.entityprofile[t.entid].reducetexture != 0 ) 
			{
				if (  t.entityprofile[t.entid].reducetexture == -1 ) 
				{
					t.tfullorhalfdivide=1;
				}
				else
				{
					t.tfullorhalfdivide=2;
				}
			}
		}
	}

	//  Apply TEXTURE to entity object
	bool bMultiMaterialObject = false;
	t.tuseeffecttexture=0;
	t.tfile_s=t.entityprofile[t.entid].texd_s;
	t.tfilealt_s=t.entityprofile[t.entid].texaltd_s;
	if ( t.tfile_s != "" ) 
	{
		//  optional texture path for getting at any texture (gamecore gun textures, etc)
		if (  t.entityprofile[t.entid].texpath_s != "" ) 
		{
			t.texdir_s=t.entityprofile[t.entid].texpath_s+t.tfile_s;
			t.texaltdir_s=t.entityprofile[t.entid].texpath_s+t.tfilealt_s;
		}
		else
		{
			t.texdir_s=t.entdir_s+t.entpath_s+t.tfile_s;
			t.texaltdir_s=t.entdir_s+t.entpath_s+t.tfilealt_s;
		}
		t.tthistexdir_s=t.texdir_s;
		if (  g.gdividetexturesize == 0  )  t.tthistexdir_s = "effectbank\\reloaded\\media\\white_D.dds";
		if (  t.entityprofile[t.entid].transparency == 0 ) 
		{
			t.texuseid=loadinternaltextureex(t.tthistexdir_s.Get(),1,t.tfullorhalfdivide);
		}
		else
		{
			t.texuseid=loadinternaltextureex(t.tthistexdir_s.Get(),5,t.tfullorhalfdivide);
		}
		if (  t.texuseid == 0 ) 
		{
			t.texdir_s=t.entityprofile[t.entid].texd_s;
			t.texaltdir_s=t.entityprofile[t.entid].texaltd_s;
			t.texuseid=loadinternaltextureex(t.texdir_s.Get(),1,t.tfullorhalfdivide);
		}
		if (  t.texuseid == 0 ) 
		{
			//  if still no texture, maybe FPE specifies it WRONG and correct is inside model
			for ( t.tlimbindex = 0 ; t.tlimbindex<=  999; t.tlimbindex++ )
			{
				if (  LimbExist(t.entobj,t.tlimbindex) == 1 ) 
				{
					t.tlimbtex_s=t.entdir_s+t.entpath_s+LimbTextureName(t.entobj,t.tlimbindex);
					t.tispresent=FileExist(t.tlimbtex_s.Get());
					if (  t.tispresent == 1 ) 
					{
						break;
					}
				}
			}
			t.texdir_s=t.tlimbtex_s;
			t.texaltdir_s=t.entityprofile[t.entid].texaltd_s;
			t.texuseid=loadinternaltextureex(t.texdir_s.Get(),1,t.tfullorhalfdivide);
		}

		//  Load ALT texture if available
		t.texaltdid=loadinternalimagecompressquality(t.texaltdir_s.Get(),1,t.tfullorhalfdivide);
		t.entityprofile[t.entid].texaltdid=t.texaltdid;
		bMultiMaterialObject = false;
	}
	else
	{
		// textured field left blank to indicate multi-material has filled in and loaded texture stages already
		t.texdir_s="" ; t.texuseid=0;
		t.texaltdid=0 ; t.entityprofile[t.entid].texaltdid=t.texaltdid;
		bMultiMaterialObject = true;
	}
	
	// 261117 - intercept and replace any legacy shaders with new PBR ones if game visuals using RealtimePBR (3) mode
	cstr EffectFile_s = t.entityprofile[t.entid].effect_s;
	int iEffectProfile = t.entityprofile[t.entid].effectprofile;
	/* back to load step which will make them permanent and mans effectprofile flag allows cheapshadow gen at stage 4 to be skipped!
	if ( g.gpbroverride == 1 )
	{
		int iReplaceMode = 0;
		LPSTR pTryMatch = "effectbank\\reloaded\\entity_basic.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 1;
		pTryMatch = "effectbank\\reloaded\\character_static.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 1;
		pTryMatch = "effectbank\\reloaded\\entity_anim.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
		pTryMatch = "effectbank\\reloaded\\character_basic.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
		pTryMatch = "effectbank\\reloaded\\character_transparency.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 2;
		pTryMatch = "effectbank\\reloaded\\tree_basic.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 3;			
		pTryMatch = "effectbank\\reloaded\\treea_basic.fx";
		if ( strnicmp ( EffectFile_s.Get(), pTryMatch, strlen(pTryMatch) ) == NULL ) iReplaceMode = 4;			
		if ( iReplaceMode > 0 )
		{
			if ( iReplaceMode == 1 ) EffectFile_s = "effectbank\\reloaded\\apbr_basic.fx";
			if ( iReplaceMode == 2 ) EffectFile_s = "effectbank\\reloaded\\apbr_anim.fx";
			if ( iReplaceMode == 3 ) EffectFile_s = "effectbank\\reloaded\\apbr_tree.fx";
			if ( iReplaceMode == 4 ) EffectFile_s = "effectbank\\reloaded\\apbr_treea.fx";
		}
	}
	if ( iEffectProfile == 0 )
	{
		LPSTR pPBREffectMatch = "effectbank\\reloaded\\apbr";
		if ( strnicmp ( EffectFile_s.Get(), pPBREffectMatch, strlen(pPBREffectMatch) ) == NULL ) 
			iEffectProfile = 1;
	}
	*/

	//  Load entity effect
	t.entityprofile[t.entid].usingeffect=0;
	if (  t.entityprofile[t.entid].ismarker == 0 ) 
	{
		t.tfile_s=EffectFile_s;
		common_wipeeffectifnotexist ( );
		if (  t.tfile_s != "" ) 
		{
			t.teffectid=loadinternaleffect(t.tfile_s.Get());
			if (  t.teffectid>0 ) 
			{
				t.entityprofile[t.entid].usingeffect=t.teffectid;
			}
		}
	}
	else
	{
		// 110517 - no fixed function any more
		t.entityprofile[t.entid].usingeffect = g.guishadereffectindex;
	}

	// Texture and apply effect
	if ( t.entobj>0 ) 
	{
		// lee - 300518 - added extra code in LoadObject to detect DNS and PBR texture file sets and set the mesh, so 
		// skip the override code below if the object has a good texture in place
		//bool bGotAO = false - replaced this with a later scan to add AO only when missing
		bool bGotNormal = false, bGotMetalness = false, bGotGloss = false;
		sObject* pObject = GetObjectData ( t.entobj );
		if ( pObject )
		{
			for ( int iMeshIndex = 0; iMeshIndex < pObject->iMeshCount; iMeshIndex++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
				if ( pMesh )
				{
					for ( int iTextureIndex = 2; iTextureIndex < pMesh->dwTextureCount; iTextureIndex++ )
					{
						if ( pMesh->pTextures[iTextureIndex].iImageID > 0 )
						{
							//if ( iTextureIndex == 1 ) bGotAO = true;
							if ( iTextureIndex == 2 ) bGotNormal = true;
							if ( iTextureIndex == 3 ) bGotMetalness = true;
							if ( iTextureIndex == 4 ) bGotGloss = true;
						}
					}
				}
			}
		}

		// detect if using an effect or not
		int use_illumination = false;
		if ( t.entityprofile[t.entid].usingeffect == 0 ) 
		{
			//  No effect
			t.entityprofile[t.entid].texdid=t.texuseid;
			t.entityprofile[t.entid].texnid=0;
			t.entityprofile[t.entid].texsid=0;
			TextureObject ( t.entobj, t.texuseid );
		}
		else
		{
			if ( bMultiMaterialObject == false )
			{
				// Strip out _D.dds or COLOR.dds 
				char pNoExtFilename[1024];
				strcpy ( pNoExtFilename, t.texdir_s.Get() );
				pNoExtFilename[strlen(pNoExtFilename)-4] = 0;
				//PE: Some textures do not have _d,_color,_albedo , so always reset.
				t.texdirnoext_s = "";
				if ( strnicmp ( pNoExtFilename+strlen(pNoExtFilename)-2, "_d", 2 ) == NULL )
				{
					t.texdirnoext_s=Left(pNoExtFilename,Len(pNoExtFilename)-Len("_d"));
				}
				else
				{
					if ( strnicmp ( pNoExtFilename+strlen(pNoExtFilename)-6, "_color", 6 ) == NULL )
						t.texdirnoext_s=Left(pNoExtFilename,Len(pNoExtFilename)-Len("_color"));
					else
						if ( strnicmp ( pNoExtFilename+strlen(pNoExtFilename)-7, "_albedo", 7 ) == NULL )
							t.texdirnoext_s=Left(pNoExtFilename,Len(pNoExtFilename)-Len("_albedo"));
				}

				//  Assign DIFFUSE
				t.entityprofile[t.entid].texdid = t.texuseid;

				// Assign NORMAL
				if ( iEffectProfile == 1 )
					t.texdirN_s = t.texdirnoext_s+"_normal.dds";
				else
					t.texdirN_s = t.texdirnoext_s+"_n.dds";
				t.texuseid = loadinternaltextureex(t.texdirN_s.Get(),1,t.tfullorhalfdivide);
				if ( t.texuseid == 0 ) 
				{
					if ( iEffectProfile == 1 )
						t.texdirN_s = t.texdirnoext_s+"_n.dds";
					else
						t.texdirN_s = t.texdirnoext_s+"_normal.dds";
					t.texuseid = loadinternaltextureex(t.texdirN_s.Get(),1,t.tfullorhalfdivide);
					if ( t.texuseid == 0 ) 
					{
						t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_N.dds",1,t.tfullorhalfdivide);
					}
				}
				t.entityprofile[t.entid].texnid = t.texuseid;

				// Assign SPECULAR (PBR Metalness)
				if ( t.entityprofile[t.entid].specular == 0 ) 
				{
					if ( iEffectProfile == 1 )
					{
						t.texdirS_s = t.texdirnoext_s+"_metalness.dds";
						t.texuseid = loadinternaltextureex(t.texdirS_s.Get(),1,t.tfullorhalfdivide);
						if ( t.texuseid == 0 ) 
						{
							t.texdirS_s = t.texdirnoext_s+"_specular.dds";
							t.texuseid = loadinternaltextureex(t.texdirS_s.Get(),1,t.tfullorhalfdivide);
							if ( t.texuseid == 0 ) 
							{
								// 261117 - search material for stock metalness
								char pFullFilename[1024];
								int iMaterialIndex = t.entityprofile[t.entid].materialindex;
								sprintf ( pFullFilename, "effectbank\\reloaded\\media\\materials\\%d_Metalness.dds", iMaterialIndex );
								t.texdirS_s = pFullFilename;
								t.texuseid = loadinternaltextureex(t.texdirS_s.Get(),1,t.tfullorhalfdivide);
								if ( t.texuseid == 0 ) 
								{
									t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_black.dds",1,t.tfullorhalfdivide);
								}
							}
						}
					}
					else
					{
						t.texdirS_s = t.texdirnoext_s+"_s.dds";
						t.texuseid = loadinternaltextureex(t.texdirS_s.Get(),1,t.tfullorhalfdivide);
					}
				}
				else
				{
					if (  t.entityprofile[t.entid].specular == 1  )  t.texdirS_s = "effectbank\\reloaded\\media\\blank_none_S.dds";
					if (  t.entityprofile[t.entid].specular == 2  )  t.texdirS_s = "effectbank\\reloaded\\media\\blank_low_S.dds";
					if (  t.entityprofile[t.entid].specular == 3  )  t.texdirS_s = "effectbank\\reloaded\\media\\blank_medium_S.dds";
					if (  t.entityprofile[t.entid].specular == 4  )  t.texdirS_s = "effectbank\\reloaded\\media\\blank_high_S.dds";
					t.texuseid = loadinternaltextureex(t.texdirS_s.Get(),1,t.tfullorhalfdivide);
				}
				t.entityprofile[t.entid].texsid = t.texuseid;

				// Assign ILLUMINATION or CUBE (or real-time 'ENVCUBE for PBR' later)
				if ( iEffectProfile == 0 )
				{
					// non-PBR legacy behaviour
					t.texdirI_s = t.texdirnoext_s+"_i.dds";
					t.texuseid = loadinternaltextureex(t.texdirI_s.Get(),1,t.tfullorhalfdivide);
					if ( t.texuseid == 0 )
					{
						// if no _I file, try to find and load _CUBE file (load mode 2 = cube)
						t.texdirI_s = t.texdirnoext_s+"_cube.dds";
						t.texuseid = loadinternaltexturemode(t.texdirI_s.Get(),2);
						if ( t.texuseid == 0 )
						{
							// if no local CUBE, see if the level has generated one (matches sky and terrain)
							t.texuseid = t.terrain.imagestartindex+31;
						}
					}
					t.entityprofiletexiid = t.texuseid;
				}
				else
				{
					// PBR behaviour only allow _CUBE to override PBR reflection
					t.texdirI_s = t.texdirnoext_s+"_cube.dds";
					t.texuseid = loadinternaltexturemode(t.texdirI_s.Get(),2);
					if ( t.texuseid == 0 )
					{
						// if no local CUBE, see if the level has generated one (matches sky and terrain)
						t.texuseid = t.terrain.imagestartindex+31;
					}
					t.entityprofiletexiid = t.texuseid;
				}
				t.entityprofile[t.entid].texiid = t.entityprofiletexiid;

				// Assign AMBIENT OCCLUSION MAP
				t.texdirO_s = t.texdirnoext_s+"_o.dds";
				t.texuseid = loadinternaltextureex(t.texdirO_s.Get(),1,t.tfullorhalfdivide);
				if ( t.texuseid == 0 ) 
				{
					t.texdirO_s = t.texdirnoext_s+"_ao.dds";
					t.texuseid = loadinternaltextureex(t.texdirO_s.Get(),1,t.tfullorhalfdivide);
					if ( t.texuseid == 0 ) 
					{
						t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_O.dds",1,t.tfullorhalfdivide);
					}
					else
					{
						// disable override to AO that exists can be used
						if ( strlen ( t.entityprofile[t.entid].texd_s.Get() ) > 0 )
						{
							// but only if texture was specified in FPE, not if we assume model based textures
							//bGotAO = false; // see replacement solution below
						}
					}
				}
				t.entityprofiletexoid = t.texuseid;

				//PE: IBR old t7 is now t8 , detail/illum t8 is now t7. Done so we can skip t8, we still need the correct order of textures.
				//PE: IBR was not large but generate tons of stage changes.

				// Assign textures for PBR
				if ( iEffectProfile == 1 )
				{
					// gloss texture
					cstr pGlosstex_s = t.texdirnoext_s+"_gloss.dds";
					t.texuseid = loadinternaltextureex(pGlosstex_s.Get(),1,t.tfullorhalfdivide);
					if ( t.texuseid == 0 ) 
					{
						// 261117 - search material for stock metalness
						char pFullFilename[1024];
						int iMaterialIndex = t.entityprofile[t.entid].materialindex;
						sprintf ( pFullFilename, "effectbank\\reloaded\\media\\materials\\%d_Gloss.dds", iMaterialIndex );
						pGlosstex_s = pFullFilename;
						t.texuseid = loadinternaltextureex(pGlosstex_s.Get(),1,t.tfullorhalfdivide);
						if ( t.texuseid == 0 ) 
						{
							t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\white_D.dds",1,t.tfullorhalfdivide);
						}
					}
					t.entityprofile[t.entid].texgid = t.texuseid;

					// height texture
					cstr pHeighttex_s = t.texdirnoext_s+"_height.dds";
					t.texuseid = loadinternaltextureex(pHeighttex_s.Get(),1,t.tfullorhalfdivide);
					if ( t.texuseid == 0 ) 
					{
						t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_black.dds",1,t.tfullorhalfdivide);
					}
					t.entityprofile[t.entid].texhid = t.texuseid;

					// IBR texture
					if (g.memskipibr == 0) 
					{
						t.entityprofiletexibrid = t.terrain.imagestartindex + 32;
					}

					//PE: Use illumination instead of detail if found.
					//PE: Illumination overwrite detail.
					use_illumination = true;
					cstr pDetailtex_s = t.texdirnoext_s + "_illumination.dds";
					t.entityprofile[t.entid].texlid = loadinternaltextureex(pDetailtex_s.Get(), 1, t.tfullorhalfdivide);
					if (t.entityprofile[t.entid].texlid == 0)
					{
						cstr pDetailtex_s = t.texdirnoext_s + "_emissive.dds"; // _emissive
						t.entityprofile[t.entid].texlid = loadinternaltextureex(pDetailtex_s.Get(), 1, t.tfullorhalfdivide);
						if (t.entityprofile[t.entid].texlid == 0)
						{
							if (g.gpbroverride == 1) 
							{
								// PE: Also support _i when using gpbroverride == 1.
								t.texdirI_s = t.texdirnoext_s + "_i.dds";
								t.entityprofile[t.entid].texlid = loadinternaltextureex(t.texdirI_s.Get(), 1, t.tfullorhalfdivide);
							}
							if (t.entityprofile[t.entid].texlid == 0) 
							{
								// Detail texture
								cstr pDetailtex_s = t.texdirnoext_s + "_detail.dds";
								t.entityprofile[t.entid].texlid = loadinternaltextureex(pDetailtex_s.Get(), 1, t.tfullorhalfdivide);
								if (t.entityprofile[t.entid].texlid == 0)
								{
									t.entityprofile[t.entid].texlid = loadinternaltextureex("effectbank\\reloaded\\media\\detail_default.dds", 1, t.tfullorhalfdivide);
								}
								use_illumination = false;
							}
						}
					}
				}

				// Apply all textures to NON-MULTIMATERIAL entity parent object (D O N S)
				if ( t.entityprofile[t.entid].texdid > 0 ) 
				{
					// but only if diffuse specified, else use texture already loaded for model
					TextureObject ( t.entobj, 0, t.entityprofile[t.entid].texdid );
				}
				TextureObject ( t.entobj, 2, t.entityprofile[t.entid].texnid );
				TextureObject ( t.entobj, 3, t.entityprofile[t.entid].texsid );

				// Additional texture assignments required for PBR mode
				if ( iEffectProfile == 1 )
				{
					if (g.memskipibr == 0) TextureObject ( t.entobj, 8, t.entityprofiletexibrid );
					TextureObject ( t.entobj, 7, t.entityprofile[t.entid].texlid );
					TextureObject ( t.entobj, 4, t.entityprofile[t.entid].texgid );
					TextureObject ( t.entobj, 5, t.entityprofile[t.entid].texhid );
				}
			}
			else
			{
				// object entity uses multi-material (Fuse character)
				// and already loaded D N and S into 0, 2 and 3
				t.entityprofile[t.entid].texiid = 0;

				// PE 240118: not always if missing texture= in fpe , normal/spec is not always in the objects texture lists. ( we must have normal maps on all objects ).
				// Fix: https://github.com/LeeBamberTGC/GameGuruRepo/issues/10
				// lee - 300518 - added extra code in LoadObject to detect DNS and PBR texture file sets and set the mesh, so 
				// skip the override code below if the object has a good texture in place

				if (t.entityprofile[t.entid].texnid == 0 && bGotNormal == false )
				{
					t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_N.dds", 1, t.tfullorhalfdivide);
					t.entityprofile[t.entid].texnid = t.texuseid;
					TextureObject(t.entobj, 2, t.entityprofile[t.entid].texnid);
				}
				if (t.entityprofile[t.entid].texsid == 0 && bGotMetalness == false )
				{
					t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_black.dds", 1, t.tfullorhalfdivide);
					t.entityprofile[t.entid].texsid = t.texuseid;
					TextureObject(t.entobj, 3, t.entityprofile[t.entid].texsid);
				}
				// PE: iEffectProfile != 1 for this type of objects at this point in code.
				if (t.entityprofile[t.entid].texiid == 0)
				{
					if (g.gpbroverride == 1) // iEffectProfile == 1
					{
						// if no local CUBE, see if the level has generated one (matches sky and terrain)
						t.entityprofile[t.entid].texiid = t.terrain.imagestartindex + 31;
					}
				}

				// Not set anywhere, so just use blank_o.
				t.texuseid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_O.dds", 1, t.tfullorhalfdivide);
				t.entityprofiletexoid = t.texuseid; // must always be set.

				// PE: PBR shader support for old media with no texture in fpe.
				if (g.gpbroverride == 1) 
				{
					// None of the below will be in old media , so setup using fallback textures.
					t.entityprofile[t.entid].texlid = loadinternaltextureex("effectbank\\reloaded\\media\\detail_default.dds", 1, t.tfullorhalfdivide);
					t.entityprofile[t.entid].texgid = loadinternaltextureex("effectbank\\reloaded\\media\\white_D.dds", 1, t.tfullorhalfdivide);
					t.entityprofile[t.entid].texhid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_black.dds", 1, t.tfullorhalfdivide);
					if (g.memskipibr == 0) TextureObject(t.entobj, 8, t.entityprofiletexibrid);
					TextureObject(t.entobj, 7, t.entityprofile[t.entid].texlid);
					if ( bGotGloss == false ) TextureObject(t.entobj, 4, t.entityprofile[t.entid].texgid);
					TextureObject(t.entobj, 5, t.entityprofile[t.entid].texhid);
				}
			}

			// 230618 - apply AO texture ONLY when missing
			// if ( bGotAO == false ) TextureObject ( t.entobj, 1, t.entityprofiletexoid );
			sObject* pObject = GetObjectData ( t.entobj );
			if ( pObject )
			{
				for ( int iFrameIndex = 0; iFrameIndex < pObject->iFrameCount; iFrameIndex++ )
				{
					sFrame* pFrame = pObject->ppFrameList[iFrameIndex];
					if ( pFrame )
					{
						sMesh* pMesh = pFrame->pMesh;
						if ( pMesh )
						{
							for ( int iTextureIndex = 1; iTextureIndex < pMesh->dwTextureCount; iTextureIndex++ )
							{
								if ( pMesh->pTextures[iTextureIndex].iImageID == 0 )
								{
									if ( iTextureIndex == 1 ) TextureLimbStage ( t.entobj, iFrameIndex, iTextureIndex, t.entityprofiletexoid );
								}
							}
						}
					}
				}
			}

			// Apply all textures to REMAINING entity parent object (V C I)
			TextureObject ( t.entobj, 6, t.entityprofile[t.entid].texiid );

			// PBR or non-PBR modes
			LPSTR pEntityBasic = "effectbank\\reloaded\\entity_basic.fx";
			LPSTR pEntityAnim = "effectbank\\reloaded\\entity_anim.fx";
			LPSTR pCharacterBasic = "effectbank\\reloaded\\character_basic.fx";
			LPSTR pEntityBasicIllum = "effectbank\\reloaded\\apbr_illum.fx";
			LPSTR pCharacterBasicIllum = "effectbank\\reloaded\\apbr_illum_anim.fx";

			if ( g.gpbroverride == 1 )
			{
				pEntityBasic = "effectbank\\reloaded\\apbr_basic.fx";
				pEntityAnim = "effectbank\\reloaded\\apbr_anim.fx";
				pCharacterBasic = "effectbank\\reloaded\\apbr_anim.fx";
			}
			
			// 100718 - fix issue where old effect (non-illum) is retained for non-bone shader
			if ( stricmp ( EffectFile_s.Get(), pEntityBasic)==NULL ) 
			{
				if ( g.gpbroverride == 1 && use_illumination )
				{
					t.entityprofile[t.entid].usingeffect = loadinternaleffect(pEntityBasicIllum);
				}
			}

			// Special case for character_basic shader, when has meshes with no bones, use entity_basic instead
			t.teffectid2=0;
			if ( stricmp(EffectFile_s.Get(), pCharacterBasic) == NULL )  
			{
				if ( g.gpbroverride == 1 && use_illumination)
					t.teffectid2 = loadinternaleffect(pEntityBasicIllum);
				else
					t.teffectid2 = loadinternaleffect(pEntityBasic);
			}

			// 010917 - or if using entity_basic shader, and HAS anim meshes with bones, use entity_anim instead
			if ( stricmp ( EffectFile_s.Get(), pEntityBasic)==NULL ) 
			{
				if ( t.entityprofile[t.entid].animmax > 0 )
				{
					t.teffectid2 = t.entityprofile[t.entid].usingeffect;
					if (g.gpbroverride == 1 && use_illumination)
						t.entityprofile[t.entid].usingeffect = loadinternaleffect(pCharacterBasicIllum);
					else
						t.entityprofile[t.entid].usingeffect = loadinternaleffect(pEntityAnim);
				}
				else 
				{
					// PE: Change basic effect to use illumination
					if ( g.gpbroverride == 1 && use_illumination )
						t.entityprofile[t.entid].usingeffect = loadinternaleffect(pEntityBasicIllum);
				}
			}

			if (stricmp(EffectFile_s.Get(), pCharacterBasic) == NULL) 
			{
				// PE: Change character effect to use illumination
				if (g.gpbroverride == 1 && use_illumination) 
				{
					t.entityprofile[t.entid].usingeffect = loadinternaleffect(pCharacterBasicIllum);
				}
			}

			// Apply effect and textures
			if ( t.lightmapper.onlyloadstaticentitiesduringlightmapper == 0 )
			{
				// don't use shader effects when lightmapping
				SetObjectEffectCore ( t.entobj, t.entityprofile[t.entid].usingeffect, t.teffectid2, t.entityprofile[t.entid].cpuanims );
			}
		}
	}

	// Set any entity transparenct
	if ( t.entobj>0 ) 
	{
		SetObjectTransparency ( t.entobj, t.entityprofile[t.entid].transparency );
	}

	// Set entity culling (added COLLMODE 300114)
	if (  t.entityprofile[t.entid].cullmode != 0 ) 
	{
		// cull mode OFF used for single sided polygon models
		SetObjectCull ( t.entobj,0 );
	}
	else
	{
		SetObjectCull ( t.entobj,1 );
	}

	// Set cull mode for limbs (if hair specified)
	if ( t.entityprofile[t.entid].hairframestart != -1 )
	{
		for ( int tlmb = t.entityprofile[t.entid].hairframestart; tlmb <= t.entityprofile[t.entid].hairframefinish; tlmb++ )
		{
			if ( LimbExist ( t.entobj, tlmb ) == 1 )
			{
				SetLimbCull ( t.entobj, tlmb, 0 );
			}
		}
	}

	// hide specified limbs
	if ( t.entityprofile[t.entid].hideframestart != -1 )
	{
		for ( int tlmb = t.entityprofile[t.entid].hideframestart; tlmb <= t.entityprofile[t.entid].hideframefinish; tlmb++ )
		{
			if ( LimbExist ( t.entobj, tlmb ) == 1 )
			{
				ExcludeLimbOn ( t.entobj, tlmb );
			}
		}
	}

	if ( t.entobj > 0 )
	{
		//  If transparent, no need to Z write
		if (  t.entityprofile[t.entid].transparency>0 ) 
		{
			if ( t.entityprofile[t.entid].transparency >= 2 ) 
			{
				DisableObjectZWrite ( t.entobj );
			}
		}
	}
}

void entity_loadelementsdata ( void )
{
	//  Free any old elements
	entity_deleteelementsdata ( );

	//  Uses elementfilename$ (element data from test game quick build - not from universe.ele created during full build)
	if (  t.elementsfilename_s == ""  )  t.elementsfilename_s = g.mysystem.levelBankTestMap_s+"map.ele"; //"levelbank\\testmap\\map.ele";

	// load entity element list
	t.failedtoload=0;
	t.versionnumbersupported = 312;
	if ( FileExist(t.elementsfilename_s.Get()) == 1 ) 
	{
		OpenToRead (  1,t.elementsfilename_s.Get() );
		t.versionnumberload = ReadLong ( 1 );
		if (  t.versionnumberload<100 ) 
		{
			//  Pre-version data - development only
			g.entityelementlist=t.versionnumberload;
			t.versionnumberload=100;
		}
		else
		{
			g.entityelementlist = ReadLong ( 1 );
		}
		if (  t.versionnumberload <= t.versionnumbersupported ) 
		{
			if (  g.entityelementlist>0 ) 
			{
				UnDim (  t.entityelement );
				UnDim2 (  t.entityshadervar );
				UnDim (  t.entitydebug_s );
				g.entityelementmax=g.entityelementlist;
				Dim (  t.entityelement,g.entityelementmax );
				Dim2(  t.entityshadervar,g.entityelementmax, g.globalselectedshadermax  );
				Dim (  t.entitydebug_s,g.entityelementmax  );
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
					//  actual file data
					if (  t.versionnumberload >= 101 ) 
					{
						//  Version 1.01
						t.a = ReadLong ( 1 ); t.entityelement[t.e].maintype=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].bankindex=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].staticflag=t.a;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].x=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].y=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].z=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].rx=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].ry=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].rz=t.a_f;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.name_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aiinit_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aimain_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aidestroy_s=t.a_s;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.isobjective=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.usekey_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.ifused_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.ifusednear_s=t.a_s;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.uniqueelement=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.texd_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.texaltd_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.effect_s=t.a_s;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.transparency=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].editorfixed=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.soundset_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.soundset1_s=t.a_s;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnmax=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawndelay=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnqty=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.hurtfall=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.castshadow=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.reducetexture=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.speed=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aishoot_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.hasweapon_s=t.a_s;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.lives=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].spawn.max=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].spawn.delay=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].spawn.qty=t.a;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.scale=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.coneheight=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.coneangle=t.a_f;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.strength=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.isimmobile=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.cantakeweapon=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.quantity=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.markerindex=t.a;
						t.a = ReadLong ( 1 ); t.dw=t.a ; t.dw=t.dw+0xFF000000 ; t.entityelement[t.e].eleprof.light.color=t.dw;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.light.range=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.trigger.stylecolor=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.trigger.waypointzoneindex=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.basedecal_s=t.a_s;
					}
					if (  t.versionnumberload >= 102 ) 
					{
						//  Version 1.02
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.rateoffire=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.damage=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.accuracy=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.reloadqty=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.fireiterations=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.lifespan=t.a;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.throwspeed=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.throwangle=t.a_f;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.bounceqty=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.explodeonhit=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.weaponisammo=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnupto=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnafterdelay=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnwhendead=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spare1=t.a;
					}
					if (  t.versionnumberload >= 103 ) 
					{
						//  Version 1.03 - V1 draft physics
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.physics=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phyweight=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phyfriction=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phyforcedamage=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.rotatethrow=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.explodable=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.explodedamage=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phydw4=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phydw5=t.a;
					}
					if (  t.versionnumberload >= 104 ) 
					{
						//  Version 1.04 - BETA4 extra field
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.phyalways=t.a;
					}
					if (  t.versionnumberload >= 105 ) 
					{
						//  Version 1.05 - BETA8 extra fields
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawndelayrandom=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnqtyrandom=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnvel=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnvelrandom=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnangle=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnanglerandom=t.a;
					}
					if (  t.versionnumberload >= 106 ) 
					{
						//  Version 1.06 - BETA10 extra fields
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnatstart=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.spawnlife=t.a;
					}
					if (  t.versionnumberload >= 107 ) 
					{
						//  FPSCV104RC8 - forgot to save infinilight index (dynamic lights in final build never worked)
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.light.index=t.a;
					}
					if (  t.versionnumberload >= 199 ) 
					{
						//  X10 EXTRAS - Ignored in X9
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
					}
					if (  t.versionnumberload >= 200 ) 
					{
						//  X10 EXTRAS 190707 - Ignored in X9
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
						t.a = ReadLong ( 1 );
					}
					if (  t.versionnumberload >= 217 ) 
					{
						//  FPGC - 300710 - save new entity element data
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particleoverride=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.offsety=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.scale=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randomstartx=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randomstarty=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randomstartz=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.linearmotionx=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.linearmotiony=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.linearmotionz=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randommotionx=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randommotiony=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.randommotionz=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.mirrormode=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.camerazshift=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.scaleonlyx=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.lifeincrement=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.alphaintensity=t.a;
					}
					if (  t.versionnumberload >= 218 ) 
					{
						//  V118 - 060810 - knxrb - Decal animation setting (Added animation choice setting).
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.particle.animated=t.a;
					}
					if (  t.versionnumberload >= 301 ) 
					{
						//  Reloaded ALPHA 1.0045
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aiinitname_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aimainname_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aidestroyname_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.aishootname_s=t.a_s;
					}
					if (  t.versionnumberload >= 302 ) 
					{
						//  Reloaded BETA 1.005
					}
					if (  t.versionnumberload >= 303 ) 
					{
						//  Reloaded BETA 1.007
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.animspeed=t.a;
					}
					if (  t.versionnumberload >= 304 ) 
					{
						//  Reloaded BETA 1.007-200514
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.conerange=t.a_f;
					}
					if (  t.versionnumberload >= 305 ) 
					{
						//  Reloaded BETA 1.0085
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].scalex=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].scaley=t.a_f;
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].scalez=t.a_f;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.range=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.dropoff=t.a;
					}
					if (  t.versionnumberload >= 306 ) 
					{
						//  GameGuru 1.00.010
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.isviolent=t.a;
					}
					if (  t.versionnumberload >= 307 ) 
					{
						//  GameGuru 1.00.020
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.teamfield=t.a;
					}
					if (  t.versionnumberload >= 308 ) 
					{
						//  GameGuru 1.01.001
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.usespotlighting=t.a;
					}
					if (  t.versionnumberload >= 309 ) 
					{
						//  GameGuru 1.01.002
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.lodmodifier=t.a;
					}
					if (  t.versionnumberload >= 310 ) 
					{
						//  GameGuru 1.133
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.isocluder=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.isocludee=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.colondeath=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.parententityindex=t.a;
						t.a = ReadLong ( 1 ); t.entityelement[t.e].eleprof.parentlimbindex=t.a;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.soundset2_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.soundset3_s=t.a_s;
						t.a_s = ReadString ( 1 ); t.entityelement[t.e].eleprof.soundset4_s=t.a_s;
					}
					if (  t.versionnumberload >= 311 ) 
					{
						//  GameGuru 1.133B
						t.a_f = ReadFloat ( 1 ); t.entityelement[t.e].eleprof.specularperc=t.a_f;
					}
					if (  t.versionnumberload >= 312 ) 
					{
						//  GameGuru 1.14 EBE
						t.a = ReadLong ( 1 ); t.entityelement[t.e].iHasParentIndex = t.a;
					}

					//  fill in the blanks if load older version
					t.ttentid=t.entityelement[t.e].bankindex;
					if (  t.versionnumberload<103 ) 
					{
						//  Version 1.03 - V1 draft physics (-1 means calculate at entobj-loadtime)
						t.entityelement[t.e].eleprof.physics=t.entityprofile[t.ttentid].physics;
						t.entityelement[t.e].eleprof.phyweight=t.entityprofile[t.ttentid].phyweight;
						t.entityelement[t.e].eleprof.phyfriction=t.entityprofile[t.ttentid].phyfriction;
						t.entityelement[t.e].eleprof.phyforcedamage=t.entityprofile[t.ttentid].phyforcedamage;
						t.entityelement[t.e].eleprof.rotatethrow=t.entityprofile[t.ttentid].rotatethrow;
						t.entityelement[t.e].eleprof.explodable=t.entityprofile[t.ttentid].explodable;
						t.entityelement[t.e].eleprof.phydw3=0;
						t.entityelement[t.e].eleprof.phydw4=0;
						t.entityelement[t.e].eleprof.phydw5=0;
					}
					if (  t.versionnumberload<104 ) 
					{
						//  Version 1.04 - BETA4 extra field
						t.entityelement[t.e].eleprof.phyalways=t.entityprofile[t.ttentid].phyalways;
					}
					if (  t.versionnumberload<105 ) 
					{
						//  Version 1.05 - BETA8
						t.entityelement[t.e].eleprof.spawndelayrandom=t.entityprofile[t.ttentid].spawndelayrandom;
						t.entityelement[t.e].eleprof.spawnqtyrandom=t.entityprofile[t.ttentid].spawnqtyrandom;
						t.entityelement[t.e].eleprof.spawnvel=t.entityprofile[t.ttentid].spawnvel;
						t.entityelement[t.e].eleprof.spawnvelrandom=t.entityprofile[t.ttentid].spawnvelrandom;
						t.entityelement[t.e].eleprof.spawnangle=t.entityprofile[t.ttentid].spawnangle;
						t.entityelement[t.e].eleprof.spawnanglerandom=t.entityprofile[t.ttentid].spawnanglerandom;
					}
					if (  t.versionnumberload<106 ) 
					{
						//  Version 1.06 - BETA10
						t.entityelement[t.e].eleprof.spawnatstart=t.entityprofile[t.ttentid].spawnatstart;
						t.entityelement[t.e].eleprof.spawnlife=t.entityprofile[t.ttentid].spawnlife;
					}
					if (  t.versionnumberload<217 ) 
					{
						//  FPGC - 300710 - older levels dont use particle override
						t.entityelement[t.e].eleprof.particleoverride=0;
					}
					if (  t.versionnumberload<303 ) 
					{
						//  Reloaded BETA 1.007
						t.entityelement[t.e].eleprof.animspeed=t.entityprofile[t.ttentid].animspeed;
					}
					if (  t.versionnumberload<304 ) 
					{
						//  Reloaded BETA 1.007-200514
						t.entityelement[t.e].eleprof.conerange=t.entityprofile[t.ttentid].conerange;
					}
					if (  t.versionnumberload<306 ) 
					{
						//  GameGuru 1.00.010
						t.entityelement[t.e].eleprof.isviolent=t.entityprofile[t.ttentid].isviolent;
					}
					if (  t.versionnumberload<307 ) 
					{
						//  GameGuru 1.00.020
						t.entityelement[t.e].eleprof.teamfield=t.entityprofile[t.ttentid].teamfield;
					}
					if (  t.versionnumberload<310 ) 
					{
						//  GameGuru 1.133
						t.entityelement[t.e].eleprof.isocluder=t.entityprofile[t.ttentid].isocluder;
						t.entityelement[t.e].eleprof.isocludee=t.entityprofile[t.ttentid].isocludee;
						t.entityelement[t.e].eleprof.colondeath=t.entityprofile[t.ttentid].colondeath;
						t.entityelement[t.e].eleprof.parententityindex=t.entityprofile[t.ttentid].parententityindex;
						t.entityelement[t.e].eleprof.parentlimbindex=t.entityprofile[t.ttentid].parentlimbindex;
						t.entityelement[t.e].eleprof.soundset2_s=t.entityprofile[t.ttentid].soundset2_s;
						t.entityelement[t.e].eleprof.soundset3_s=t.entityprofile[t.ttentid].soundset3_s;
						t.entityelement[t.e].eleprof.soundset4_s=t.entityprofile[t.ttentid].soundset4_s;
					}
					if (  t.versionnumberload<311 ) 
					{
						//  GameGuru 1.133B
						t.entityelement[t.e].eleprof.specularperc=t.entityprofile[t.ttentid].specularperc;
					}
					if (  t.versionnumberload<312 ) 
					{
						//  GameGuru 1.14 EBE
						t.entityelement[t.e].iHasParentIndex = 0;
					}
					t.entityelement[t.e].entitydammult_f=1.0;
					t.entityelement[t.e].entityacc=1.0;

					// 131115 - transparency control was removed from GG properties IDE, so ensure
					// it reflects the latest entity profile information (until we allow this value back in)
					t.entityelement[t.e].eleprof.transparency = t.entityprofile[t.ttentid].transparency;
				}
			}
		}
		else
		{
			t.failedtoload=1;
		}
		CloseFile (  1 );

		// 050416 - remove any weapons from start marker if parental control, no weapon, no violence
		if ( g.quickparentalcontrolmode == 2 )
		{
			for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
			{
				t.entid = t.entityelement[t.e].bankindex;
				if (  t.entityprofile[t.entid].ismarker == 1 ) 
				{
					//  Player Start Marker Settings
					t.entityelement[t.e].eleprof.hasweapon_s = "";
					t.entityelement[t.e].eleprof.hasweapon = 0;
					t.entityelement[t.e].eleprof.quantity = 0;
					t.entityelement[t.e].eleprof.isviolent = 0;
				}
			}
		}

		// entity AI replace (getting a level to work then disable the code until next time)
		if ( 0 )
		{
			bool bMissingScriptFoundAndReplaced = false;
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				for ( t.tcheck = 1 ; t.tcheck<=  3; t.tcheck++ )
				{
					if (  t.tcheck == 1  )  t.tcheck_s = t.entityelement[t.e].eleprof.aimain_s;
					if (  t.tcheck == 2  )  t.tcheck_s = t.entityelement[t.e].eleprof.aiinit_s;
					if (  t.tcheck == 3  )  t.tcheck_s = t.entityelement[t.e].eleprof.aidestroy_s;
					t.ttry_s="";
					for ( t.nn = 1 ; t.nn<=  Len(t.tcheck_s.Get()); t.nn++ )
					{
						t.ttry_s=t.ttry_s+Mid(t.tcheck_s.Get(),t.nn);
						if (  (cstr(Mid(t.tcheck_s.Get(),t.nn)) == "\\" && cstr(Mid(t.tcheck_s.Get(),t.nn+1)) == "\\") || (cstr(Mid(t.tcheck_s.Get(),t.nn)) == "/" && cstr(Mid(t.tcheck_s.Get(),t.nn+1)) == "/") ) 
						{
							++t.nn;
						}
					}
					t.ttry_s=Lower(t.ttry_s.Get());
					if ( strlen(t.ttry_s.Get()) > 4 && FileExist(cstr(cstr("scriptbank\\")+t.ttry_s).Get()) == 0 ) 
					{
						if (  t.tcheck == 1 ) t.entityelement[t.e].eleprof.aimain_s = cstr(cstr("stories\\Cell Shaded\\")+cstr(t.ttry_s)).Get();
						if (  t.tcheck == 2 ) t.entityelement[t.e].eleprof.aiinit_s = cstr(cstr("stories\\Cell Shaded\\")+cstr(t.ttry_s)).Get();
						if (  t.tcheck == 3 ) t.entityelement[t.e].eleprof.aidestroy_s = cstr(cstr("stories\\Cell Shaded\\")+cstr(t.ttry_s)).Get();
						bMissingScriptFoundAndReplaced = true;
					}
				}
			}
		}

		//  If replacement file active, can swap in new SCRIPT and SOUND references
		if (  Len(t.editor.replacefilepresent_s.Get())>1 ) 
		{
			//  now go through ELEPROF enrties to update any SCRIPTBANK references and SOUNDSET references
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				for ( t.tcheck = 1 ; t.tcheck <= 6; t.tcheck++ )
				{
					if (  t.tcheck == 1  )  t.tcheck_s = t.entityelement[t.e].eleprof.aimain_s;
					if (  t.tcheck == 2  )  t.tcheck_s = t.entityelement[t.e].eleprof.soundset_s;
					if (  t.tcheck == 3  )  t.tcheck_s = t.entityelement[t.e].eleprof.soundset1_s;
					if (  t.tcheck == 4  )  t.tcheck_s = t.entityelement[t.e].eleprof.soundset2_s;
					if (  t.tcheck == 5  )  t.tcheck_s = t.entityelement[t.e].eleprof.soundset3_s;
					if (  t.tcheck == 6  )  t.tcheck_s = t.entityelement[t.e].eleprof.soundset4_s;
					t.ttry_s="";
					for ( t.nn = 1 ; t.nn<=  Len(t.tcheck_s.Get()); t.nn++ )
					{
						t.ttry_s=t.ttry_s+Mid(t.tcheck_s.Get(),t.nn);
						if (  (cstr(Mid(t.tcheck_s.Get(),t.nn)) == "\\" && cstr(Mid(t.tcheck_s.Get(),t.nn+1)) == "\\") || (cstr(Mid(t.tcheck_s.Get(),t.nn)) == "/" && cstr(Mid(t.tcheck_s.Get(),t.nn+1)) == "/") ) 
						{
							++t.nn;
						}
					}
					t.ttry_s=Lower(t.ttry_s.Get());
					for ( t.tt = 1 ; t.tt<=  t.treplacementmax; t.tt++ )
					{
						if (  t.replacements_s[t.tt][0] == t.ttry_s ) 
						{
							//  found entry we can replace
							if (  t.tcheck == 1 ) { t.entityelement[t.e].eleprof.aimain_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
							if (  t.tcheck == 2 ) { t.entityelement[t.e].eleprof.soundset_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
							if (  t.tcheck == 3 ) { t.entityelement[t.e].eleprof.soundset1_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
							if (  t.tcheck == 4 ) { t.entityelement[t.e].eleprof.soundset2_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
							if (  t.tcheck == 5 ) { t.entityelement[t.e].eleprof.soundset3_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
							if (  t.tcheck == 6 ) { t.entityelement[t.e].eleprof.soundset4_s = t.replacements_s[t.tt][1]  ; t.tt = t.treplacementmax+1; }
						}
					}
				}
			}
			//  free usages
			UnDim (  t.replacements_s );
		}
	}

	//  and erase any elements that DO NOT have a valid profile (file moved/deleted)
	if (  t.failedtoload == 1 ) 
	{
		//  FPGC - 270410 - if entity binary from X10 (or just not supported), ensure NO entities!
		g.entityelementlist=0;
		g.entityelementmax=0;
	}
	else
	{
		for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
		{
			t.entid=t.entityelement[t.e].bankindex;
			if (  t.entid>0 ) 
			{
				if (  t.entid>ArrayCount(t.entitybank_s) ) 
				{
					t.entityelement[t.e].bankindex=0;
				}
				else
				{
					if (  Len(t.entitybank_s[t.entid].Get()) == 0 ) 
					{
						//  030715 - but only erase if entity not a marker
						if (  t.entityprofile[t.entid].ismarker == 0 ) 
						{
							t.entityelement[t.e].bankindex=0;
						}
					}
				}
			}
		}
	}
}

void entity_saveelementsdata ( void )
{
	//  Uses elementfilename$
	if (  t.elementsfilename_s == ""  )  t.elementsfilename_s = g.mysystem.levelBankTestMap_s+"map.ele"; //"levelbank\\testmap\\map.ele";

	//  Reduce list size if later elements blank
	t.e=g.entityelementlist;
	while (  t.e>0 ) 
	{
		if (  t.entityelement[t.e].maintype == 0  )  --t.e; else break;
	}
	g.entityelementlist=t.e;

	//  Save entity element list
	t.versionnumbersave = 312;
	if ( FileExist(t.elementsfilename_s.Get()) == 1  )  DeleteAFile ( t.elementsfilename_s.Get() );
	OpenToWrite (  1,t.elementsfilename_s.Get() );
	WriteLong (  1,t.versionnumbersave );
	WriteLong (  1,g.entityelementlist );
	if ( g.entityelementlist>0 ) 
	{
		for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
		{
			if ( t.versionnumbersave >= 101 ) 
			{
				//  Version 1.01 - EA
				t.a=t.entityelement[t.e].maintype ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].bankindex ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].staticflag ; WriteLong (  1,t.a );
				t.a_f=t.entityelement[t.e].x ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].y ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].z ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].rx ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].ry ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].rz ; WriteFloat (  1,t.a_f );
				t.a_s=t.entityelement[t.e].eleprof.name_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aiinit_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aimain_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aidestroy_s ; WriteString (  1,t.a_s.Get() );
				t.a=t.entityelement[t.e].eleprof.isobjective ; WriteLong (  1,t.a );
				t.a_s=t.entityelement[t.e].eleprof.usekey_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.ifused_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.ifusednear_s ; WriteString (  1,t.a_s.Get() );
				t.a=t.entityelement[t.e].eleprof.uniqueelement ; WriteLong (  1,t.a );
				t.a_s=t.entityelement[t.e].eleprof.texd_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.texaltd_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.effect_s ; WriteString (  1,t.a_s.Get() );
				t.a=t.entityelement[t.e].eleprof.transparency ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].editorfixed ; WriteLong (  1,t.a );
				t.a_s=t.entityelement[t.e].eleprof.soundset_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.soundset1_s ; WriteString (  1,t.a_s.Get() );
				t.a=t.entityelement[t.e].eleprof.spawnmax ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawndelay ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnqty ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.hurtfall ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.castshadow ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.reducetexture ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.speed ; WriteLong (  1,t.a );
				t.a_s=t.entityelement[t.e].eleprof.aishoot_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.hasweapon_s ; WriteString (  1,t.a_s.Get() );
				t.a=t.entityelement[t.e].eleprof.lives ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].spawn.max ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].spawn.delay ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].spawn.qty ; WriteLong (  1,t.a );
				t.a_f=t.entityelement[t.e].eleprof.scale ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].eleprof.coneheight ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].eleprof.coneangle ; WriteFloat (  1,t.a_f );
				t.a=t.entityelement[t.e].eleprof.strength ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.isimmobile ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.cantakeweapon ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.quantity ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.markerindex ; WriteLong (  1,t.a );
				t.dw=t.entityelement[t.e].eleprof.light.color ; t.dw=(t.dw<<8)>>8 ; t.a=t.dw ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.light.range ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.trigger.stylecolor ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.trigger.waypointzoneindex ; WriteLong (  1,t.a );
				t.a_s=t.entityelement[t.e].eleprof.basedecal_s ; WriteString (  1,t.a_s.Get() );
			}
			if (  t.versionnumbersave >= 102 ) 
			{
				t.a=t.entityelement[t.e].eleprof.rateoffire ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.damage ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.accuracy ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.reloadqty ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.fireiterations ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.lifespan ; WriteLong (  1,t.a );
				t.a_f=t.entityelement[t.e].eleprof.throwspeed ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].eleprof.throwangle ; WriteFloat (  1,t.a_f );
				t.a=t.entityelement[t.e].eleprof.bounceqty ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.explodeonhit ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.weaponisammo ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnupto ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnafterdelay ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnwhendead ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spare1 ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 103 ) 
			{
				//  V1 first draft - physics
				t.a=t.entityelement[t.e].eleprof.physics ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.phyweight ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.phyfriction ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.phyforcedamage ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.rotatethrow ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.explodable ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.explodedamage ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.phydw4 ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.phydw5 ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 104 ) 
			{
				//  Addition of new physics field for BETA4
				t.a=t.entityelement[t.e].eleprof.phyalways ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 105 ) 
			{
				//  Addition of new spawn fields for BETA8
				t.a=t.entityelement[t.e].eleprof.spawndelayrandom ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnqtyrandom ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnvel ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnvelrandom ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnangle ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnanglerandom ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 106 ) 
			{
				//  Addition of new fields for BETA10
				t.a=t.entityelement[t.e].eleprof.spawnatstart ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.spawnlife ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 107 ) 
			{
				//  FPSCV104RC8 - forgot to save infinilight index (dynamic lights in final build never worked)
				t.a=t.entityelement[t.e].eleprof.light.index ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 199 ) 
			{
				//  X10 specific version - any new save data must be higher than 200
				t.a=0;
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 200 ) 
			{
				//  X10 specific version - any new save data must be higher than 200
				t.a=0;
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
				WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 217 ) 
			{
				//  FPGC - 300710 - save new entity element data
				t.a=t.entityelement[t.e].eleprof.particleoverride ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.offsety ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.scale ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randomstartx ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randomstarty ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randomstartz ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.linearmotionx ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.linearmotiony ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.linearmotionz ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randommotionx ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randommotiony ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.randommotionz ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.mirrormode ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.camerazshift ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.scaleonlyx ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.lifeincrement ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.particle.alphaintensity ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 218 ) 
			{
				//  V118 - 060810 - knxrb - Decal animation setting (Added animation choice setting).
				t.a=t.entityelement[t.e].eleprof.particle.animated ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 301 ) 
			{
				//  Reloaded ALPHA 1.0045
				t.a_s=t.entityelement[t.e].eleprof.aiinitname_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aimainname_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aidestroyname_s ; WriteString (  1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.aishootname_s ; WriteString (  1,t.a_s.Get() );
			}
			if (  t.versionnumbersave >= 302 ) 
			{
				//  Reloaded BETA 1.005
			}
			if (  t.versionnumbersave >= 303 ) 
			{
				//  Reloaded BETA 1.007
				t.a=t.entityelement[t.e].eleprof.animspeed ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 304 ) 
			{
				//  Reloaded BETA 1.007-200514
				t.a_f=t.entityelement[t.e].eleprof.conerange ; WriteFloat (  1,t.a_f );
			}
			if (  t.versionnumbersave >= 305 ) 
			{
				//  Reloaded BETA 1.009
				t.a_f=t.entityelement[t.e].scalex ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].scaley ; WriteFloat (  1,t.a_f );
				t.a_f=t.entityelement[t.e].scalez ; WriteFloat (  1,t.a_f );
				t.a=t.entityelement[t.e].eleprof.range ; WriteLong (  1,t.a );
				t.a=t.entityelement[t.e].eleprof.dropoff ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 306 ) 
			{
				//  Guru 1.00.010
				t.a=t.entityelement[t.e].eleprof.isviolent ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 307 ) 
			{
				//  Guru 1.00.020
				t.a=t.entityelement[t.e].eleprof.teamfield ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 308 ) 
			{
				//  Guru 1.01.001
				t.a=t.entityelement[t.e].eleprof.usespotlighting ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 309 )
			{
				//  Guru 1.01.002
				t.a=t.entityelement[t.e].eleprof.lodmodifier ; WriteLong (  1,t.a );
			}
			if (  t.versionnumbersave >= 310 )
			{
				//  Guru 1.133
				t.a=t.entityelement[t.e].eleprof.isocluder; WriteLong ( 1, t.a );
				t.a=t.entityelement[t.e].eleprof.isocludee; WriteLong ( 1, t.a );
				t.a=t.entityelement[t.e].eleprof.colondeath; WriteLong ( 1, t.a );
				t.a=t.entityelement[t.e].eleprof.parententityindex; WriteLong ( 1, t.a );
				t.a=t.entityelement[t.e].eleprof.parentlimbindex; WriteLong ( 1, t.a );
				t.a_s=t.entityelement[t.e].eleprof.soundset2_s ; WriteString ( 1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.soundset3_s ; WriteString ( 1,t.a_s.Get() );
				t.a_s=t.entityelement[t.e].eleprof.soundset4_s ; WriteString ( 1,t.a_s.Get() );
			}
			if (  t.versionnumbersave >= 311 )
			{
				//  Guru 1.133B
				t.a_f=t.entityelement[t.e].eleprof.specularperc; WriteFloat ( 1, t.a_f );
			}
			if (  t.versionnumbersave >= 312 )
			{
				//  Guru 1.14 EBE
				t.a = t.entityelement[t.e].iHasParentIndex; WriteLong ( 1, t.a );
			}			
		}
	}
	CloseFile (  1 );
}

void entity_savebank ( void )
{
	//  Scan entire entityelement, delete all entitybank entries not used
	if (  g.gcompilestandaloneexe == 0 && g.gpretestsavemode == 0 ) 
	{
		if (  g.entidmaster>0 ) 
		{
			Dim ( t.entitybankused,g.entidmaster  );
			for ( t.tttentid = 1 ; t.tttentid<= g.entidmaster; t.tttentid++ )
			{
				t.entitybankused[t.tttentid]=0;
			}
			for ( t.ttte = 1 ; t.ttte <= g.entityelementlist; t.ttte++ )
			{
				t.tttentid = t.entityelement[t.ttte].bankindex;
				if (  t.tttentid > 0 && t.tttentid <= g.entidmaster ) 
				{
					t.entitybankused[t.tttentid]=1;
				}
			}
			for ( t.tttentid = 1 ; t.tttentid <= g.entidmaster; t.tttentid++ )
			{
				if (  t.entitybankused[t.tttentid] == 0 ) 
				{
					// free RLE data in profile
					ebe_freecubedata ( t.tttentid );
			
					//  remove entity entry if not used when save FPM
					t.entitybank_s[t.tttentid]="";
				}
			}
			//  shuffle to remove empty entries
			for ( t.tttentid = 1 ; t.tttentid <= g.entidmaster; t.tttentid++ )
			{
				//  not used to record where entities have been moved to
				t.entitybankused[t.tttentid]=0;
			}
			t.treadentid=1 ; t.tlargest=0;
			for ( t.tttentid = 1 ; t.tttentid<=  g.entidmaster; t.tttentid++ )
			{
				if (  t.treadentid <= g.entidmaster ) 
				{
					while (  t.entitybank_s[t.treadentid] == "" ) 
					{
						++t.treadentid ; if (  t.treadentid>g.entidmaster  )  break;
					}
					if (  t.treadentid <= g.entidmaster ) 
					{
						t.entitybank_s[t.tttentid] = t.entitybank_s[t.treadentid];
						t.entityprofileheader[t.tttentid]=t.entityprofileheader[t.treadentid];
						t.entityprofile[t.tttentid]=t.entityprofile[t.treadentid];
						if ( t.entityprofile[t.treadentid].ebe.pRLEData != NULL && t.tttentid < t.treadentid )
						{
							// if we are shifting an EBE entity into place
							// EBE entity parents can be saved shortly after, so ensure object is copied over
							t.sourceobj = g.entitybankoffset + t.tttentid;
							if ( ObjectExist ( t.sourceobj ) == 1 ) DeleteObject ( t.sourceobj );
							int iSourceObjBeingMoved = g.entitybankoffset + t.treadentid;
							if ( ObjectExist ( iSourceObjBeingMoved ) == 1 ) CloneObject ( t.sourceobj, iSourceObjBeingMoved );
							
							// this overrites regular saved EBE entities
							t.entitybank_s[t.tttentid] = cstr("EBE") + cstr(t.tttentid);
						}
						t.entityprofile[t.tttentid].ebe.dwRLESize=t.entityprofile[t.treadentid].ebe.dwRLESize;
						t.entityprofile[t.tttentid].ebe.pRLEData=t.entityprofile[t.treadentid].ebe.pRLEData;
						t.entityprofile[t.tttentid].ebe.dwMatRefCount=t.entityprofile[t.treadentid].ebe.dwMatRefCount;
						t.entityprofile[t.tttentid].ebe.iMatRef=t.entityprofile[t.treadentid].ebe.iMatRef;
						t.entityprofile[t.tttentid].ebe.dwTexRefCount=t.entityprofile[t.treadentid].ebe.dwTexRefCount;
						t.entityprofile[t.tttentid].ebe.pTexRef=t.entityprofile[t.treadentid].ebe.pTexRef;
						for ( t.tt = 0 ; t.tt <=  100 ; t.tt++ ) t.entitybodypart[t.tttentid][t.tt]=t.entitybodypart[t.treadentid][t.tt] ;
						for ( t.tt = 0 ; t.tt <=  g.animmax ; t.tt++ ) t.entityanim[t.tttentid][t.tt]=t.entityanim[t.treadentid][t.tt] ;
						for ( t.tt = 0 ; t.tt <=  g.footfallmax ; t.tt++ ) t.entityfootfall[t.tttentid][t.tt]=t.entityfootfall[t.treadentid][t.tt] ; 
						for ( t.tt = 0 ; t.tt <=  100 ; t.tt++ ) t.entitydecal_s[t.tttentid][t.tt]=t.entitydecal_s[t.treadentid][t.tt] ;
						for ( t.tt = 0 ; t.tt <=  100 ; t.tt++ ) t.entitydecal[t.tttentid][t.tt]=t.entitydecal[t.treadentid][t.tt] ;
						t.entitybankused[t.treadentid]=t.tttentid;
						t.tlargest=t.tttentid;
					}
					else
					{
						// wipe after end of shuffle
						t.entitybank_s[t.tttentid] = "";
						t.entityprofile[t.tttentid].ebe.dwRLESize = 0;
						t.entityprofile[t.tttentid].ebe.pRLEData = NULL;
						t.entityprofile[t.tttentid].ebe.dwMatRefCount = 0;
						t.entityprofile[t.tttentid].ebe.iMatRef = NULL;
						t.entityprofile[t.tttentid].ebe.dwTexRefCount = 0;
						t.entityprofile[t.tttentid].ebe.pTexRef = NULL;
					}
				}
				else
				{
					// wipe after end of shuffle
					t.entitybank_s[t.tttentid]="";
					t.entityprofile[t.tttentid].ebe.dwRLESize = 0;
					t.entityprofile[t.tttentid].ebe.pRLEData = NULL;
					t.entityprofile[t.tttentid].ebe.dwMatRefCount = 0;
					t.entityprofile[t.tttentid].ebe.iMatRef = NULL;
					t.entityprofile[t.tttentid].ebe.dwTexRefCount = 0;
					t.entityprofile[t.tttentid].ebe.pTexRef = NULL;
				}
				++t.treadentid;
			}
			//  update bank index numbers in entityelements
			for ( t.ttte = 1 ; t.ttte<=  g.entityelementlist; t.ttte++ )
			{
				t.tttentid=t.entityelement[t.ttte].bankindex;
				if (  t.tttentid>0 && t.tttentid <= g.entidmaster ) 
				{
					//  new entity entry place index
					t.entityelement[t.ttte].bankindex=t.entitybankused[t.tttentid];
				}
			}
			UnDim (  t.entitybankused );

			//  new list size
			if (  g.entidmaster != t.tlargest ) 
			{
				g.entidmaster=t.tlargest;
				t.entityorsegmententrieschanged=1;
			}
		}
	}

	//  Save segment bank
	cstr segmentbank_s = g.mysystem.levelBankTestMap_s+"map.ent";
	//if (  FileExist("levelbank\\testmap\\map.ent") == 1  )  DeleteAFile (  "levelbank\\testmap\\map.ent" );
	//OpenToWrite (  1,"levelbank\\testmap\\map.ent" );
	if (  FileExist(segmentbank_s.Get()) == 1 )  DeleteAFile ( segmentbank_s.Get() );
	OpenToWrite ( 1, segmentbank_s.Get() );
	WriteLong ( 1,g.entidmaster );
	if (  g.entidmaster>0 ) 
	{
		for ( t.entid = 1 ; t.entid<=  g.entidmaster; t.entid++ )
		{
			WriteString (  1,t.entitybank_s[t.entid].Get() );
		}
	}
	CloseFile ( 1 );
}

void entity_savebank_ebe ( void )
{
	// Empty EBEs from testmap folder
	// 190417 - oops, this is deleting perfectly needed textures from testmap folder
	// when it should leave textures alone that may belong to the level EBEs
	// so set a flag to protect textures when save
	cstr pStoreOld = GetDir(); SetDir ( g.mysystem.levelBankTestMap_s.Get() ); //"levelbank\\testmap\\" );
	mapfile_emptyebesfromtestmapfolder(true);
	SetDir ( pStoreOld.Get() );

	// now save all EBE to testmap folder
	for ( t.tttentid = 1 ; t.tttentid <= g.entidmaster; t.tttentid++ )
	{
		if ( strlen(t.entitybank_s[t.tttentid].Get()) > 0 )
		{
			if ( t.entityprofile[t.tttentid].ebe.dwRLESize > 0 )
			{
				// Save EBE to represent this creation in the level
				//cStr tSaveFile = cstr("levelbank\\testmap\\ebe") + cstr(t.tttentid) + cstr(".ebe");
				cStr tSaveFile = g.mysystem.levelBankTestMap_s + cstr("ebe") + cstr(t.tttentid) + cstr(".ebe");
				
				ebe_save_ebefile ( tSaveFile, t.tttentid );
			}
		}
	}
}

void entity_loadbank ( void )
{
	// 050416 - build a black list for parental control (used below)
	if ( g_pBlackList == NULL && g.quickparentalcontrolmode == 2 )
	{
		// first pass count, second one writes into array
		for ( int iPass = 0; iPass < 2; iPass++ )
		{
			// reset count
			g_iBlackListMax = 0;

			// open blacklist file and go through all contents
			FILE* fp = fopen ( ".\\..\\parentalcontrolblacklist.ini", "rt" );
			if ( fp )
			{
				char c;
				fread ( &c, sizeof ( char ), 1, fp );
				while ( !feof ( fp ) )
				{
					// get string from file
					char szEntNameFromFile [ MAX_PATH ] = "";
					int iOffset = 0;
					while ( !feof ( fp ) && c!=13 && c!=10 )
					{
						szEntNameFromFile [ iOffset++ ] = c;
						fread ( &c, sizeof ( char ), 1, fp );
					}
					szEntNameFromFile [ iOffset ] = 0;

					// skip beyond CR
					while ( !feof ( fp ) && (c==13 || c==10) )
						fread ( &c, sizeof ( char ), 1, fp );

					// count or write
					if ( iPass==0 )
					{
						// count
						g_iBlackListMax++;
					}
					else
					{
						// write into array
						g_pBlackList[g_iBlackListMax] = new char[512];
						strlwr ( szEntNameFromFile );
						strcpy ( g_pBlackList[g_iBlackListMax], szEntNameFromFile );
						g_iBlackListMax++;
					}
				}
				fclose ( fp );
			}
			
			// at end, create dynamic string array
			if ( iPass==0 ) g_pBlackList = new LPSTR[g_iBlackListMax];
		}
	}

	//  If ent file exists
	t.filename_s=t.levelmapptah_s+"map.ent";
	if (  FileExist(t.filename_s.Get()) == 1 ) 
	{
		//  Destroy old entities
		entity_deletebank ( );

		//  Load entity bank
		OpenToRead (  1, cstr(t.levelmapptah_s+"map.ent").Get() );
		g.entidmaster = ReadLong ( 1 );
		if (  g.entidmaster>0 ) 
		{
			entity_validatearraysize ( );
			for ( t.entid = 1 ; t.entid<=  g.entidmaster; t.entid++ )
			{
				t.entitybank_s[t.entid] = ReadString ( 1 );
			}
		}
		CloseFile (  1 );

		// 050416 - blank out any entities which are blacklisted
		g_bBlackListRemovedSomeEntities = false;
		if ( g_pBlackList != NULL )
		{
			char pThisEntityFilename[512];
			for ( t.entid = 1 ; t.entid <= g.entidmaster; t.entid++ )
			{
				strcpy ( pThisEntityFilename, t.entitybank_s[t.entid].Get() );
				strlwr ( pThisEntityFilename );
				for ( int iBlackListIndex=0; iBlackListIndex<g_iBlackListMax; iBlackListIndex++ )
				{
					int iBlacklistEntityNameLength = strlen ( g_pBlackList[iBlackListIndex] );
					int iCompareEnd = strlen ( pThisEntityFilename ) - iBlacklistEntityNameLength;
					if ( strnicmp ( g_pBlackList[iBlackListIndex], pThisEntityFilename + iCompareEnd, iBlacklistEntityNameLength )==NULL )
					{
						// this entity has been banned by parents
						g_bBlackListRemovedSomeEntities = true;
						t.entitybank_s[t.entid] = "";
					}
				}
			}
		}

		//  260215 - Do a pre-scan to determine if any entities are missing
		if (  Len(t.editor.replacefilepresent_s.Get())>1 ) 
		{
			//  load all replacements in a table
			Dim2(  t.replacements_s,1000, 1  );
			t.treplacementmax=0;
			if (  FileExist(t.editor.replacefilepresent_s.Get()) == 1 ) 
			{
				LoadArray (  t.editor.replacefilepresent_s.Get(),t.replacements_s );
				for ( t.l = 0 ; t.l <= ArrayCount2(t.replacements_s); t.l++ )
				{
					t.tline_s=ArrayAt(t.replacements_s , t.l );
					for ( t.n = 1 ; t.n<=  Len(t.tline_s.Get()); t.n++ )
					{
						if (  cstr(Mid(t.tline_s.Get(),t.n)) == "=" ) 
						{
							t.told_s=Left(t.tline_s.Get(),t.n-1);
							t.told2_s="";
							for ( t.nn = 1 ; t.nn<=  Len(t.told_s.Get()); t.nn++ )
							{
								t.told2_s=t.told2_s+Mid(t.told_s.Get(),t.nn);
								if ( (cstr(Mid(t.told_s.Get(),t.nn)) == "\\" && cstr(Mid(t.told_s.Get(),t.nn+1)) == "\\") || (cstr(Mid(t.told_s.Get(),t.nn)) == "/" && cstr(Mid(t.told_s.Get(),t.nn+1)) == "/" ) )
								{
									++t.nn;
								}
							}
							t.tnew_s=Right(t.tline_s.Get(),Len(t.tline_s.Get())-t.n);
							++t.treplacementmax;
							t.replacements_s[t.treplacementmax][0]=Lower(t.told2_s.Get());
							t.replacements_s[t.treplacementmax][1]=Lower(t.tnew_s.Get());
						}
					}
				}
			}
			//  go through all entities FPM is about to use
			for ( t.entid = 1 ; t.entid <= g.entidmaster; t.entid++ )
			{
				if ( strlen ( t.entitybank_s[t.entid].Get() ) > 0 )
				{
					t.tfile_s=g.fpscrootdir_s+"\\Files\\entitybank\\"+t.entitybank_s[t.entid];
					if (  FileExist(t.tfile_s.Get()) == 0 ) 
					{
						t.tbinfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4))+cstr(".bin");
						if (  FileExist(t.tbinfile_s.Get()) == 0 ) 
						{
							//  cannot find FPE or BIN, so search replace file if we have a substitute
							t.tent_s=cstr("entitybank\\")+t.entitybank_s[t.entid];
							t.ttry2_s="";
							for ( t.nn = 1 ; t.nn<=  Len(t.tent_s.Get()); t.nn++ )
							{
								t.ttry2_s=t.ttry2_s+Mid(t.tent_s.Get(),t.nn);
								if (  (cstr(Mid(t.tent_s.Get(),t.nn)) == "\\" && cstr(Mid(t.tent_s.Get(),t.nn+1)) == "\\") || (cstr(Mid(t.tent_s.Get(),t.nn)) == "/" && cstr(Mid(t.tent_s.Get(),t.nn+1)) == "/") ) 
								{
									++t.nn;
								}
							}
							t.tent_s=Lower(t.ttry2_s.Get());
							t.tfoundmatch=0;
							for ( t.tt = 1 ; t.tt<=  t.treplacementmax; t.tt++ )
							{
								if (  t.replacements_s[t.tt][0] == t.tent_s ) 
								{
									//  found a match with an entry in the .REPLACE file
									//  so replace master entry (entity instances will continue to reference by index)
									t.tnewent_s=t.replacements_s[t.tt][1];
									t.entitybank_s[t.entid]=Right(t.tnewent_s.Get(),Len(t.tnewent_s.Get())-Len("entitybank\\"));
									t.tfoundmatch=1;
								}
							}
						}
					}
				}
			}
			//  retain replacements$() for later in loadelementsdata
		}

		//  Load in all entity objects and data
		entity_loadentitiesnow ( );
	}
}

void entity_loadentitiesnow ( void )
{
	//  Load entities specified by bank
	if ( g.entidmaster>0 ) 
	{
		for ( t.entid = 1 ; t.entid <= g.entidmaster; t.entid++ )
		{
			t.entdir_s = "entitybank\\";
			t.ent_s = t.entitybank_s[t.entid];
			t.entpath_s = getpath(t.ent_s.Get());
			if (  t.lightmapper.onlyloadstaticentitiesduringlightmapper == 1 ) 
			{
				t.tonscreenprompt_s=cstr("Load ")+Str(t.entid)+" : "+Right(t.ent_s.Get(),Len(t.ent_s.Get())-1)  ; lm_flashprompt ( );
			}
			// if not an FPE, load FPE from EBE source
			if ( strcmp ( Lower(Right(t.ent_s.Get(),4)), ".fpe" ) != NULL )
			{
				// special EBE entity
				//ebe_load_ebefile ( cstr("levelbank\\testmap\\ebe") + cstr(t.entid) + cstr(".ebe"), t.entid );
				ebe_load_ebefile ( g.mysystem.levelBankTestMap_s + cstr("ebe") + cstr(t.entid) + cstr(".ebe"), t.entid );
				
				t.entityprofileheader[t.entid].desc_s = cstr("EBE") + cstr(t.entid);
				t.entdir_s = g.mysystem.levelBankTestMap_s; //"levelbank\\testmap\\";
				t.ent_s = cstr("ebe") + cstr(t.entid) + cstr(".fpe");
				t.entpath_s = "";
			}
			// regular FPE entity
			entity_load ( );
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
			if (  t.desc_s == "" ) 
			{
				// free RLE data in profile
				ebe_freecubedata ( t.entid );
			
				//  where entities have been lost, delete from list
				t.entitybank_s[t.entid]="";
			}
		}
	}
}

void entity_deletebank ( void )
{
	//  Destroy old entities
	if ( g.entidmastermax>0 ) 
	{
		for ( t.entid = 1 ; t.entid<=  g.entidmastermax; t.entid++ )
		{
			// delete parent entity object
			t.entobj = g.entitybankoffset+t.entid;
			if ( ObjectExist(t.entobj) == 1  ) DeleteObject (  t.entobj );
			characterkit_deleteBankObject ( );

			// free RLE data in profile
			ebe_freecubedata ( t.entid );

			// wipe from table
			t.entitybank_s[t.entid]="";
		}
	}

	//  reset character creator bankoffset
	t.characterkitcontrol.bankOffset = 0;
	t.characterkitcontrol.count = 0; // 150216 - Dave needs to learn how to clean up after himself!

	//C++ISSUE ADDED THIS IN - but commented it out, left it here tho
	//deleteallinternalimages();

	//  Destroy profile data
	UnDim (  t.entityprofile );
	Dim (  t.entityprofile,100 );
	g.entidmastermax=100;
	g.entidmaster=0;
}

void entity_deleteelementsdata ( void )
{

//  Free any old elements
entity_deleteelements ( );

//  Clear counter for new load
g.entityelementlist=0;
g.entityelementmax=0;

return;

}

void entity_deleteelements ( void )
{
	//  Quick deletes
	if ( g.entityelementlist > 0 ) 
	{
		DeleteObjects (  g.entityviewstartobj+1, g.entityviewstartobj+g.entityelementlist );
	}
	if ( g.entityattachmentindex > 0 ) 
	{
		DeleteObjects (  g.entityattachmentsoffset+1, g.entityattachmentsoffset+g.entityattachmentindex );
	}

	//  Clear any associations with infinilights and clones
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entityelement[t.e].eleprof.light.index=0;
		t.entityelement[t.e].isclone = 0;
	}

	//  clear any character creator objects associated with this entity
	for ( t.ccobjToDelete = 1 ; t.ccobjToDelete<=  g.entityelementlist; t.ccobjToDelete++ )
	{
		characterkit_deleteEntity ( );
	}

	//  set character creator offset back to 0 (which is a nice indicator that it is not in use also)
	t.characterkitcontrol.offset = 0;
}

void entity_assignentityparticletodecalelement ( void )
{
	if (  t.originatore>0 ) 
	{
		if (  t.entityelement[t.originatore].eleprof.particleoverride == 1 ) 
		{
			t.decalelement[t.d].particle=t.entityelement[t.originatore].eleprof.particle;
		}
	}
return;


//Subroutines used both in GRIDEDIT and in 3D Editing Mode


}

void entity_addentitytomap_core ( void )
{
	//  called from _entity_addentitytomap and also _game_masterroot
	if ( t.gridentityoverwritemode == 0 )
	{
		//  Create new or use free entity element
		t.tokay=0;
		if (  g.entityelementlist>0 ) 
		{
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				if (  t.entityelement[t.e].maintype == 0 ) { t.tokay = t.e  ; break; }
			}
		}
		if (  t.tokay == 0 ) 
		{
			++g.entityelementlist;
			t.e=g.entityelementlist;
			if (  g.entityelementlist>g.entityelementmax ) 
			{
				Dim ( t.storeentityelement,g.entityelementmax );
				for ( t.e = 1 ; t.e<=  g.entityelementmax; t.e++ )
				{
					t.storeentityelement[t.e]=t.entityelement[t.e];
				}
				UnDim (  t.entityelement );
				UnDim (  t.entityshadervar );
				g.entityelementmax +=10;
				Dim (  t.entityelement,g.entityelementmax );
				Dim2(  t.entityshadervar,g.entityelementmax, g.globalselectedshadermax  );
				for ( t.e = 1 ; t.e<=  g.entityelementmax-10; t.e++ )
				{
					t.entityelement[t.e]=t.storeentityelement[t.e];
				}
			}
		}
		else
		{
			t.e=t.tokay;
		}
	}
	else
	{
		// can force new entity into a specific slot (when undo entiy group delete)
		t.e = t.gridentityoverwritemode;
	}

	//  Fill entity element details
	t.entityelement[t.e].editorfixed=t.gridentityeditorfixed;
	t.entityelement[t.e].maintype=t.entitymaintype;
	t.entityelement[t.e].bankindex=t.entitybankindex;
	t.entityelement[t.e].staticflag=t.gridentitystaticmode;
	t.entityelement[t.e].iHasParentIndex=t.gridentityhasparent;
	t.entityelement[t.e].x=t.gridentityposx_f;
	t.entityelement[t.e].z=t.gridentityposz_f;
	t.entityelement[t.e].y=t.gridentityposy_f;
	t.entityelement[t.e].rx=t.gridentityrotatex_f;
	t.entityelement[t.e].ry=t.gridentityrotatey_f;
	t.entityelement[t.e].rz=t.gridentityrotatez_f;
	t.entityelement[t.e].scalex=t.gridentityscalex_f-100;
	t.entityelement[t.e].scaley=t.gridentityscaley_f-100;
	t.entityelement[t.e].scalez=t.gridentityscalez_f-100;
	t.entityelement[t.e].eleprof=t.grideleprof;
	t.entityelement[t.e].eleprof.light.index=0;
	t.entityelement[t.e].soundset=0;
	t.entityelement[t.e].soundset1=0;
	t.entityelement[t.e].soundset2=0;
	t.entityelement[t.e].soundset3=0;
	t.entityelement[t.e].soundset4=0;
	t.entityelement[t.e].underground=0;
	t.entityelement[t.e].beenmoved=1;
}

void entity_addentitytomap ( void )
{
	// Entity To Add
	t.entitymaintype=1;
	t.entitybankindex=t.gridentity;
	entity_addentitytomap_core ( );

	// transfer waypoint zone index to entityelement
	t.waypointindex=t.grideleprof.trigger.waypointzoneindex;
	t.entityelement[t.e].eleprof.trigger.waypointzoneindex=t.waypointindex;
	t.waypoint[t.waypointindex].linkedtoentityindex=t.e;

	//  as create entity, apply any texture change required
	t.stentid=t.entid ; t.entid=t.entitybankindex;
	t.entdir_s="entitybank\\" ; t.ent_s=t.entitybank_s[t.entid] ; t.entpath_s=getpath(t.ent_s.Get());

	//  GRIDELEPROF might contain GUN+FLAK Data
	t.entid=t.entityelement[t.e].bankindex;
	t.tgunid_s=t.entityprofile[t.entid].isweapon_s;
	entity_getgunidandflakid ( );
	if (  t.tgunid>0 ) 
	{
		// populate the actual gun and flak settings (for further weapon entity creations)
		int firemode = 0; // 110718 - entity properties should only edit first primary gun settings (so we dont mess up enhanced weapons)
		//for ( int firemode = 0; firemode < 2; firemode++ )
		g.firemodes[t.tgunid][firemode].settings.damage=t.grideleprof.damage;
		g.firemodes[t.tgunid][firemode].settings.accuracy=t.grideleprof.accuracy;
		g.firemodes[t.tgunid][firemode].settings.reloadqty=t.grideleprof.reloadqty;
		g.firemodes[t.tgunid][firemode].settings.iterate=t.grideleprof.fireiterations;
		g.firemodes[t.tgunid][firemode].settings.range=t.grideleprof.range;
		g.firemodes[t.tgunid][firemode].settings.dropoff=t.grideleprof.dropoff;
		g.firemodes[t.tgunid][firemode].settings.usespotlighting=t.grideleprof.usespotlighting;
		if (  t.tflakid>0 ) 
		{
			// flak to follow
		}
		//  which must also populate ALL other entities of same weapon
		t.tgunidchanged=t.tgunid;
		for ( t.te = 1 ; t.te<=  g.entityelementlist; t.te++ )
		{
			t.tentid=t.entityelement[t.te].bankindex;
			t.tgunid_s=t.entityprofile[t.tentid].isweapon_s;
			entity_getgunidandflakid ( );
			if (  t.tgunid == t.tgunidchanged ) 
			{
				t.entityelement[t.te].eleprof.damage=t.grideleprof.damage;
				t.entityelement[t.te].eleprof.accuracy=t.grideleprof.accuracy;
				t.entityelement[t.te].eleprof.reloadqty=t.grideleprof.reloadqty;
				t.entityelement[t.te].eleprof.fireiterations=t.grideleprof.fireiterations;
				t.entityelement[t.te].eleprof.range=t.grideleprof.range;
				t.entityelement[t.te].eleprof.dropoff=t.grideleprof.dropoff;
				t.entityelement[t.te].eleprof.usespotlighting=t.grideleprof.usespotlighting;
				t.entityelement[t.te].eleprof.lifespan=t.grideleprof.lifespan;
				t.entityelement[t.te].eleprof.throwspeed=t.grideleprof.throwspeed;
				t.entityelement[t.te].eleprof.throwangle=t.grideleprof.throwangle;
				t.entityelement[t.te].eleprof.bounceqty=t.grideleprof.bounceqty;
				t.entityelement[t.te].eleprof.explodeonhit=t.grideleprof.explodeonhit;
			}
		}
	}

	//  If multiplayer start marker, must propogate any script change to all others
	if (  t.entityprofile[t.entid].ismarker == 7 ) 
	{
		for ( t.te = 1 ; t.te<=  g.entityelementlist; t.te++ )
		{
			if (  t.te != t.e ) 
			{
				t.tentid=t.entityelement[t.te].bankindex;
				if (  t.entityprofile[t.tentid].ismarker == 7 ) 
				{
					t.entityelement[t.te].eleprof.aiinit_s=t.entityelement[t.e].eleprof.aiinit_s;
					t.entityelement[t.te].eleprof.aimain_s=t.entityelement[t.e].eleprof.aimain_s;
					t.entityelement[t.te].eleprof.aidestroy_s=t.entityelement[t.e].eleprof.aidestroy_s;
					t.entityelement[t.te].eleprof.aishoot_s=t.entityelement[t.e].eleprof.aishoot_s;
					t.entityelement[t.te].eleprof.soundset_s=t.entityelement[t.e].eleprof.soundset_s;
					t.entityelement[t.te].eleprof.soundset1_s=t.entityelement[t.e].eleprof.soundset1_s;
					t.entityelement[t.te].eleprof.soundset2_s=t.entityelement[t.e].eleprof.soundset2_s;
					t.entityelement[t.te].eleprof.soundset3_s=t.entityelement[t.e].eleprof.soundset3_s;
					t.entityelement[t.te].eleprof.soundset4_s=t.entityelement[t.e].eleprof.soundset4_s;
					t.entityelement[t.te].eleprof.strength=t.entityelement[t.e].eleprof.strength;
					t.entityelement[t.te].eleprof.speed=t.entityelement[t.e].eleprof.speed;
					t.entityelement[t.te].eleprof.animspeed=t.entityelement[t.e].eleprof.animspeed;
				}
			}
		}
	}

	//  Add entity reference into map
	t.tupdatee=t.e; entity_updateentityobj ( );

	// mark as static if it was
	if ( t.entityelement[t.tupdatee].staticflag == 1 ) g.projectmodifiedstatic = 1;

	// 160616 - just added EBE Builder New Site Entity 
	if ( t.entityprofile[t.gridentity].isebe > 0 )
	{
		if ( stricmp ( t.entitybank_s[t.gridentity].Get(), "..\\ebebank\\_builder\\New Site.fpe" ) == NULL )
		{
			// NEW FPE and EBE SITE
			t.entitybankindex = t.gridentity;

			// add new entity to entity library
			t.entityprofileheader[t.entitybankindex].desc_s = cstr("EBE") + cstr(t.entitybankindex);

			// update entityelement with new parent entity details
			t.entityelement[t.e].bankindex = t.entitybankindex;

			// change to site name after above creation from orig FPE template
			ebe_newsite ( t.e );

			// update entity FPE parent object from above newsite entity element obj
			t.entitybank_s[t.entitybankindex] = t.entityprofileheader[t.entitybankindex].desc_s;
			t.entityelement[t.e].bankindex = t.entitybankindex;
			t.entityelement[t.e].eleprof.name_s = t.entityprofileheader[t.entitybankindex].desc_s;
			ebe_updateparent ( t.e );

			// ensure mouse is released before painting on grid
			t.ebe.bReleaseMouseFirst = true;
		}
		else
		{
			// Selected an existing EBE entity from library
		}
	}

	//PE: Moved here as we use the object direction vector for spot lights.
	// update infinilight list with addition
	if (t.entityprofile[t.entitybankindex].ismarker == 2 || t.entityprofile[t.entitybankindex].ismarker == 5 || t.entityelement[t.e].eleprof.usespotlighting)
	{
		lighting_refresh();
	}


}

void entity_deleteentityfrommap ( void )
{
	//  Entity Type To Delete
	t.entitymaintype=1;

	//  Use entity coord to find tile
	t.tupdatee=t.tentitytoselect;

	// remember entity bank index for later light refresh
	int iWasEntID = t.entityelement[t.tupdatee].bankindex;

	//  cleanup character creator
	t.ccobjToDelete = t.tupdatee;
	characterkit_deleteEntity ( );

	// mark as static if it was
	if ( t.entityelement[t.tupdatee].staticflag == 1 ) g.projectmodifiedstatic = 1;

	//  blank from entity element list
	t.entityelement[t.tupdatee].bankindex=0;
	t.entityelement[t.tupdatee].maintype=0;
	t.entityelement[t.tupdatee].iHasParentIndex = 0;
	deleteinternalsound(t.entityelement[t.tupdatee].soundset) ; t.entityelement[t.tupdatee].soundset=0;
	deleteinternalsound(t.entityelement[t.tupdatee].soundset1) ; t.entityelement[t.tupdatee].soundset1=0;
	deleteinternalsound(t.entityelement[t.tupdatee].soundset2) ; t.entityelement[t.tupdatee].soundset2=0;
	deleteinternalsound(t.entityelement[t.tupdatee].soundset3) ; t.entityelement[t.tupdatee].soundset3=0;
	deleteinternalsound(t.entityelement[t.tupdatee].soundset4) ; t.entityelement[t.tupdatee].soundset4=0;

	//  Delete any associated waypoint/trigger zone
	//  SK; This sub can be called from the WP module when it's deleting a whole WP path, and in turn deleting the entity using it.
	//  In this situation we DO NOT want it to try and delete it's own WPs as this becomes cyclic!
	t.waypointindex=t.entityelement[t.tupdatee].eleprof.trigger.waypointzoneindex;
	if (  t.waypointindex > 0 ) 
	{
		if (  t.grideleprof.trigger.waypointzoneindex != t.waypointindex && t.tDontDeleteWPFlag  ==  0 ) 
		{
			t.w=t.waypoint[t.waypointindex].start;
			waypoint_delete ( );
		}
		t.entityelement[t.tupdatee].eleprof.trigger.waypointzoneindex=0;
	}
	t.tDontDeleteWPFlag = 0;

	// update infinilight list with removal
	if ( t.entityprofile[iWasEntID].ismarker == 2 || t.entityprofile[iWasEntID].ismarker == 5 ) 
	{
		//  refresh existing lights
		lighting_refresh ( );
	}

	// update real ent obj (.obj=0 inside)
	entity_updateentityobj ( );
}

void entity_recordbuffer_add ( void )
{
	t.terrainundo.bufferfilled=0;
	t.entityundo.action=1;
	t.entityundo.undoperformed=0;
	t.entityundo.entityindex=t.e;
	t.entityundo.bankindex=t.entityelement[t.e].bankindex;
}

void entity_recordbuffer_delete ( void )
{
	t.terrainundo.bufferfilled=0;
	t.entityundo.action=2;
	t.entityundo.undoperformed=0;
	t.entityundo.entityindex=t.tentitytoselect;
	t.entityundo.bankindex=t.entityelement[t.tentitytoselect].bankindex;
}

void entity_recordbuffer_move ( void )
{
	t.terrainundo.bufferfilled=0;
	t.entityundo.action=3;
	t.entityundo.undoperformed=0;
	t.entityundo.entityindex=t.tentitytoselect;
	t.entityundo.posbkup=t.entityundo.pos;
	t.entityundo.pos.staticflag=t.entityelement[t.tentitytoselect].staticflag;
	t.entityundo.pos.x=t.entityelement[t.tentitytoselect].x;
	t.entityundo.pos.y=t.entityelement[t.tentitytoselect].y;
	t.entityundo.pos.z=t.entityelement[t.tentitytoselect].z;
	t.entityundo.pos.rx=t.entityelement[t.tentitytoselect].rx;
	t.entityundo.pos.ry=t.entityelement[t.tentitytoselect].ry;
	t.entityundo.pos.rz=t.entityelement[t.tentitytoselect].rz;
	t.entityundo.pos.scalex=t.entityelement[t.tentitytoselect].scalex;
	t.entityundo.pos.scaley=t.entityelement[t.tentitytoselect].scaley;
	t.entityundo.pos.scalez=t.entityelement[t.tentitytoselect].scalez;
}

void entity_undo ( void )
{
	t.tactioninput=t.entityundo.action;
	if (  t.tactioninput == 1 ) 
	{
		//  undo addition DELETE IT
		t.tentitytoselect=t.entityundo.entityindex;
		entity_deleteentityfrommap ( );
		t.entityundo.action=2;
	}
	if (  t.tactioninput == 2 ) 
	{
		//  undo deletion ADD IT BACK
		if ( t.entityundo.entityindex == -123 )
		{
			// undo delete of rubberbandlist
			for ( int i = 0; i < (int)g.entityrubberbandlistundo.size(); i++ )
			{
				t.e = g.entityrubberbandlistundo[i].entityindex;
				t.entid = g.entityrubberbandlistundo[i].bankindex;
				t.entitybankindex=t.entid;
				t.gridentityeditorfixed=t.entityelement[t.e].editorfixed;
				t.entitymaintype=t.entityelement[t.e].maintype;
				t.gridentitystaticmode=t.entityelement[t.e].staticflag;
				t.gridentityhasparent=t.entityelement[t.e].iHasParentIndex;
				t.gridentityposx_f=t.entityelement[t.e].x;
				t.gridentityposz_f=t.entityelement[t.e].z;
				t.gridentityposy_f=t.entityelement[t.e].y;
				t.gridentityrotatex_f=t.entityelement[t.e].rx;
				t.gridentityrotatey_f=t.entityelement[t.e].ry;
				t.gridentityrotatez_f=t.entityelement[t.e].rz;
				t.gridentityscalex_f=t.entityelement[t.e].scalex+100;
				t.gridentityscaley_f=t.entityelement[t.e].scaley+100;
				t.gridentityscalez_f=t.entityelement[t.e].scalez+100;
				t.grideleprof=t.entityelement[t.e].eleprof;
				t.gridentity=t.entid;
				t.gridentityoverwritemode=t.e;
				entity_addentitytomap ( );
				t.gridentityoverwritemode=0;
			}
			g.entityrubberbandlistundo.clear();
			t.gridentity=0;
			t.entityundo.action=0;
		}
		else
		{
			// undo delete of single entity
			t.e=t.entityundo.entityindex;
			t.entid=t.entityundo.bankindex;
			t.entitybankindex=t.entid;
			t.gridentityeditorfixed=t.entityelement[t.e].editorfixed;
			t.entitymaintype=t.entityelement[t.e].maintype;
			t.gridentitystaticmode=t.entityelement[t.e].staticflag;
			t.gridentityhasparent=t.entityelement[t.e].iHasParentIndex;
			t.gridentityposx_f=t.entityelement[t.e].x;
			t.gridentityposz_f=t.entityelement[t.e].z;
			t.gridentityposy_f=t.entityelement[t.e].y;
			t.gridentityrotatex_f=t.entityelement[t.e].rx;
			t.gridentityrotatey_f=t.entityelement[t.e].ry;
			t.gridentityrotatez_f=t.entityelement[t.e].rz;
			t.gridentityscalex_f=t.entityelement[t.e].scalex+100;
			t.gridentityscaley_f=t.entityelement[t.e].scaley+100;
			t.gridentityscalez_f=t.entityelement[t.e].scalez+100;
			t.grideleprof=t.entityelement[t.e].eleprof;
			t.gridentity=t.entid;
			entity_addentitytomap ( );
			t.gridentity=0;
			t.entityundo.action=1;
		}
	}

	//  These cano update existing entity
	t.tupdatentpos=0;
	if (  t.tactioninput == 3 ) 
	{
		//  undo MOVE and restore previous position
		if ( t.entityundo.entityindex == -123 )
		{
			// undo move of rubberbandlist
			for ( int i = 0; i < (int)g.entityrubberbandlistundo.size(); i++ )
			{
				t.e = g.entityrubberbandlistundo[i].entityindex;
				//t.entityundo.posbkup.staticflag=t.entityelement[t.e].staticflag; // no REDO for rubber band undo!
				//t.entityundo.posbkup.x=t.entityelement[t.e].x;
				//t.entityundo.posbkup.y=t.entityelement[t.e].y;
				//t.entityundo.posbkup.z=t.entityelement[t.e].z;
				//t.entityundo.posbkup.rx=t.entityelement[t.e].rx;
				//t.entityundo.posbkup.ry=t.entityelement[t.e].ry;
				//t.entityundo.posbkup.rz=t.entityelement[t.e].rz;
				//t.entityundo.posbkup.scalex=t.entityelement[t.e].scalex;
				//t.entityundo.posbkup.scaley=t.entityelement[t.e].scaley;
				//t.entityundo.posbkup.scalez=t.entityelement[t.e].scalez;
				t.entityelement[t.e].staticflag=g.entityrubberbandlistundo[i].pos.staticflag;
				t.entityelement[t.e].x=g.entityrubberbandlistundo[i].pos.x;
				t.entityelement[t.e].y=g.entityrubberbandlistundo[i].pos.y;
				t.entityelement[t.e].z=g.entityrubberbandlistundo[i].pos.z;
				t.entityelement[t.e].rx=g.entityrubberbandlistundo[i].pos.rx;
				t.entityelement[t.e].ry=g.entityrubberbandlistundo[i].pos.ry;
				t.entityelement[t.e].rz=g.entityrubberbandlistundo[i].pos.rz;
				t.entityelement[t.e].scalex=g.entityrubberbandlistundo[i].pos.scalex;
				t.entityelement[t.e].scaley=g.entityrubberbandlistundo[i].pos.scaley;
				t.entityelement[t.e].scalez=g.entityrubberbandlistundo[i].pos.scalez;
			}
			t.entityundo.action=0;
		}
		else
		{
			// single entity position undo
			t.e=t.entityundo.entityindex;
			t.entityundo.posbkup.staticflag=t.entityelement[t.e].staticflag;
			t.entityundo.posbkup.x=t.entityelement[t.e].x;
			t.entityundo.posbkup.y=t.entityelement[t.e].y;
			t.entityundo.posbkup.z=t.entityelement[t.e].z;
			t.entityundo.posbkup.rx=t.entityelement[t.e].rx;
			t.entityundo.posbkup.ry=t.entityelement[t.e].ry;
			t.entityundo.posbkup.rz=t.entityelement[t.e].rz;
			t.entityundo.posbkup.scalex=t.entityelement[t.e].scalex;
			t.entityundo.posbkup.scaley=t.entityelement[t.e].scaley;
			t.entityundo.posbkup.scalez=t.entityelement[t.e].scalez;
			t.entityelement[t.e].staticflag=t.entityundo.pos.staticflag;
			t.entityelement[t.e].x=t.entityundo.pos.x;
			t.entityelement[t.e].y=t.entityundo.pos.y;
			t.entityelement[t.e].z=t.entityundo.pos.z;
			t.entityelement[t.e].rx=t.entityundo.pos.rx;
			t.entityelement[t.e].ry=t.entityundo.pos.ry;
			t.entityelement[t.e].rz=t.entityundo.pos.rz;
			t.entityelement[t.e].scalex=t.entityundo.pos.scalex;
			t.entityelement[t.e].scaley=t.entityundo.pos.scaley;
			t.entityelement[t.e].scalez=t.entityundo.pos.scalez;
			t.entityundo.action=4;
		}
		t.tupdatentpos=1;
	}

	if (  t.tactioninput == 4 ) 
	{
		//  restore previous POS from BKUP
		t.e=t.entityundo.entityindex;
		t.entityelement[t.e].staticflag=t.entityundo.posbkup.staticflag;
		t.entityelement[t.e].x=t.entityundo.posbkup.x;
		t.entityelement[t.e].y=t.entityundo.posbkup.y;
		t.entityelement[t.e].z=t.entityundo.posbkup.z;
		t.entityelement[t.e].rx=t.entityundo.posbkup.rx;
		t.entityelement[t.e].ry=t.entityundo.posbkup.ry;
		t.entityelement[t.e].rz=t.entityundo.posbkup.rz;
		t.entityelement[t.e].scalex=t.entityundo.posbkup.scalex;
		t.entityelement[t.e].scaley=t.entityundo.posbkup.scaley;
		t.entityelement[t.e].scalez=t.entityundo.posbkup.scalez;
		t.entityundo.action=3;
		t.tupdatentpos=1;
	}

	// update single entity or list
	if ( t.tupdatentpos == 1 ) 
	{
		if ( g.entityrubberbandlistundo.size() > 0 )
		{
			bool bUpdateLights = false;
			for ( int i = 0; i < (int)g.entityrubberbandlistundo.size(); i++ )
			{
				t.e = g.entityrubberbandlistundo[i].entityindex;
				t.tentid = t.entityelement[t.e].bankindex;
				if ( t.entityprofile[t.tentid].ismarker == 2 || t.entityprofile[t.tentid].ismarker == 5 ) bUpdateLights = true;
				t.tte = t.e; 
				t.tobj = t.entityelement[t.e].obj;
				t.tentid = t.entityelement[t.e].bankindex;
				entity_positionandscale ( );
			}
			if ( bUpdateLights==true )
				lighting_refresh ( );
		}
		else
		{
			t.tentid=t.entityelement[t.e].bankindex;
			if (  t.entityprofile[t.tentid].ismarker == 2 || t.entityprofile[t.tentid].ismarker == 5 ) 
			{
				lighting_refresh ( );
			}
			t.tte=t.e ; t.tobj=t.entityelement[t.e].obj ; t.tentid=t.entityelement[t.e].bankindex;
			entity_positionandscale ( );
		}
	}

}

void entity_redo ( void )
{
	entity_undo ( );
return;
}
