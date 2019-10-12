string Description = "Terrain Shader";                          
#include "settings.fx"                                                                                   

// shadow mapping
matrix          m_mShadow;
float4          m_vCascadeOffset[8];
float4          m_vCascadeScale[8];
float           m_fCascadeBlendArea;
float           m_fTexelSize; 
float           m_fCascadeFrustumsEyeSpaceDepths[8];

float ShadowStrength
<    string UIName =  "ShadowStrength";    
> = {1.0f};

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

// regular shader constants   
float4x4 World : World;
float4x4 WorldInverse : WorldInverse;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 View : View;
float4x4 ViewInverse : ViewInverse;
float4x4 ViewIT : ViewInverseTranspose;
float4x4 ViewProjection : ViewProjection;
float4x4 Projection : Projection;
float4 eyePos : CameraPosition;

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

float4 LightSource
<   string UIType = "Fixed Light Direction";
> = {-1.0f, -1.0f, -1.0f, 1.0f};

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

// RENDERCOLORTARGET simply indicates that we are using an RT (depth texture at bottom)
texture VegShadowMap : RENDERCOLORTARGET
<
    string Name = "D.tga";
    string type = "2D";
>;
texture DynTerShaMap : DiffuseMap
<
    string Name = "D.tga";
    string type = "2D";
>;
texture DiffuseMap : DiffuseMap
<
    string Name = "D.tga";
    string type = "2D";
>;
texture HighlighterMap : DiffuseMap
<
    string Name = "D.tga";
    string type = "2D";
>;
texture NormalMap : DiffuseMap
<
    string Name = "N.tga";
    string type = "2D";
>;

texture DepthMapTX1 : DiffuseMap
<
    string Name = "DEPTH1.tga";
    string type = "2D";
>;
texture DepthMapTX2 : DiffuseMap
<
    string Name = "DEPTH1.tga";
    string type = "2D";
>;
texture DepthMapTX3 : DiffuseMap
<
    string Name = "DEPTH1.tga";
    string type = "2D";
>;
texture DepthMapTX4 : DiffuseMap
<
    string Name = "DEPTH1.tga";
    string type = "2D";
>;

