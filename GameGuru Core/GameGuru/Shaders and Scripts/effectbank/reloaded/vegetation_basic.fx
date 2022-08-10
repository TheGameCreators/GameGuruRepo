#include "settings.fx"

#ifdef USEPBRVEGETATION

	string Description = "PBR Shader (vegetation)";
	#define PBRVEGETATION
	#define ALPHACLIP 0.42
	#include "apbr_core.fx"

#else

string Description = "Vegetation Shader";
#include "constantbuffers.fx"
//#include "settings.fx"
#include "cascadeshadows.fx"

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

// regular shader constants   
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 ViewProjection : ViewProjection;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float time: Time;
float4 eyePos : CameraPosition;
float4 clipPlane : ClipPlane;  //cliplane for water plane

//Supports dynamic lights (using CalcLightingNoNormal function)
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
float4 dl_pos[82];
float4 dl_diffuse[82];
float4 dl_angle[82];


float4 SpotFlashPos;  //SpotFlashPos.w is carrying the spotflash fadeout value
float4 SpotFlashColor; //RGB of flash colour

//**TWEAKABLES********************************************************/

float SwayAmount
<
   string UIWidget = "slider";
   float UIMax = 0.3;
   float UIMin = 0.0;
   float UIStep = 0.01;
> = 0.05f;

float SwaySpeed
<
   string UIWidget = "slider";
   float UIMax = 10;
   float UIMin = 0.0;
   float UIStep = 0.1;
> = 1.0f;

float ScaleOverride
<
   string UIWidget = "slider";
   float UIMax = 3.0;
   float UIMin = 0.1;
   float UIStep = 0.1;
> = 2.5f;

float ColorVarAmount
<
   string UIWidget = "slider";
   float UIMax = 10.0;
   float UIMin = 0.0;
   float UIStep = 0.1;
> = 3.5f;

float ColorSpeed
<
   string UIWidget = "slider";
   float UIMax = 10.0;
   float UIMin = 0.1;
   float UIStep = 0.1;
> = 3.0f;

float RedTint
<
   string UIWidget = "slider";
   float UIMax = 0.1;
   float UIMin = 0.0;
   float UIStep = 0.01;
> = 0.01f;

float GreenTint
<
   string UIWidget = "slider";
   float UIMax = 0.1;
   float UIMin = 0.0;
   float UIStep = 0.01;
> = 0.01f;

float BlueTint
<
   string UIWidget = "slider";
   float UIMax = 0.1;
   float UIMin = 0.0;
   float UIStep = 0.01;
> = 0.00f;

//WATER Fog Color
float4 FogColor : Diffuse
<   string UIName =  "Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

//HUD Fog Color
float4 HudFogColor : Diffuse
<   string UIName =  "Hud Fog Color";    
> = {0.0f, 0.0f, 0.0f, 0.0000001f};

//HUD Fog Distances (near,far,0,0)
float4 HudFogDist : Diffuse
<   string UIName =  "Hud Fog Dist";    
> = {1.0f, 0.0f, 0.0f, 0.0000001f};

float4 AmbiColorOverride
<    string UIName =  "AmbiColorOverride";    
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 AmbiColor : Ambient
<
    string UIName =  "Ambient Light Color";
> = {0.1f, 0.1f, 0.1f, 1.0f};

float4 SurfColor : Diffuse
<
    string UIName =  "Surface Color";
    string UIType = "Color";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {1.0, 1.0, 1.0, 1.0f};

Texture2D BaseTex : register( t0 );
Texture2D NotUsed : register( t1 );
Texture2D NotUsed2 : register( t2 );
Texture2D NotUsed3 : register( t3 );
Texture2D NotUsed4 : register( t4 );
Texture2D NotUsed5 : register( t5 );
Texture2D NotUsed6 : register( t6 );
Texture2D NotUsed7 : register( t7 );
Texture2D NotUsed8 : register( t8 );

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

struct app_in
{
    float3 pos    : POSITION;
    float3 Normal : NORMAL;
    float2 uv     : TEXCOORD0;
};

struct vs_out
{
   float4 pos         : POSITION;
   float2 uv          : TEXCOORD0;
   float4 coloroffset : TEXCOORD1;
   float2 vegshadowuv : TEXCOORD2;
   float clip         : TEXCOORD3;
   float4 WPos        : TEXCOORD4;
   float3 LightVec    : TEXCOORD6;
   float3 WorldNormal : TEXCOORD7; 
   float3 VertexLight : TEXCOORD8;
};

