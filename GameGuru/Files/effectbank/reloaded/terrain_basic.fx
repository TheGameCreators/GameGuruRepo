string Description = "Terrain Shader";                          
#include "settings.fx"
#ifdef USEPBRTERRAIN
	#define PBRTERRAIN
	#define ALPHADISABLED
    #define CALLEDFROMOLDTERRAIN
	#include "apbr_core.fx"
#else
#include "cascadeshadows.fx"

// constant buffer for rendering terrain chunks
cbuffer cbChangeEachTerrainChunk : register( b0 )
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};
//PE: lee without this, all the other shader variables are overwritten.
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
// regular shader constants   
float4 eyePos : CameraPosition;

float GrassFadeDistance
<    string UIName =  "GrassFadeDistance";    
> = {10000.0f};

float SurfaceSunFactor
<    string UIName =  "SurfaceSunFactor";    
> = {1.0f};

float GlobalSpecular
<    string UIName =  "GlobalSpecular";    
> = {0.5f};

float GlobalSurfaceIntensity
<    string UIName =  "GlobalSurfaceIntensity";    
> = {1.0f};

float DetailMapScale
<    string UIName =  "DetailMapScale";    
> = {1.5f};

float4 HighlightCursor
<   string UIType = "HighlightCursor";
> = {0.0f,0.0f,0.0f,1.0f};

float4 HighlightParams
<   string UIType = "HighlightParams";
> = {0.0f,0.0f,0.0f,1.0f};

float4 DistanceTransition
<   string UIType = "DistanceTransition";
> = {1600.0f,2000.0f,400.0f,0.0f};

/**************VALUES PROVIDED FROM FPSC - NON TWEAKABLE**************************************/

float4 clipPlane : ClipPlane;  //cliplane for water plane

//Supports dynamic lights (using CalcLighting function)
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

float dl_lights;
float dl_lightsVS;
float4 dl_pos[40];
float4 dl_diffuse[40];
float4 dl_angle[40];

//SpotFlash Values
float4 SpotFlashPos;  //SpotFlashPos.w is carrying the spotflash fadeout value
float4 SpotFlashColor; //RGB of flash colour

//WATER Fog Color
float4 FogColor : Diffuse
<   string UIName =  "Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.000001f};

//HUD Fog Color
float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 1.0f};

//HUD Fog Distances (near,far,0,0)
float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 AmbiColorOverride
<    string UIName =  "AmbiColorOverride";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 AmbiColor : Ambient
<    string UIName =  "AmbiColor";    
> = {0.2f, 0.2f, 0.3f, 0.0f};

float4 SurfColor : Diffuse
<    string UIName =  "SurfColor";    
> = {1.0f, 0.9f, 0.8f, 1.0f};

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {1.0, 1.0, 1.0, 1.0f};

float4 FloorColor : Diffuse
<    string UIName =  "FloorColor";    
> = {1.0, 1.0, 1.00, 1.0f};

//Shader Variables pulled from FPI scripting 
float4 ShaderVariables : ShaderVariables
<    string UIName =  "Shader Variables";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

/***************TEXTURES AND SAMPLERS***************************************************/

Texture2D VegShadowSampler : register( t0 );
Texture2D Reserved0Map : register( t1 );
Texture2D DiffuseSampler : register( t2 );
Texture2D HighlighterSampler : register( t3 );
Texture2D NormalMapSampler : register( t4 );
Texture2D Reserved1Map : register( t5 );
Texture2D Reserved2Map : register( t6 );
Texture2D Reserved3Map : register( t7 );

SamplerState SampleWrap
{
#ifdef TRILINEAR
    Filter = MIN_MAG_MIP_LINEAR;
#else
    Filter = ANISOTROPIC;
    MaxAnisotropy = MAXANISOTROPYTERRAIN;
#endif
    AddressU = Wrap;
    AddressV = Wrap;
    MAXLOD = 6;
};
SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
SamplerState SampleBorder
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Border;
    AddressV = Border;
};

// structures for final render
struct appdata
{
    float3 Position	: POSITION0;
    float3 Normal   : NORMAL0;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float2 TexCoordLM   : TEXCOORD1;
    float3 LightVec       : TEXCOORD2;
    float3 WorldNormal   : TEXCOORD3;
    float4 WPos         : TEXCOORD4; 
    float  clip         : TEXCOORD6;
    float  vDepth       : TEXCOORD7;
    float3 Tn            : TEXCOORD8;    
    float3 Bn            : TEXCOORD9;  
    float3 VertexLight  : TEXCOORD10;
};

struct vertexOutput_low
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 LightVec       : TEXCOORD1;
    float3 WorldNormal   : TEXCOORD2;
    float4 WPos         : TEXCOORD3;
    float  clip         : TEXCOORD4;
    float  vDepth       : TEXCOORD5;
    float3 VertexLight  : TEXCOORD6;
};


float3 CalcExtSpot( float3 worldNormal, float3 worldPos , float3 SpotPos , float3 SpotColor , float range, float3 angle,float3 diffusemap)
{
    float conewidth = 24;
	float toLight = length(SpotPos.xyz - worldPos) * 2.0;
	float4 local_lights_atten = float4(1.0, 1.0/range, 1.0/(range*range), 0.0);
	float intensity = 1.0/dot( local_lights_atten, float4(1,toLight,toLight*toLight,0) );
    float3 V  = SpotPos.xyz - worldPos;  
    float3 Vn  = normalize(V); 
    float3 lightvector = Vn;
    //float3 lightdir = (2.0/(360.0/angle)) - 1.0;
    //float3 lightdir = normalize(  SpotPos.xyz -(SpotPos.xyz+angle+angle) );
    float3 lightdir = normalize(float3(angle.x,angle.y*2.0,angle.z));
    intensity = clamp(intensity * (dot(-lightdir,worldNormal)),0.0,1.0);
    return (SpotColor.xyz * pow(max(dot(-lightvector, lightdir ),0),conewidth) * 2.5 ) * intensity * diffusemap;
}


