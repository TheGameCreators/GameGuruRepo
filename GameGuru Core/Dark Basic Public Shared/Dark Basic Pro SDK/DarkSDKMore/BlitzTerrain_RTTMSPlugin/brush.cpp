#include "brush.h"
#include <math.h>

#include "BlitzTerrain.h"

void BT_SetPointHeight(unsigned long TerrainID,unsigned long TVrow,unsigned long TVcol,float Height)
{
//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Get terrain info
		BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)BT_GetTerrainInfo(TerrainID);

	//Check that the terrain is built and generated
		if(TerrainInfo->Built==true && TerrainInfo->Generated==true)
		{
		//Check that the point is on the terrain
			if(TVrow>=0 && TVrow<=TerrainInfo->Heightmapsize && TVcol>=0 && TVcol<=TerrainInfo->Heightmapsize)
			{
			//Loop through LODLevels
				bool Break=false;
				unsigned char LODLevel=0;
				do{
				//Get LODLevel info
					BT_LODLevelInfo* LODLevelInfo=(BT_LODLevelInfo*)BT_GetLODLevelInfo(TerrainID,LODLevel);

				//Find which sector(s) the point is on
					unsigned short TwoPowerLODLevel=unsigned short(pow(2.0f,float(LODLevel)));
					unsigned short Vrow=unsigned short(TVrow/TwoPowerLODLevel);
					unsigned short Vcol=unsigned short(TVcol/TwoPowerLODLevel);
					if(Vrow*TwoPowerLODLevel!=TVrow || Vcol*TwoPowerLODLevel!=TVcol)
					{
						Break=true;
					}else{
						unsigned short Srow=unsigned short(Vrow/float(LODLevelInfo->SectorDetail));
						unsigned short Scol=unsigned short(Vcol/float(LODLevelInfo->SectorDetail));
						bool LeftSide=false;
						bool TopSide=false;
						if(Srow*LODLevelInfo->SectorDetail==Vrow)
							TopSide=true;
						if(Scol*LODLevelInfo->SectorDetail==Vcol)
							LeftSide=true;

						/*
						BT_UnlockVertexData();
						BT_LockVertexdataForSector(TerrainID,LODLevel,Srow*LODLevelInfo->Split+Scol);
						int x,z;
						for(x=0;x<=LODLevelInfo->SectorDetail;x++){
							for(z=0;z<=LODLevelInfo->SectorDetail;z++){
								BT_SetVertexHeight(x,z,Height);
							}
						}
						BT_UnlockVertexData();
						*/

					//Set the height of the point on this sector
						BT_UnlockVertexData();
						BT_LockVertexdataForSector(TerrainID,LODLevel,Srow*LODLevelInfo->Split+Scol);
						Vrow=Vrow-Srow*LODLevelInfo->SectorDetail;
						Vcol=Vcol-Scol*LODLevelInfo->SectorDetail;
						BT_SetVertexHeight(Vrow,Vcol,Height);
						BT_UnlockVertexData();

					//Set the height of the point on the left sector
						if(LeftSide==true && Scol>0)
						{
							BT_UnlockVertexData();
							BT_LockVertexdataForSector(TerrainID,LODLevel,Srow*LODLevelInfo->Split+(Scol-1));
							BT_SetVertexHeight(Vrow,LODLevelInfo->SectorDetail,Height);
							BT_UnlockVertexData();
						}

					//Set the height of the point on the top sector
						if(TopSide==true && Srow>0)
						{
							BT_UnlockVertexData();
							BT_LockVertexdataForSector(TerrainID,LODLevel,(Srow-1)*LODLevelInfo->Split+Scol);
							BT_SetVertexHeight(LODLevelInfo->SectorDetail,Vcol,Height);
							BT_UnlockVertexData();
						}

					//Set the height of the point on the top left sector
						if(LeftSide==true && TopSide==true && Srow>0 && Scol>0)
						{
							BT_UnlockVertexData();
							BT_LockVertexdataForSector(TerrainID,LODLevel,(Srow-1)*LODLevelInfo->Split+(Scol-1));
							BT_SetVertexHeight(LODLevelInfo->SectorDetail,LODLevelInfo->SectorDetail,Height);
							BT_UnlockVertexData();
						}

					//Increase LODLevel
						LODLevel++;
						if(LODLevel==TerrainInfo->LODLevels)
							Break=true;
					}
				}while(Break==false);
			}
		}
	}
}

void BT_SetGroundHeight(unsigned long TerrainID,float X,float Z,float Height)
{
//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Get terrain info
		BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)BT_GetTerrainInfo(TerrainID);

	//Check that the terrain is built and generated
		if(TerrainInfo->Built==true && TerrainInfo->Generated==true)
		{
		//Set the height
			BT_SetPointHeight(TerrainID,unsigned short((X/TerrainInfo->Scale)+0.5),unsigned short((Z/TerrainInfo->Scale)+0.5),Height);
		}
	}
}