struct vs_out_lowest
{
   float4 pos         : POSITION;
   float2 uv          : TEXCOORD0;
   float2 vegshadowuv : TEXCOORD1;
   float4 WPos        : TEXCOORD2;
   float DistAlpha    : TEXCOORD3;
   float clip         : TEXCOORD4;
   float4 cameraPos   : TEXCOORD5;
   float3 VertexLight : TEXCOORD6;   
};

// CALC LIGHT

float4 CalcSpotFlashNoNormal( float3 worldPos )
{
    float4 output = (float4)0.0;
    float3 toLight = (SpotFlashPos.xyz - worldPos.xyz);
    float3 lightDir = normalize( toLight );
    float lightDist = length( toLight );
    
    float MinFalloff = 100;  //falloff start distance
    float LinearFalloff = 1;
    float ExpFalloff = .005;  // 1/200
    SpotFlashPos.w = clamp(0,1,SpotFlashPos.w);
    
    //classic attenuation - but never actually reaches zero
    float fAtten = 1.0/(MinFalloff + (LinearFalloff*lightDist)+(ExpFalloff*lightDist*lightDist));
    output += (SpotFlashColor) * fAtten * (SpotFlashPos.w); //don't use normal, faster
        
    return output;
}

float4 CalcLightingNoNormal(float3 worldPos)
{
   // No normals to calculate, veg receives light for all sides
    float4 output = (float4)0.0;
#ifdef SKIPIFNODYNAMICLIGHTS
   if ( g_lights_data.x == 0 ) return output;
#endif       

    // light 0
    float3 toLight = g_lights_pos0.xyz - worldPos;
    float lightDist = length( toLight );
    float fAtten;
    
//    if( lightDist < g_lights_pos0.w*4.0 ) {  
       //float fAtten = 1.0/dot( g_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten0 = float4(1.0, 1.0/g_lights_pos0.w, 1.0/(g_lights_pos0.w*g_lights_pos0.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) );
       output+= g_lights_diffuse0 * fAtten * 1.7;
//    }   
    // light 1
    toLight = g_lights_pos1.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos1.w*4.0 ) {  
       //fAtten = 1.0/dot( g_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten1 = float4(1.0, 1.0/g_lights_pos1.w, 1.0/(g_lights_pos1.w*g_lights_pos1.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) );
       output+= g_lights_diffuse1 * fAtten * 1.7; 
//    }   
    // light 2
    toLight = g_lights_pos2.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos2.w*4.0 ) {  
       //fAtten = 1.0/dot( g_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten2 = float4(1.0, 1.0/g_lights_pos2.w, 1.0/(g_lights_pos2.w*g_lights_pos2.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) );
       output+= g_lights_diffuse2 * fAtten * 1.7;
//    }   
   // as no normal, reduce overall influence by half
   output *= 0.5f;
   
    return output;
}



float3 CalcExtSpotNoNormal( float3 worldPos , float3 SpotPos , float3 SpotColor , float range, float3 angle)
{
    float conewidth = 24;
	float toLight = length(SpotPos.xyz - worldPos);
	float4 local_lights_atten = float4(1.0, 1.0/range, 1.0/(range*range), 0.0);
	float intensity = 1.0/dot( local_lights_atten, float4(1,toLight,toLight*toLight,0) );
    float3 V  = SpotPos.xyz - worldPos;  
    float3 Vn  = normalize(V); 
    float3 lightvector = Vn;
    float3 lightdir = normalize(float3(angle.x,angle.y*2.0,angle.z));
    intensity = clamp(intensity ,0.0,1.0);
    return (SpotColor.xyz * pow(max(dot(-lightvector, lightdir ),0),conewidth) * 2.0 ) * intensity;
}

float3 CalcExtLightingNoNormal(float3 worldPos)
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
			output += CalcExtSpotNoNormal(worldPos,dl_pos[i].xyz,dl_diffuse[i].xyz,dl_pos[i].w,dl_angle[i].xyz);
		} else {
		
			toLight = dl_pos[i].xyz - worldPos;
			lightDist = length( toLight );
			local_lights_atten = float4(1.0, 1.0/dl_pos[i].w, 1.0/(dl_pos[i].w*dl_pos[i].w), 0.0);
			fAtten = 1.0/dot( local_lights_atten, float4(1,lightDist,lightDist*lightDist,0) );
			lightDir = normalize( toLight );
			output+= (dl_diffuse[i].xyz * fAtten * 1.25 );
		}
	}
	output *= 0.5f;
	return output;
}