float3 CalcExtLightingVS(float3 Nb, float3 worldPos, float3 Vn )
{
	float3 output = float3(0,0,0);
    float3 toLight;
    float lightDist;
    float fAtten;
    float3 lightDir;
    float3 halfvec;
    float4 lit0;
	float4 local_lights_atten;
	
	//dl_pos[i].w = range.

	for( int i=dl_lights ; i < dl_lightsVS+dl_lights ; i++ )
	{
		if( dl_diffuse[i].w == 3.0 ) {
			output += CalcExtSpot(Nb,worldPos,dl_pos[i].xyz,dl_diffuse[i].xyz,dl_pos[i].w,dl_angle[i].xyz, float3(0.75,0.75,0.75));
		} else {
			toLight = dl_pos[i].xyz - worldPos;
			lightDist = length( toLight ) * 2.0;
			local_lights_atten = float4(1.0, 1.0/dl_pos[i].w, 1.0/(dl_pos[i].w*dl_pos[i].w), 0.0);
			fAtten = 1.0/dot( local_lights_atten, float4(1,lightDist,lightDist*lightDist,0) );
			lightDir = normalize( toLight );
			halfvec = normalize(Vn + lightDir);
			lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
			lit0.z = clamp( ( lit0.z * GlobalSpecular) ,0.0,1.0);
			output+= (lit0.y *dl_diffuse[i].xyz * fAtten ); //PE: no spec + (lit0.z * dl_diffuse[i].xyz * fAtten );   
		}
	}
	return output;
}

/*******Vertex Shader***************************/

vertexOutput mainVS_highest(appdata IN)   
{
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.WPos = worldSpacePos;  
    OUT.LightVec = normalize(LightSource.xyz);
    OUT.WorldNormal = IN.Normal.xyz;
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
    float2 tc = IN.UV0 * 500.0f; 
    OUT.TexCoord  = IN.UV0 * 500.0f; 
    OUT.TexCoordLM  = IN.UV1 / 2.5f; 
     
    // all shaders should send the clip value to the pixel shader (for refr/refl)                                                                     
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      

    // SHADOW MAPPING - world position and projected depth (for cascade distance calc)
    OUT.vDepth = OUT.Position.z; 

    float3 c1 = cross(OUT.WorldNormal, float3(0.0, 0.0, 1.0)); 
    float3 c2 = cross(OUT.WorldNormal, float3(0.0, 1.0, 0.0)); 
    if (length(c1) > length(c2)) {
      OUT.Tn = c1;   
    } else {
      OUT.Tn = c2;   
    }
    OUT.Tn = normalize( OUT.Tn);
    OUT.Bn = normalize(cross(OUT.WorldNormal, OUT.Tn)); 

	float3 WorldEyeVec = (eyePos.xyz - worldSpacePos.xyz);
	OUT.VertexLight.xyz = CalcExtLightingVS(OUT.WorldNormal.xyz, worldSpacePos.xyz, WorldEyeVec );


//PE: Something is wrong with the terrin normals, they are often set is flat output.normal.y > 0.9985 , but are not flat ?
//PE: So need to disable this for now.
//    if(OUT.Position.z > 3400.0 && OUT.WPos.y < 460.0 && OUT.WorldNormal.y > 0.9985 ) {
//      OUT.clip=-1.0;
//      OUT.Position.z = 100000.0;
//      OUT.Position.x = 100000.0;
//      OUT.Position.y = 100000.0;
//      OUT.Position.w = 0.0;
//    }
    return OUT;
}

vertexOutput_low mainVS_lowest(appdata IN)   
{
    vertexOutput_low OUT;
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
    OUT.vDepth = OUT.Position.z; 
    OUT.LightVec = normalize(LightSource.xyz);
    OUT.WorldNormal = IN.Normal.xyz;
    OUT.TexCoord  = IN.UV0 * 500.0f; 
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.WPos = worldSpacePos;  
    OUT.clip = dot(worldSpacePos, clipPlane); 

	float3 WorldEyeVec = (eyePos.xyz - worldSpacePos.xyz);
	OUT.VertexLight.xyz = CalcExtLightingVS(OUT.WorldNormal.xyz, worldSpacePos.xyz, WorldEyeVec );
    
//PE: Something is wrong with the terrin normals, they are often set is flat output.normal.y > 0.9985 , but are not flat ?
//PE: So need to disable this for now.
//    if(OUT.Position.z > 3400.0 && OUT.WPos.y < 460.0 && OUT.WorldNormal.y > 0.9985 ) 
//    {
//      OUT.clip=-1.0;
//      OUT.Position.z = 100000.0;
//      OUT.Position.x = 100000.0;
//      OUT.Position.y = 100000.0;
//      OUT.Position.w = 0.0;
//    }
    return OUT;
}

/****************Framgent Shader*****************/

float4 CalcSpotFlash( float3 worldNormal, float3 worldPos )
{
    float4 output = (float4)0.0;
    float3 toLight = (SpotFlashPos.xyz - worldPos.xyz);
    float3 lightDir = normalize( toLight );
    float lightDist = length( toLight );
    
    float MinFalloff = 100;  //falloff start distance
    float LinearFalloff = 1;
    float ExpFalloff = .005;  // 1/200
    float fSpotFlashPosW = clamp(0,1,SpotFlashPos.w);
    
    //classic attenuation - but never actually reaches zero
    float fAtten = 1.0/(MinFalloff + (LinearFalloff*lightDist)+(ExpFalloff*lightDist*lightDist));
    output += (SpotFlashColor) *fAtten * (fSpotFlashPosW); //don't use normal, faster
        
    return output;
}


