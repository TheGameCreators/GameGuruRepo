string Description = "Static Shader";
#define ALPHACLIP 0.42
#include "constantbuffers.fx"
#include "settings.fx"                                                    
#include "cascadeshadows.fx"                                                    

float SurfaceSunFactor
<    string UIName =  "SurfaceSunFactor";    
> = {1.0f};

float GlobalSpecular
<    string UIName =  "GlobalSpecular";    
> = {0.5f};

float GlobalSurfaceIntensity
<    string UIName =  "GlobalSurfaceIntensity";    
> = {1.0f};

// standard constants
float4x4 WorldInverse : WorldInverse;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 ViewInverse : ViewInverse;
float4x4 ViewIT : ViewInverseTranspose;
float4x4 ViewProjection : ViewProjection;
float4 eyePos : CameraPosition;
float m_fClippingOnState = 1;

/**************VALUES PROVIDED FROM FPSC - NON TWEAKABLE**************************************/

float4 clipPlane : ClipPlane;  //cliplane for water plane

//SpotFlash Values from FPSC (SpotFlashPos.w is carrying the spotflash fadeout value + SpotFlashColor.w carries FlashLightStrength)
float4 SpotFlashPos;
float4 SpotFlashColor;

float4 FogColor : Diffuse
<   string UIName =  "Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 AmbiColorOverride
<    string UIName =  "AmbiColorOverride";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 AmbiColor : Ambient
<    string UIName =  "AmbiColor";    
> = {0.1f, 0.1f, 0.1f, 1.0f};

float4 SurfColor : Diffuse
<    string UIName =  "SurfColor";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float SpecularOverride = 1.0f;

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {1.0, 1.0, 1.0, 1.0f};

float4 FloorColor : Diffuse
<    string UIName =  "FloorColor";    
> = {1.0, 1.0, 1.00, 1.0f};

float alphaoverride  : alphaoverride;

float4 EntityEffectControl
<    string UIName =  "EntityEffectControl";    
> = {0.0f, 0.0f, 0.0f, 0.0f};

//Shader Variables pulled from FPI scripting 
float4 ShaderVariables : ShaderVariables
<    string UIName =  "Shader Variables";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

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

Texture2D DiffuseMap : register( t0 );
Texture2D OcclusionMap : register( t1 );
Texture2D NormalMap : register( t2 );
Texture2D SpecularMap : register( t3 );
Texture2D VegShadowTex : register( t4 );
Texture2D DynTerShaMap : register( t5 );
Texture2D IlluminationMap : register( t6 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
SamplerState SampleClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct appdata 
{
    float4 Position : POSITION;
    float4 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;
    float2 UV2      : TEXCOORD1;
    float4 Tangent  : TANGENT0;
    float4 Binormal : BINORMAL0;
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float2 TexCoord2    : TEXCOORD1;
    float3 LightVec     : TEXCOORD2;
    float3 WorldNormal  : TEXCOORD3;
    float3 WorldTangent : TEXCOORD4;
    float3 WorldBinorm  : TEXCOORD5;
    float4 WPos         : TEXCOORD7;
    float  clip         : TEXCOORD8;
    float2 vDepth       : TEXCOORD9;
};

struct vertexOutput_low
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float2 TexCoord2    : TEXCOORD1;
    float3 LightVec     : TEXCOORD2;
    float3 WorldNormal  : TEXCOORD3;
    float4 WPos         : TEXCOORD4;
    float2 vegshadowuv  : TEXCOORD5;
    float  clip         : TEXCOORD6;
    float  vDepth       : TEXCOORD7;
};