/****** vertex shader *****/

vs_out mainVS_highest( app_in IN )
{
   vs_out OUT;

   // Grass clumps are hidden by setting the verts to a 200+ Y position offset in the grass area model. So anything higher than
   // this we can skip processing (perhaps! With pipeling it may still be processed!). When coloroffset.a = 0 is passed to the pixel 
   // shader, the pixel will be clipped straight away
   OUT.coloroffset = float4(0,0,0,0);
   if (IN.pos.y < 199)
   {   
      //animate the verts - model must have pivot at base of model on export
      float amplitude = pow( abs(SwayAmount * (1-IN.uv.y) * 50.0f),1); //power function biases movement toward the top of the UV map (veg can be high)
      float4 wave = amplitude * float4(sin(time * SwaySpeed +IN.pos.x),0,cos(time *SwaySpeed +IN.pos.z),0);
      float4 vert = float4(IN.pos,1) + wave;
      vert.w = 1;
      /////////////////
      
      float4 temppos = mul(vert, World);
      float4 tempposforcol = temppos*ColorSpeed; //using world space pos to randomize colors so same mesh will not all look the same

      //offset colors
      float thesin = sin(time+tempposforcol.x +tempposforcol.y + tempposforcol.z);
      OUT.coloroffset.r = (RedTint*thesin); //may want to saturate these 3 values to prevent subtracting color
      OUT.coloroffset.g = (GreenTint*thesin); //try x or z for a different color effect
      OUT.coloroffset.b = (BlueTint*thesin);	  
      OUT.pos = mul( vert, WorldViewProjection ); //use modified vert position instead of IN.pos
      
      // fade alpha with distance from camera
      float3 diff = temppos.xyz - eyePos.xyz;
      float fDist = sqrt(diff.x*diff.x+diff.z*diff.z);
      float fEdgePerc = max(0,fDist-(GrassFadeDistance*0.4f)) / (GrassFadeDistance*0.6f);
      OUT.coloroffset.a = 1.0f - fEdgePerc;

      // shadow painted in veg shadow map generated from cascade shadow capture and user painting?)
      // and needs a UV coordinate derived from the world position of the vegetable
      OUT.vegshadowuv = float2(temppos.x/51100.0f,temppos.z/51100.0f);
      
      OUT.uv = IN.uv;
      OUT.LightVec = normalize(LightSource.xyz);
      OUT.WorldNormal = float3(0,1,0);//normalize(mul(float4(IN.Normal,1), WorldIT).xyz);
      
      // calculate Water FOG colour
      float4 cameraPos = mul( temppos, View );
      float fogstrength = cameraPos.z * FogColor.w;
      OUT.WPos = temppos;

      // all shaders should send the clip value to the pixel shader
      OUT.clip = dot(temppos, clipPlane); 
   }
   else
   {
      OUT.pos = float4(0,0,0,0);
      OUT.vegshadowuv = float2(0,0);
      OUT.uv = float2(0,0);
      OUT.LightVec = float3(0,0,0);
      OUT.WorldNormal = float3(0,0,0);
      OUT.WPos = float4(0,0,0,0);
      OUT.clip = -1; 
   }
   OUT.VertexLight = float3(0,0,0);
   return OUT;
}

vs_out_lowest mainVS_lowest( app_in IN )
{
   // uses no IF branches, no animation
   vs_out_lowest OUT;
   OUT.pos = mul(float4(IN.pos.xyz,1), WorldViewProjection ); 
   OUT.uv = IN.uv;
   float4 temppos = mul(float4(IN.pos,1), World);
   OUT.vegshadowuv = float2(temppos.x/51100.0f,temppos.z/51100.0f);
   float3 diff = temppos.xyz - eyePos.xyz;
   float fDist = sqrt(diff.x*diff.x+diff.z*diff.z);
   OUT.DistAlpha = 1.0 - (fDist / (GrassFadeDistance*0.4f));
   float alphaoffset = 0 - max(0,fDist-(GrassFadeDistance*0.4f)) / (GrassFadeDistance*0.6f); 
   OUT.WPos = temppos;
   OUT.clip = alphaoffset - (max(0,IN.pos.y-199));
   OUT.cameraPos = mul(temppos, View);
   //if( OUT.DistAlpha < 0.05 ) OUT.clip = -1;
   OUT.VertexLight = float3(0,0,0);

   return OUT;
}

