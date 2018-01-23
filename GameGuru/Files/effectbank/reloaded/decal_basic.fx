string Description = "Smoke and Particle Shader";
#include "constantbuffers.fx"
#include "settings.fx"   

float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float alphaoverride  : alphaoverride;
float4 clipPlane : ClipPlane;

float4 FogColor : Diffuse
<   string UIName =  "Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 UVScaling : uvscaling
<   string UIName =  "UV Scaling";    
> = {0, 0, 0, 0};

// Global constants passed in
float4 AmbiColorOverride;
float4 AmbiColor;
float4 SurfColor;
float4 SkyColor;
float ShadowStrength;
float4 LightSource;
float4 FloorColor;
float4 EntityEffectControl;
float SurfaceSunFactor;
float GlobalSpecular;
float GlobalSurfaceIntensity;
float4 SpotFlashPos;
float4 SpotFlashColor;
float4 g_lights_data;
float4 g_lights_pos0;
float4 g_lights_pos1;
float4 g_lights_pos2;
float4 g_lights_atten0;
float4 g_lights_atten1;
float4 g_lights_atten2;
float4 g_lights_diffuse0;
float4 g_lights_diffuse1;
float4 g_lights_diffuse2;

Texture2D DiffuseMap : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct appdata 
{
    float3 Position	: POSITION0;
    float3 Normal   : NORMAL0;
    float2 UV       : TEXCOORD0;
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 atlasUV      : TEXCOORD1; 
    float  WaterFog     : TEXCOORD2; 
    float4 WPos         : TEXCOORD3;
    float  clip         : TEXCOORD4;   
};

vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.Position = mul(float4(IN.Position,1), WorldViewProjection);
 	OUT.atlasUV = IN.UV + UVScaling.xy;
    float4 cameraPos = mul( worldSpacePos, View );
    float fogstrength = cameraPos.z * FogColor.w;
    OUT.WaterFog = min(fogstrength,1.0);
    OUT.clip = dot(worldSpacePos, clipPlane);    
    OUT.WPos = worldSpacePos;   
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
    float4 finalcolor;
    clip(IN.clip);
	    
    float4 diffusemap = DiffuseMap.SampleLevel(SampleWrap,IN.atlasUV,0);	
    float alpha = diffusemap.a * alphaoverride;    
    float4 result =  diffusemap;
    
    // calculate hud (scene) pixel-fog
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    
    // mix in HUD (scene) Fog with final color;
    float4 hudfogresult = lerp(result,HudFogColor,hudfogfactor);
    
    // and Finally add in any Water Fog   
    float4 waterfogresult = lerp(hudfogresult,FogColor,IN.WaterFog);   
    finalcolor=float4(waterfogresult.xyz,alpha);  
	
    return finalcolor;
}

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}

technique11 Medium
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}
//PE: GameGuru Editor crash ? When DepthMap is in the decal shaders ?
//Not sure what the real reason for this crash is.(thanks cybernescence)
//technique11 DepthMap
//{
//    pass MainPass
//    {
//        SetVertexShader(NULL);
//        SetPixelShader(NULL);
//        SetGeometryShader(NULL);
//    }
//}
//
//technique11 DepthMapNoAnim
//{
//    pass MainPass
//    {
//        SetVertexShader(NULL);
//        SetPixelShader(NULL);
//        SetGeometryShader(NULL);
//    }
//}
