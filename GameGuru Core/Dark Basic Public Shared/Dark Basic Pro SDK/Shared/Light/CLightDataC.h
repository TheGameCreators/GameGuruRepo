#ifndef _CLIGHTDATA_H_
#define _CLIGHTDATA_H_

enum eLightType
{
    eDirectionalLight = 1,
    ePointLight = 2,
    eSpotLight = 3,

    // Original light type was int, force this enum to the same size
    eLightType_Force_DWORD = 0x7FFFFFFF,
};

struct tagLightColorValue
{
    float r;
    float g;
    float b;
    float a;
};

struct tagLightVector
{
    float x;
    float y;
    float z;
};

struct tagLight
{
    DWORD				Type;             /* Type of light source */
    tagLightColorValue  Diffuse;          /* Diffuse color of light */
    tagLightColorValue  Specular;         /* Specular color of light */
    tagLightColorValue  Ambient;          /* Ambient color of light */
    tagLightVector		Position;         /* Position in world space */
    tagLightVector		Direction;        /* Direction in world space */
    float				Range;            /* Cutoff range */
    float				Falloff;          /* Falloff */
    float				Attenuation0;     /* Constant attenuation */
    float				Attenuation1;     /* Linear attenuation */
    float				Attenuation2;     /* Quadratic attenuation */
    float				Theta;            /* Inner angle of spotlight cone */
    float				Phi;              /* Outer angle of spotlight cone */
};

struct tagLightData
{
	tagLight	light;

	eLightType	eType;
	bool		bEnable;
	float		fRange;
	DWORD		dwColor;
};

#endif _CLIGHTDATA_H_
