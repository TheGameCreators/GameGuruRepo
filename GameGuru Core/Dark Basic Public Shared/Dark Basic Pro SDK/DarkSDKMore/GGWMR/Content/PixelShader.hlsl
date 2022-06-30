// Per-pixel color data passed through the pixel shader.
Texture2D objectTexture: t0;
SamplerState samp: s0;

struct PixelShaderInput
{
    min16float4 pos   : SV_POSITION;
    min16float3 color : COLOR0;
    min16float2 uv    : TEXCOORD0;
};

// The pixel shader passes through the color data. The color data from 
// is interpolated and assigned to a pixel at the rasterization step.
min16float4 main(PixelShaderInput input) : SV_TARGET
{
	min16float4 output = min16float4(objectTexture.Sample(samp, input.uv));
    return min16float4(output.xyz, 1.0f);
}
