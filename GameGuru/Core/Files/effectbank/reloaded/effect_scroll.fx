string Description = "Effect Scroll Shader";
#include "constantbuffers.fx"
#include "settings.fx"   

float4x4 WorldViewProj : WorldViewProjection;
float time : time;
float sintime : SinTime;
float directionX = 0.1f;
float directionY = 0.0f;

Texture2D BaseTex : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
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
};

vs_out DefaultVertexShader( app_in IN )
{
	vs_out OUT;
	OUT.pos = mul( float4(IN.pos,1), WorldViewProj );
	OUT.uv = IN.uv + float2(directionX,directionY)*time;
	return OUT;
}

float4 DefaultPixelShader( vs_out IN ) : COLOR
{
	return BaseTex.Sample(SampleWrap,IN.uv);
}

technique11 All
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, DefaultVertexShader()));
        SetPixelShader(CompileShader(ps_5_0, DefaultPixelShader()));
        SetGeometryShader(NULL);
    }
}
