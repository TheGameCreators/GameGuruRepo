// Lightray Shader
#include "constantbuffers.fx"

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {1.0, 1.0, 1.0, 1.0f};

matrix ViewProjection:ViewProjection;
matrix ProjMat={0.5,0,0,0.5,0,-0.5,0,0.5,0,0,0.5,0.5,0,0,0,1}; 

float3 LightDir={-0.0,-0.5,-0.723};
float Density=1.0f;
float AlphaAngle=1.0;
float LightRayLength=0.5;
float LightRayQuality=15.0f;
float LightRayDecay=0.975f;
float LightRayFactor=1.0f;
float4 ScrollScaleUV;
float SpecularOverride;

Texture2D BlackTexture : register( t0 );
SamplerState SampleClamp
{
    //Filter = MIN_MAG_MIP_POINT;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct InPut
{
    float3 Pos         : POSITION;
    float3 Normal      : NORMAL;
    float2 UV          : TEXCOORD0;
};
struct OutPut
{
	float4 OPos : POSITION; 
 	float2 Tex  : TEXCOORD0;
};

OutPut VS(InPut IN) 
{
 	OutPut OUT;
	OUT.OPos=float4(IN.Pos.xyz,1);
 	OUT.Tex=IN.UV;
	return OUT;
}

float4 PSScatter(OutPut IN, uniform int SampleNum) : COLOR
{
	float2 NewUv = IN.Tex;
	NewUv.x = 1.0 - NewUv.x; //PE: direction is inverse.
	float4 ScreenToLight=mul(ProjMat,mul(float4(-LightDir.xyz,0),ViewProjection));
	float2 DeltaTex=((ScreenToLight.xy/ScreenToLight.z)- NewUv )*sign(ScreenToLight.z);
	
	DeltaTex=float2(0.5,0.5)- NewUv;
	
	float thelength=length(DeltaTex);DeltaTex /=thelength;
	thelength=min(thelength,Density);DeltaTex *=thelength;
	
	LightRayLength = ((1.0 - 0.6) * (LightRayLength - 0.0) / (1.0 - 0.0)) + 0.6; //remap slider so more of it has an affect
	LightRayDecay = ((1.0 - 0.9) * (LightRayDecay) / (1.0)) + 0.9; //remap slider so more of it has an affect

	float3 Scatter=0;
	float FallOff=2.0;//1.0;
	DeltaTex /= SampleNum;
	DeltaTex *= LightRayLength;
	for (int i=0; i < SampleNum; i++)
	{		
	  NewUv += (DeltaTex*LightRayLength);
	  Scatter += (BlackTexture.Sample(SampleClamp,NewUv).xyz*AlphaAngle)*FallOff;
	  FallOff *= LightRayDecay;
	}
	Scatter /= SampleNum;
	float3 result = Scatter.xyz*SkyColor.xyz*SkyColor.a*LightRayFactor;
	//float alphafactor = (result.x + result.y + result.z ) / 3.0f; //using add blend now
	float visibility = LightDir.y > 0.0 ? 1 : 0; //don't show intersect with terrain when sun low
	
	return float4(result.xyz*visibility, 1);//alphafactor );
}

BlendState AddAlpha
{
	BlendEnable[0] = TRUE;	
	BlendOp = Add;//Max;
	SrcBlend = One;
	DestBlend = One;
};

float4 PSNone(OutPut IN) : COLOR
{
	return BlackTexture.Sample(SampleClamp,IN.Tex);
}

// techniques   

technique11 ScatterLOW
{
	pass p1
	{		
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PSScatter(10) ) );
        SetGeometryShader( NULL );
		SetBlendState(AddAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
	}
}

technique11 ScatterMEDIUM
{
	pass p1
	{		
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PSScatter(20) ) );
        SetGeometryShader( NULL );
		SetBlendState(AddAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
	}
}

technique11 ScatterHIGH
{
	pass p1
	{		
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PSScatter(35) ) );
        SetGeometryShader( NULL );
		SetBlendState(AddAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
	}
}

technique11 ScatterHIGHEST
{
	pass p1
	{		
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PSScatter(80) ) );
        SetGeometryShader( NULL );
		SetBlendState(AddAlpha, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
	}
}

technique11 NoScatter
{
	pass p1
	{		
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PSNone() ) );
        SetGeometryShader( NULL );
	}
}