sampler2D VegShadowSampler = sampler_state
{
    Texture   = <VegShadowMap>;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D DynTerShaSampler = sampler_state
{
    Texture   = <DynTerShaMap>;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
sampler2D DiffuseSampler = sampler_state
{
    Texture   = <DiffuseMap>;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
sampler2D HighlighterSampler = sampler_state
{
    Texture   = <HighlighterMap>;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
sampler2D NormalMapSampler = sampler_state
{
    Texture   = <NormalMap>;
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

sampler2D DepthMap1 = sampler_state
{
	Texture = <DepthMapTX1>;   
	Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
sampler2D DepthMap2 = sampler_state
{
	Texture = <DepthMapTX2>;   
	Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
sampler2D DepthMap3 = sampler_state
{
	Texture = <DepthMapTX3>;   
	Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
sampler2D DepthMap4 = sampler_state
{
	Texture = <DepthMapTX4>;   
	Filter = MIN_MAG_MIP_POINT;
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

/*data passed to pixel shader*/
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
};
struct vertexOutputdepth
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float2 TexCoordLM   : TEXCOORD1;
    float3 LightVec       : TEXCOORD2;
    float3 WorldNormal   : TEXCOORD3;
    float4 WPos         : TEXCOORD4;  
    float  clip         : TEXCOORD6;
    float  vDepth       : TEXCOORD7;
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
};

/****** helper functions for shadow mapping*****/

void ComputeCoordinatesTransform( in int iCascadeIndex,
                                      in float4 InterpolatedPosition ,
                                      in out float4 vShadowTexCoord ) 
{
    float4 vLightDot = mul ( InterpolatedPosition, m_mShadow );
    vLightDot *= m_vCascadeScale[iCascadeIndex];
    vLightDot += m_vCascadeOffset[iCascadeIndex];
    vShadowTexCoord.xyz = vLightDot.xyz;
} 
float texture2DCompare(sampler2D depths, float2 uv, float compare)
{
    float depth = tex2D(depths, uv).r;
    return step(compare, depth);
}
float PCF5x5(sampler2D depths, float2 size, float2 uv, float compare)
{
    float result = 0.0;
    for(int x=-2; x<=2; x++){
        for(int y=-2; y<=2; y++){
            float2 off = float2(x,y)/size;
            result += texture2DCompare(depths, uv+off, compare);
        }
    }
    return result/25.0;
}
float PCF5x3(sampler2D depths, float2 size, float2 uv, float compare)
{
    float result = 0.0;
    for(int x=-1; x<=1; x++){
        for(int y=-2; y<=2; y++){
            float2 off = float2(x,y)/size;
            result += texture2DCompare(depths, uv+off, compare);
        }
    }
    return result/15.0;
}
float PCF3x3(sampler2D depths, float2 size, float2 uv, float compare)
{
	// DX11 CANNOT have gradients in DX11 loops related to flow control
    float result = 0.0;
    //for(int x=-1; x<=1; x++){
    //    for(int y=-1; y<=1; y++){
    //        float2 off = float2(x,y)/size;
    //        result += texture2DCompare(depths, uv+off, compare);
    //    }
    //}
    result = texture2DCompare(depths, uv, compare) * 9.0f;
    return result/9.0;
}
void CalculatePCFPercentLit_highest ( in int iCurrentCascadeIndex,
                             in float4 vShadowTexCoord, 
                             in float2 mwpos,
                             out float fPercentLit ) 
{
   // Use PCF to sample the depth map and return a percent lit value.
   fPercentLit = 1.0f;
   /*
   fPercentLit = 0.0f;
   float fTS = 1.0f/1024.0f;//m_fTexelSize;
   float2 tsize = float2( SHADOWTEXELX ,SHADOWTEXELY );
   if ( iCurrentCascadeIndex==0 )
   {
      #ifdef DEBUGSHADOW
         fPercentLit += vShadowTexCoord.z > tex2D(DepthMap1,float2(vShadowTexCoord.x,vShadowTexCoord.y) ).x ? 1.0f : 0.0f;
      #else    
       #ifdef BETTERSHADOWS
          fPercentLit = 1.0-PCF3x3(DepthMap1, tsize, vShadowTexCoord.xy , vShadowTexCoord.z );
       #else
          // test dither with random offsets.
          float2 o = fmod(floor(mwpos), 1.50) * 0.25;
          fPercentLit += vShadowTexCoord.z > tex2D(DepthMap1,float2(vShadowTexCoord.x,vShadowTexCoord.y)+(  (float2(-0.75, 0.75) + o) *fTS)  ).x ? 1.0f : 0.0f;
          fPercentLit += vShadowTexCoord.z > tex2D(DepthMap1,float2(vShadowTexCoord.x,vShadowTexCoord.y)+(  (float2( 0.25, 0.75) + o) *fTS)  ).x ? 1.0f : 0.0f;
          fPercentLit += vShadowTexCoord.z > tex2D(DepthMap1,float2(vShadowTexCoord.x,vShadowTexCoord.y)+(  (float2(-0.75,-0.25) + o) *fTS)  ).x ? 1.0f : 0.0f;
          fPercentLit += vShadowTexCoord.z > tex2D(DepthMap1,float2(vShadowTexCoord.x,vShadowTexCoord.y)+(  (float2( 0.75,-0.75) + o) *fTS)  ).x ? 1.0f : 0.0f;
          fPercentLit *= 0.25;
       #endif
      #endif
   }
   else if ( iCurrentCascadeIndex==1 )
   {
      #ifdef DEBUGSHADOW
         fPercentLit += vShadowTexCoord.z > tex2D(DepthMap2,float2(vShadowTexCoord.x,vShadowTexCoord.y) ).x ? 1.0f : 0.0f;
      #else    
         // test dither with random offsets.
         float2 o = fmod(floor(mwpos), 1.50) * 0.25;
         fPercentLit += vShadowTexCoord.z > tex2D(DepthMap2,float2(vShadowTexCoord.x,vShadowTexCoord.y)+( (float2( 0.25, 0.75) + o) *fTS)).x ? 1.0f : 0.0f;
         fPercentLit += vShadowTexCoord.z > tex2D(DepthMap2,float2(vShadowTexCoord.x,vShadowTexCoord.y)+( (float2(-0.75,-0.25) + o) *fTS)).x ? 1.0f : 0.0f;
         fPercentLit *= 0.5;
      #endif
   }
   else if ( iCurrentCascadeIndex==2 )
   {
      #ifdef DEBUGSHADOW
       fPercentLit += vShadowTexCoord.z > tex2D(DepthMap3,float2(vShadowTexCoord.x,vShadowTexCoord.y) ).x ? 1.0f : 0.0f;
      #else    
       fPercentLit += vShadowTexCoord.z > tex2D(DepthMap3,float2(vShadowTexCoord.x,vShadowTexCoord.y)).x ? 1.0f : 0.0f;
      #endif
   }
   else if ( iCurrentCascadeIndex==3 && vShadowTexCoord.z<1.0 )
   {
      #ifdef DEBUGSHADOW
         fPercentLit += vShadowTexCoord.z > tex2D(DepthMap4,float2(vShadowTexCoord.x,vShadowTexCoord.y) ).x ? 1.0f : 0.0f;
      #else    
       fPercentLit += vShadowTexCoord.z > tex2D(DepthMap4,float2(vShadowTexCoord.x,vShadowTexCoord.y)).x ? 1.0f : 0.0f;
      #endif
   }
   */
}

void CalculateBlendAmountForInterval ( in int iCurrentCascadeIndex, 
                                       in out float fPixelDepth, 
                                       in out float fCurrentPixelsBlendBandLocation,
                                       out float fBlendBetweenCascadesAmount ) 
{
   // Calculate amount to blend between two cascades and the band where blending will occure.
   // We need to calculate the band of the current shadow map where it will fade into the next cascade.
   // We can then early out of the expensive PCF for loop. 
   float fBlendInterval = m_fCascadeFrustumsEyeSpaceDepths[ iCurrentCascadeIndex ];
   
   int fBlendIntervalbelowIndex = min(0, iCurrentCascadeIndex-1);
   if ( fBlendIntervalbelowIndex>1 )
   {
      fPixelDepth -= m_fCascadeFrustumsEyeSpaceDepths[ fBlendIntervalbelowIndex ];
      fBlendInterval -= m_fCascadeFrustumsEyeSpaceDepths[ fBlendIntervalbelowIndex ];
   }
   
   // The current pixel's blend band location will be used to determine when we need to blend and by how much.
   fCurrentPixelsBlendBandLocation = 1.0f - (fPixelDepth / fBlendInterval);

   // The fBlendBetweenCascadesAmount is our location in the blend band.
   fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / m_fCascadeBlendArea;
}

/*******Vertex Shader***************************/

vertexOutput mainVS_highest(appdata IN)   
{
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,0), World);
    OUT.WPos = worldSpacePos;  

    OUT.LightVec = normalize(LightSource.xyz);
    OUT.WorldNormal = normalize(mul(float4(IN.Normal,0), WorldIT).xyz);
    OUT.Position = mul(float4(IN.Position,0), WorldViewProjection);
    float2 tc = IN.UV0 * 500.0f; 
    OUT.TexCoord  = IN.UV0 * 500.0f; 
    OUT.TexCoordLM  = IN.UV1 / 2.5f; 
     
    // all shaders should send the clip value to the pixel shader (for refr/refl)                                                                     
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      

    // SHADOW MAPPING - world position and projected depth (for cascade distance calc)
    // OUT.vInterpPos = mul ( IN.Position, World );   - see OUT.WPos above!
//    OUT.vDepth = mul( IN.Position, WorldViewProjection ).z; 
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
   
    if(OUT.Position.z > 3400.0 && OUT.WPos.y < 460.0 && OUT.WorldNormal.y > 0.9985 ) {
      OUT.clip=-1.0;
      OUT.Position.z = 100000.0;
      OUT.Position.x = 100000.0;
      OUT.Position.y = 100000.0;
      OUT.Position.w = 0.0;
    }
    return OUT;
}

vertexOutput_low mainVS_lowest(appdata IN)   
{
    vertexOutput_low OUT;
    OUT.Position = mul(float4(IN.Position,0), WorldViewProjection);
    OUT.vDepth = OUT.Position.z; 
    OUT.LightVec = normalize(LightSource.xyz);
    OUT.WorldNormal = normalize(mul(float4(IN.Normal,0), WorldIT).xyz);
    OUT.TexCoord  = IN.UV0 * 500.0f; 
    float4 worldSpacePos = mul(float4(IN.Position,0), World);
    OUT.WPos = worldSpacePos;  
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      

    if(OUT.Position.z > 3400.0 && OUT.WPos.y < 460.0 && OUT.WorldNormal.y > 0.9985 ) 
    {
      OUT.clip=-1.0;
      OUT.Position.z = 100000.0;
      OUT.Position.x = 100000.0;
      OUT.Position.y = 100000.0;
      OUT.Position.w = 0.0;
    }
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

float4 CalcLighting(float3 Nb, float3 worldPos, float3 Vn, float4 diffusemap,float4 specmap)
{
    float4 output = (float4)0.0;
    #ifdef VERSION114
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
   diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;          
}

void Atlas16DiffuseLookupCenterDist( in float4 VegShadowColor, in float2 TexCoord, in out float4 diffusemap, in float vDepth )
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

   float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
   float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   float4 diffusemapA = tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;          

   texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
   finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   float4 diffusemapB = tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;          

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
    //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * secondarylayer;
    diffusemap = lerp(diffusemap,tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy), secondarylayer);
   }
   float invsecondarylayer = 1.0f - secondarylayer;
   if(VegShadowColor.b >= 0.227 && VegShadowColor.b <= 0.283 ) {
      return;
   }

   // center sample
   float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
   float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor * invsecondarylayer;
   diffusemap = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
   
   // higher sample
   float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
   finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
   //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texhigherfactor * invsecondarylayer;
   diffusemap = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );

   // lower sample
   if ( texselectorV >= 0.0625f )
   {
    float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texlowerfactor * invsecondarylayer;
    diffusemap = lerp(diffusemap, tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
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
    float4 diffusemapA = tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;
    normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * texcenterfactor;
    texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapB = tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;
    diffusemap += lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );    
   #else
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor;
    normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * texcenterfactor;
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
    //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * secondarylayer;
    //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * secondarylayer;
    diffusemap = lerp(diffusemap,tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy),secondarylayer);
    normalmap = lerp(normalmap,tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy),secondarylayer);
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
    float4 diffusemapA = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , ( texcenterfactor * invsecondarylayer) );
    //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * texcenterfactor * invsecondarylayer;
    normalmap = lerp(normalmap, tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texcenterfactor* invsecondarylayer ) );
    texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    float4 diffusemapB = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );   
    diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
    //diffusemap = lerp( diffusemap ,  lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) )  , (texcenterfactor* invsecondarylayer ) );  //* invsecondarylayer
   #else
    float texcenterfactor = Atlas16GetUV(texselectorV,texselectorV,TexCoord,texatlasuv,texcol,texrow);
    float2 finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texcenterfactor * invsecondarylayer;
    diffusemap = lerp(diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
    //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy).xyz * texcenterfactor * invsecondarylayer;
    normalmap = lerp(normalmap , tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texcenterfactor * invsecondarylayer) );
   #endif

   // higher sample
   #ifdef IMPROVEDISTANCE
    float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemapA = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * texhigherfactor * invsecondarylayer;
    normalmap = lerp(normalmap , tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    diffusemapB = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );    
    diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