float3 CalcExtLighting(float3 Nb, float3 worldPos, float3 Vn, float3 diffusemap, float3 specmap )
{
	float3 output = float3(0,0,0);
    float3 toLight;
    float lightDist;
    float fAtten;
    float3 lightDir;
    float3 halfvec;
    float4 lit0;
	float4 local_lights_atten;
	
	//dl_pos[i].w = range.

	for( int i=0 ; i < dl_lights ; i++ ) {

		if( dl_diffuse[i].w == 3.0 ) {
			output += CalcExtSpot(Nb,worldPos,dl_pos[i].xyz,dl_diffuse[i].xyz,dl_pos[i].w,dl_angle[i].xyz,diffusemap);
		} else {
		
			toLight = dl_pos[i].xyz - worldPos;
			lightDist = length( toLight ) * 2.0;
			local_lights_atten = float4(1.0, 1.0/dl_pos[i].w, 1.0/(dl_pos[i].w*dl_pos[i].w), 0.0);
			fAtten = 1.0/dot( local_lights_atten, float4(1,lightDist,lightDist*lightDist,0) );
			lightDir = normalize( toLight );
			halfvec = normalize(Vn + lightDir);
			lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
			lit0.z = clamp( ( lit0.z * GlobalSpecular) ,0.0,1.0);
			output+= (lit0.y *dl_diffuse[i].xyz * fAtten * 1.25 * diffusemap) + (lit0.z * dl_diffuse[i].xyz * fAtten * 0.5 );
		}
	}
	return output;
}


float4 CalcLighting(float3 Nb, float3 worldPos, float3 Vn, float4 diffusemap,float4 specmap)
{
    float4 output = (float4)0.0;
    #ifdef SKIPIFNODYNAMICLIGHTS
     // in beta 1.14 alwaqys zero. enable anyway
     if ( g_lights_data.x == 0 ) return output;
    #endif
    
    // light 0
    float3 toLight = g_lights_pos0.xyz - worldPos;
    float lightDist = length( toLight );
    float fAtten;
    float3 lightDir;
    float3 halfvec;
    float4 lit0;
    float4 local_lights_atten0 = float4(1.0, 1.0/g_lights_pos0.w, 1.0/(g_lights_pos0.w*g_lights_pos0.w), 0.0);
    fAtten = 1.0/dot( local_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) );
    lightDir = normalize( toLight );
    halfvec = normalize(Vn + lightDir);
    lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
    output+= (lit0.y *g_lights_diffuse0 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse0 * fAtten *specmap );   
    
    // light 1
    toLight = g_lights_pos1.xyz - worldPos;
    lightDist = length( toLight );
    float4 local_lights_atten1 = float4(1.0, 1.0/g_lights_pos1.w, 1.0/(g_lights_pos1.w*g_lights_pos1.w), 0.0);
    fAtten = 1.0/dot( local_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) );
    lightDir = normalize( toLight );
    halfvec = normalize(Vn + lightDir);
    lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
    output+= (lit0.y *g_lights_diffuse1 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse1 * fAtten *specmap );   

    // light 2
    toLight = g_lights_pos2.xyz - worldPos;
    lightDist = length( toLight );
    float4 local_lights_atten2 = float4(1.0, 1.0/g_lights_pos2.w, 1.0/(g_lights_pos2.w*g_lights_pos2.w), 0.0);
    fAtten = 1.0/dot( local_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) );
    lightDir = normalize( toLight );
    halfvec = normalize(Vn + lightDir);
    lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
    output+= (lit0.y *g_lights_diffuse2 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse2 * fAtten *specmap );   
      
    // return light composite
    return output;
}
         
float Atlas16GetUV ( in float textargetselectorV, in float texselectorV, in float2 TexCoord, out float2 texatlasuv, out int texcol, out int texrow )
{
   // select tex from 16atlas
   float factor = 0.0f;
   uint texindex = textargetselectorV * 16.0f;
   float deductV = texselectorV - (texindex*0.0625f);
   factor = max(0,0.0625f-abs(deductV))*16.0f;
   texrow = texindex / 4;
   texcol = texindex - (texrow*4);

   texatlasuv = TexCoord/4.0f;
   int udiv = texatlasuv.x / 0.25f;
   int vdiv = texatlasuv.y / 0.25f;
   texatlasuv.x = texatlasuv.x - (udiv*0.25f);
   texatlasuv.y = texatlasuv.y - (vdiv*0.25f);   

   // crop outer pixel edges for seamless mipmaps
   texatlasuv = texatlasuv / 1024.0f;
   texatlasuv = texatlasuv * 512.0f;
   texatlasuv = texatlasuv + ((0.25f/1024.0f)*256.0f);

   return factor;
}
      
void Atlas16DiffuseLookupCenter( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap )
{
   // vars
   int texcol = 0;
   int texrow = 0;
   float2 texatlasuv = float2(0,0);
   //float fround = float( round(VegShadowColor.b * 16.0) / 16 ); // hard edge.
   float fround = VegShadowColor.b;
   
   float texselectorV = min(fround,0.9375f);
   float2 texDdx = ddx(TexCoord*0.125f);
   float2 texDdy = ddy(TexCoord*0.125f);

   // center sample
   float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
   float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   diffusemap += DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;    
}

void Atlas16DiffuseLookupCenterDist( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap, in float vDepth )
{
   // vars
   int texcol = 0;
   int texrow = 0;
   float2 texatlasuv = float2(0,0);
   float fround = VegShadowColor.b;
   
   float texselectorV = min(fround,0.9375f);
   float2 texDdx = ddx(TexCoord*0.125f);
   float2 texDdy = ddy(TexCoord*0.125f);

   float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
   float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   float4 diffusemapA = DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;          

   texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
   finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   float4 diffusemapB = DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;          

   diffusemap += lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );    
}

void Atlas16DiffuseLookup( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap )
{            
   // vars
   //float fround = float(round(VegShadowColor.b*16.0)/16); // hard edge.
   float fround = VegShadowColor.b;
   int texcol = 0;
   int texrow = 0;
   float2 texatlasuv = float2(0,0);
   float texselectorV = min(fround,0.9375f);
   float2 texDdx = ddx(TexCoord*0.125f);
   float2 texDdy = ddy(TexCoord*0.125f);
   
   // secondary sample
   float secondarylayer = VegShadowColor.g;
   if ( secondarylayer > 0 )
   {
    float texsecondaryfactor = Atlas16GetUV(0,0,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap = lerp(diffusemap,DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy), secondarylayer);
   }
   float invsecondarylayer = 1.0f - secondarylayer;
   if(VegShadowColor.b >= 0.227 && VegShadowColor.b <= 0.283 ) {
      return;
   }

   // center sample
   float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
   float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   diffusemap = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
   
   // higher sample
   float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
   finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   diffusemap = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );

   // lower sample
   if ( texselectorV >= 0.0625f )
   {
    float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap = lerp(diffusemap, DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
   }
}
            