//CIRCLE BRUSHES

typedef float(*CircleBrush_t)(float XDist,float ZDist,float MidHeight,float Radius,float Amount,float CurrentHeight,float CapHeight);

static void BT_CircleBrush(unsigned long TerrainID,float X,float Z,float Radius,float Amount,CircleBrush_t BrushFunc,bool GetMidHeight,float CapHeight)
{
//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Get terrain info
		BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)BT_GetTerrainInfo(TerrainID);

	//Check that the terrain is built and generated
		if(TerrainInfo->Built==true && TerrainInfo->Generated==true)
		{
		//Get mid height
			float MidHeight=0.0;

#ifdef COMPILE_GDK
			if(GetMidHeight==true)
				MidHeight=BT_GetGroundHeight(TerrainID,X,Z);
#else
			if(GetMidHeight==true)
			{
				unsigned long MidHeightDword=(unsigned long)BT_GetGroundHeight(TerrainID,X,Z);
				MidHeight=*(float*)&MidHeightDword;
			}
#endif

		//Find sector(s) the brush is on
				float OuterLeft=max(X-Radius,0);
			float OuterRight=min(X+Radius,TerrainInfo->TerrainSize);
			float OuterTop=max(Z-Radius,0);
			float OuterBottom=min(Z+Radius,TerrainInfo->TerrainSize);

		/* For future optimisation
			float InnerRadius=Radius*0.707106781f;
			float InnerLeft=max(X-InnerRadius,0);
			float InnerRight=X+InnerRadius;
			float InnerTop=max(Z-InnerRadius,0);
			float InnerBottom=Z+InnerRadius;
		*/

		//Loop through LODLevels
			bool Break=false;
			unsigned char LODLevel=0;

			unsigned short MidVrow=unsigned short((X/TerrainInfo->Scale));
			unsigned short MidVcol=unsigned short((Z/TerrainInfo->Scale));
			do{
			//Get LODLevel info
				BT_LODLevelInfo* LODLevelInfo=(BT_LODLevelInfo*)BT_GetLODLevelInfo(TerrainID,LODLevel);

			//Find the vertices the box is on
				unsigned short TwoPowerLODLevel=unsigned short(pow(2.0f,float(LODLevel)));
				unsigned short OuterTopV=unsigned short(floor((OuterTop/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterLeftV=unsigned short(floor((OuterLeft/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterBottomV=unsigned short(ceil((OuterBottom/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterRightV=unsigned short(ceil((OuterRight/TerrainInfo->Scale)/TwoPowerLODLevel));

			/* For future optimisation
				unsigned short InnerTopV=unsigned short(floor((InnerTop/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short InnerLeftV=unsigned short(floor((InnerLeft/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short InnerBottomV=unsigned short(min(ceil((InnerBottom/TerrainInfo->Scale)/TwoPowerLODLevel),TerrainInfo->Heightmapsize));
				unsigned short InnerRightV=unsigned short(min(ceil((InnerRight/TerrainInfo->Scale)/TwoPowerLODLevel),TerrainInfo->Heightmapsize));
			*/

			//Find the sectors the box is on
				unsigned short TopS=unsigned short(OuterTopV/float(LODLevelInfo->SectorDetail));
				unsigned short LeftS=unsigned short(OuterLeftV/float(LODLevelInfo->SectorDetail));
				unsigned short BottomS=unsigned short(OuterBottomV/float(LODLevelInfo->SectorDetail));
				unsigned short RightS=unsigned short(OuterRightV/float(LODLevelInfo->SectorDetail));
				if(TopS*LODLevelInfo->SectorDetail==OuterTopV && TopS>0)
					TopS--;
				if(LeftS*LODLevelInfo->SectorDetail==OuterLeftV && LeftS>0)
					LeftS--;

			//Loop through sectors
				for(unsigned short SectorX=LeftS;SectorX<=RightS;SectorX++)
				{
					for(unsigned short SectorY=TopS;SectorY<=BottomS;SectorY++)
					{
					//Lock vertexdata for this sector
						BT_UnlockVertexData();
						BT_LockVertexdataForSector(TerrainID,LODLevel,SectorX*LODLevelInfo->Split+SectorY);

					//Edit every vertex in sector, leave it to distance check to find which ones to do
						//This will be changed to inner and outer box in the future for a big speed increase
						for(unsigned short Vrow=0;Vrow<=LODLevelInfo->SectorDetail;Vrow++)
						{
							for(unsigned short Vcol=0;Vcol<=LODLevelInfo->SectorDetail;Vcol++)
							{
							#ifdef COMPILE_GDK
								float CurrentHeight=BT_GetVertexPositionY(Vrow,Vcol);
							#else
								unsigned long temp=BT_GetVertexPositionY(Vrow,Vcol);
								float CurrentHeight=*(float*)&temp;
							#endif

								float XDist=(MidVrow-(SectorX*LODLevelInfo->SectorDetail+Vrow)*TwoPowerLODLevel)*TerrainInfo->Scale;
								float ZDist=(MidVcol-(SectorY*LODLevelInfo->SectorDetail+Vcol)*TwoPowerLODLevel)*TerrainInfo->Scale;
								BT_SetVertexHeight(Vrow,Vcol,BrushFunc(XDist,ZDist,MidHeight,Radius,Amount,CurrentHeight,CapHeight));
							}
						}
						BT_UnlockVertexData();
					}
				}
				
			//Increase LODLevel
				LODLevel++;
				if(LODLevel==TerrainInfo->LODLevels)
					Break=true;
			}while(Break==false);
		}
	}
}


//Currently, distance checks are done here. This will be changed in the future when I add an inner and outer box system.

static float BT_CircleBrush_Raise(float XDist,float ZDist,float MidHeight,float Radius,float Amount,float CurrentHeight,float CapHeight)
{
	float Dist=(float)sqrt(XDist*XDist+ZDist*ZDist);
	if(Dist<Radius)
	{
		// leelee - 190713 - cap height for finer editing control
		float fDegree = (float)((cos(float((Dist*1.57079633)/(Radius/2.0f)))+1.0f)/2.0f);
		float fNewHeight = (float)(CurrentHeight+fDegree*Amount);
		if ( CapHeight!=-1.0f && fNewHeight>CurrentHeight )
		{
			//if ( fNewHeight > CapHeight )
			//{
				fNewHeight = (CapHeight*fDegree) + (CurrentHeight*(1.0f-fDegree));
			//}
		}
		return fNewHeight;
	}else{
		return CurrentHeight;
	}
}

static float BT_CircleBrush_Flatten(float XDist,float ZDist,float MidHeight,float Radius,float Amount,float CurrentHeight,float CapHeight)
{
	float Dist=XDist*XDist+ZDist*ZDist;
	if(Dist<Radius*Radius)
	{
		return MidHeight;
	}else{
		return CurrentHeight;
	}
}

void BT_RaiseTerrain(unsigned long TerrainID,float X,float Z,float Radius,float Amount)
{
	//Raise the terrain
	BT_CircleBrush(TerrainID,X,Z,Radius,Amount,BT_CircleBrush_Raise,false,-1.0f);
}

void BT_RaiseTerrain(unsigned long TerrainID,float X,float Z,float Radius,float Amount,float capheight)
{
	//Raise the terrain
	BT_CircleBrush(TerrainID,X,Z,Radius,Amount,BT_CircleBrush_Raise,false,capheight);
}

void BT_FlattenTerrain(unsigned long TerrainID,float X,float Z,float Radius)
{
	//Flatten the terrain
	BT_CircleBrush(TerrainID,X,Z,Radius,0.0f,BT_CircleBrush_Flatten,true,-1.0f);
}

void BT_BlendTerrain(unsigned long TerrainID,float X,float Z,float Radius,float Amount,float reserved)
{
	// THIS DOES NOT WORK AS WE NEED TO CROSS SECTIONS FOR PERFECT BLENDING!!!

	// Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
		// Get terrain info
		BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)BT_GetTerrainInfo(TerrainID);

		// Check that the terrain is built and generated
		if(TerrainInfo->Built==true && TerrainInfo->Generated==true)
		{
			// Get mid height
			float MidHeight=0.0;
			unsigned long MidHeightDword = (unsigned long)BT_GetGroundHeight(TerrainID,X,Z);
			MidHeight=*(float*)&MidHeightDword;

			// Find sector(s) the brush is on
			float OuterLeft=max(X-Radius,0);
			float OuterRight=min(X+Radius,TerrainInfo->TerrainSize);
			float OuterTop=max(Z-Radius,0);
			float OuterBottom=min(Z+Radius,TerrainInfo->TerrainSize);

			// Loop through LODLevels
			bool Break=false;
			unsigned char LODLevel=0;
			unsigned short MidVrow=unsigned short((X/TerrainInfo->Scale));
			unsigned short MidVcol=unsigned short((Z/TerrainInfo->Scale));
			do
			{
				// Get LODLevel info
				BT_LODLevelInfo* LODLevelInfo=(BT_LODLevelInfo*)BT_GetLODLevelInfo(TerrainID,LODLevel);

				// Find the vertices the box is on
				unsigned short TwoPowerLODLevel=unsigned short(pow(2.0f,float(LODLevel)));
				unsigned short OuterTopV=unsigned short(floor((OuterTop/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterLeftV=unsigned short(floor((OuterLeft/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterBottomV=unsigned short(ceil((OuterBottom/TerrainInfo->Scale)/TwoPowerLODLevel));
				unsigned short OuterRightV=unsigned short(ceil((OuterRight/TerrainInfo->Scale)/TwoPowerLODLevel));

				// Find the sectors the box is on
				unsigned short TopS=unsigned short(OuterTopV/float(LODLevelInfo->SectorDetail));
				unsigned short LeftS=unsigned short(OuterLeftV/float(LODLevelInfo->SectorDetail));
				unsigned short BottomS=unsigned short(OuterBottomV/float(LODLevelInfo->SectorDetail));
				unsigned short RightS=unsigned short(OuterRightV/float(LODLevelInfo->SectorDetail));
				if(TopS*LODLevelInfo->SectorDetail==OuterTopV && TopS>0)
					TopS--;
				if(LeftS*LODLevelInfo->SectorDetail==OuterLeftV && LeftS>0)
					LeftS--;

				// Loop through sectors
				for(unsigned short SectorX=LeftS;SectorX<=RightS;SectorX++)
				{
					for(unsigned short SectorY=TopS;SectorY<=BottomS;SectorY++)
					{
						// Lock vertexdata for this sector
						BT_UnlockVertexData();
						BT_LockVertexdataForSector(TerrainID,LODLevel,SectorX*LODLevelInfo->Split+SectorY);

						// record old data
						int iOldHeightsSize = LODLevelInfo->SectorDetail + 1;
						float* pOldHeights = new float[iOldHeightsSize*iOldHeightsSize];
						for(unsigned short Vrow=0;Vrow<=LODLevelInfo->SectorDetail;Vrow++)
						{
							for(unsigned short Vcol=0;Vcol<=LODLevelInfo->SectorDetail;Vcol++)
							{
								unsigned long temp=BT_GetVertexPositionY(Vrow,Vcol);
								float CurrentHeight=*(float*)&temp;
								pOldHeights[Vrow+(Vcol*iOldHeightsSize)] = CurrentHeight;
							}
						}

						// Edit every vertex in sector, leave it to distance check to find which ones to do
						for(unsigned short Vrow=0;Vrow<=LODLevelInfo->SectorDetail;Vrow++)
						{
							for(unsigned short Vcol=0;Vcol<=LODLevelInfo->SectorDetail;Vcol++)
							{
								// get old Y pos
								float CurrentHeight = pOldHeights[Vrow+(Vcol*iOldHeightsSize)];
								float XDist=(MidVrow-(SectorX*LODLevelInfo->SectorDetail+Vrow)*TwoPowerLODLevel)*TerrainInfo->Scale;
								float ZDist=(MidVcol-(SectorY*LODLevelInfo->SectorDetail+Vcol)*TwoPowerLODLevel)*TerrainInfo->Scale;

								// work out Y pos
								float fNewYValue = 0.0f;
								float Dist=(float)sqrt(XDist*XDist+ZDist*ZDist);
								if(Dist<Radius)
								{
									int iBlCound = 0;
									float blCurrentHeight = 0.0f;
									for ( int blRow=Vrow-1; blRow<=Vrow+1; blRow++ )
									{
										for ( int blCol=Vcol-1; blCol<=Vcol+1; blCol++ )
										{
											int blthisR = blRow;
											int blthisC = blCol;
											if ( blthisR<0 ) blthisR = 0;
											if ( blthisR>LODLevelInfo->SectorDetail ) blthisR = LODLevelInfo->SectorDetail;
											if ( blthisC<0 ) blthisC = 0;
											if ( blthisC>LODLevelInfo->SectorDetail ) blthisC = LODLevelInfo->SectorDetail;
											float fThisHeight = pOldHeights[blthisR+(blthisC*iOldHeightsSize)];
											blCurrentHeight = blCurrentHeight + fThisHeight;
											iBlCound++;
										}
									}
									if ( iBlCound==9 )
									{
										blCurrentHeight = blCurrentHeight /  iBlCound;
										fNewYValue = (blCurrentHeight*Amount) + (CurrentHeight*(1.0f-Amount));
									}
									else
										fNewYValue = CurrentHeight;
								}
								else
									fNewYValue = CurrentHeight;

								// assign new Y pos
								BT_SetVertexHeight(Vrow,Vcol,fNewYValue);
							}
						}

						// free usages
						if ( pOldHeights )
						{
							delete pOldHeights;
							pOldHeights = NULL;
						}

						// Unlock vertex data
						BT_UnlockVertexData();
					}
				}
				
				// Increase LODLevel
				LODLevel++;
				if(LODLevel==TerrainInfo->LODLevels)
					Break=true;
			}
			while(Break==false);
		}
	}
}