float4 mainPS_highest( vs_out IN ) : COLOR
{       
   // clip
   clip(IN.clip);
  
   // texture reference
   float4 diffusemap = BaseTex.Sample(SampleWrap,IN.uv);
   clip(diffusemap.a < 0.7f ? -1:1); 

   // lighting
    float3 Ln = normalize(IN.LightVec);
   float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
    float3 Hn = normalize(Vn+Ln);
    float4 lighting = lit((dot(Ln,IN.WorldNormal)),dot(Hn,IN.WorldNormal),24);
    lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);
   
   // dynamic lighting
    //float4 spotflashlighting = CalcSpotFlashNoNormal (IN.WPos.xyz);
    float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);
//    float4 dynamicContrib = (CalcLightingNoNormal(IN.WPos.xyz) * diffusemap) + spotflashlighting;
    float4 dynamicContrib = ( float4( CalcExtLightingNoNormal(IN.WPos.xyz) , 1.0) * diffusemap) + spotflashlighting;


   
   // flash light system (flash light control carried in SpotFlashColor.w )
    float4 cameraPos = mul(IN.WPos, View);
   float conewidth = 24;
   float intensity = max(0, 1.5f - (cameraPos.z/500.0f));
   float3 lightvector = Vn;
   float3 lightdir = float3(View._m02,View._m12,View._m22);
   float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
   
   // spherical ambience not used
   float4 ambContrib = diffusemap * (AmbiColor) * AmbiColorOverride * 2;
   
   // Shadows
   float fShadow = GetShadowCascade ( 3, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz) );
   
   // finally modulate shadow with strength
   float fInvShadow = (1.0-(fShadow*0.65f));
   
   // paint
   float4 diff = float4(diffusemap.xyz+(IN.coloroffset.xyz*ColorVarAmount),1);
   float4 diffuseContrib = SurfColor * diff * 0.61 * GlobalSurfaceIntensity;
   float4 specContrib = lighting.z * diff * SurfColor * GlobalSpecular;
   
   // apply shadow mapping to final render
   //ambContrib.xyz = ambContrib.xyz * fInvShadow; // 291115 - ambient is NOT affected by shadow!
   diffuseContrib.xyz = diffuseContrib.xyz * fInvShadow;
   specContrib.xyz = specContrib.xyz * fInvShadow;
   float4 result = diffuseContrib + ambContrib + specContrib + dynamicContrib;
   
   // fog
   float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   float3 hudfogresult = lerp(result.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);
     
   // modulate with distance alpha (fade out)
   result.a = IN.coloroffset.a;
   
   // return final pixel
   return float4(hudfogresult.xyz,result.a);
}

float4 mainPS_high( vs_out IN ) : COLOR
{
   // clip
   clip(IN.clip);

   // texture reference
   float4 diffusemap = BaseTex.Sample(SampleWrap,IN.uv);
   clip(diffusemap.a < 0.7f ? -1:1); 

   // dynamic lighting
    //float4 spotflashlighting = CalcSpotFlashNoNormal (IN.WPos.xyz);
    float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);
    //float4 dynamicContrib = (CalcLightingNoNormal(IN.WPos.xyz) * diffusemap) + spotflashlighting;
    float4 dynamicContrib = ( float4( CalcExtLightingNoNormal(IN.WPos.xyz) , 1.0) * diffusemap) + spotflashlighting;
   
   // flash light system (flash light control carried in SpotFlashColor.w )
   float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
   float conewidth = 24;
   float intensity = max(0, 1.5f - 0.0f);//(IN.cameraPos.z/500.0f));
   float3 lightvector = Vn;
   float3 lightdir = float3(View._m02,View._m12,View._m22);
   float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
   
   // spherical ambience from sky only
   float4 ambContrib = diffusemap * AmbiColor * AmbiColorOverride * 2;
   
   // paint
   float4 diffuseContrib = SurfColor * diffusemap * 0.61 * GlobalSurfaceIntensity;
   
   // Shadows
   int iCurrentCascadeIndex = 0;
   float fShadow = GetShadow ( 500.0f, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
   
   // finally modulate shadow with strength
   float fInvShadow = (1.0-(fShadow*0.65f));
   diffuseContrib = diffuseContrib * fInvShadow;
   
   // diffuse + ambient
   float4 result = diffuseContrib + ambContrib + dynamicContrib;
   
   // fog
    float4 cameraPos = mul(IN.WPos, View);
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float3 hudfogresult = lerp(result.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);

    // water fog
//    hudfogresult = lerp(hudfogresult,FogColor,IN.WaterFog);
   
   // modulate with distance alpha (fade out)
   result.a = IN.coloroffset.a;
   
   // return final pixel
   return float4(hudfogresult.xyz,result.a);
   
   // modulate with distance alpha (fade out)
   //result.a = 1.0f;//IN.DistAlpha;
   //result.x = 1.0f;
   // return final pixel
   //return float4(hudfogresult.xyz,result.a);
}

