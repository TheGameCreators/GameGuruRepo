#include "LMGlobal.h"
#include "directx-macros.h"
#include <math.h>
#include "LMPolyGroup.h"
#include "Lumel.h"

LumelLite::LumelLite( )
{
	colr = 0;
	colg = 0;
	colb = 0;
}

LumelLite::~LumelLite( )
{
}

float LumelLite::GetColR( ) { return ((float)colr)/255.0f; }
float LumelLite::GetColG( ) { return ((float)colg)/255.0f; }
float LumelLite::GetColB( ) { return ((float)colb)/255.0f; }

int LumelLite::GetFinalColR( ) 
{ 
	return colr;
}

int LumelLite::GetFinalColG( )
{ 
	return colg;
}

int LumelLite::GetFinalColB( )
{ 
	return colb;
}

DWORD LumelLite::GetColor( )
{
	return 0xff000000 | ( colr << 16 ) | ( colg << 8 ) | ( colb );
}

DWORD LumelLite::GetReversedColor( )
{
	return 0xff000000 | ( colb << 16 ) | ( colg << 8 ) | ( colr );
}

/*
LumelLite::LumelLite( )
{
	//posX = 0; posY = 0; posZ = 0;
	colR = 0; colG = 0; colB = 0;
	//pRefPoly = 0;

	//fShadow = 0;
}

LumelLite::~LumelLite( )
{
	
}

float LumelLite::GetColR( ) { return colR; }
float LumelLite::GetColG( ) { return colG; }
float LumelLite::GetColB( ) { return colB; }

int LumelLite::GetFinalColR( ) 
{ 
	int iValue = FtoI(colR*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

int LumelLite::GetFinalColG( )
{ 
	int iValue = FtoI(colG*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

int LumelLite::GetFinalColB( )
{ 
	int iValue = FtoI(colB*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

DWORD LumelLite::GetColor( )
{
	int red = FtoI(colR*255);
	if ( red > 255 ) red = 255;
	if ( red < 0 ) red = 0;
	
	int green = FtoI(colG*255);
	if ( green > 255 ) green = 255;
	if ( green < 0 ) green = 0;

	int blue = FtoI(colB*255);
	if ( blue > 255 ) blue = 255;
	if ( blue < 0 ) blue = 0;

	return 0xff000000 | ( red << 16 ) | ( green << 8 ) | ( blue );
}
*/

// LUMEL

Lumel::Lumel( )
{
	//posX = 0; posY = 0; posZ = 0;
	colR = 0; colG = 0; colB = 0;
	//pRefPoly = 0;

	//fShadow = 0;
}

Lumel::~Lumel( )
{
	
}

//float Lumel::GetPosX( ) { return posX; }
//float Lumel::GetPosY( ) { return posY; }
//float Lumel::GetPosZ( ) { return posZ; }

//GGVECTOR3 Lumel::GetPos( ) 
//{ 
//	return GGVECTOR3( posX, posY, posZ ); 
//}

float Lumel::GetColR( ) { return colR; }
float Lumel::GetColG( ) { return colG; }
float Lumel::GetColB( ) { return colB; }

int Lumel::GetFinalColR( ) 
{ 
	int iValue = FtoI(colR*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

int Lumel::GetFinalColG( )
{ 
	int iValue = FtoI(colG*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

int Lumel::GetFinalColB( )
{ 
	int iValue = FtoI(colB*255);
	if ( iValue > 255 ) iValue = 255;
	if ( iValue < 0 ) iValue = 0;

	return iValue;
}

DWORD Lumel::GetColor( )
{
	int red = FtoI(colR*255);
	if ( red > 255 ) red = 255;
	if ( red < 0 ) red = 0;
	
	int green = FtoI(colG*255);
	if ( green > 255 ) green = 255;
	if ( green < 0 ) green = 0;

	int blue = FtoI(colB*255);
	if ( blue > 255 ) blue = 255;
	if ( blue < 0 ) blue = 0;

	return 0xff000000 | ( red << 16 ) | ( green << 8 ) | ( blue );
}
