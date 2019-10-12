#include "settings.fx"
#include "constantbuffers.fx"

// standard constants
float alphaoverride : alphaoverride;
float time : time;
float4 clipPlane : ClipPlane;
float4 ScrollScaleUV;
float SpecularOverride;

Texture2D DiffuseMap : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct appdata
{
    float3 Position : POSITION;
    float3 Normal	: NORMAL;
    float2 UV		: TEXCOORD0;    
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
	#ifndef SKYSCROLL
    float clip          : TEXCOORD1;
    float4 ObjPos       : TEXCOORD2;
	#endif
};
      
#ifdef SKYSCROLL

float directionX
<
   string UIWidget = "slider";
   float UIMax = 3.0;
   float UIMin = -3.0;
   float UIStep = 0.01;
> = 0.003;

float directionY
<
   string UIWidget = "slider";
   float UIMax = 3.0;
   float UIMin = -3.0;
   float UIStep = 0.01;
> = -0.000;

#else

float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};
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

#endif

vertexOutput mainVS(appdata IN)   
{                   
    vertexOutput OUT;
	#ifdef SKYSCROLL
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);   
    OUT.TexCoord = IN.UV + float2(directionX,directionY)*time;
	#else
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
    OUT.ObjPos = float4(IN.Position,1);
    OUT.TexCoord = IN.UV; 
	#endif
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	#ifdef SKYSCROLL
    return DiffuseMap.Sample(SampleWrap,IN.TexCoord);
	#else
    float4 result;
    result = float4(DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy).xyz,alphaoverride);
    float distantfogfactor = 1.0 - (HudFogDist.y / 50000.0f);
    distantfogfactor = clamp(distantfogfactor,0.0,1.0);
    #ifndef ADDSKYBOXFOG
#ifdef ROUNDEDSKYBOXFOG
	 //PE: let box look like its round.
     float hudfogfactor = (1 - saturate( (IN.ObjPos.y / 1.5) * cos( length(IN.ObjPos)/8.0 ) )) * distantfogfactor;
#else
     float hudfogfactor = (1 - saturate(IN.ObjPos.y / 1.5)) * distantfogfactor;
#endif
    #else
     float hudfogfactor = distantfogfactor;
    #endif
    hudfogfactor = clamp(hudfogfactor*HudFogColor.w*distantfogfactor,0.0,1.0);
    result = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor); //*HudFogColor.w
    return float4(result.xyz,1);
	#endif
}

technique11 Main
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}