void Atlas16DiffuseNormalLookupCenter( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap, in out float4 normalmap, in float vDepth )
{
   // vars
   int texcol = 0;
   int texrow = 0;
   float2 texatlasuv = float2(0,0);
   float texselectorV = min(VegShadowColor.b,0.9375f);
   float2 texDdx = ddx(TexCoord*0.125f);
   float2 texDdy = ddy(TexCoord*0.125f);

   // center sample
   #ifdef IMPROVEDISTANCE
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapA = DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;
    normalmap += NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;
    texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapB = DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;
    diffusemap += lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );    
   #else
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap += DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;
    normalmap += NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) * texcenterfactor;
   #endif
}
       
void Atlas16DiffuseNormalLookup( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap, in out float4 normalmap, in float vDepth )
{
   // vars
   int texcol = 0;
   int texrow = 0;
   float2 texatlasuv = float2(0,0);
   float texselectorV = min(VegShadowColor.b,0.9375f);
   float2 texDdx = ddx(TexCoord*0.125f);
   float2 texDdy = ddy(TexCoord*0.125f);
   
   // secondary sample
   float secondarylayer = VegShadowColor.g;
   if ( secondarylayer > 0 )
   {
    float texsecondaryfactor = Atlas16GetUV(0,0,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap = lerp(diffusemap,DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy),secondarylayer);
    normalmap = lerp(normalmap,NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy),secondarylayer);
   }
   float invsecondarylayer = 1.0f - secondarylayer;
   if(VegShadowColor.b >= 0.227 && VegShadowColor.b <= 0.283 ) {
      return;
   }
   // IMPROVEDISTANCE: Use same normal texture in lerp, not visible at distance anyway.
   
   // center sample
   #ifdef IMPROVEDISTANCE
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapA = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , ( texcenterfactor * invsecondarylayer) );
    normalmap = lerp(normalmap, NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texcenterfactor* invsecondarylayer ) );
    texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapB = lerp( diffusemap , DiffuseSampler.Sample(SampleWrap,finaluv) , (texcenterfactor * invsecondarylayer) );   
    diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
   #else
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap = lerp(diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
    normalmap = lerp(normalmap , NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
   #endif

   // higher sample
   #ifdef IMPROVEDISTANCE
    float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemapA = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    normalmap = lerp(normalmap , NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemapB = lerp( diffusemap , DiffuseSampler.Sample(SampleWrap,finaluv) , (texhigherfactor * invsecondarylayer) );    
    diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
   #else
    float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    normalmap = lerp( normalmap , NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
   #endif

   // lower sample
   if ( texselectorV >= 0.0625f )
   {
    #ifdef IMPROVEDISTANCE
     float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     diffusemapA = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     normalmap = lerp( normalmap , NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     diffusemapB = lerp( diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );   
     diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
    #else
     float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     diffusemap = lerp(diffusemap , DiffuseSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     normalmap = lerp(normalmap , NormalMapSampler.SampleGrad(SampleWrap,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
    #endif
   }
}

float4 mainlightPS_highest(vertexOutput IN) : COLOR
{
   // clip       
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample ( SampleWrap, IN.TexCoord/500.0f);
             
   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);
   float4 rocknormalmap = float4(0,0,0,0);
   
   float3 Nn = normalize(IN.WorldNormal);

   // 12 samples. reduced to 3 samples. fading is done by slope anyway.
#ifdef FASTROCKTEXTURE                                   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ,rocknormalmap,IN.vDepth);   
#else
   float3 rockuv = float3(IN.WPos.x,IN.WPos.y,IN.WPos.z)/100.0f;
   float4 cXY = float4(0,0,0,0);
   float4 cYZ = float4(0,0,0,0);
   float4 nXY = float4(0,0,0,0);
   float4 nXZ = float4(0,0,0,0);
   float4 nYZ = float4(0,0,0,0);

   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.xy,cXY,nXY,IN.vDepth);   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.xz,cXZ,nXZ,IN.vDepth);   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.yz,cYZ,nYZ,IN.vDepth);   

   float mXY = pow(abs(Nn.z),6);
   float mXZ = pow(abs(Nn.y),2);
   float mYZ = pow(abs(Nn.x),6);
   float total = mXY + mXZ + mYZ;
   mXY /= total;
   mXZ /= total;
   mYZ /= total;
   rocknormalmap = nXY*mXY + nXZ *mXZ + nYZ*mYZ;
   cXZ = cXY*mXY + cXZ * mXZ + cYZ*mYZ;
#endif   
      
   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);

#ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {
#endif

      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;
     
     // atlas lookup for 16 textures
     float4 diffusemap = float4(0,0,0,0);
     float4 normalmap = float4(0,0,0,0);
     float4 diffusemap2 = float4(0,0,0,0);
     float4 normalmap2 = float4(0,0,0,0);

      float4 grass_d = float4(0,0,0,0);
      float4 grass_n = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 sand_n = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 mud_n = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); // 14   

#ifdef REMOVEGRASSNORMALS
      Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,IN.vDepth);
      grass_n = float4(0.5,0.5,1.0,1.0); // 126,128 , neutral normal.
#else
      Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,grass_n,IN.vDepth);