//    diffusemap = lerp( diffusemap , lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) ) , (texhigherfactor * invsecondarylayer) );    
   #else
    float texhigherfactor = Atlas16GetUV((texselectorV+0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
    finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
    //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texhigherfactor * invsecondarylayer;
    diffusemap = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
    //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy).xyz * texhigherfactor * invsecondarylayer;   
    normalmap = lerp( normalmap , tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texhigherfactor * invsecondarylayer) );
   #endif

   // lower sample
   if ( texselectorV >= 0.0625f )
   {
    #ifdef IMPROVEDISTANCE
     float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     diffusemapA = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) * texlowerfactor * invsecondarylayer;
     normalmap = lerp( normalmap , tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord/5.0,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     diffusemapB = lerp( diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );   
     diffusemap = lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) );
//     diffusemap = lerp( diffusemap , lerp( diffusemapA, diffusemapB, clamp(vDepth/1200.0,0.00,0.50) ) , (texlowerfactor * invsecondarylayer) );    
    #else
     float texlowerfactor = Atlas16GetUV((texselectorV-0.0625f),texselectorV,TexCoord,texatlasuv,texcol,texrow);
     finaluv = float2(texatlasuv+float2(texcol*0.25f,texrow*0.25f));
     //diffusemap += tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) * texlowerfactor * invsecondarylayer;
     diffusemap = lerp(diffusemap , tex2Dgrad(DiffuseSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
     //normalmap += tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy).xyz * texlowerfactor * invsecondarylayer;   
     normalmap = lerp(normalmap , tex2Dgrad(NormalMapSampler,finaluv,texDdx,texDdy) , (texlowerfactor * invsecondarylayer) );
    #endif
   }
}