float4 mainPS_medium( vs_out_lowest IN ) : COLOR
{
   // clip
   clip(IN.clip);

   // texture reference
   float4 diffusemap = BaseTex.Sample(SampleWrap,IN.uv);
   clip(diffusemap.a < 0.7f ? -1:1); 

   // dynamic lighting
    //float4 spotflashlighting = CalcSpotFlashNoNormal (IN.WPos.xyz);
    float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);
    //float4 dynamicContrib = (CalcLightingNoNormal(IN.WPos.xyz) * diffusemap) + spotflashlighting;
    float4 dynamicContrib = ( float4( CalcExtLightingNoNormal(IN.WPos.xyz) , 1.0) * diffusemap) + spotflashlighting;
   
   // flash light system (flash light control carried in SpotFlashColor.w )
   float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
   float conewidth = 24;
   float intensity = max(0, 1.5f - (IN.cameraPos.z/500.0f));
   float3 lightvector = Vn;
   float3 lightdir = float3(View._m02,View._m12,View._m22);
   float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
   
   // spherical ambience from sky only
   float4 ambContrib = diffusemap * AmbiColor * AmbiColorOverride * 2;
   
   // paint
   float4 diffuseContrib = diffusemap;//SurfColor * diffusemap * 0.61 * GlobalSurfaceIntensity;
   
   // diffuse + ambient
   float4 result = diffuseContrib + ambContrib + dynamicContrib;
   
   // fog
    float hudfogfactor = saturate((IN.cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float3 hudfogresult = lerp(result.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);
   
   // modulate with distance alpha (fade out)
   result.a = IN.DistAlpha;
      
   // return final pixel
   return float4(hudfogresult.xyz,result.a);
}

float4 mainPS_lowest( vs_out_lowest IN ) : COLOR
{
   // clip
   clip(IN.clip);
   
   // texture reference
   float4 diffusemap = BaseTex.Sample(SampleWrap,IN.uv);
   clip(diffusemap.a < 0.7f ? -1:1); 
   
   // CHEAPEST flash light system (flash light control carried in SpotFlashColor.w )
   float flashlight = (1.0f-min(1,IN.cameraPos.z/300.0f)) * SpotFlashColor.w;   
   
   // spherical ambience from sky only
   float4 ambContrib = diffusemap * AmbiColor * AmbiColorOverride * 2;
   
   // paint
   float4 diffuseContrib = SurfColor * (diffusemap * (1.0+flashlight) * 0.61 * GlobalSurfaceIntensity);
   
   // diffuse + ambient
   float4 result = diffuseContrib + ambContrib;
   
   // fog
    float hudfogfactor = saturate((IN.cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float3 hudfogresult = lerp(result.xyz,HudFogColor.xyz,hudfogfactor*HudFogColor.w);
   
   // return final pixel
   return float4(hudfogresult.xyz,IN.DistAlpha);
}
   
float4 BlackPixelShader( vs_out_lowest IN ) : COLOR
{
   // clip
   clip(IN.clip);
      
   // clip grass pixels where the texture has low transparency
   float4 result = BaseTex.Sample(SampleWrap,IN.uv*1.0);   
   clip(result.a < 0.7f ? -1:1); 
   
   // return final pixel
   return float4(0,0,0,result.a);
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

technique11 High
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_high()));
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

technique11 blacktextured
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, BlackPixelShader()));
        SetGeometryShader(NULL);
    }
}
#endif