#endif

      diffusemap = grass_d;
      normalmap = grass_n;

      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d,sand_n,IN.vDepth); // 12
         Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d,mud_n,IN.vDepth); // 11
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         grass_n = lerp( mud_n ,grass_n, variation_d.r );                                             
         diffusemap = grass_d;
         normalmap = grass_n;
         normalmap = lerp(sand_n, normalmap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand normal
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }
              
      //add last hand drawed textures if exist.
       Atlas16DiffuseNormalLookup(VegShadowColor,IN.TexCoord,diffusemap,normalmap,IN.vDepth);

       // rock last.
       // should be moved up at some point so texture painting on rocks is possible.
       // this is only to make old levels look like they used to.
      normalmap = lerp(rocknormalmap, normalmap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock normal
      diffusemap = lerp(cXZ, diffusemap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

     // specular inside diffuse alpha
      float4 specmap = diffusemap.w;

      // tangent/binorm from vertex shader
      float3x3 tangentbasis = float3x3( 2*normalize(IN.Tn), 2*normalize(IN.Bn), Nn );

      // lighting
      float3 Nb = normalmap.xyz;
      Nb.xy = Nb.xy * 2.0 - 1.0;
      Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
      Nb = mul(Nb,tangentbasis);
      Nb = normalize(Nb);
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,Nb))+0.5),2),dot(Hn,Nb),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // some falloff to blend away as distance increases
	  //LEE060517 - this causes artifacts for specular in HIGHEST rendering
      //lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0); 

      // dynamic lighting
      //float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
	  float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);
      
      //LEE060517-deathvalleycanyonmessesup!float4 dynamicContrib = CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
//      float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  

      float4 dynamicContrib = float4( CalcExtLighting (IN.WorldNormal.xyz,IN.WPos.xyz,Vn,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz) , 1.0 );


     
      // flash light system (flash light control carried in SpotFlashColor.w )
      float conewidth = 24;
      float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
      float3 lightvector = Vn;
      float3 lightdir = float3(View._m02,View._m12,View._m22);
      float flashlight = pow(max( dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w * MAXFLASHLIGHT;   
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = (float4(bouncelightcolor,1) * (AmbiColor) * AmbiColorOverride) * 2;

 	  // Shadows
 	  int iCurrentCascadeIndex = 0;
	  float fShadow = GetShadow ( IN.vDepth, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
         
      // paint
      float fInvShadow = 1.0-fShadow;
      dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      float4 specContrib = lighting.z * specmap * SurfColor * GlobalSpecular;
	  
      // apply shadow mapping to final render
      diffuseContrib.xyz = diffuseContrib.xyz * fInvShadow;
      specContrib.xyz = specContrib.xyz * fInvShadow;
      float4 result = diffuseContrib + ambContrib + specContrib + dynamicContrib;
	  
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
      
      //calculate hud pixel-fog
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

#ifdef USEDISTBASEMAP

   }
   else // if ( viewspacePos.z > DistanceTransition.x )
   {
      // copied from Distant technique (any way to re-use code here!?)
      
      // read from pre-rendered mega texture of whole terrain

      float4 diffusemap = float4(0,0,0,0);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock
     
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
#endif   

   // final color
   #ifdef DEBUGSHADOW
    finalcolor = TintDebugShadow ( iCurrentCascadeIndex, finalcolor );
   #endif
   return finalcolor;
}

float4 mainlightPS_medium(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   float4 diffusemap = float4(0,0,0,0);
   float4 diffusemap2 = float4(0,0,0,0);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample(SampleWrap,IN.TexCoord/500.0f);
   
   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);

#ifdef FASTROCKTEXTURE                                   

#ifdef MEDIUMIMPROVEDISTANCE
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ,IN.vDepth);
#else
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ);
#endif

#else
   float3 Nn = normalize(IN.WorldNormal);
   float3 rockuv = float3(IN.WPos.x,IN.WPos.y,IN.WPos.z)/100.0f;
   float4 cXY = float4(0,0,0,0);
   float4 cYZ = float4(0,0,0,0);
   float4 nXY = float4(0,0,0,0);
   float4 nXZ = float4(0,0,0,0);
   float4 nYZ = float4(0,0,0,0);

#ifdef MEDIUMIMPROVEDISTANCE
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.xy,cXY,IN.vDepth);
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.xz,cXZ,IN.vDepth);
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.yz,cYZ,IN.vDepth);
#else
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.xy,cXY);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.xz,cXZ);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.yz,cYZ);
#endif
   float mXY = pow(abs(Nn.z),6);
   float mXZ = pow(abs(Nn.y),2);
   float mYZ = pow(abs(Nn.x),6);
   float total = mXY + mXZ + mYZ;
   mXY /= total;
   mXZ /= total;
   mYZ /= total;
   cXZ = cXY*mXY + cXZ * mXZ + cYZ*mYZ;
#endif   

   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);

#ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {
#endif
      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;

     // atlas lookup for 16 textures

      float4 grass_d = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); //14  
#ifdef MEDIUMIMPROVEDISTANCE
      Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,IN.vDepth); //13
#else
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d); //13
#endif
      diffusemap = grass_d;

#ifdef MEDIUMIMPROVEDISTANCE
      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d,IN.vDepth); // 12  
         Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d,IN.vDepth);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }
#else
      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d); // 12  
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }
#endif

      Atlas16DiffuseLookup(VegShadowColor,IN.TexCoord,diffusemap);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

//      diffusemap = lerp(diffusemap,diffusemap2,clamp((diffusemap2.r+diffusemap2.g+diffusemap2.b)*100.0,0.0,1.0) );

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit( pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
      
      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0); 

 	  // Shadows
 	  int iCurrentCascadeIndex = 0;
	  float fShadow = GetShadow ( IN.vDepth, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
      fShadow = fShadow * 0.675f * ShadowStrength;


	  // ensure cheap shadows fade out if camera too high (editor view)
	  fShadow = fShadow * max(0,1.0f-((eyePos.y-1200.0f)/3000.0f));
   
      // dynamic lighting
//      float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
	  float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);

      //float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  

      float4 dynamicContrib = float4( CalcExtLighting (IN.WorldNormal.xyz,IN.WPos.xyz,Vn,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz) , 1.0 );  


      // flash light system (flash light control carried in SpotFlashColor.w )
      float conewidth = 24;
      float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
      float3 lightvector = Vn;
      float3 lightdir = float3(View._m02,View._m12,View._m22);
      float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * (AmbiColor) * AmbiColorOverride * 2;
         
      // paint lighting in
      dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      float fInvShadow = 1.0-fShadow;
      //ambContrib.xyz = ambContrib.xyz * fInvShadow; // 291115 - ambient should not be affected by shadow!
      diffuseContrib.xyz = diffuseContrib.xyz * fInvShadow;
      float4 result = ambContrib + diffuseContrib + dynamicContrib;
      
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

