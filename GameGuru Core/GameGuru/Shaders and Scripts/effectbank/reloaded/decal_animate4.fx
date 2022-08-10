string Description = "Decal Animate Shader";
#include "constantbuffers.fx"
#include "settings.fx" 

float4x4 WorldView : WorldView;
float4x4 WorldViewProj : WorldViewProjection;
float Time: Time;
float sintime : SinTime;

float SpriteRows : Power
<
    string UIName =  "Rows";
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 16.0;
    float UIStep = 1.0;
> = 4.000000;

float SpriteColumns : Power
<
    string UIName =  "Columns";
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 16.0;
    float UIStep = 1.0;
> = 4.000000;

float FramesPerSec : Power
<
    string UIName =  "Frames Per Second";
    string UIWidget = "slider";
    float UIMin = 0.0;
    float UIMax = 30.0;
    float UIStep = 0.5;
> = 25.000000;

float Looptime : Power
<
    string UIName =  "Loop Time";
    string UIWidget = "slider";
    float UIMin = 1.0;
    float UIMax = 200.0;
    float UIStep = 1.0;
> = 100.000000;

// Global constants passed in
float4 AmbiColorOverride;
float4 AmbiColor;
float4 SurfColor;
float4 SkyColor;
float4 HudFogColor;
float4 HudFogDist;
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
    float4 Position    : POSITION;
    float2 atlasUV     : TEXCOORD0;
};

vertexOutput mainVS(appdata IN)   
{
	vertexOutput OUT;
    OUT.Position = mul(float4(IN.Position,1), WorldViewProj);
	float2 DimensionsXY = float2(SpriteRows,SpriteColumns); 
	float2 atlasUVtemp = IN.UV; 				
	float loopcounter  = floor(Time/Looptime);
	float offset = Looptime*loopcounter;
	float speed =(Time*FramesPerSec) - (offset*FramesPerSec);
	float index = floor( speed);
	float rowCount = floor( (index / DimensionsXY.y) );
	float2 offsetVector = float2(index, rowCount);
	float2 atlas = (1.0 / DimensionsXY) ;
	float2 move = (offsetVector + atlasUVtemp);
	OUT.atlasUV = (atlas.xy * move);
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
   float4 diffuse = DiffuseMap.SampleLevel(SampleWrap,IN.atlasUV,0);
#ifdef ENABLEDECALALPHACLIP
   if( diffuse.a < 0.42 ) 
   {
      clip(-1);
      return float4(0,0,0,1);
   }
#endif
   return diffuse;
}

BlendState DecalAlpha
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = INV_SRC_ALPHA;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F; // color write enable all.
};

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
        SetBlendState(DecalAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
    }
}

technique11 Medium
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
        SetBlendState(DecalAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
        SetBlendState(DecalAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
    }
}
//PE: technique11 DepthMap removed this has been fixed in c code.
