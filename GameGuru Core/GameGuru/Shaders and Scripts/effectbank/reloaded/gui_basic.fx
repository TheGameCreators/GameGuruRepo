#include "constantbuffers.fx"
#ifdef GUISHOWSHADOWDEPTHMAPS
 #include "settings.fx"    
 #include "cascadeshadows.fx"
#else
 float ShadowStrength;
 float4 LightSource;
#endif

// Global constants passed in
float4 ScrollScaleUV;
float4 HighlightCursor;
float4 HighlightParams;
float4 GlowIntensity;
float AlphaOverride;
float SpecularOverride;
float4 EntityEffectControl;
float4 ShaderVariables;
float4 AmbiColorOverride;
float4 AmbiColor;
float4 clipPlane : ClipPlane;
float SurfaceSunFactor;
float GlobalSpecular;
float GlobalSurfaceIntensity;
float4 SurfColor;
float4 SkyColor;
float4 FloorColor;
float4 DistanceTransition;
float4 HudFogColor;
float4 HudFogDist;
float4 SpotFlashPos;
float4 SpotFlashColor;
float4 g_lights_data;
float4 g_lights_pos0;
float4 g_lights_pos1;
float4 g_lights_pos2;
float4 g_lights_atten0;
float4 g_lights_atten1;
float4 g_lights_atten2;
float4 g_lights_diffuse0;
float4 g_lights_diffuse1;
float4 g_lights_diffuse2;

Texture2D DiffuseMap : register( t0 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

#ifdef GUISHOWSHADOWDEPTHMAPS
SamplerState SamplePoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
#endif

struct appdata 
{
    float3 Position	: POSITION;
    float3 Normal	: NORMAL;
	#ifdef GUIUSEDIFFUSE
	uint Diffuse    : COLOR0;
	#endif
    float2 UV		: TEXCOORD0;    
};

struct vertexOutput
{
    float4 Position : POSITION;
    float2 UV       : TEXCOORD0; 
	#ifdef GUIUSEDIFFUSE
	float3 Diffuse  : TEXCOORD1;
	#endif
};

vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    //OUT.Position = mul(float4(IN.Position,1), WorldViewProjection); -add this to constant buffer
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
 	OUT.UV = IN.UV;
	#ifdef GUIUSEDIFFUSE
	float red = ((IN.Diffuse & (255<<16))>>16) / 255.0f;
	float green = ((IN.Diffuse & (255<<8))>>8) / 255.0f;
	float blue = ((IN.Diffuse & (255))) / 255.0f;
 	OUT.Diffuse.x = red;
 	OUT.Diffuse.y = green;
 	OUT.Diffuse.z = blue;
	#endif
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	#ifdef GUISHOWSHADOWDEPTHMAPS
	 //float fDepthValue = (DepthMapTX1.Sample( SamplePoint, IN.UV ).r-0.0f)*1.0f; 
	 float fDepthValue = (DepthMapTX1.Sample( SamplePoint, IN.UV ).r-0.91f)*50.0f; 
	 return float4(fDepthValue,fDepthValue,fDepthValue,1);
	#endif
	
	float4 result = DiffuseMap.Sample(SampleWrap,IN.UV);
	if ( result.w == 0 )
	{
		if ( MaterialEmissive.w == 0 )
		{	
			clip(-1);
		}
		else
		{
			result.w = 1;
		}
	}
	result.xyz = (result.xyz * (1-MaterialEmissive.w)) + (MaterialEmissive.xyz * MaterialEmissive.w);
	#ifdef GUIUSEDIFFUSE
 	result.xyz *= IN.Diffuse;
	#endif
	result.w *= fAlphaOverride;
    return result;
}

#ifdef GUIUSEWIREFRAME
RasterizerState WireframeMode
{
	FillMode = WIREFRAME;
	CullMode = NONE;
};
#endif

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
		#ifdef GUIUSEWIREFRAME
		SetRasterizerState(WireframeMode);
		#endif
    }
}

technique11 Medium
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
		#ifdef GUIUSEWIREFRAME
		SetRasterizerState(WireframeMode);
		#endif
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
		#ifdef GUIUSEWIREFRAME
		SetRasterizerState(WireframeMode);
		#endif
    }
}

technique11 DepthMap
{
    pass MainPass
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
    }
}

technique11 DepthMapNoAnim
{
    pass MainPass
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
    }
}