#ifdef USEDISTBASEMAP
   }
   else //if ( viewspacePos.z > DistanceTransition.x )
   {
      // copied from Distant technique (any way to re-use code here!?)
      
      // read from pre-rendered mega texture of whole terrain
      diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
#endif   
   // final color
   return finalcolor;
}

float4 mainlightPS_lowest(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample(SampleWrap,IN.TexCoord/500.0f);

   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ);
         
   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);
   #ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {
   #endif
      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;
     
      // atlas lookup for 16 textures
      float4 diffusemap = float4(0,0,0,0);
      float4 diffusemap2 = float4(0,0,0,0);
      float4 grass_d = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); //14  
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d); //13
      diffusemap = grass_d;
      if( variation_d.a >= 0.98 ) 
	  {
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d); // 12  
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }       
      Atlas16DiffuseLookup(VegShadowColor,IN.TexCoord,diffusemap);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock
	  
      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit( pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
      
      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0);

      // cheap terrain shadow
      float fShadow = GetShadowCascade ( 7, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz) );
      fShadow = fShadow * 0.675f * ShadowStrength;
   
      // CHEAPEST flash light system (flash light control carried in SpotFlashColor.w )
      float flashlight = (1.0f-min(1,viewspacePos.z/300.0f)) * SpotFlashColor.w;   
      float4 dynamicContrib = diffusemap*float4(flashlight,flashlight,flashlight,1);
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
         
      // paint lighting in
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      float fInvShadow = 1.0-fShadow;
      diffuseContrib.xyz = diffuseContrib.xyz * fInvShadow;
      float4 result = ambContrib + diffuseContrib + dynamicContrib;
      
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
	  	  
      //calculate hud pixel-fog
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

   #ifdef USEDISTBASEMAP
   }
   else //if ( viewspacePos.z > DistanceTransition.x )
   {
      // copied from Distant technique (any way to re-use code here!?)
      
      // read from pre-rendered mega texture of whole terrain
      float4 diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit( pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
   #endif   
   
   // final color
   return float4(finalcolor.xyz,1);
}

float4 mainlightPS_highest_prebake(vertexOutput IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample(SampleWrap,IN.TexCoord/500.0f);
   
   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);
   float4 rocknormalmap = float4(0,0,0,0);
   float3 Nn = normalize(IN.WorldNormal);

#ifdef FASTROCKTEXTURE                                   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ,rocknormalmap,IN.vDepth);   
#else
   float3 rockuv = float3(IN.WPos.x,IN.WPos.y,IN.WPos.z)/100.0f;
   float4 cXY = float4(0,0,0,0);
   float4 cYZ = float4(0,0,0,0);
   float4 nXY = float4(0,0,0,0);
   float4 nXZ = float4(0,0,0,0);
   float4 nYZ = float4(0,0,0,0);

   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.xy,cXY,nXY,IN.vDepth);   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.xz,cXZ,nXZ,IN.vDepth);   
   Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*15,0),rockuv.yz,cYZ,nYZ,IN.vDepth);   

   float mXY = pow(abs(Nn.z),6);
   float mXZ = pow(abs(Nn.y),2);
   float mYZ = pow(abs(Nn.x),6);
   float total = mXY + mXZ + mYZ;
   mXY /= total;
   mXZ /= total;
   mYZ /= total;
   rocknormalmap = nXY*mXY + nXZ *mXZ + nYZ*mYZ;
   cXZ = cXY*mXY + cXZ * mXZ + cYZ*mYZ;
#endif   
      
   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);

#ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {   
#endif
      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;
     
     // atlas lookup for 16 textures
     float4 diffusemap = float4(0,0,0,0);
     float4 normalmap = float4(0,0,0,0);
     float4 diffusemap2 = float4(0,0,0,0);
     float4 normalmap2 = float4(0,0,0,0);

      float4 grass_d = float4(0,0,0,0);
      float4 grass_n = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 sand_n = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 mud_n = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);
      
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); // 14   

#ifdef REMOVEGRASSNORMALS
      Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,IN.vDepth);
      grass_n = float4(0.5,0.5,1.0,1.0); // 126,128 , neutral normal.
#else
      Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,grass_n,IN.vDepth);
#endif

      diffusemap = grass_d;
      normalmap = grass_n;

      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d,sand_n,IN.vDepth); // 12
         Atlas16DiffuseNormalLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d,mud_n,IN.vDepth); // 11
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         grass_n = lerp( mud_n ,grass_n, variation_d.r );                                             
         diffusemap = grass_d;
         normalmap = grass_n;
         normalmap = lerp(sand_n, normalmap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand normal
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }


      //add last
       Atlas16DiffuseNormalLookup(VegShadowColor,IN.TexCoord,diffusemap,normalmap,IN.vDepth);

      // merge rock texture on deep slopes   
      // merge rock normal on deep slopes   
      normalmap = lerp(rocknormalmap, normalmap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock normal
      diffusemap = lerp(cXZ, diffusemap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

//       diffusemap = lerp(diffusemap,diffusemap2,clamp((diffusemap2.r+diffusemap2.g+diffusemap2.b)*100.0,0.0,1.0) );

      // specular stored in diffuse alpha
      float4 specmap = diffusemap.w;
     
      // get tangent/binorm from vertex shader
      float3x3 tangentbasis = float3x3( 2*normalize(IN.Tn), 2*normalize(IN.Bn), Nn );

      // lighting
      float3 Nb = normalmap.xyz;
      Nb.xy = Nb.xy * 2.0 - 1.0;
      Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
      Nb = mul(Nb,tangentbasis);
      Nb = normalize(Nb);
     
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,Nb))+0.5),2),dot(Hn,Nb),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0); 

      // dynamic lighting
