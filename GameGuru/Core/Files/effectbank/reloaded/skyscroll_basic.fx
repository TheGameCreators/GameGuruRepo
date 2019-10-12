string Description = "Sky Scroll Shader";
#include "settings.fx"

#include "constantbuffers.fx"
//float4x4 World : World;
//float4x4 View : View;
//float4x4 Projection : Projection;

/**************MATRICES & UNTWEAKABLES *****************************************************/

// standard constants
float4x4 WorldInverse : WorldInverse;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 ViewInverse : ViewInverse;
float4x4 ViewIT : ViewInverseTranspose;
float4x4 ViewProjection : ViewProjection;

float4 eyePos : CameraPosition;
float time : Time;
float sintime : SinTime;
float alphaoverride  : alphaoverride;
float4 ScrollScaleUV;
float SpecularOverride;

/**************VALUES PROVIDED FROM FPSC - NON TWEAKABLE**************************************/

float4 clipPlane : ClipPlane;  //cliplane for water plane

//HUD Fog Color
float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

//HUD Fog Distances (near,far,0,0)
float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 AmbiColorOverride
<    string UIName =  "AmbiColorOverride";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 AmbiColor : Ambient
<    string UIName =  "AmbiColor";    
> = {0.1f, 0.1f, 0.1f, 1.0f};

float4 SurfColor : Diffuse
<    string UIName =  "SurfColor";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 SkyScrollValues : Diffuse
<    string UIName =  "SkyScrollValues";    
> = {0.0f, 0.0f, 0.0f, 0.0f};

/***************TEXTURES AND SAMPLERS***************************************************/

Texture2D DiffuseMap : register( t0 );
Texture2D PortalMap : register( t1 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};
SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

/************* DATA STRUCTS **************/

struct appdata 
{
    float3 Position	: POSITION;
    float2 UV0		: TEXCOORD0;
    float2 UV1		: TEXCOORD1;	
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 TexCoord0    : TEXCOORD0;
    float2 TexCoord1    : TEXCOORD1;
    float clip          : TEXCOORD2;
	float4 WPos			: TEXCOORD3;
};

/*******Main Vertex Shader***************************/

vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.Position = mul(float4(IN.Position,1), WorldViewProjection);
    OUT.TexCoord0  = IN.UV0; 
    OUT.TexCoord1  = IN.UV1; 
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
	OUT.WPos = worldSpacePos;
    return OUT;
}

/****************Framgent Shader*****************/

float4 mainPS(vertexOutput IN) : COLOR
{
    float4 finalcolor;
	
	// advanced portal effect (alpha is static mask to fade out extreme edges)
    float portalalpha = PortalMap.Sample(SampleClamp,IN.TexCoord0.xy).a;
	
	// legacy effect with no cloud portal
	finalcolor = DiffuseMap.Sample(SampleWrap,IN.TexCoord0.xy+float2(SkyScrollValues.x,SkyScrollValues.y));
	
	// SK: Adding in fog
	float3 eyePosFlat = eyePos.xyz; eyePosFlat.y *= 0.1f;
	float3 wPosFlat = IN.WPos.xyz; wPosFlat.y *= 0.1f;
    float thedistance = distance(eyePosFlat,wPosFlat) / 500.0f; // Skybox smaller than landscape size
    float hudfogfactor = saturate((thedistance - HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float3 hudfogresult = lerp(finalcolor.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);
	
	// cannot trust alpha of cloud scroll texture, so calc from RGB
	float fCloudAlpha = finalcolor.a;

	// final scrolling cloud pixel
    return float4(hudfogresult,fCloudAlpha*portalalpha);
}

/****** technique *****************************************************************************/

technique11 Main
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}
