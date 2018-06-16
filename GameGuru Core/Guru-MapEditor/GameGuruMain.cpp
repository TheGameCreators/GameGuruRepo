//
// GameGuru Main Engine
//

// Engine includes
#include "GameGuruMain.h"
#include <stdio.h>
#include <string.h>
#include "gameguru.h"

// FBX Importing
#include <iostream>
#include "FBXExporter\FBXExporter.h"

void GuruMain ( void )
{
	// Launch GameGuru 
	common_init();
}

void LoadFBX ( LPSTR szFilename, int iID )
{
	// External FBX Importer and Converter (by Bob)
	cstr sOldDir = GetDir();

	// 0. Exit if no file
	if ( FileExist ( szFilename ) == 0 )
	{
		// import file not exist
		return;
	}

	// 1a. Copy FBX to 'temp folder' and rename so has no spaces
	cstr sTempFolder = g.rootdir_s + "importer\\temp\\";
	cstr sTempFile = sTempFolder + "tempFBX.fbx";
	CopyFile ( szFilename, sTempFile.Get(), FALSE );

	// 1b. Delete all old PNG files
	SetDir ( sTempFolder.Get() );
	for ( int iDeletePass = 0; iDeletePass < 2; iDeletePass++ )
	{
		if ( iDeletePass == 1 && PathExist ( "tempFBX.fbm" ) == 1 ) SetDir ( "tempFBX.fbm" );
		ChecklistForFiles();
		for ( int n = 1; n <= ChecklistQuantity(); n++ )
		{
			cstr sFile = ChecklistString(n);
			if ( strlen ( sFile.Get() ) > 4 )
			{
				if ( FileExist ( sFile.Get() ) == 1 ) 
				{
					if ( strnicmp ( sFile.Get() + strlen(sFile.Get()) - 4, ".png", 4 ) == NULL
					||   strnicmp ( sFile.Get() + strlen(sFile.Get()) - 4, ".tga", 4 ) == NULL
					||   strnicmp ( sFile.Get() + strlen(sFile.Get()) - 4, ".jpg", 4 ) == NULL
					||   strnicmp ( sFile.Get() + strlen(sFile.Get()) - 4, ".dds", 4 ) == NULL
					||   strnicmp ( sFile.Get() + strlen(sFile.Get()) - 4, ".bmp", 4 ) == NULL )
						DeleteFile ( sFile.Get() );
				}
			}
		}
	}

	// 2a. Switch to converter tool folder
	cstr sConverterFolder = g.rootdir_s + "importer\\converter\\";
	SetDir ( sConverterFolder.Get() );

	// 2b. Convert the copied FBX file to Version 6
	cstr sConvertCall1 = cstr(Chr('"')) + sTempFile + cstr(Chr('"'));
	ExecuteFile ( "vconv.exe", sConvertCall1.Get(), "", 1 );
	cstr sConvertedFile = sTempFolder + "tempFBXVersion6.fbx";
	if ( FileExist ( sConvertedFile.Get() ) == 0 )
	{
		// failed to convert FBX to version 6
		SetDir ( sOldDir.Get() );
		return;
	}

	// 2c. Delete original temp FBX now we have it converted
	if ( FileExist ( sTempFile.Get() ) == 1 ) DeleteFile ( sTempFile.Get() );

	// 3a. Convert the Version 6 FBX file to an X file
	cstr sConvertCall2 = cstr(Chr('"')) + sConvertedFile + cstr(Chr('"'));
	ExecuteFile ( "Fbx2X.exe", sConvertCall2.Get(), "", 1 );
	cstr sNewXFile = sTempFolder + "tempFBXVersion6.x";

	// 3b. Delete version 6 FBX file now we have the X file
	if ( FileExist ( sConvertedFile.Get() ) == 1 ) DeleteFile ( sConvertedFile.Get() );

	// 4a. Check if X file exists and success
	if ( FileExist ( sNewXFile.Get() ) == 0 )
	{
		// failed to convert version 6 FBX file to X file
		SetDir ( sOldDir.Get() );
		return;
	}
	else
	{
		// success, now load X file into object passed in (for eventualy saving as DBO)
		if ( ObjectExist ( iID ) == 1 ) DeleteObject ( iID );
		LoadObject ( sNewXFile.Get(), iID );

		// change textures inside DBO object to suit new PBR system
		bool bDetectTextureShiftChanges = false;
		SetDir ( sTempFolder.Get() );
		sObject* pObject = GetObjectData ( iID );
		if ( pObject )
		{
			int iMeshCount = pObject->iMeshCount;
			for ( int iMesh = 0; iMesh < iMeshCount; iMesh++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iMesh];
				if ( pMesh )
				{
					DWORD dwTextureCount = pMesh->dwTextureCount;
					for ( int iTexture = 0; iTexture < dwTextureCount; iTexture++ )
					{
						// original name in mesh
						sTexture* pTexture = &pMesh->pTextures[iTexture];
						char pNewTextureName[1024];
						strcpy ( pNewTextureName, pTexture->pName );

						// check if needs changing
						char pCheckTextureName[1024];
						strcpy ( pCheckTextureName, pTexture->pName );
						if ( strlen ( pCheckTextureName ) > 12 )
						{
							// remove extension
							pCheckTextureName[strlen(pCheckTextureName)-4] = 0;

							// relace other designations with _color
							if ( strnicmp ( pCheckTextureName + strlen(pCheckTextureName) - 8, "_diffuse", 8 ) == NULL )
							{
								bDetectTextureShiftChanges = true;
								strcpy ( pNewTextureName, pCheckTextureName );
								pNewTextureName[strlen(pNewTextureName)-8] = 0;
								strcat ( pNewTextureName, "_color.png" );
							}
							if ( strnicmp ( pCheckTextureName + strlen(pCheckTextureName) - 10, "_basecolor", 10 ) == NULL )
							{
								bDetectTextureShiftChanges = true;
								strcpy ( pNewTextureName, pCheckTextureName );
								pNewTextureName[strlen(pNewTextureName)-10] = 0;
								strcat ( pNewTextureName, "_color.png" );
							}
							if ( strnicmp ( pCheckTextureName + strlen(pCheckTextureName) - 7, "_albedo", 7 ) == NULL )
							{
								bDetectTextureShiftChanges = true;
								strcpy ( pNewTextureName, pCheckTextureName );
								pNewTextureName[strlen(pNewTextureName)-7] = 0;
								strcat ( pNewTextureName, "_color.png" );
							}
						}

						// Only if new texture file does not exist
						if ( FileExist ( pNewTextureName ) == 0 )
						{
							// Create new texture file with new name
							if ( FileExist ( pTexture->pName ) == 1 )
							{
								// Copy new file and delete old
								bDetectTextureShiftChanges = true;
								CopyFile ( pTexture->pName, pNewTextureName, FALSE );
								DeleteFile ( pTexture->pName );
							}
							else
							{
								// Check the subfolder to see if it exists there
								cstr pSubFolder = sTempFolder + "tempFBX.fbm\\";
								cstr pSubFile = pSubFolder + pTexture->pName;
								if ( FileExist ( pSubFile.Get() ) == 1 )
								{
									// Copy file from subfolder
									bDetectTextureShiftChanges = true;
									CopyFile ( pSubFile.Get(), pNewTextureName, FALSE );
									//DeleteFile ( pSubFile.Get() );
								}
								else
								{
									// texture not alongside FBX, or in subfolder, so the FBX may
									// have texture/material assignments that differ from the texture filenames
									// so find closest match to texture in subfolder
									if ( PathExist ( pSubFolder.Get() ) == 1 )
									{
										cstr sStoreDir = GetDir();
										SetDir ( pSubFolder.Get() );
										ChecklistForFiles();
										int iBestScore = 0;
										cstr sBestFile = "";
										for ( int n = 1; n <= ChecklistQuantity(); n++ )
										{
											cstr sFile = ChecklistString(n);
											if ( strlen ( sFile.Get() ) > 4 )
											{
												char pScanA[1024];
												char pScanB[1024];
												strcpy ( pScanA, pTexture->pName );
												strcpy ( pScanB, sFile.Get() );
												strlwr ( pScanA );
												strlwr ( pScanB );
												int iScanSize = strlen(pScanA);
												if ( strlen(pScanB) < iScanSize ) iScanSize = strlen(pScanB);
												iScanSize=iScanSize-4;
												int iScore = 0;
												for ( int scan = 0; scan < iScanSize; scan++ )
												{
													if ( pScanA[scan] == pScanB[scan] ) 
														iScore++;
													else
														break;
												}
												if ( iScore > iBestScore )
												{
													iBestScore = iScore;
													sBestFile = sFile;
												}
											}
										}
										if ( strlen ( sBestFile.Get() ) > 0 )
										{
											// found the most likely match for this texture reference
											// final test is to check the original file and the best scoring one are of the right 'type'
											int iTextureTypeWanted[2];
											char pTextureFileToCheck[1024];
											iTextureTypeWanted[0] = 0;
											iTextureTypeWanted[1] = 0;
											for ( int i = 0; i < 2; i++ )
											{
												if ( i == 0 ) strcpy ( pTextureFileToCheck, pCheckTextureName ); 
												if ( i == 1 ) { strcpy ( pTextureFileToCheck, sBestFile.Get() ); pTextureFileToCheck[strlen(pTextureFileToCheck)-4]=0; } 
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 2, "_d", 2 ) == NULL ) iTextureTypeWanted[i] = 1;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 6, "_color", 6 ) == NULL ) iTextureTypeWanted[i] = 1;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 8, "_diffuse", 8 ) == NULL ) iTextureTypeWanted[i] = 1;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 10, "_basecolor", 10 ) == NULL ) iTextureTypeWanted[i] = 1;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 11, "_base_color", 11 ) == NULL ) iTextureTypeWanted[i] = 1;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 7, "_normal", 7 ) == NULL ) iTextureTypeWanted[i] = 2;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 2, "_n", 2 ) == NULL ) iTextureTypeWanted[i] = 2;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 6, "_gloss", 6 ) == NULL ) iTextureTypeWanted[i] = 3;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 10, "_roughness", 10 ) == NULL ) iTextureTypeWanted[i] = 3;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 2, "_r", 2 ) == NULL ) iTextureTypeWanted[i] = 3;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 10, "_metalness", 10 ) == NULL ) iTextureTypeWanted[i] = 4;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 2, "_m", 2 ) == NULL ) iTextureTypeWanted[i] = 4;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 9, "_specular", 9 ) == NULL ) iTextureTypeWanted[i] = 4;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 2, "_s", 2 ) == NULL ) iTextureTypeWanted[i] = 4;
												if ( strnicmp ( pTextureFileToCheck + strlen(pTextureFileToCheck) - 3, "_ao", 3 ) == NULL ) iTextureTypeWanted[i] = 5;
											}
											if ( iTextureTypeWanted[0] > 0 && iTextureTypeWanted[1] > 0 && iTextureTypeWanted[0] == iTextureTypeWanted[1] )
											{
												// best choice and original name from FBX have matching texture file types, 
												strcpy ( pNewTextureName, sBestFile.Get() );

												// so make this file the one to use, and copy it over
												SetDir ( sStoreDir.Get() );
												sBestFile = pSubFolder + sBestFile;
												char pExt[1024];
												strcpy ( pExt, pNewTextureName+strlen(pNewTextureName)-4);
												pNewTextureName[strlen(pNewTextureName)-4]=0;
												if ( strnicmp ( pNewTextureName + strlen(pNewTextureName) - 10, "_basecolor", 10 ) == NULL )
												{
													pNewTextureName[strlen(pNewTextureName)-10] = 0;
													strcat ( pNewTextureName, "_color" );
												}
												if ( strnicmp ( pNewTextureName + strlen(pNewTextureName) - 11, "_base_color", 11 ) == NULL )
												{
													pNewTextureName[strlen(pNewTextureName)-11] = 0;
													strcat ( pNewTextureName, "_color" );
												}
												strcat ( pNewTextureName, pExt );
												CopyFile ( sBestFile.Get(), pNewTextureName, FALSE );
											}
										}
										SetDir ( sStoreDir.Get() );
									}
								}
							}
						}

						// retain original extension type
						cstr sExt = Right ( pNewTextureName, 4 );

						// Now convert the texture file to DDS if original file exists (to convert)
						if ( FileExist ( pNewTextureName ) == 1 )
						{
							cstr pDDSFinalFile = Left ( pNewTextureName, strlen(pNewTextureName)-4 );
							pDDSFinalFile = pDDSFinalFile + ".dds";
							if ( FileExist ( pDDSFinalFile.Get() ) == 0 )
							{
								// create DDS using DirectX Tex functions
								if ( LoadAndSaveUsingDirectXTex ( pNewTextureName, pDDSFinalFile.Get() ) == true )
								{
									// successful DDS saved
								}
							}
							if ( FileExist ( pDDSFinalFile.Get() ) == 1 )
							{
								// successful DDS file exists, remove old file
								DeleteFile ( pNewTextureName );
							}
							strcpy ( pNewTextureName, pDDSFinalFile.Get() );

						}

						// Update object with new texture name
						strcpy ( pTexture->pName, pNewTextureName );

						// if a single stage texture, look for neighboring textures to copy over
						if ( dwTextureCount == 1 )
						{
							cstr sBaseFile = Left ( pNewTextureName, strlen(pNewTextureName)-4 );
							if ( strnicmp ( sBaseFile.Get() + strlen(sBaseFile.Get()) - 6, "_color", 6 ) == NULL )
							{
								cstr sRelSubFolder = "tempFBX.fbm\\";
								sBaseFile = Left ( sBaseFile.Get(), strlen(sBaseFile.Get())-6 );
								for ( int iNeighbor = 0; iNeighbor < 4; iNeighbor++ )
								{
									cstr sNeighborFile = "";
									if ( iNeighbor == 0 ) sNeighborFile = sBaseFile + "_normal";
									if ( iNeighbor == 1 ) sNeighborFile = sBaseFile + "_gloss";
									if ( iNeighbor == 2 ) sNeighborFile = sBaseFile + "_metalness";
									if ( iNeighbor == 3 ) sNeighborFile = sBaseFile + "_ao";
									cstr pSaveAsDDSFile = sNeighborFile + ".dds";
									sNeighborFile = sRelSubFolder + sNeighborFile + sExt;
									if ( FileExist ( sNeighborFile.Get() ) == 1 && FileExist ( pSaveAsDDSFile.Get() ) == 0 )
									{
										if ( LoadAndSaveUsingDirectXTex ( sNeighborFile.Get(), pSaveAsDDSFile.Get() ) == true )
										{
											// successful DDS saved
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// if texture file moved/renamed, reload textures
		pObject = GetObjectData ( iID );
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			LoadInternalTextures ( pObject->ppMeshList [ iMesh ], sTempFolder.Get(), 0, 0 );
	}

	// 4b. Delete X file now we have loaded it
	if ( FileExist ( sNewXFile.Get() ) == 1 ) DeleteFile ( sNewXFile.Get() );

	// Restore previous folder
	SetDir ( sOldDir.Get() );

	/* Lee's own internal FBX importer
	// import FBX, convert to DBO
	FBXExporter* myExporter = new FBXExporter();
	myExporter->Initialize();
	myExporter->LoadScene( szFilename, "" );
	myExporter->ExportFBX( iID );
	*/
}
