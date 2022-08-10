#include "constantbuffers.fx"

float2 ViewSize = {1920,1080};

Texture2D DiffuseMap : register( t0 );

float4 ScreenColor
<   string UIType = "Screen Color Effect";
> = {0.0f, 0.0f, 0.0f, 0.0f};

float4 OverallColor
<   string UIType = "Overall Color Effect";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float PostContrast
<
   string UIWidget = "slider";
   float UIMax = 5.0;
   float UIMin = 0.0;
   float UIStep = 0.001;
> = 2.0f;

float PostBrightness
<
   string UIWidget = "slider";
   float UIMax = 1.0;
   float UIMin = 0.0;
   float UIStep = 0.001;
> = 0.4f;


SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct appdata 
{
    float3 Position	: POSITION;
    float3 Normal	: NORMAL;
    float2 UV		: TEXCOORD0;    
};

struct vertexOutput
{
    float4 Position : POSITION;
    float2 UV       : TEXCOORD0; 
};

vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    float4 oPos = float4( IN.Position.xy + float2( -1.0f/ViewSize.x, 1.0f/ViewSize.y ),0.0,1.0 );
    OUT.Position = oPos;
    float2 uv = (IN.Position.xy + 1.0) / 2.0;
    uv.y = 1 - uv.y; 
    OUT.UV = uv;
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	float4 result = DiffuseMap.Sample(SampleWrap,IN.UV);
	result.w = 1;
	
    // add any screen color effect and overall modulation control for spin-up
    #ifndef NOCOLOREFFECT
     result.xyz += ScreenColor.xyz;
    #endif

    // Apply contrast
    result.xyz = ((result.xyz - 0.5f) * max(PostContrast, 0)) + 0.5f;

    // Apply brightness
    result.xyz += PostBrightness;

    result.xyz *= OverallColor.xyz;
	
    return result;
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