vertexOutput mainVS_highest(appdata IN)   
{
   vertexOutput OUT;
   
   float4 worldSpacePos = mul(IN.Position, World);
   OUT.WPos =   worldSpacePos; 
      
   OUT.WorldNormal = normalize(mul(IN.Normal, WorldIT).xyz);
   float3 c1 = cross(OUT.WorldNormal, float3(0.0, 0.0, 1.0)); 
   float3 c2 = cross(OUT.WorldNormal, float3(0.0, 1.0, 0.0)); 
   if (length(c1) > length(c2)) {
     OUT.WorldTangent = c1;   
   } else {
     OUT.WorldTangent = c2;   
   }
   OUT.WorldTangent = normalize(OUT.WorldTangent); 
   OUT.WorldBinorm = cross(OUT.WorldNormal,OUT.WorldTangent);
      
   OUT.LightVec = normalize(LightSource.xyz);
   OUT.Position = mul(IN.Position, WorldViewProjection);
   OUT.TexCoord = IN.UV; 
   OUT.TexCoord2 = IN.UV2; 
                
   // all shaders should send the clip value to the pixel shader (for refr/refl)                                                                     
   OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
     
   // SHADOW MAPPING - world position and projected depth (for cascade distance calc)
   OUT.vDepth.x = mul( IN.Position, WorldViewProjection ).z; 
   // defeat projective aliasing by detecting when parallel to light direction
   OUT.vDepth.y = max(0,abs(dot(OUT.LightVec, OUT.WorldNormal))-0.25f)*1.333f;
   
    return OUT;
}

vertexOutput_low mainVS_lowest(appdata IN)   
{
   vertexOutput_low OUT;
   float4 worldSpacePos = mul(IN.Position, World);
   OUT.WPos =   worldSpacePos; 
   OUT.WorldNormal = normalize(mul(IN.Normal, WorldIT).xyz);
   OUT.LightVec = normalize(LightSource.xyz);
   OUT.Position = mul(IN.Position, WorldViewProjection);
   OUT.TexCoord  = IN.UV; 
   OUT.TexCoord2 = IN.UV2; 
   OUT.vegshadowuv = float2(worldSpacePos.x/51200.0f,worldSpacePos.z/51200.0f);
   OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
   OUT.vDepth = mul( IN.Position, WorldViewProjection ).z; 
   return OUT;
}

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
    output += (float4(SpotFlashColor.xyz,1)) *fAtten * (fSpotFlashPosW) * max(0,dot(worldNormal,lightDir));
        
    return output;
}

float4 CalcLighting(float3 Nb, float3 worldPos, float3 Vn, float4 diffusemap, float4 specmap)
{           
    float4 output = (float4)0.0;
#ifdef VERSION114
   if ( g_lights_data.x == 0 ) return output;
#endif    
    // light 0
    float3 toLight = g_lights_pos0.xyz - worldPos;
    float lightDist = length( toLight );

    float fAtten;
    float3 lightDir;
    float3 halfvec;
    float4 lit0;
//    if( lightDist < g_lights_pos0.w*4.0 ) {  
       float4 local_lights_atten0 = float4(1.0, 1.0/g_lights_pos0.w, 1.0/(g_lights_pos0.w*g_lights_pos0.w), 0.0);
       //float fAtten = 1.0/dot( g_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       fAtten = 1.0/dot( local_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse0 * fAtten * diffusemap) + (lit0.z * g_lights_diffuse0 * fAtten *specmap );   
//    }   
    // light 1
    toLight = g_lights_pos1.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos1.w*4.0 ) {  
       float4 local_lights_atten1 = float4(1.0, 1.0/g_lights_pos1.w, 1.0/(g_lights_pos1.w*g_lights_pos1.w), 0.0);
       //fAtten = 1.0/dot( g_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       fAtten = 1.0/dot( local_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse1 * fAtten * diffusemap) + (lit0.z * g_lights_diffuse1 * fAtten *specmap );   
//    }   
    // light 2
    toLight = g_lights_pos2.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos2.w*4.0 ) {  
       float4 local_lights_atten2 = float4(1.0, 1.0/g_lights_pos2.w, 1.0/(g_lights_pos2.w*g_lights_pos2.w), 0.0);
       //fAtten = 1.0/dot( g_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       fAtten = 1.0/dot( local_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse2 * fAtten * diffusemap) + (lit0.z * g_lights_diffuse2 * fAtten *specmap );   
//    }   
    return output;
}

float4 mainPS_highest(vertexOutput IN) : COLOR
{
   // clip
    clip(IN.clip);
   
   // source textures
    float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy);
    float3 normalmap = NormalMap.Sample(SampleWrap,IN.TexCoord.xy).xyz * 2 - 1;
    float4 specmap = SpecularMap.Sample(SampleWrap,IN.TexCoord.xy);
    float4 illummap = IlluminationMap.Sample(SampleWrap,IN.TexCoord.xy);
   
   // work out normal from normmap/tangent/binormal
#ifdef DISABLELIGHTMAPNORMALS
    float3 Ln = (float3(0,1,0)); // 011215 - baked lights are everywhere! (IN.LightVec);