float4 mainlightPS_highest(vertexOutput IN) : COLOR
{   
   // clip       
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);
             
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
      
   // moved here for debugging
   int iCurrentCascadeIndex=3;
    
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
              
      // merge rock texture on deep slopes   
      // merge rock normal on deep slopes   
      //float3 Nn = normalize(IN.WorldNormal);

//      normalmap = lerp(rocknormalmap, normalmap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock normal
//      diffusemap = lerp(cXZ, diffusemap, clamp((Nn.y- TERRAINROCKSLOPE )*2.5, 0.0, 1.0) ); // rock

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
      float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
      //LEE060517-deathvalleycanyonmessesup!float4 dynamicContrib = CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
      float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
     
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
      float4 ambContrib = (float4(bouncelightcolor,1) * (AmbiColor) * AmbiColorOverride) * 2;

      // shadow mapping code
      float4 vShadowMapTextureCoord = 0.0f;
      float fShadow = 0.0f;
      if ( ShadowStrength > 0.0f )
      {
         float4 vShadowMapTextureCoord_blend = 0.0f;
         float fPercentLit = 0.0f;
         float fPercentLit_blend = 0.0f;

         // The interval based selection technique compares the pixel's depth against the frustum's cascade divisions.
         float fCurrentPixelDepth;
         fCurrentPixelDepth = IN.vDepth;
         if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[2] ) iCurrentCascadeIndex = 2;
         if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[1] ) iCurrentCascadeIndex = 1;
         if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[0] ) iCurrentCascadeIndex = 0;

         // Repeat text coord calculations for the next cascade - the next cascade index is used for blurring between maps.
         int iNextCascadeIndex = 1;
         iNextCascadeIndex = min ( 4 - 1, iCurrentCascadeIndex + 1 ); 
         float fBlendBetweenCascadesAmount = 1.0f;
         float fCurrentPixelsBlendBandLocation = 1.0f;
         CalculateBlendAmountForInterval ( iCurrentCascadeIndex, fCurrentPixelDepth, 
         fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
            
         // World out texture coordinate into specified shadow map
         float4 finalwpos = IN.WPos;
         ComputeCoordinatesTransform( iCurrentCascadeIndex, finalwpos, vShadowMapTextureCoord );    

         // work out how much shadow
         CalculatePCFPercentLit_highest ( iCurrentCascadeIndex, vShadowMapTextureCoord, finalwpos.xz, fShadow );
                            
         if( fCurrentPixelsBlendBandLocation < m_fCascadeBlendArea ) 
         {  
            // the current pixel is within the blend band.
            // Repeat text coord calculations for the next cascade. 
            // The next cascade index is used for blurring between maps.
            ComputeCoordinatesTransform( iNextCascadeIndex, finalwpos, vShadowMapTextureCoord_blend );  
         
            // the current pixel is within the blend band.
            CalculatePCFPercentLit_highest ( iNextCascadeIndex, vShadowMapTextureCoord_blend, finalwpos.xz, fPercentLit_blend );
                              
            // Blend the two calculated shadows by the blend amount.
            fShadow = lerp( fPercentLit_blend, fShadow, fBlendBetweenCascadesAmount ); 
         }
         
         // extra shadow term comes from veg map (can paint and pre-bake this one)
         //fShadow = fShadow + (tex2D(VegShadowSampler,IN.TexCoord/500.0f).g ); //channel now used for overlay
         fShadow *= 2.5;
      }
      
      // finally modulate shadow with strength
      fShadow = min ( fShadow * ShadowStrength, 0.85f );
      float fInvShadow = 1.0-fShadow;
         
      // paint
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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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

      diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
    float4 hudfogresult = finalcolor;
    if( iCurrentCascadeIndex == 0 ) 
    {
       return float4( hudfogresult.r , hudfogresult.g*0.35 , hudfogresult.b*0.35 , hudfogresult.a );
    }
    if( iCurrentCascadeIndex == 1 ) 
    {
       return float4( hudfogresult.r *0.35, hudfogresult.g , hudfogresult.b*0.35 , hudfogresult.a );
    }
    if( iCurrentCascadeIndex == 2 ) 
    {
       return float4( hudfogresult.r*0.35 , hudfogresult.g*0.35 , hudfogresult.b , hudfogresult.a );
    }
    return hudfogresult;
   #else
    return finalcolor;
   #endif
}

