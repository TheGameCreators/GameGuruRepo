//----------------------------------------------------
//--- GAMEGURU - M-OBS
//----------------------------------------------------

#include "gameguru.h"

// 
//  OBS Obstacle Generation Module
// 

void obs_memorychunkinit ( void )
{
	//  Allocate this chunk once at very start to avoid fragmentation issues
	Dim2(  t.terrainmark,terrain_chunk_size, terrain_chunk_size  );
	Dim2(  t.terraindot,terrain_chunk_size, terrain_chunk_size  );
}

void obs_fillterraindot ( void )
{

	//  Reset obstacle builder
	g.obsindex=0 ; g.obsmax=10;
	Dim (  t.obs,g.obsmax );

	//  generate terrain Dot (  array data )
	for ( t.z = 0 ; t.z<=  terrain_chunk_size; t.z++ )
	{
		for ( t.x = 0 ; t.x<=  terrain_chunk_size; t.x++ )
		{
			t.terraindot[t.x][t.z]=0;
		}
	}
	for ( t.z = 10 ; t.z<=  terrain_chunk_size-10; t.z++ )
	{
		for ( t.x = 10 ; t.x<=  terrain_chunk_size-10; t.x++ )
		{
			if (  t.terrainmatrix[t.x][t.z]<t.terrain.waterliney_f ) 
			{
				//  water body
				t.terraindot[t.x][t.z]=1;
			}
			else
			{
				//  detect hill slopes too steep for AI to climb
				//  too many strange conbinations creating dead zones and incorrect waypoint node selections!!
//     `h#=terrainmatrix(x,z)

//     `h1#=terrainmatrix(x-1,z-1)

//     `h2#=terrainmatrix(x,z-1)

//     `h3#=terrainmatrix(x+1,z-1)

//     `h4#=terrainmatrix(x-1,z)

//     `h6#=terrainmatrix(x+1,z)

//     `h7#=terrainmatrix(x-1,z+1)

//     `h8#=terrainmatrix(x,z+1)

//     `h9#=terrainmatrix(x+1,z+1)

//     `tstepval#=-20.0

//     `if h#>h1#-tstepval# or h#>h2#-tstepval# or h#>h3#-tstepval# or h#>h4#-tstepval# or h#>h6#-tstepval# or h#>h7#-tstepval# or h#>h8#-tstepval# or h#>h9#-tstepval#

				//terraindot(x,z)=1
//     `else

					t.terraindot[t.x][t.z]=0;
//     `endif

			}
		}
	}

return;

}

void obs_freeterraindot ( void )
{
	//  free resources used
	UnDim (  t.obs );
return;

}

