string Description = "Dynamic Terrain Shadow Texture Camera Shader";
#include "constantbuffers.fx"
#include "settings.fx"

// shadow mapping constants
matrix          m_mShadow;
float4          m_vCascadeOffset[8];
float4          m_vCascadeScale[8];
int             m_nCascadeLevels;
float           m_fMinBorderPadding;     
float           m_fMaxBorderPadding;
float           m_fShadowBiasFromGUI; 
float           m_fCascadeBlendArea;
float           m_fTexelSize; 
float           m_fCascadeFrustumsEyeSpaceDepths[8];
float3          m_vLightDir;

// constants
float4x4 WorldViewProjection : WorldViewProjection;
float2 ViewSize : ViewSize;

Texture2D HeightMap : register( t0 );
Texture2D DepthMapTX4 : register( t1 );

SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct input 
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD0;
};
 
struct output 
{
	float4 pos: POSITION;
	float4 WPos: TEXCOORD0;
};

output VS( input IN ) 
{
	output OUT;
	
	float4 vert = float4(IN.pos,1);
	float4 temppos = mul(vert, World);
	temppos.z = 51200.0f - temppos.z;
	OUT.WPos = temppos;
	OUT.pos = mul( vert, WorldViewProjection );		
	return OUT;	
}

void ComputeCoordinatesTransform( in float4 InterpolatedPosition, in out float4 vShadowTexCoord ) 
{
	float4 vLightDot = mul ( InterpolatedPosition, m_mShadow );
    vLightDot *= m_vCascadeScale[3];
    vLightDot += m_vCascadeOffset[3];
	vShadowTexCoord.xyz = vLightDot.xyz;
} 

float4 PSPresent( output IN ) : COLOR
{
	// account for terrain height at XZ coordinates
	float4 HeightCol = HeightMap.Sample(SampleClamp,float2(IN.WPos.x/51200.0f,IN.WPos.z/51200.0f));
	float fThisHeight = (((HeightCol.z*255))+((HeightCol.y*255)*256)+((HeightCol.x*255)*65536))/100.0f;
	float4 actualPos = float4(IN.WPos.x,IN.WPos.y+fThisHeight,IN.WPos.z,IN.WPos.w);	
	float4 color;
	float4 vShadowMapTextureCoord = 0.0f;
	ComputeCoordinatesTransform( actualPos, vShadowMapTextureCoord );    
	if ( vShadowMapTextureCoord.x>=0 && vShadowMapTextureCoord.x<=1.0 && vShadowMapTextureCoord.y>=0.0f && vShadowMapTextureCoord.y<=1.0 )
	{
		// commit this shadow pixel to the dynamic shadow long-term-storage
		float fShadow = vShadowMapTextureCoord.z > DepthMapTX4.Sample(SampleClamp,float2(vShadowMapTextureCoord.x,vShadowMapTextureCoord.y)).x ? 1.0f : 0.0f;
		color = float4(fShadow,fShadow,fShadow,1);
	}
	else
	{
		// reject pixel
		clip(-1);
		color = float4(0,0,0,0);
	}	
    return color;
}

float4 PSPresent_superflat( output IN ) : COLOR
{
	// account for terrain height at XZ coordinates
	float4 HeightCol = HeightMap.Sample(SampleClamp,float2(IN.WPos.x/51200.0f,IN.WPos.z/51200.0f));
	float fThisHeight = 1000.0f;
	float4 actualPos = float4(IN.WPos.x,IN.WPos.y+fThisHeight,IN.WPos.z,IN.WPos.w);	
	float4 color;
	float4 vShadowMapTextureCoord = 0.0f;
	ComputeCoordinatesTransform( actualPos, vShadowMapTextureCoord );    
	if ( vShadowMapTextureCoord.x>=0 && vShadowMapTextureCoord.x<=1.0 && vShadowMapTextureCoord.y>=0.0f && vShadowMapTextureCoord.y<=1.0 )
	{
		// commit this shadow pixel to the dynamic shadow mong-term-storage
		float fShadow = vShadowMapTextureCoord.z > DepthMapTX4.Sample(SampleClamp,float2(vShadowMapTextureCoord.x,vShadowMapTextureCoord.y)).x ? 1.0f : 0.0f;
		color = float4(fShadow,fShadow,fShadow,1);
	}
	else
	{
		// reject pixel
		clip(-1);
		color = float4(0,0,0,0);
	}	
    return color;
}

technique11 Terrain
{
	pass Present
	{
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PSPresent()));
        SetGeometryShader(NULL);
	}
}

technique11 SuperFlat
{
	pass Present
	{
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PSPresent_superflat()));
        SetGeometryShader(NULL);
	}
}