//      float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
	  float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);

      //LEE060517-deathvalleycanyonmessesup!CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  potflashlighting;  
      //float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  

      float4 dynamicContrib = float4( CalcExtLighting (IN.WorldNormal.xyz,IN.WPos.xyz,Vn,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz) , 1.0 );


      // flash light system (flash light control carried in SpotFlashColor.w )
      float conewidth = 24;
      float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
      float3 lightvector = Vn;
      float3 lightdir = float3(View._m02,View._m12,View._m22);
      float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;
      dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = (float4(bouncelightcolor,1) * (AmbiColor) * AmbiColorOverride) * 2;
      
      // paint
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      float4 specContrib = lighting.z * specmap * SurfColor * GlobalSpecular;
      
      // apply shadow mapping to final render
      float4 result = diffuseContrib + ambContrib + specContrib + dynamicContrib;
            
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

#ifdef USEDISTBASEMAP
   }
   else //if ( viewspacePos.z > DistanceTransition.x )
   {
      // read from pre-rendered mega texture of whole terrain
      float4 diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
#endif
   
   // final color
   return finalcolor;
}

float4 mainlightPS_medium_prebake(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample(SampleWrap,IN.TexCoord/500.0f);
   
   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);

#ifdef FASTROCKTEXTURE                                   

#ifdef MEDIUMIMPROVEDISTANCE
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ,IN.vDepth);
#else
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ);
#endif

#else
   float3 Nn = normalize(IN.WorldNormal);
   float3 rockuv = float3(IN.WPos.x,IN.WPos.y,IN.WPos.z)/100.0f;
   float4 cXY = float4(0,0,0,0);
   float4 cYZ = float4(0,0,0,0);
   float4 nXY = float4(0,0,0,0);
   float4 nXZ = float4(0,0,0,0);
   float4 nYZ = float4(0,0,0,0);

#ifdef MEDIUMIMPROVEDISTANCE
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.xy,cXY,IN.vDepth);
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.xz,cXZ,IN.vDepth);
   Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*15,0),rockuv.yz,cYZ,IN.vDepth);
#else
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.xy,cXY);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.xz,cXZ);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),rockuv.yz,cYZ);
#endif
   float mXY = pow(abs(Nn.z),6);
   float mXZ = pow(abs(Nn.y),2);
   float mYZ = pow(abs(Nn.x),6);
   float total = mXY + mXZ + mYZ;
   mXY /= total;
   mXZ /= total;
   mYZ /= total;
   cXZ = cXY*mXY + cXZ * mXZ + cYZ*mYZ;
#endif   
      
   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);

#ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {  
#endif
      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;
     
     // atlas lookup for 16 textures
      float4 diffusemap = float4(0,0,0,0);
      float4 diffusemap2 = float4(0,0,0,0);
      float4 grass_d = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);

      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); //14  
#ifdef MEDIUMIMPROVEDISTANCE
      Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d,IN.vDepth); //13
#else
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d); //13
#endif
      diffusemap = grass_d;

#ifdef MEDIUMIMPROVEDISTANCE
      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d,IN.vDepth); // 12  
         Atlas16DiffuseLookupCenterDist(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d,IN.vDepth);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }
#else
      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d); // 12  
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }
#endif


      Atlas16DiffuseLookup(VegShadowColor,IN.TexCoord,diffusemap);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

//      diffusemap = lerp(diffusemap,diffusemap2,clamp((diffusemap2.r+diffusemap2.g+diffusemap2.b)*100.0,0.0,1.0) );

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
      
      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0); 

      // dynamic lighting
//      float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
	  float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);

      //float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;

      float4 dynamicContrib = float4( CalcExtLighting (IN.WorldNormal.xyz,IN.WPos.xyz,Vn,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz), 1.0 );  

      
      // flash light system (flash light control carried in SpotFlashColor.w )
      float conewidth = 24;
      float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
      float3 lightvector = Vn;
      float3 lightdir = float3(View._m02,View._m12,View._m22);
      float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
      dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * (AmbiColor) * AmbiColorOverride * 2;
         
      // paint lighting in
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      float4 result = ambContrib + diffuseContrib + dynamicContrib;
      
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

#ifdef USEDISTBASEMAP
   }
   else //if ( viewspacePos.z > DistanceTransition.x )
   {
      // read from pre-rendered mega texture of whole terrain
      float4 diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // merge rock texture on deep slopes       
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
#endif   
   // final color
   return finalcolor;
}

float4 mainlightPS_lowest_prebake(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = VegShadowSampler.Sample(SampleWrap,IN.TexCoord/500.0f);

   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ);
        
   // transition to Very Low Distant technique
   float4 finalcolor = float4(0,0,0,0);

#ifdef USEDISTBASEMAP
   if ( viewspacePos.z < DistanceTransition.y )
   {
#endif
      // texture selection
      float fShadowFromNormal = IN.WorldNormal.y;
     
      // atlas lookup for 16 textures
      float4 diffusemap = float4(0,0,0,0);
      float4 diffusemap2 = float4(0,0,0,0);
      float4 grass_d = float4(0,0,0,0);
      float4 sand_d = float4(0,0,0,0);
      float4 mud_d = float4(0,0,0,0);
      float4 variation_d = float4(0,0,0,0);
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*14,0),IN.TexCoord/16.0,variation_d); //14  
      Atlas16DiffuseLookupCenter(float4(0,0,0.0625*4,0),IN.TexCoord,grass_d); //13
      diffusemap = grass_d;

      if( variation_d.a >= 0.98 ) {
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*1,0),IN.TexCoord,sand_d); // 12  
         Atlas16DiffuseLookupCenter(float4(0,0,0.0625*9,0),IN.TexCoord,mud_d);  // 11 
         grass_d = lerp( mud_d ,grass_d, variation_d.r );                                             
         diffusemap = grass_d;
         diffusemap = lerp(sand_d, diffusemap, clamp( ( (IN.WPos.y-520.0f)/40.0f) , 0.0, 1.0) ); // sand
      }

      Atlas16DiffuseLookup(VegShadowColor,IN.TexCoord,diffusemap);
      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