void ode_doterraindotwork ( void )
{

	//  Trace using MOORE-NEIGHBORHOOD technique
	if (  t.mooreneighborhood.mode == 0 ) 
	{
		//  find entry GetPoint (  )
		t.tfound=0;
		for ( t.z = 0 ; t.z<=  terrain_chunk_size; t.z++ )
		{
			for ( t.x = 0 ; t.x<=  terrain_chunk_size; t.x++ )
			{
				if (  t.terraindot[t.x][t.z] == 1 ) 
				{
					t.mooreneighborhood.mode=1;
					t.mooreneighborhood.startx=t.x;
					t.mooreneighborhood.startz=t.z;
					t.mooreneighborhood.startd=1;
					t.mooreneighborhood.currentx=t.x;
					t.mooreneighborhood.currentz=t.z;
					t.mooreneighborhood.currentd=1;
					t.terrainmark[t.mooreneighborhood.currentx][t.mooreneighborhood.currentz]=1;
					t.obsx_f=t.x ; t.obsz_f=t.z ; obs_add ( );
					t.mooreneighborhood.countneighborscan=0;
					t.mooreneighborhood.countlaps=0;
					t.mooreneighborhood.laststretchlen_f=0;
					t.tfound=1 ; t.x=terrain_chunk_size ; t.z=terrain_chunk_size ; break;
				}
			}
		}
		if (  t.tfound == 0 ) 
		{
			t.mooreneighborhood.mode=99;
		}
	}
	if (  t.mooreneighborhood.mode == 1 ) 
	{
		//  backtrack
		switch (  t.mooreneighborhood.currentd ) 
		{
		case 1 : --t.mooreneighborhood.currentx ; break; 
		case 2 : --t.mooreneighborhood.currentz ; break;
		case 3 : ++t.mooreneighborhood.currentx ; break;
		case 4 : ++t.mooreneighborhood.currentz ; break;
		}		//~   mooreneighborhood.mode=2
		t.mooreneighborhood.mode=2;
	}
	if (  t.mooreneighborhood.mode == 2 ) 
	{
		//  follow clockwise - turn left 90 degrees
		--t.mooreneighborhood.currentd;
		if (  t.mooreneighborhood.currentd == 0  )  t.mooreneighborhood.currentd = 4;
		t.mooreneighborhood.currentclockwise=0;
		t.mooreneighborhood.mode=3;
	}
	if (  t.mooreneighborhood.mode == 3 ) 
	{
		//  follow clockwise - advance or turn
		//  1-advance
		//  2-turn right
		//  3-advance
		//  4-advance
		//  5-turn right
		//  6-advance
		//  7-advance
		//  8-turn right
		//  9-advance
		//  10-advance
		//  11-turn right
		//  12-advance
		//  13-STOP
		++t.mooreneighborhood.currentclockwise;
		t.tccv=t.mooreneighborhood.currentclockwise;
		if (  t.tccv == 2 || t.tccv == 5 || t.tccv == 8 || t.tccv == 11 ) 
		{
			//  turn
			++t.mooreneighborhood.currentd;
			if (  t.mooreneighborhood.currentd == 5  )  t.mooreneighborhood.currentd = 1;
		}
		else
		{
			//  advance
			switch (  t.mooreneighborhood.currentd ) 
			{
			case 1 : ++t.mooreneighborhood.currentx ; break ;
			case 2 : ++t.mooreneighborhood.currentz ; break ;
			case 3 : --t.mooreneighborhood.currentx ; break ;
			case 4 : --t.mooreneighborhood.currentz ; break ;
			}		//~   endif
		}
		if (  t.tccv == 13 ) 
		{
			++t.mooreneighborhood.countneighborscan;
			if (  t.mooreneighborhood.countneighborscan>5 ) 
			{
				//  destroy start Dot (  if goes no-where )
				t.terrainmark[t.mooreneighborhood.startx][t.mooreneighborhood.startz]=2;
				t.terraindot[t.mooreneighborhood.startx][t.mooreneighborhood.startz]=0;
			}
		}
		//  when hit black square
		t.tokay=0;
		if (  t.mooreneighborhood.currentx >= 0 && t.mooreneighborhood.currentz>= 0 ) 
		{
			if (  t.terraindot[t.mooreneighborhood.currentx][t.mooreneighborhood.currentz] == 1 ) 
			{
				t.tokay=1;
			}
			if (  abs(t.mooreneighborhood.startx-t.mooreneighborhood.currentx)>50 ) 
			{
				t.tokay=0;
			}
			if (  abs(t.mooreneighborhood.startz-t.mooreneighborhood.currentz)>50 ) 
			{
				t.tokay=0;
			}
		}
		if (  t.tokay == 1 ) 
		{
			//  and when hit stop condition
			if (  t.mooreneighborhood.currentx == t.mooreneighborhood.startx && t.mooreneighborhood.currentz == t.mooreneighborhood.startz ) 
			{
				//  back at start position and entering from same direction
				++t.mooreneighborhood.countlaps;
				if (  t.mooreneighborhood.currentd == t.mooreneighborhood.startd || t.mooreneighborhood.countlaps >= 1 ) 
				{
					t.tccv=13;
				}
			}
			if (  t.tccv<13 ) 
			{
				//  mark as part of border
				t.terrainmark[t.mooreneighborhood.currentx][t.mooreneighborhood.currentz]=1;
				t.obsx_f=t.mooreneighborhood.currentx ; t.obsz_f=t.mooreneighborhood.currentz ; obs_add ( );
				t.mooreneighborhood.mode=1;
			}
		}
		if (  t.tccv >= 13 ) 
		{
			t.mooreneighborhood.mode=5;
		}
	}
	if (  t.mooreneighborhood.mode == 5 ) 
	{
		//  stop condition met
		t.mooreneighborhood.mode=0;
		//  add termionator to obj list
		obs_finishedone ( );
		//  extract from Dot (  map so can work on other shapes )
		for ( t.z = 0 ; t.z<=  terrain_chunk_size; t.z++ )
		{
			for ( t.x = 0 ; t.x<=  terrain_chunk_size; t.x++ )
			{
				if (  t.terrainmark[t.x][t.z] == 1 ) 
				{
					//  For each mark, flood fill any pool data to the right of this mark
					t.terraindot[t.x][t.z]=0;
					t.terrainmark[t.x][t.z]=2;
					t.thitfill=0;
					for ( t.floodx = t.x+1 ; t.floodx<=  terrain_chunk_size-1; t.floodx++ )
					{
						if (  t.terrainmark[t.floodx][t.z] != 1 ) 
						{
							t.tValid=0;
							for ( t.scanaheadx = t.floodx+1 ; t.scanaheadx<=  terrain_chunk_size-1; t.scanaheadx++ )
							{
								if (  t.terrainmark[t.scanaheadx][t.z] == 1  )  t.tValid = 1;
							}
							if (  t.tValid == 1 ) 
							{
								t.thitfill=1;
							}
							else
							{
								break;
							}
						}
						if (  t.thitfill == 1 && t.terrainmark[t.floodx][t.z] == 1  )  break;
						t.terraindot[t.floodx][t.z]=0;
						t.terrainmark[t.floodx][t.z]=2;
					}
					t.x=t.floodx;
					t.terraindot[t.x][t.z]=0;
					t.terrainmark[t.x][t.z]=2;
				}
			}
		}

	}
	if (  t.mooreneighborhood.mode == 99 ) 
	{
		//  finished entire scan, we have our bounds!
		obs_finishedone ( );
	}

return;

}