float4 mainlightPS_medium(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   float4 diffusemap = float4(0,0,0,0);
   float4 diffusemap2 = float4(0,0,0,0);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);
   
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

      // cheap terrain shadow floor texture read
      float fShadow = float4(tex2D(DynTerShaSampler,(IN.TexCoord/500.0f)-float2(0.0005f,0.0005f)).xyz,1).r;
      fShadow = fShadow * 0.675f * ShadowStrength;
   
      // dynamic lighting
      float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
      float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  

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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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
      diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);
   
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
      float4 lighting = lit( pow( abs(0.5*(dot(Ln,IN.WorldNormal))+0.5),2),dot(Hn,IN.WorldNormal),24);
      lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
      
      // some falloff to blend away as distance increases
      lighting.z = lighting.z * max(1.0f-(viewspacePos.z/1000.0f),0);

      // cheap terrain shadow floor texture read
      float fShadow = float4(tex2D(DynTerShaSampler,(IN.TexCoord/500.0f)-float2(0.0005f,0.0005f)).xyz,1).r;
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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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
      float4 diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   return finalcolor;
}

float4 mainlightPS_highest_prebake(vertexOutput IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);
   
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
      float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
      //LEE060517-deathvalleycanyonmessesup!CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  potflashlighting;  
      float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
      
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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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
      float4 diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   #ifdef DEBUGSHADOW
    int iCurrentCascadeIndex=3;
    float fCurrentPixelDepth;
    fCurrentPixelDepth = IN.vDepth;
    if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[2] ) iCurrentCascadeIndex = 2;
    if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[1] ) iCurrentCascadeIndex = 1;
    if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[0] ) iCurrentCascadeIndex = 0;
    float4 hudfogresult = finalcolor;
    if( iCurrentCascadeIndex == 0 ) {
       return float4( hudfogresult.r , hudfogresult.g*0.35 , hudfogresult.b*0.35 , hudfogresult.a );
    }
    if( iCurrentCascadeIndex == 1 ) {
       return float4( hudfogresult.r *0.35, hudfogresult.g , hudfogresult.b*0.35 , hudfogresult.a );
    }
    if( iCurrentCascadeIndex == 2 ) {
       return float4( hudfogresult.r*0.35 , hudfogresult.g*0.35 , hudfogresult.b , hudfogresult.a );
    }
    return hudfogresult;
   #else
    return finalcolor;
   #endif
}

