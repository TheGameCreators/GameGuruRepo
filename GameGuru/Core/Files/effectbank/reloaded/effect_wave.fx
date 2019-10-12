string Description = "Effect Wave Scroll Shader";
#include "constantbuffers.fx"
#include "settings.fx"   
#include "cascadeshadows.fx"   

float4x4 WorldViewProjection : WorldViewProjection;
float4x4 WorldIT : WorldInverseTranspose;
float time : time;
float4 eyePos : CameraPosition;

float directionX
<
	string UIWidget = "slider";
	float UIMax = 3.0;
	float UIMin = -3.0;
	float UIStep = 0.01;
> = 0.10;

float directionY
<
	string UIWidget = "slider";
	float UIMax = 3.0;
	float UIMin = -3.0;
	float UIStep = 0.01;
> = 0.00;

float4 clipPlane : ClipPlane;  //cliplane for water plane

float4 SpotFlashPos;
float4 SpotFlashColor;

float GlobalSurfaceIntensity
<    string UIName =  "GlobalSurfaceIntensity";    
> = {1.0f};

float SurfaceSunFactor
<    string UIName =  "SurfaceSunFactor";    
> = {1.0f};

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

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {1.0, 1.0, 1.0, 1.0f};

float4 FloorColor : Diffuse
<    string UIName =  "FloorColor";    
> = {1.0, 1.0, 1.00, 1.0f};

float alphaoverride  : alphaoverride;

float4 EntityEffectControl
<    string UIName =  "EntityEffectControl";    
> = {0.0f, 0.0f, 0.0f, 0.0f};

Texture2D DiffuseMap : register( t0 );
Texture2D NotUsed : register( t1 );
Texture2D NotUsed2 : register( t2 );
Texture2D NotUsed3 : register( t3 );
Texture2D NotUsed4 : register( t4 );
Texture2D NotUsed5 : register( t5 );
Texture2D NotUsed6 : register( t6 );

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

struct app_in
{
    float3 pos	    : POSITION0;
    float3 Normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
};

struct vs_out
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
	float2 uv2 : TEXCOORD1;
    float4 WPos : TEXCOORD2;
	float clip : TEXCOORD3;
	float vDepth : TEXCOORD4;
    float3 LightVec : TEXCOORD5;
    float3 WorldNormal : TEXCOORD6;
    float2 vegshadowuv : TEXCOORD7;	
};

vs_out DefaultVertexShader( app_in IN )
{
	vs_out OUT;
	float4 tempPos = mul( float4(IN.pos,1), World );
	float radian = tempPos.x+tempPos.z+(time*2.0f);
	float rise = 1.5f;
	tempPos.y = tempPos.y + rise + cos(radian)*rise;
	OUT.pos = mul( tempPos, View );
	OUT.pos = mul( OUT.pos, Projection );
	OUT.uv = IN.uv + float2(directionX,directionY)*time;
	OUT.uv2 = IN.uv + float2(directionX,directionY)*time*0.7f;
	OUT.WPos = tempPos; 
    OUT.clip = dot(tempPos, clipPlane);                                                                      
    OUT.vDepth = mul( float4(IN.pos,1), WorldViewProjection ).z; 
    OUT.WorldNormal = normalize(mul(float4(IN.Normal,0), WorldIT).xyz);
    OUT.LightVec = normalize(LightSource.xyz);
    OUT.vegshadowuv = float2(tempPos.x/51200.0f,tempPos.z/51200.0f);
	return OUT;
}

float4 DefaultPixelShader( vs_out IN ) : COLOR
{
    clip(IN.clip);
	float4 diff = DiffuseMap.Sample(SampleWrap,IN.uv)*0.5f;
	float4 diff2 = DiffuseMap.Sample(SampleWrap,float2(1-IN.uv2.x,IN.uv2.y))*0.5f;
    float4 diffusemap = diff+diff2;
    float3 Ln = normalize(IN.LightVec);
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
    float3 Hn = normalize(Vn+Ln);
    float4 lighting = lit(dot(Ln,IN.WorldNormal),dot(Hn,IN.WorldNormal),24);
    lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
	
	// Shadows
	int iCurrentCascadeIndex = 0;
	float fShadow = GetShadow ( IN.vDepth.x, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
	
    float ShadowSunFactor = lerp(1.0,dot(Ln,IN.WorldNormal),SurfaceSunFactor);
    fShadow = clamp( fShadow * ShadowSunFactor * 3.5f, 0.0 , 0.85 );
    float fInvShadow = 1.0-fShadow;
    lighting.y *= fInvShadow;
	
    float4 dynamicContrib = float4(0,0,0,0);
    float4 viewspacePos = mul(IN.WPos, View);
    float conewidth = 24;
    float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
    float3 lightvector = Vn;
    float3 lightdir = float3(View._m02,View._m12,View._m22);
    float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
    dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
    float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
    float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
    float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
    bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
    float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
    float4 result = diffuseContrib + ambContrib + dynamicContrib;
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
    hudfogresult.a = diffusemap.a * alphaoverride;    
    float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
    hudfogresult.a = hudfogresult.a * alphaslice;
    return hudfogresult;
}

float4 DefaultPixelShaderNoShadow( vs_out IN ) : COLOR
{
    clip(IN.clip);
	float4 diff = DiffuseMap.Sample(SampleWrap,IN.uv)*0.5f;
	float4 diff2 = DiffuseMap.Sample(SampleWrap,float2(1-IN.uv2.x,IN.uv2.y))*0.5f;
    float4 diffusemap = diff+diff2;
    float3 Ln = normalize(IN.LightVec);
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
    float3 Hn = normalize(Vn+Ln);
    float4 lighting = lit(dot(Ln,IN.WorldNormal),dot(Hn,IN.WorldNormal),24);
    lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
    float4 dynamicContrib = float4(0,0,0,0);
    float4 viewspacePos = mul(IN.WPos, View);
    float conewidth = 24;
    float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
    float3 lightvector = Vn;
    float3 lightdir = float3(View._m02,View._m12,View._m22);
    float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
    dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
    float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
    float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
    float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
    bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
    float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
    float4 result = diffuseContrib + ambContrib + dynamicContrib;
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
    hudfogresult.a = diffusemap.a * alphaoverride;    
    float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
    hudfogresult.a = hudfogresult.a * alphaslice;
    return hudfogresult;
}

float4 blackPS(vs_out IN) : COLOR
{
   clip(IN.clip); // all shaders should receive the clip value  
   return IN.vDepth;
}

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, DefaultPixelShader()));
        SetGeometryShader(NULL);
    }
}

technique11 Medium
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, DefaultPixelShaderNoShadow()));
        SetGeometryShader(NULL);
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, DefaultPixelShaderNoShadow()));
        SetGeometryShader(NULL);
    }
}

technique11 DepthMap
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}

technique blacktextured
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}