//      diffusemap = lerp(diffusemap,diffusemap2,clamp((diffusemap2.r+diffusemap2.g+diffusemap2.b)*100.0,0.0,1.0) );

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos).xyz;  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
      
      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0); 

      // CHEAPEST flash light system (flash light control carried in SpotFlashColor.w )
      float flashlight = (1.0f-min(1,viewspacePos.z/300.0f)) * SpotFlashColor.w;   
      
      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
         
      // paint lighting in
      float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
      diffuseContrib.xyz = diffuseContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
      float4 result = ambContrib + diffuseContrib;
      
      // lime green tint to show where grass is being painted (editor control)
      float fVeg = VegShadowColor.r;
      result.xyz = result.xyz + float3(HighlightParams.y/8.0f,HighlightParams.y/2.0,HighlightParams.y/8.0f) * fVeg;
      
      // highlighter stage : HighlightCursor
      // radius of 500=50.0f and radius of 50.0f=500.0f
      // radius = 500 = 9.75f
      // radius = 50 = 0.975f
      // radius = 5 = 0.0975f
      // radius = 1 = 0.0195f
      float highlightsize = (1.0f/HighlightCursor.z)*25600.0f;
      float2 highlightuv = (((IN.TexCoord/500.0f)-float2(0.5f,0.5f))*highlightsize) + float2(0.5f,0.5f) - (HighlightCursor.xy/(HighlightCursor.z*0.0195));
      float4 highlighttex = HighlighterSampler.SampleLevel(SampleClamp,highlightuv,0);
      float highlightalpha = (highlighttex.a*0.5f);
      result.xyz = result.xyz + (HighlightParams.x*float3(highlightalpha*HighlightParams.z,highlightalpha*HighlightParams.a,0));
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      finalcolor = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);

#ifdef USEDISTBASEMAP
   }
   else //if ( viewspacePos.z > DistanceTransition.x )
   {
      // copied from Distant technique (any way to re-use code here!?)
      
      // read from pre-rendered mega texture of whole terrain
      float4 diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

      // cheapest directional lighting
      float3 Ln = normalize(IN.LightVec);
      float3 V  = (eyePos - IN.WPos);  
      float3 Vn  = normalize(V); 
      float3 Hn = normalize(Vn+Ln);
      float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

      // merge rock texture on deep slopes   
      diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

      // spherical ambience
      float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
      float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio) * diffusemap.xyz * 0.8;
      bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
      float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
      // paint lighting in
      float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
      float4 result = ambContrib + diffuseContrib;
      
      //calculate hud pixel-fog
      //float4 cameraPos = mul(IN.WPos, View);
      float hudfogfactor = saturate((viewspacePos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
      float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
      // combine for transition effect
      finalcolor = lerp ( finalcolor, cheaphudfogresult, min((viewspacePos.z-DistanceTransition.x)/DistanceTransition.z,1.0f) );
   }
#endif   
   // final color
   return finalcolor;
}

float4 mainlightPS_distant(vertexOutput_low IN) : COLOR
{
   // clip
   clip(IN.clip);

   // read from pre-rendered mega texture of whole terrain
   float4 diffusemap = float4(HighlighterSampler.Sample(SampleClamp,IN.TexCoord/500.0f).xyz,1);

   // atlas lookup for rock texture
   float4 cXZ = float4(0,0,0,0);
   Atlas16DiffuseLookupCenter(float4(0,0,0.0625*15,0),IN.TexCoord,cXZ);
   
   // cheapest directional lighting
   float3 Ln = normalize(IN.LightVec);
   float3 V  = (eyePos - IN.WPos).xyz;  
   float3 Vn  = normalize(V); 
   float3 Hn = normalize(Vn+Ln);
   float4 lighting = lit(pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
   lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

   // merge rock texture on deep slopes   
   diffusemap = lerp(cXZ, diffusemap, clamp((IN.WorldNormal.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor,SkyColor,fSkyFloorRatio).xyz * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
   float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;
      
   // paint lighting in
   float4 diffuseContrib = SurfColor * (diffusemap * lighting.y * GlobalSurfaceIntensity);
   float4 result = ambContrib + diffuseContrib;
      
   //calculate hud pixel-fog
   float4 cameraPos = mul(IN.WPos, View);
   float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   float4 cheaphudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
      
   // cheapest chips!
   return cheaphudfogresult;
}

float4 blackPS(vertexOutput_low IN) : COLOR
{
   clip(IN.clip);
   return float4(0,0,0,1);
}

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_highest()));
        SetGeometryShader(NULL);
    }
}

technique11 High
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_highest()));
        SetGeometryShader(NULL);
   }
}

technique11 Medium
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_medium()));
        SetGeometryShader(NULL);
   }
}

technique11 Lowest
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_lowest()));
        SetGeometryShader(NULL);
   }
}

technique11 Highest_Prebake
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_highest_prebake()));
        SetGeometryShader(NULL);
   }
}

technique11 High_Prebake
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_highest_prebake()));
        SetGeometryShader(NULL);
   }
}

technique11 Medium_Prebake
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_medium_prebake()));
        SetGeometryShader(NULL);
   }
}

technique11 Lowest_Prebake
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_lowest_prebake()));
        SetGeometryShader(NULL);
   }
}

technique11 Distant
{
   pass MainPass
   {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_lowest()));
        SetGeometryShader(NULL);
   }
}

technique11 DepthMap
{
    pass p0
    {      
      SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
      SetPixelShader(NULL);
      SetGeometryShader(NULL);
      ///CullMode = NONE;
   }
}

technique11 ReflectedOnly
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
		#ifdef WATERREFLECTIONMEDIUM
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_medium()));
		#else
        SetPixelShader(CompileShader(ps_5_0, mainlightPS_distant()));
		#endif
        SetGeometryShader(NULL);
    }
}

technique11 blacktextured
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}
#endif