void obs_expand ( void )
{
	if (  g.obsindex+10>g.obsmax ) 
	{
		g.obsmax=g.obsmax+50;
		Dim (  t.obs,g.obsmax );
	}
return;

}

void obs_add ( void )
{

	//  expand if too small
	obs_expand ( );

	//  can we skip this raw data?
	t.tstretchit=0;
	if (  g.obsindex >= 1 ) 
	{

		//  also re-use if resolution suggests they are too close together
		if (  t.tstretchit == 0 ) 
		{
			if (  t.obs[g.obsindex-1].x != -1 ) 
			{
				t.tobsdx_f=t.obs[g.obsindex-1].x-t.obsx_f;
				t.tobsdz_f=t.obs[g.obsindex-1].z-t.obsz_f;
				t.tobsdd_f=Sqrt(abs(t.tobsdx_f*t.tobsdx_f)+abs(t.tobsdz_f*t.tobsdz_f));
				if (  t.tobsdd_f<5.0 ) 
				{
					if (  t.tobsdd_f >= t.mooreneighborhood.laststretchlen_f ) 
					{
						t.mooreneighborhood.laststretchlen_f=t.tobsdd_f;
						t.tstretchit=1;
					}
				}
			}
		}

	}

	//  add obstacle coordinate
	if (  t.tstretchit == 0 ) 
	{
		t.mooreneighborhood.laststretchlen_f=0;
		++g.obsindex;
	}
	else
	{
		//  can re-use this slot as we're simply stretching the coordinate
	}
	t.obs[g.obsindex].x=t.obsx_f;
	t.obs[g.obsindex].z=t.obsz_f;

return;

}

void obs_finishedone ( void )
{

	//  expand if too small
	obs_expand ( );

	//  add obstacle terminator
	if (  t.obs[g.obsindex].x != -1 ) 
	{
		++g.obsindex;
		t.obs[g.obsindex].x=-1;
		t.obs[g.obsindex].z=-1;
	}

return;
}
