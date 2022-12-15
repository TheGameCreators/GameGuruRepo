string Description = "Procedural Sky Scroll Shader";
//cybernescence - inspired by Wicked Engine

#include "settings.fx"
#include "constantbuffers.fx"

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {0.9, 0.9, 1.0, 1.0f};

float4 SkyClouds : Diffuse
<    string UIName =  "SkyClouds";    
> = {0.1, 0.1, 0.5, 1.0f};

const float cloudscale = 1.1;
const float clouddark = 0.5;
const float cloudlight = 0.3;
const float3 skycolour1 = float3(0.353, 0.408, 0.474);
const float3 skycolour2 = float3(0.290, 0.349, 0.506);
const float2x2 m = float2x2( 1.6,  1.2, -1.2,  1.6 ); 

float2 hash(float2 p)
{
	p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
	return -1.0 + 2.0*frac(sin(p)*43758.5453123);
}

float noise(in float2 p)
{
	const float K1 = 0.366025404; 
	const float K2 = 0.211324865; 
	float2 i = floor(p + (p.x + p.y)*K1);
	float2 a = p - i + (i.x + i.y)*K2;
	float2 o = (a.x > a.y) ? float2(1.0, 0.0) : float2(0.0, 1.0); 
	float2 b = a - o + K2;
	float2 c = a - 1.0 + 2.0*K2;
	float3 h = max(0.5 - float3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	float3 n = h * h*h*h*float3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
	return dot(n, float3(70.0, 70.0, 70.0));
}

float fbm(float2 n) 
{
	float total = 0.0, amplitude = 0.1;
	for (int i = 0; i < 7; i++) 
	{
		total += noise(n) * amplitude;
		n = mul(n, m); 
		amplitude *= 0.4;
	}
	return total;
}

float4x4 WorldInverse 		: WorldInverse;
float4x4 WorldIT 				: WorldInverseTranspose;
float4x4 WorldView 			: WorldView;
float4x4 WorldViewProjection 	: WorldViewProjection;
float4x4 ViewInverse 			: ViewInverse;
float4x4 ViewIT 				: ViewInverseTranspose;
float4x4 ViewProjection 		: ViewProjection;
float4 	eyePos 					: CameraPosition;
float 	sintime 				: SinTime;
float 	alphaoverride			: alphaoverride;
float4 	ScrollScaleUV;
float 	SpecularOverride;
float 	SurfaceSunFactor = {1.0f};
float 	iTime 					: Time;
float4 	clipPlane 				: ClipPlane;  

float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 SkyScrollValues : Diffuse
<    string UIName =  "SkyScrollValues";    
> = {0.0f, 0.0f, 0.1f, 0.002f};

Texture2D DiffuseMap 	: register( t0 );
Texture2D PortalMap 	: register( t1 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};
SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct appdata 
{
    float3 Position		: POSITION;
    float2 UV0				: TEXCOORD0;
    float2 UV1				: TEXCOORD1;	
};

struct vertexOutput
{
    float4 Position     	: POSITION;
    float2 TexCoord0    	: TEXCOORD0;
    float2 TexCoord1    	: TEXCOORD1;
    float clip          	: TEXCOORD2;
	float4 WPos			: TEXCOORD3;
};


vertexOutput mainVS(appdata IN)   
{
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.Position = mul(float4(IN.Position,1), WorldViewProjection);
    OUT.TexCoord0  = IN.UV0; 
    OUT.TexCoord1  = IN.UV1; 
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
	OUT.WPos = worldSpacePos;
    return OUT;
}


float4 mainPS(in vertexOutput IN, uniform int blacktextureneeded) : COLOR
{
    float4 finalcolor;
	float speed = SkyScrollValues.w;
	float cloudcover = clamp(1 - SkyScrollValues.z,0,1);
	float cloudalpha = SkyClouds.y * 20;
	float skytint = SkyClouds.x; 

    float portalalpha = PortalMap.Sample(SampleClamp,IN.TexCoord0.xy).a;
	float2 p = IN.TexCoord0.xy+float2(SkyScrollValues.x,SkyScrollValues.y);
	float2 uv = p;     
    float time = iTime * speed;
    float q = fbm(uv * cloudscale * 0.5);
   
	float r = 0.0;
	uv *= cloudscale;
    uv -= q - time;
    float weight = 0.8;
    for (int i=0; i<8; i++)
	{
		r += abs(weight*noise( uv ));
		uv = mul(uv, m) + time;
		weight *= 0.7;
    }
  
	float f = 0.0;
    uv = p;
	uv *= cloudscale;
    uv -= q - time;
    weight = 0.7;
    for (int i2=0; i2<8; i2++)
	{
		f += weight*noise( uv );
		uv = mul(uv, m) + time;
		weight *= 0.6;
    }
    
    f *= r + f;
    
    float c = 0.0;
    time = iTime * speed * 2.0;
    uv = p;
	uv *= cloudscale*2.0;
    uv -= q - time;
    weight = 0.4;
    for (int i3=0; i3<7; i3++)
	{
		c += weight*noise( uv );
		uv = mul(uv, m) + time;
		weight *= 0.6;
    }
 
    float c1 = 0.0;
    time = iTime * speed * 3.0;
    uv = p;
	uv *= cloudscale*3.0;
    uv -= q - time;
    weight = 0.4;
    for (int i4=0; i4<7; i4++)
	{
		c1 += abs(weight*noise( uv ));
		uv = mul(uv, m) + time;
		weight *= 0.6;
    }
	
    c += c1;
    float3 skycolour = lerp(skycolour2, skycolour1, p.y);
	float3 cloudcolour = SkyColor.rgb * clamp((skytint + cloudlight*c), 0.0, 1.0);
    ///float3 cloudcolour = SkyColor.rgb * clamp((clouddark + cloudlight*c), 0.0, 1.0);
   
    f = cloudcover + cloudalpha*f*r;
    
    ///float3 result = lerp(skycolour, clamp(skytint * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));
	float3 result = cloudcolour; //lerp(skycolour, clamp(cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));
    
	float alphaFade2 = 1 - clamp(f + c, 0.0, 1.0);
	finalcolor = float4(result,1);
	
	//fog and hole
    float3 eyePosFlat = eyePos.xyz; eyePosFlat.y *= 0.1f;
    float3 wPosFlat = IN.WPos.xyz; wPosFlat.y *= 0.1f;
    float thedistance = distance(eyePosFlat,wPosFlat) / 500.0f; // Skybox smaller than landscape size
    float hudfogfactor = saturate((thedistance - HudFogDist.x)/(HudFogDist.y - HudFogDist.x));

  	//float fCloudAlpha = SurfaceSunFactor * finalcolor.a * alphaFade2;
	float fCloudAlpha = finalcolor.a * alphaFade2;
    float3 hudfogresult = lerp(finalcolor.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);
	
	return float4(hudfogresult*blacktextureneeded,fCloudAlpha*portalalpha);
	
}


technique11 Main
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS(1)));
        SetGeometryShader(NULL);
    }
}

technique11 blacktextured //allow sun behind clouds
{
   pass P0
    {
		SetVertexShader(CompileShader(vs_5_0, mainVS()));
		SetPixelShader(CompileShader(ps_5_0, mainPS(0)));
		SetGeometryShader(NULL);
    }
}