float4 mainlightPS_medium_prebake(vertexOutput_low IN) : COLOR
{   
   // clip
   clip(IN.clip);
   float4 viewspacePos = mul(IN.WPos, View);
   
   // veg and rock common texture lookups
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);
   
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
      float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
      float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
      
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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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
      float4 diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   float4 VegShadowColor = tex2D(VegShadowSampler,IN.TexCoord/500.0f);

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
      float4 highlighttex = tex2D(HighlighterSampler,highlightuv);
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
      float4 diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   float4 diffusemap = float4(tex2D(HighlighterSampler,IN.TexCoord/500.0f).xyz,1);

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
   return float4(1,1,1,1);
}

/****** render depth information ***************/

vertexOutputdepth mainVS_renderdepth(appdata IN)   
{
   vertexOutputdepth OUT;
   OUT.Position = mul(float4(IN.Position,0), WorldViewProjection);
   OUT.LightVec = float3(0,0,0);
   OUT.TexCoordLM  = float2(0,0); 
   OUT.vDepth = 0;
   OUT.TexCoord = float2(0,0);
   OUT.WorldNormal = float3(0,0,0);
   float4 worldSpacePos = mul(float4(IN.Position,0), World);
   OUT.clip = dot(worldSpacePos, clipPlane);         
   worldSpacePos.w = 7000-mul(float4(IN.Position,0), WorldView ).z;
   OUT.WPos = worldSpacePos;  
   return OUT;
}
float4 mainPS_renderdepth(vertexOutputdepth IN) : COLOR
{
    clip(IN.clip);
    return IN.WPos.wxyz;
}

