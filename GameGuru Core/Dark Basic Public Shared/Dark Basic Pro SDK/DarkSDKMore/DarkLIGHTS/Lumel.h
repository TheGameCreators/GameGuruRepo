#ifndef LUMEL_H
#define LUMEL_H

int FtoI(float f);

//class GGVECTOR3;
class LMPolyGroup;

// Newer lightweight Lumel (to reduce memory requirement of lightmapper)
class LumelLite
{
private:
	unsigned char colr;
	unsigned char colg;
	unsigned char colb;

public:
	LumelLite( );
	~LumelLite( );
	float GetColR( );
	float GetColG( );
	float GetColB( );
	int GetFinalColR( );
	int GetFinalColG( );
	int GetFinalColB( );
	DWORD GetColor( );
	DWORD GetReversedColor( );
	void SetCol( float red, float green, float blue );
	void AddCol( float red, float green, float blue );
	void AddColR( float red );
	void AddColG( float green );
	void AddColB( float blue );
	void Ambient( float red, float green, float blue );
};

inline void LumelLite::SetCol( float red, float green, float blue )
{
	int tmp = FtoI(red*255);
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colr = tmp;

	tmp = FtoI(green*255);
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colg = tmp;

	tmp = FtoI(blue*255);
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colb = tmp;
}

inline void LumelLite::AddCol( float red, float green, float blue )
{
	AddColR( red );	
	AddColG( green );	
	AddColB( blue );	
}

inline void LumelLite::AddColR( float red ) 
{
	int tmp = FtoI(red*255);
	tmp=tmp+colr;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colr = tmp;
}

inline void LumelLite::AddColG( float green )
{
	int tmp = FtoI(green*255);
	tmp=tmp+colg;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colg = tmp;
}

inline void LumelLite::AddColB( float blue )
{
	int tmp = FtoI(blue*255);
	tmp=tmp+colb;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colb = tmp;
}

inline void LumelLite::Ambient( float red, float green, float blue )
{
	//ambient light to fake radiosity
	int tmp = FtoI(red*255);
	tmp=tmp+colr;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colr = tmp;

	tmp = FtoI(green*255);
	tmp=tmp+colg;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colg = tmp;

	tmp = FtoI(blue*255);
	tmp=tmp+colb;
	if ( tmp > 255 ) tmp = 255;
	if ( tmp < 0 ) tmp = 0;
	colb = tmp;
}

/*
class LumelLite
{

private:

	float colR, colG, colB;

public:

	LumelLite( );
	~LumelLite( );

	float GetColR( );
	float GetColG( );
	float GetColB( );

	int GetFinalColR( );
	int GetFinalColG( );
	int GetFinalColB( );

	DWORD GetColor( );

	void SetCol( float red, float green, float blue );
	void AddCol( float red, float green, float blue );
	void AddColR( float red );
	void AddColG( float green );
	void AddColB( float blue );

	void Ambient( float red, float green, float blue );
};

inline void LumelLite::SetCol( float red, float green, float blue )
{
	colR = red;	
	colG = green;
	colB = blue;
}

inline void LumelLite::AddCol( float red, float green, float blue )
{
	AddColR( red );	
	AddColG( green );
	AddColB( blue );
}

inline void LumelLite::AddColR( float red ) 
{
	colR += red;
}

inline void LumelLite::AddColG( float green )
{
	colG += green;
}

inline void LumelLite::AddColB( float blue )
{
	colB += blue;
}

inline void LumelLite::Ambient( float red, float green, float blue )
{
	//ambient light to fake radiosity
	//adding ambient
	colR += red;
	colG += green;
	colB += blue;
}
*/

//LUMEL

class Lumel
{

private:

	//float posX, posY, posZ;
	float colR, colG, colB;

public:

	//float fShadow;
	//LMPolyGroup *pRefPoly;

	Lumel( );
	~Lumel( );

	//float GetPosX( );
	//float GetPosY( );
	//float GetPosZ( );
	//GGVECTOR3 GetPos( );

	float GetColR( );
	float GetColG( );
	float GetColB( );

	int GetFinalColR( );
	int GetFinalColG( );
	int GetFinalColB( );

	DWORD GetColor( );

	void SetCol( float red, float green, float blue );
	void AddCol( float red, float green, float blue );
	void AddColR( float red );
	void AddColG( float green );
	void AddColB( float blue );

	void Ambient( float red, float green, float blue );
};

inline void Lumel::SetCol( float red, float green, float blue )
{
	colR = red;	
	colG = green;
	colB = blue;
}

inline void Lumel::AddCol( float red, float green, float blue )
{
	AddColR( red );	
	AddColG( green );
	AddColB( blue );
}

inline void Lumel::AddColR( float red ) 
{
	//if ( red < 0 ) red = 0;
	colR += red;
}

inline void Lumel::AddColG( float green )
{
	//if ( green < 0 ) green = 0;
	colG += green;
}

inline void Lumel::AddColB( float blue )
{
	//if ( blue < 0 ) blue = 0;
	colB += blue;
}

inline void Lumel::Ambient( float red, float green, float blue )
{
	//ambient light to fake radiosity

	//limiting ambient
	//if ( colR < red ) colR = red;
	//if ( colG < green ) colG = green;
	//if ( colB < blue ) colB = blue;

	//adding ambient
	colR += red;
	colG += green;
	colB += blue;
}

#endif