string Description = "Shadow Shader";
#include "constantbuffers.fx"
#include "settings.fx"                  

// shadow mapping
matrix          m_mShadow;
float4          m_vCascadeOffset[8];
float4          m_vCascadeScale[8];
float           m_fCascadeFrustumsEyeSpaceDepths[8];
float ShadowStrength   
<    string UIName =  "ShadowStrength";    
> = {1.0f};

// standard constants
float4x4 WorldInverse : WorldInverse;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 ViewInverse : ViewInverse;
float4x4 ViewIT : ViewInverseTranspose;
float4x4 ViewProjection : ViewProjection;
float4 clipPlane : ClipPlane;  //cliplane for water plane

float4 FogColor : Diffuse
<   string UIName =  "Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

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

float GlobalSurfaceIntensity
<    string UIName =  "GlobalSurfaceIntensity";    
> = {1.0f};

float alphaoverride  : alphaoverride;

float4 SkyColor;
float4 FloorColor;
float4 LightSource;
float4 EntityEffectControl;
float SurfaceSunFactor;
float GlobalSpecular;
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
SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct appdata 
{
    float3 Position    : POSITION;
    float2 UV0         : TEXCOORD0;
};

struct vertexOutput
{
    float4 Position   : POSITION;
    float2 TexCoord   : TEXCOORD0;
    float4 Pos        : TEXCOORD1;
    float vDepth      : TEXCOORD2;
    float4 WPos       : TEXCOORD3;
};

vertexOutput mainVS(appdata IN)   
{
   vertexOutput OUT;
   float4 tPos = mul(float4(IN.Position,1), WorldViewProjection);
   OUT.Position = tPos;
   OUT.TexCoord = IN.UV0;
   OUT.Pos = float4(IN.Position,1);
   OUT.vDepth = tPos.z; 
   float4 worldSpacePos = mul(float4(IN.Position,1), World);
   OUT.WPos = worldSpacePos; 
   return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
   // dynamic shadow mapping
   float4 vSMapTexCoord;
   float2 offset = 0;
   float fDynamicShadow = 0;
   float fFarShadow = 0.0;
   float fTS = 1.0f / 1024.0f;
   float4 result = float4(DiffuseMap.SampleLevel(SampleWrap,IN.TexCoord.xy,0).xyz/3.0f,1);

   // multiply lightmap directly with 'frame buffer pixel thanks to destblend=srccolor'
   result = result * 2.0f;
   result.x = max(min(result.x,0.2f),result.x-fDynamicShadow);
   result.y = max(min(result.y,0.2f),result.y-fDynamicShadow);
   result.z = max(min(result.z,0.2f),result.z-fDynamicShadow);

   // fog diminishes shadow strength
   float finalshadowstrength = max((ShadowStrength+0.45) - AmbiColorOverride.x,0);
   float4 cameraPos = mul(IN.WPos, View);
   float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   finalshadowstrength = finalshadowstrength - (hudfogfactor*HudFogColor.w);

   // 291115 - diminish dynamic shadow based on ambient intensity contribution - required?
   finalshadowstrength = min(finalshadowstrength * (GlobalSurfaceIntensity*3.0f),finalshadowstrength);

   // PE: Fog and low ShadowStrength can make it go minus. clamp.
   finalshadowstrength = clamp(finalshadowstrength,0.0,0.825);

   // apply strength of shadow/lm
   result = lerp(float4(0.5,0.5,0.5,1),result,finalshadowstrength);

   // done   
   return result;
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

technique11 High
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

technique11 DepthMap
{
    pass MainPass
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
    }
}

technique11 DepthMapNoAnim
{
    pass MainPass
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
    }
}
