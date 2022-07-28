// constant buffer for rendering
//PE: we need to align constant buffers to 256 bytes.
cbuffer cbPerMesh : register( b0 )
{
	float4x4 World; //4*4*4=64bytes
	float4x4 View; //4*4*4=64bytes total 128
	float4x4 Projection; //4*4*4=64bytes total 192
	float amd_filler[16]; //4*4*4 = 64 total 256 bytes.
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
	float4x4 PreviousViewProjectionMatrix; //200 total
	float amd_filler2[32]; //Make sure if dont overwrite anything.
};
