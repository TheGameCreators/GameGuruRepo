// Per-vertex data passed to the geometry shader.
struct VertexShaderOutput
{
    min16float4 pos     : SV_POSITION;
    min16float3 color   : COLOR0;
    min16float2 uv      : TEXCOORD0;

    // The render target array index will be set by the geometry shader.
    uint        viewId  : TEXCOORD1;
};

#include "VertexShaderShared.hlsl"