#else
    float3 Ln = normalize(IN.LightVec);
#endif
    float3 Nn = (IN.WorldNormal);
    float3 Tn = (IN.WorldTangent);
    float3 Bn = (IN.WorldBinorm);
    float3 Nb = (normalmap.z * Nn) + ((normalmap.x * Tn + normalmap.y * Bn));
    Nb = normalize(Nb);
   
   // lighting
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
    float3 Hn = normalize(Vn+Ln);
    float4 lighting = lit(dot(Ln,Nb),dot(Hn,Nb),24);
   lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

   // choose between surface lighting and lightmap based on shadow strength
   float4 rawlightmap = OcclusionMap.Sample(SampleWrap,IN.TexCoord2.xy);
           
   // remove lightmapper blur artifacts.
   rawlightmap = clamp(rawlightmap,0.265,1.0);

   // produced final light-color
   float4 fLightOfShadow = max(min(float4((rawlightmap.xyz-0.25f)*2,1),1),0); // 0.25=BLACK 0.75=WHITE -> 0-1 range
   float4 flatlighting = lit(dot(Ln,IN.WorldNormal),dot(Hn,IN.WorldNormal),24);
   flatlighting.y = lerp(0.65,flatlighting.y,SurfaceSunFactor);
   fLightOfShadow = fLightOfShadow + (lighting.y-flatlighting.y);
   
   // Shadows
   int iCurrentCascadeIndex = 0;
   float fShadow = GetShadow ( IN.vDepth.x, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
       
   // inverse to modulate surface lighting (and only shadow surfaces facing sunlight)
   float ShadowSunFactor = lerp(1.0,dot(Ln,IN.WorldNormal),SurfaceSunFactor);
   fShadow = fShadow * ShadowSunFactor * 3.5f; // PE: Let it look like baked shadows.
   fLightOfShadow = min(fLightOfShadow,clamp(1-fShadow,0.0,0.85));
   
    // dynamic lighting
    float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
    float4 dynamicContrib = CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,specmap) + spotflashlighting; 
   
   // 310315 - make dynamic shadow more pronounced by having it block dynamic lights
   // PE: Dynamic is already selected in fLightOfShadow.
   // PE: This way we select baked or dynamic shadow but dont mix them.
   //dynamicContrib = dynamicContrib * (1-fShadow);
  
   // flash light system (flash light control carried in SpotFlashColor.w )
   float conewidth = 24;
    float4 viewspacePos = mul(IN.WPos, View);
    float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
   float3 lightvector = Vn;
    float3 lightdir = float3(View._m02,View._m12,View._m22);
    float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));

   // paint
   float4 diffuseContrib = SurfColor * diffusemap * fLightOfShadow * GlobalSurfaceIntensity;
#ifdef PEROJECTLIGHT
    float4 specContrib = lighting.z * specmap * SurfColor * fLightOfShadow * GlobalSpecular;
#else
    float4 specContrib = lighting.z * specmap * SurfColor * fLightOfShadow * SpecularOverride * GlobalSpecular;
#endif
#ifdef SPECULARCAMERA
    float3 lightdirsc = normalize(float3(View._m02,View._m12,View._m22));
    lightdirsc = dot(-lightvector, lightdirsc);
    float4 lightingsc = lit(dot(lightvector,Nb),dot(lightvector,Nb),24);
    intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
    lightingsc.z = lightingsc.z * intensity;
    specContrib = ( specContrib + ( (lightingsc.z *  SPECULARCAMERAINTENSITY ) * SurfColor * GlobalSpecular) ) * 0.5;
#endif

   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
    float4 ambContrib = (float4(bouncelightcolor,1) * ((AmbiColor * AmbiColorOverride)+float4(illummap.xxx,1))) * 2;
      
    // combine
#ifdef PEROJECTLIGHT
    float4 result = (diffuseContrib + ambContrib + specContrib + dynamicContrib) * SpecularOverride;
#else
    float4 result = diffuseContrib + ambContrib + specContrib + dynamicContrib;
#endif
   
   //calculate hud pixel-fog
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
   // original entity diffuse alpha with override
    hudfogresult.a = diffusemap.a * alphaoverride;    
   
   // entity effect control can slice alpha based on a world Y position
   float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
   hudfogresult.a = hudfogresult.a * alphaslice;
   
   // final pixel color
   #ifdef DEBUGSHADOW
    hudfogresult = TintDebugShadow ( iCurrentCascadeIndex, hudfogresult );
   #endif	
   return hudfogresult;   
}

