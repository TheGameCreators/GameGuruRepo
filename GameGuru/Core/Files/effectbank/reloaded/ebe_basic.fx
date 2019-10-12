#include "constantbuffers.fx"

float4 GlowIntensity;

Texture2D DiffuseMap : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
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
    //OUT.Position = mul(float4(IN.Position,1), WorldViewProjection); -add this to constant buffer
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
 	OUT.UV = IN.UV;
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	float4 result = DiffuseMap.Sample(SampleWrap,IN.UV);
	result.xyz += GlowIntensity.xyz;
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
