// constant buffer for rendering
cbuffer cbPerFrame : register( b0 )
{
   float4x4 World;
   float4x4 View;
   float4x4 Projection;
};
cbuffer cbPerMeshPS : register( b1 )
{
   float4 MaterialEmissive;
   float fAlphaOverride;
   float fRes1;
   float fRes2;
   float fRes3;
   float4x4 ViewInv;
   float4x4 ViewProjectionMatrix;
   float4x4 PreviousViewProjectionMatrix;
};
//float alphaoverride  : alphaoverride;

Texture2D DiffuseMap : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct appdata 
{
    float4 Position   : POSITION;
    uint Color      : COLOR0;
    float2 UV      : TEXCOORD0;    
};

struct vertexOutput
{
    float4 Position : POSITION;
    float4 Color   : TEXCOORD0;
    float2 UV       : TEXCOORD1; 
};

vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    OUT.Position = mul(IN.Position, Projection);   
    OUT.Color = float4(0,0,0,0);
   OUT.Color.x = float((IN.Color & 0x00FF0000)>>16) / 255;
   OUT.Color.y = float((IN.Color & 0x0000FF00)>>8) / 255;
   OUT.Color.z = float((IN.Color & 0x000000FF)) / 255;
   OUT.Color.w = float((IN.Color & 0xFF000000)>>24) / 255;
    OUT.UV = IN.UV;
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
   float4 result = DiffuseMap.Sample(SampleWrap,IN.UV) * IN.Color;
    return result;
}

technique11 Main2D
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}