float4 mainPS_medium(vertexOutput_low IN) : COLOR
{
   // clip
    clip(IN.clip);
   
   // lighting
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
   
   // texture ref
    float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy);

    // dynamic lighting
   float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
   float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
   
   // flash light system (flash light control carried in SpotFlashColor.w )
    float4 viewspacePos = mul(IN.WPos, View);
   float conewidth = 24;
    float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
    float3 lightvector = Vn;
    float3 lightdir = float3(View._m02,View._m12,View._m22);
    float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
   
   // choose between surface lighting and lightmap based on shadow strength
   float4 rawlightmap = OcclusionMap.Sample(SampleWrap,IN.TexCoord2.xy);

   // remove lightmapper blur artifacts.
   rawlightmap = clamp(rawlightmap,0.265,1.0);

   float4 fLightOfShadow = max(min(float4((rawlightmap.xyz-0.25f)*2,1),1),0); // 0.25=BLACK 0.75=WHITE -> 0-1 range
   
   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
   float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;   
   float4 diffuseContrib = SurfColor * diffusemap * fLightOfShadow * GlobalSurfaceIntensity;

#ifdef PEROJECTLIGHT
   float4 result = (diffuseContrib + ambContrib + dynamicContrib) * SpecularOverride;
#else
   float4 result = diffuseContrib + ambContrib + dynamicContrib;
#endif

   //calculate hud pixel-fog
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
   // original entity diffuse alpha with override
    hudfogresult.a = diffusemap.a * alphaoverride;    
   
   // entity effect control can slice alpha based on a world Y position
   float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
   hudfogresult.a = hudfogresult.a * alphaslice;

   // final pixel color
    return hudfogresult;   
}

float4 mainPS_lowest(vertexOutput_low IN) : COLOR
{
   // clip
    clip(IN.clip);
   
   // lighting
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
   
   // CHEAPEST flash light system (flash light control carried in SpotFlashColor.w )
    float4 viewspacePos = mul(IN.WPos, View);
    float flashlight = (1.0f-min(1,viewspacePos.z/300.0f)) * SpotFlashColor.w;   
   
   // diffuse texture
    float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy);

   // choose between surface lighting and lightmap based on shadow strength
   float4 rawlightmap = OcclusionMap.Sample(SampleWrap,IN.TexCoord2.xy);

   // remove lightmapper blur artifacts.
   rawlightmap = clamp(rawlightmap,0.265,1.0);

   float4 fLightOfShadow = max(min(float4((rawlightmap.xyz-0.25f)*2,1),1),0); // 0.25=BLACK 0.75=WHITE -> 0-1 range
      
   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
    float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;   
   float4 diffuseContrib = SurfColor * diffusemap * fLightOfShadow * GlobalSurfaceIntensity;

#ifdef PEROJECTLIGHT
   float4 result = (diffuseContrib + ambContrib) * SpecularOverride;
#else
   float4 result = diffuseContrib + ambContrib;
#endif

   //calculate hud pixel-fog
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
   // original entity diffuse alpha with override
    hudfogresult.a = diffusemap.a * alphaoverride;    
   
   // entity effect control can slice alpha based on a world Y position
   float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
   hudfogresult.a = hudfogresult.a * alphaslice;

   // final pixel color
    return hudfogresult;   
}

float4 mainPS_distant(vertexOutput_low IN) : COLOR
{
   // clip
    clip(IN.clip);
   
   // final pixel color
    return float4(1,1,1,1);   
}

float4 blackPS(vertexOutput_low IN) : COLOR
{
   clip(IN.clip); // all shaders should receive the clip value  
   if( DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy).a < ALPHACLIP ) 
   {
       clip(-1);
      return IN.vDepth;
   }
   return IN.vDepth;
}

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_highest()));
        SetGeometryShader(NULL);
    }
}

technique11 Medium
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_medium()));
        SetGeometryShader(NULL);
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_lowest()));
        SetGeometryShader(NULL);
    }
}

technique11 DepthMap
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}

technique11 DepthMapNoAnim
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}

technique11 blacktextured
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, blackPS()));
        SetGeometryShader(NULL);
    }
}
