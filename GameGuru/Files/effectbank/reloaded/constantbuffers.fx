// constant buffer for rendering
cbuffer cbPerMesh : register( b0 )
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