/****** technique ********************************/

technique11 Highest
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        //VertexShader = compile vs_3_0 mainVS_renderdepth();
        //PixelShader  = compile ps_3_0 mainPS_renderdepth();
        //AlphaBlendEnable = FALSE;
        //AlphaTestEnable = FALSE;
		
        SetVertexShader( CompileShader( vs_5_0, mainVS_renderdepth() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, mainPS_renderdepth() ) );
        //SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_5_0 mainVS_highest();
        PixelShader  = compile ps_5_0 mainlightPS_highest();
        //#ifdef DEBUGLEVEL
        // FillMode = WIREFRAME;
        //#endif
        //AlphaBlendEnable = FALSE;
        //AlphaTestEnable = FALSE;
    }
}

/*
technique Highest
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_highest();
        PixelShader  = compile ps_3_0 mainlightPS_highest();
        #ifdef DEBUGLEVEL
         FillMode = WIREFRAME;
        #endif
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
    }
}

technique High
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_highest();
        PixelShader  = compile ps_3_0 mainlightPS_highest();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Medium
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_medium();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Lowest
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_lowest();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Highest_Prebake
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_highest();
        PixelShader  = compile ps_3_0 mainlightPS_highest_prebake();
        #ifdef DEBUGLEVEL
         FillMode = WIREFRAME;
        #endif
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique High_Prebake
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_highest();
        PixelShader  = compile ps_3_0 mainlightPS_highest_prebake();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Medium_Prebake
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_medium_prebake();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Lowest_Prebake
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_lowest_prebake();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique Distant
{
   pass RenderDepthPixelsPass
   <
      string RenderColorTarget = "[depthtexture]";
   >
   {
        // shaders
        VertexShader = compile vs_3_0 mainVS_renderdepth();
        PixelShader  = compile ps_3_0 mainPS_renderdepth();
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
   }
   pass MainPass
   <
      string RenderColorTarget = "";
   >
   {
        // shaders
#ifdef MEDIUMIMPROVEDISTANCE
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_medium();
#else
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_distant();
#endif
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
   }
}

technique DepthMap
{
    pass p0
    {      
      VertexShader = compile vs_2_0 mainVS_lowest(); 
      PixelShader = NULL;   
      CullMode = NONE;
   }
}

technique ReflectedOnly
{
    pass P0
    {
        // shaders
#ifdef WATERREFLECTIONMEDIUM
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_medium();
#else
        VertexShader = compile vs_3_0 mainVS_lowest();
        PixelShader  = compile ps_3_0 mainlightPS_distant();
#endif
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
    }
}

technique blacktextured
{
    pass P0
    {
        // shaders
        VertexShader = compile vs_2_0 mainVS_lowest();
        PixelShader  = compile ps_2_0 blackPS();
        AlphaBlendEnable = FALSE;
        AlphaTestEnable = FALSE;
    }
}
*/
