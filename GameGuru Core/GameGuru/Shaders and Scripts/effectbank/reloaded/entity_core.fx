#include "constantbuffers.fx"
#include "settings.fx"
#include "cascadeshadows.fx"

// useful to make entity glow (for IDE and also in-game)
float4 GlowIntensity = float4(0, 0, 0, 0);

float SurfaceSunFactor = {1.0f};
float GlobalSpecular = {0.5f};
float GlobalSurfaceIntensity = {1.0f};

// standard constants
float4x4 WorldInverse : WorldInverse;
float4x4 WorldIT : WorldInverseTranspose;
float4x4 WorldView : WorldView;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 ViewInverse : ViewInverse;
float4x4 ViewIT : ViewInverseTranspose;
float4x4 ViewProjection : ViewProjection;
float4 eyePos : CameraPosition;
float timer : Time;

float m_fClippingOnState = 1;

// Depth Write Control (1=write real depth)
float DepthWriteMode = {1.0f};

/**************VALUES PROVIDED FROM FPSC - NON TWEAKABLE**************************************/

float4 clipPlane : ClipPlane;  //cliplane for water plane

float4 SpotFlashPos;
float4 SpotFlashColor;
float4 FogColor  = {0.0f, 0.0f, 0.0f, 0.0000001f};
float4 HudFogColor  = {0.0f, 0.0f, 0.0f, 0.0000001f};
float4 HudFogDist  = {1.0f, 0.0f, 0.0f, 0.0000001f};
float4 AmbiColorOverride = {1.0f, 1.0f, 1.0f, 1.0f};
float4 AmbiColor = {0.1f, 0.1f, 0.1f, 1.0f};
float4 SurfColor = {1.0f, 1.0f, 1.0f, 1.0f};
float SpecularOverride = 1.0f;
float4 SkyColor = {1.0, 1.0, 1.0, 1.0f};
float4 FloorColor = {1.0, 1.0, 1.00, 1.0f};
float alphaoverride : alphaoverride;
float4 EntityEffectControl = {0.0f, 0.0f, 0.0f, 0.0f};

//Shader Variables pulled from scripting 
float4 ShaderVariables  = {1.0f, 1.0f, 1.0f, 1.0f};

#ifdef WITHANIMATION
float4x4 boneMatrix[170] : BoneMatrixPalette;
#endif

#ifdef WITHCHARACTERCREATORMASK
float4 ColorTone[4] = {
   float4(-1.0f, 1.0f, 1.0f, 1.0f),
   float4(-1.0f, 1.0f, 1.0f, 1.0f),
   float4(-1.0f, 1.0f, 1.0f, 1.0f),
   float4(-1.0f, 1.0f, 1.0f, 1.0f)
};
float ToneMix[4] = {
   float(0.5f),
   float(0.5f),
   float(0.5f),
   float(0.5f)
};
#endif

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
float4 dl_pos[82];
float4 dl_diffuse[82];
float4 dl_angle[82];


Texture2D DiffuseMap : register( t0 );
Texture2D OcclusionMap : register( t1 );
Texture2D NormalMap : register( t2 );
Texture2D SpecularMap : register( t3 );
Texture2D NotUsed : register( t4 );
Texture2D NotUsed2 : register( t5 );
#ifdef USECUBEMAPPING
TextureCube IlluminationOrCubeMap : register( t6 );
#else
Texture2D IlluminationOrCubeMap : register( t6 );
#endif
Texture2D MaskMap : register( t11 );

SamplerState SampleWrap
{
#ifdef TRILINEAR
    Filter = MIN_MAG_MIP_LINEAR;
#else
	Filter = ANISOTROPIC;
    MaxAnisotropy = MAXANISOTROPY;
#endif
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
    float3 Position    : POSITION;
    float3 Normal      : NORMAL;
    float2 UV          : TEXCOORD0;
    float3 Tangent     : TANGENT0;
    float3 Binormal    : BINORMAL0;
	#ifdef WITHANIMATION
    float4 Blendweight  : TEXCOORD1;
    float4 Blendindices : TEXCOORD2;	
	#endif
};

struct vertexOutput
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 LightVec     : TEXCOORD1;
    float3 WorldNormal  : TEXCOORD2;
    float3 WorldTangent : TEXCOORD3;
    float3 WorldBinorm  : TEXCOORD4; 
    float4 WPos         : TEXCOORD6;
    float  clip         : TEXCOORD7;
    float2 vDepth       : TEXCOORD8;
    float3 WorldEyeVec  : TEXCOORD9;
    float3 VertexLight  : TEXCOORD10;
};

struct vertexOutput_low
{
    float4 Position     : POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 LightVec     : TEXCOORD1;
    float3 WorldNormal  : TEXCOORD2;
    float4 WPos         : TEXCOORD3;
    float2 vegshadowuv  : TEXCOORD4;
    float  clip         : TEXCOORD6;
    float  vDepth       : TEXCOORD7;
    float3 VertexLight  : TEXCOORD8;
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

/*******Main Vertex Shader***************************/

vertexOutput mainVS_highest(appdata IN)   
{
   vertexOutput OUT;
   
   float3 netPosition = 0, netNormal = 0;
   float3 netTangent = 0, netBinormal = 0;
   #ifdef WITHANIMATION
   for (int i = 0; i < 4; i++)
   {
     float index = IN.Blendindices[i];
     float3x4 model = float3x4(boneMatrix[index][0], boneMatrix[index][1], boneMatrix[index][2]);     
     float3 vec3 = mul(model, float4(IN.Position, 1));
     vec3 = vec3 + boneMatrix[index][3].xyz;
     float3x3 rotate = float3x3(model[0].xyz, model[1].xyz, model[2].xyz); 
     netPosition += vec3.xyz * IN.Blendweight[i];
     float3 norm3 = mul(IN.Normal.xyz,rotate).xyz;
     netNormal += norm3.xyz * IN.Blendweight[i];
     float3 tang3 = mul(IN.Tangent.xyz,rotate).xyz;
     netTangent += tang3.xyz * IN.Blendweight[i];
     float3 bino3 = mul(IN.Binormal.xyz,rotate).xyz;
     netBinormal += bino3.xyz * IN.Blendweight[i];
   }
   #else
   netPosition = IN.Position.xyz;
   netNormal = IN.Normal.xyz;
   netTangent = IN.Tangent.xyz;
   netBinormal = IN.Binormal.xyz;
   #endif  
   
   float4 worldSpacePos = mul(float4(netPosition,1), World);
   OUT.WPos =   worldSpacePos; 


   //PE: better tangent / binormal calculation.
   if ( abs(netNormal.y) > 0.999 ) netTangent = float3( netNormal.y,0.0,0.0 );
   else netTangent = normalize( float3(-netNormal.z, 0.0, netNormal.x) );
   netBinormal = normalize( float3(netNormal.y*netTangent.z, netNormal.z*netTangent.x-netNormal.x*netTangent.z, -netNormal.y*netTangent.x) );

   OUT.WorldNormal = normalize(mul(float4(netNormal,1), WorldIT).xyz);
   OUT.WorldTangent = normalize(mul(float4(netTangent,1), WorldIT).xyz);
   OUT.WorldBinorm = normalize(mul(float4(netBinormal,1), WorldIT).xyz);

//   OUT.WorldNormal = normalize(mul(float4(netNormal,1), WorldIT).xyz);
//   //OUT.WorldTangent = normalize(mul(float4(netTangent,1), WorldIT).xyz); //?Wrong
//   float3 c1 = cross(OUT.WorldNormal, float3(0.0, 0.0, 1.0)); 
//   float3 c2 = cross(OUT.WorldNormal, float3(0.0, 1.0, 0.0)); 
//   if (length(c1) > length(c2)) {
//     OUT.WorldTangent = c1;   
//   } else {
//     OUT.WorldTangent = c2;   
//   }
//   OUT.WorldTangent = normalize(OUT.WorldTangent); 
//    OUT.WorldBinorm = cross(OUT.WorldNormal,OUT.WorldTangent); 
//   //OUT.WorldBinorm = normalize(mul(float4(netBinormal,1), WorldIT).xyz);
      
   OUT.LightVec = normalize(LightSource.xyz);
   OUT.Position = mul(float4(netPosition,1), WorldViewProjection);
   OUT.TexCoord  = IN.UV; 
   OUT.WorldEyeVec = (ViewInverse[3].xyz - worldSpacePos.xyz);   


	OUT.VertexLight.xyz = CalcExtLightingVS(OUT.WorldNormal.xyz, worldSpacePos.xyz, OUT.WorldEyeVec );

                 
   // all shaders should send the clip value to the pixel shader (for refr/refl)                                                                     
   #ifdef NOCLIPPINGPLANE
   OUT.clip = 0;                                                                      
   #else
   OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
   #endif
   
   // SHADOW MAPPING - world position and projected depth (for cascade distance calc)
   OUT.vDepth.x = mul( float4(netPosition,1), WorldViewProjection ).z; 

   // defeat projective aliasing by detecting when parallel to light direction
   OUT.vDepth.y = max(0,abs(dot(OUT.LightVec, OUT.WorldNormal))-0.25f)*1.333f;
   
   return OUT;
}

vertexOutput_low mainVS_lowest(appdata IN)   
{
   vertexOutput_low OUT;

   float3 netPosition = 0, netNormal = 0;
   #ifdef WITHANIMATION
   for (int i = 0; i < 4; i++)
   {
     float index = IN.Blendindices[i];
     float3x4 model = float3x4(boneMatrix[index][0], boneMatrix[index][1], boneMatrix[index][2]);     
     float3 vec3 = mul(model, float4(IN.Position, 1));
     vec3 = vec3 + boneMatrix[index][3].xyz;
     float3x3 rotate = float3x3(model[0].xyz, model[1].xyz, model[2].xyz); 
     netPosition += vec3.xyz * IN.Blendweight[i];
     float3 norm3 = mul(IN.Normal.xyz,rotate).xyz;
     netNormal += norm3.xyz * IN.Blendweight[i];
   }
   #else
   netPosition = IN.Position.xyz;
   netNormal = IN.Normal.xyz;
   #endif  
   
   float4 worldSpacePos = mul(float4(netPosition,1), World);
   OUT.WPos =   worldSpacePos; 
   OUT.WorldNormal = normalize(mul(float4(netNormal,1), WorldIT).xyz);
   OUT.LightVec = normalize(LightSource.xyz);
   
   OUT.TexCoord  = IN.UV; 
   OUT.vegshadowuv = float2(worldSpacePos.x/51200.0f,worldSpacePos.z/51200.0f);
   #ifdef NOCLIPPINGPLANE
   OUT.clip = 0;                                                                      
   #else
   OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
   #endif
   OUT.vDepth = mul( float4(netPosition,1), WorldViewProjection ).z; 

	float3 WorldEyeVec = (ViewInverse[3].xyz - worldSpacePos.xyz);
	OUT.VertexLight.xyz = CalcExtLightingVS(OUT.WorldNormal.xyz, worldSpacePos.xyz, WorldEyeVec );

   // push position of geometry by lightdirection (creates depth bias and slope bias) - not needed as bias code in effect
   //OUT.Position = mul(float4(netPosition,1), World);
   //OUT.Position -= (float4(OUT.LightVec.xyz,0)*0.5);
   //OUT.Position = mul(OUT.Position, mul(View, Projection));
   OUT.Position = mul(float4(netPosition,1), WorldViewProjection);
   
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

float3 CalcExtLighting(float3 Nb, float3 worldPos, float3 Vn, float3 diffusemap, float3 specmap )
{
	float3 output = GlowIntensity.xyz;
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

float4 CalcLighting(float3 Nb, float3 worldPos, float3 Vn, float4 diffusemap, float4 specmap)
{
   float4 output = GlowIntensity;
    
#ifdef SKIPIFNODYNAMICLIGHTS
   // early out - thanks Preben for the inspiration!
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
       //float fAtten = 1.0/dot( g_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten0 = float4(1.0, 1.0/g_lights_pos0.w, 1.0/(g_lights_pos0.w*g_lights_pos0.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten0, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse0 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse0 * fAtten *specmap );   
//    }   
    // light 1
    toLight = g_lights_pos1.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos1.w*4.0 ) {  
       //fAtten = 1.0/dot( g_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten1 = float4(1.0, 1.0/g_lights_pos1.w, 1.0/(g_lights_pos1.w*g_lights_pos1.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten1, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse1 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse1 * fAtten *specmap );   
//    }   
    // light 2
    toLight = g_lights_pos2.xyz - worldPos;
    lightDist = length( toLight );
//    if( lightDist < g_lights_pos2.w*4.0 ) {  
       //fAtten = 1.0/dot( g_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) ); // NVIDIA 361 bug
       float4 local_lights_atten2 = float4(1.0, 1.0/g_lights_pos2.w, 1.0/(g_lights_pos2.w*g_lights_pos2.w), 0.0);
       fAtten = 1.0/dot( local_lights_atten2, float4(1,lightDist,lightDist*lightDist,0) );
       lightDir = normalize( toLight );
       halfvec = normalize(Vn + lightDir);
       lit0 = lit(dot(lightDir,Nb),dot(halfvec,Nb),24); 
       output+= (lit0.y *g_lights_diffuse2 * fAtten * 1.7*diffusemap) + (lit0.z * g_lights_diffuse2 * fAtten *specmap );   
//    }   
    return output;
}

#ifdef WITHCHARACTERCREATORMASK
float4 CharacterCreatorDiffuse(float4 diffusemap,float2 uv)
{
	float amountfromMask[4];
	float amountfromPixel[4];
	float4 maskmap = MaskMap.Sample(SampleWrap,uv);
	amountfromMask[0] = maskmap.r * ToneMix[0];
	amountfromMask[1] = maskmap.g * ToneMix[1];
	amountfromMask[2] = maskmap.b * ToneMix[2];
	amountfromMask[3] = maskmap.a * ToneMix[3];
	for ( int c = 0 ; c < 4 ; c++ )
	{
	  if ( amountfromMask[c] > 0.0 && ColorTone[c].r >= 0.0f )
	  {
		 amountfromPixel[c] = 1.0f - amountfromMask[c];
		 diffusemap = (diffusemap * amountfromPixel[c]) + (ColorTone[c] * amountfromMask[c]);
	  }
	}
	return diffusemap;
}
#endif

float4 mainPS_highest(vertexOutput IN) : COLOR
{
    // clip
    clip(IN.clip);
	
    // source textures
    float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy);
	#ifdef ALPHACLIP
     if( diffusemap.a < ALPHACLIP ) 
     {
       clip(-1);
     }
	 #ifndef PRESERVETRANSPARENCY
      diffusemap.a = 1; // defeat transparent edge issue
	 #endif
	#endif
	
	#ifdef WITHCHARACTERCREATORMASK
	 diffusemap = CharacterCreatorDiffuse(diffusemap,IN.TexCoord.xy);
	#endif
	
    float3 normalmap = NormalMap.Sample(SampleWrap,IN.TexCoord.xy).xyz * 2 - 1;
    float4 specmap = SpecularMap.Sample(SampleWrap,IN.TexCoord.xy);
    #ifndef USECUBEMAPPING
    #ifndef USECUBEMAPPINGWITHALPHA
    #ifndef USECUBEMAPPINGAMBIENCE
     float4 illummap = IlluminationOrCubeMap.Sample(SampleWrap,IN.TexCoord.xy);
    #endif
    #endif
    #endif
	   
    // work out normal from normmap/tangent/binormal
    float3 Ln = (IN.LightVec);
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
   
    // dynamic lighting
//    float4 spotflashlighting = CalcSpotFlash (Nb,IN.WPos.xyz);   
	float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);

//    float4 dynamicContrib = CalcLighting (Nb,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
    float4 dynamicContrib = float4( CalcExtLighting (Nb.xyz,IN.WPos.xyz,Vn.xyz,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz) , 1.0 );  

    // flash light system (flash light control carried in SpotFlashColor.w )
    float conewidth = 24;
    float4 viewspacePos = mul(IN.WPos, View);
    float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
    float3 lightvector = Vn;
    float3 lightdir = float3(View._m02,View._m12,View._m22);
    float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
    dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));

    // spherical ambience
    float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
    float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
    bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
    #ifdef USECUBEMAPPING
    float4 ambContrib = (float4(bouncelightcolor,1) * ((AmbiColor * AmbiColorOverride))) * 2;
    #else
    #ifdef BOOSTILLUM
     float4 ambContrib = (float4(bouncelightcolor,1) * ((AmbiColor * AmbiColorOverride)+float4(illummap.xxx*1.4,1))) * 2;
    #else
     float4 ambContrib = (float4(bouncelightcolor,1) * ((AmbiColor * AmbiColorOverride)+float4(illummap.xxx,1))) * 2;
    #endif
    #endif
       
	// Shadows
	int iCurrentCascadeIndex = 0;
	#ifdef NOSHADOWS
	 float fShadow = 0.0f;
    #else
	 float fShadow = GetShadow ( IN.vDepth.x, IN.WPos, IN.WorldNormal, normalize(LightSource.xyz), iCurrentCascadeIndex );
	#endif
	
	// inverse to modulate surface lighting (and only shadow surfaces facing sunlight)
    float ShadowSunFactor = lerp(1.0,dot(Ln,IN.WorldNormal),SurfaceSunFactor);
    fShadow = clamp( fShadow * ShadowSunFactor * 3.5f, 0.0 , 0.85 ); // PE: Let it look like baked shadows.
    float fInvShadow = 1.0-fShadow;
   
    // apply shadow mapping to final render
    lighting.y *= fInvShadow;

    #ifdef USECUBEMAPPINGAMBIENCE
     float4 diffuseContrib = SurfColor * diffusemap * fInvShadow * (lighting.y) * GlobalSurfaceIntensity;
     float3 refdir = IN.WorldEyeVec;
     float3 cubereflect = reflect(-refdir, Nb );
     //float4 cuberef = texCUBE( CubeSampler, cubereflect.rgb );
     float4 cuberef = IlluminationOrCubeMap.Sample(SampleClamp,cubereflect);	  
     bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * cuberef.xyz * 0.8;
     bouncelightcolor = bouncelightcolor + (cuberef.xyz * 0.2);
     float4 ambcubeContrib = (float4(bouncelightcolor,1) * ((AmbiColor * AmbiColorOverride) )) * 2;
     cuberef = SurfColor * cuberef * fInvShadow * (lighting.y) * GlobalSurfaceIntensity;
     float mixcube = clamp( (1.0-(diffusemap.a*1.1)) , 0.0 , 1.0 );
     float4 difcubecontrib = lerp( cuberef , diffuseContrib , mixcube );
     difcubecontrib = lerp( difcubecontrib+(ambcubeContrib) , (difcubecontrib+ambContrib), mixcube );
     diffuseContrib = difcubecontrib;
     ambContrib = float4(0,0,0,0);
    #else
    #ifdef USECUBEMAPPINGWITHALPHA
      float4 diffuseContrib = SurfColor * diffusemap * fInvShadow * (lighting.y) * GlobalSurfaceIntensity;
    #else
     #ifdef USECUBEMAPPING
      float3 Vnc = normalize(IN.WorldEyeVec);
      float3 reflectvect = normalize(reflect(-Vnc,Nb));
      float4 cubemap = IlluminationOrCubeMap.SampleLevel(SampleClamp,reflectvect.xyz,0);	  
      float4 diffusemapandcubemap = (diffusemap * (lighting.y)) + (cubemap * SpecularOverride * GlobalSpecular);
      float4 diffuseContrib = SurfColor * diffusemapandcubemap * fInvShadow * GlobalSurfaceIntensity;
     #else
      float4 diffuseContrib = SurfColor * diffusemap * fInvShadow * (lighting.y) * GlobalSurfaceIntensity;
     #endif
    #endif
    #endif
    
    #ifdef PEROJECTLIGHT
     float4 specContrib = lighting.z * specmap * SurfColor * fInvShadow * GlobalSpecular;
    #else
     float4 specContrib = lighting.z * specmap * SurfColor * fInvShadow * SpecularOverride * GlobalSpecular;
    #endif
    #ifdef SPECULARCAMERA
     float3 lightdirsc = normalize(float3(View._m02,View._m12,View._m22));
     lightdirsc = dot(-lightvector, lightdirsc);
     float4 lightingsc = lit(dot(lightvector,Nb),dot(lightvector,Nb),24);
     intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
     lightingsc.z = lightingsc.z * intensity;
     specContrib = ( specContrib + ( (lightingsc.z * SPECULARCAMERAINTENSITY ) * SurfColor * fInvShadow * GlobalSpecular) ) * 0.5;
    #endif
   //calculate hud pixel-fog
    float4 cameraPos = mul(IN.WPos, View);
  
#ifdef USEWATERMAPPING   
   
                                         
   float4 watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+(timer/WaterSpeed1));
   float genericwave = (watermap.b-0.5*2.0)+0.5;
   
   float3 dudv = normalize( watermap.rgb * 2.0 - 1.0) * distortion;
   float3 dudv2 = normalize( watermap.rgb * 2.0 - 1.0) * distortion2;

   watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+dudv2.rg+(timer/WaterSpeed2) );
   // pattern removal.
   watermap = lerp( watermap , DiffuseMap.Sample(SampleWrap, (IN.TexCoord.xy/4.0)+dudv2.rg+(timer/WaterSpeed2) ) , clamp( (cameraPos.z/9500.0)-0.1,0.0,0.50) );

   genericwave = clamp(  (((watermap.b-0.75)*2.80)+0.1),genericwaveIntensity,1.0);
   genericwave = ( genericwave + clamp((watermap.a+0.4)+(cameraPos.z/7000.0),genericwaveIntensity,1.0) ) * 0.5;
   float halfinvInt = (1.0-genericwaveIntensity)*0.5;
   genericwave = clamp( (1.0-genericwave)+genericwaveIntensity+(halfinvInt*0.5)  , genericwaveIntensity , 1.0+halfinvInt); // genericwaveIntensity

   // tangent/binorm from vertex shader
   float3x3 tangentbasis = float3x3( 2*normalize(Tn), 2*normalize(Bn), Nn );
   Nb = watermap.xyz;
   float3 Nb2 = watermap.xyz;

   Nb.xy = Nb.xy * 2.0 - 1.0;
   Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
   Nb = mul(Nb,tangentbasis);
   Nb = normalize((Nb+IN.WorldNormal)*0.5);

   Nb2.xy = Nb2.xy * 2.07 - 1.0; //1.2
   Nb2.z = sqrt(1.0 - dot(Nb2.xy, Nb2.xy));
   Nb2 = mul(Nb2,tangentbasis);
   Nb2 = normalize((Nb2+IN.WorldNormal)*0.5);

   //float3 Ln = normalize(IN.LightVec);
   //float3 V  = (eyePos - IN.WPos);  
   //float3 Vn  = normalize(-V); // PE: eyePos not set ? normally use (V)
   //float3 Hn = normalize(Vn+Ln);
   lighting = lit( dot(Ln,Nb),dot(Hn,Nb),24);
   //lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor); // PE: not set in this shader.
   lighting.y = clamp(lighting.y,0.8,1.0); 
             
   //float2 textureCoord = (fromcs(IN.ReflectionCords) + dudv.rg );
   //textureCoord = clamp(textureCoord, 0.001, 0.999);
   float3 Reflection = NormalMap.Sample(SampleWrap,IN.TexCoord.xy + dudv.rg).xyz;

   float3 MaterialSpecularColor = float3(0.35,0.35,0.35);
   float3 E = normalize(IN.WPos.xyz-eyePos.xyz); // eye , facing the camera.
   float3 R = reflect(Ln,Nb); // reflect direction
   float cosAlpha = clamp(dot(E,R),0.0,1.0);
   cosAlpha = pow(cosAlpha,6.0); // 6.0 shinness
   float3 spec = MaterialSpecularColor * cosAlpha;  //Specular

   //fade away lighting.z to remove more patterns.
   lighting.z = spec.z - clamp((cameraPos.z/48500.0)-0.40,0.0,spec.z);
   
   float3 WaterColor2 = (WaterCol + lighting.z ) * lighting.y * 0.75;
   
   float3 sparckle = (clamp(dot(Ln,Nb2),0.80,1.0)*1.0605)  + (lighting.z * 0.41 ); //0.35
   sparckle = clamp( sparckle - 0.980 , 0.0,0.020 ) * 40.0;

   sparckle.r = sparckle.r - clamp((cameraPos.z/3000.0)-0.2,0.0,sparckle.r);
            
   WaterColor2 = WaterColor2 * genericwave;

   //Reflection could be a cubemap.
   float3 WaterColor = lerp( WaterColor2 , Reflection , WaterReflection );
   //float3 WaterColor = WaterColor2;
#ifdef USEREFLECTIONSPARKLE
   WaterColor = lerp( WaterColor , WaterColor*reflectionSparkleIntensity , sparckle.r );
#else
   WaterColor = lerp( WaterColor , float3(WaterSparkleCol) , sparckle.r );
#endif
   diffuseContrib = float4(WaterColor,WaterTransparancy);
   diffusemap.a = diffuseContrib.a;
   //(AmbiColor * AmbiColorOverride)    
   float4 result = diffuseContrib + (AmbiColor * AmbiColorOverride) + specContrib + dynamicContrib;   

#endif
#ifndef USEWATERMAPPING  
    #ifdef PEROJECTLIGHT
     float4 result = (diffuseContrib + ambContrib + specContrib + dynamicContrib) * SpecularOverride;
    #else
     float4 result = diffuseContrib + ambContrib + specContrib + dynamicContrib;   
    #endif   
#endif
    //calculate hud pixel-fog
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
    // original entity diffuse alpha with override
    hudfogresult.a = diffusemap.a * alphaoverride;    
   
    // entity effect control can slice alpha based on a world Y position
    #ifndef NOCLIPPINGPLANE
    float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
    hudfogresult.a = hudfogresult.a * alphaslice;
    #endif
	
    #ifdef USECUBEMAPPINGWITHALPHA
    float3 refdir = IN.WorldEyeVec;
    float3 cubereflect = reflect(-refdir, Nb );
    //float4 cuberef = texCUBE( CubeSampler, cubereflect );
    float4 cuberef = IlluminationOrCubeMap.Sample(SampleClamp,cubereflect);	  
    // add cube depending on alpha.
    float bfac = clamp( 1.0-(hudfogresult.a*1.3) , 0.0 , 1.0 );
    hudfogresult = lerp( hudfogresult, float4(cuberef.rgb,hudfogresult.a), bfac );
    #endif
  
    // final pixel color
    #ifdef DEBUGSHADOW
	 hudfogresult = TintDebugShadow ( iCurrentCascadeIndex, hudfogresult );
	#endif	
    #ifdef USECUBEMAPPINGAMBIENCE
     return float4(hudfogresult.rgb,1.0);
    #else
     return hudfogresult;
    #endif
}

float4 mainPS_medium(vertexOutput_low IN) : COLOR
{
   // clip
   clip(IN.clip);
	
   // texture ref
   float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy); 
   #ifdef ALPHACLIP
    if( diffusemap.a < ALPHACLIP ) 
    {
       clip(-1);
    }
	#ifndef PRESERVETRANSPARENCY
     diffusemap.a = 1; // defeat transparent edge issue
	#endif
   #endif

   #ifdef WITHCHARACTERCREATORMASK
    diffusemap = CharacterCreatorDiffuse(diffusemap,IN.TexCoord.xy);
   #endif

   // lighting
   float3 Ln = normalize(IN.LightVec);
   float3 V  = (eyePos.xyz - IN.WPos.xyz);  
   float3 Vn  = normalize(V); 
   float3 Hn = normalize(Vn+Ln);
   float4 lighting = lit(dot(Ln,IN.WorldNormal),dot(Hn,IN.WorldNormal),24);
   lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

   // dynamic lighting
   //float4 spotflashlighting = CalcSpotFlash (IN.WorldNormal,IN.WPos.xyz);   
   float4 spotflashlighting = float4(0.0,0.0,0.0,0.0);

//   float4 dynamicContrib = CalcLighting (IN.WorldNormal,IN.WPos.xyz,Vn,diffusemap,float4(0,0,0,0)) + spotflashlighting;  
   float4 dynamicContrib = float4( CalcExtLighting (IN.WorldNormal.xyz,IN.WPos.xyz,Vn,diffusemap.xyz,float3(0,0,0)) + spotflashlighting.xyz + (IN.VertexLight.xyz * 1.25 * diffusemap.xyz) , 1.0 ); 
   
   // flash light system (flash light control carried in SpotFlashColor.w )
   float4 viewspacePos = mul(IN.WPos, View);
   float conewidth = 24;
   float intensity = max(0, 1.5f - (viewspacePos.z/500.0f));
   float3 lightvector = Vn;
   float3 lightdir = float3(View._m02,View._m12,View._m22);
   float flashlight = pow(max(dot(-lightvector, lightdir),0),conewidth) * intensity * SpotFlashColor.w;   
   dynamicContrib.xyz = dynamicContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));
   
   // paint
   float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;

   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
   float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;

   float4 cameraPos = mul(IN.WPos, View);

#ifdef USEWATERMAPPING   
                                         
   float3 Tn;  
   float3 Bn;
   float3 c1 = cross(IN.WorldNormal, float3(0.0, 0.0, 1.0)); 
   float3 c2 = cross(IN.WorldNormal, float3(0.0, 1.0, 0.0)); 
   if (length(c1) > length(c2)) {
     Tn = c1;   
   } else {
     Tn = c2;   
   }
   Tn = normalize(Tn); 
   Bn = normalize(cross(IN.WorldNormal, Tn));
   
   float4 watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+(timer/WaterSpeed1));
   float genericwave = (watermap.b-0.5*2.0)+0.5;
   
   float3 dudv = normalize( watermap.rgb * 2.0 - 1.0) * distortion;
   float3 dudv2 = normalize( watermap.rgb * 2.0 - 1.0) * distortion2;

   watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+dudv2.rg+(timer/WaterSpeed2) );
   // pattern removal.
   watermap = lerp( watermap , DiffuseMap.Sample(SampleWrap, (IN.TexCoord.xy/4.0)+dudv2.rg+(timer/WaterSpeed2) ) , clamp( (cameraPos.z/9500.0)-0.1,0.0,0.50) );

   genericwave = clamp(  (((watermap.b-0.75)*2.80)+0.1),genericwaveIntensity,1.0);
   genericwave = ( genericwave + clamp((watermap.a+0.4)+(cameraPos.z/7000.0),genericwaveIntensity,1.0) ) * 0.5;
   float halfinvInt = (1.0-genericwaveIntensity)*0.5;
   genericwave = clamp( (1.0-genericwave)+genericwaveIntensity+(halfinvInt*0.5)  , genericwaveIntensity , 1.0+halfinvInt); // genericwaveIntensity

   float3x3 tangentbasis = float3x3( 2*normalize(Tn), 2*normalize(Bn), IN.WorldNormal );
   float3 Nb = watermap.xyz;
   float3 Nb2 = watermap.xyz;

   Nb.xy = Nb.xy * 2.0 - 1.0;
   Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
   Nb = mul(Nb,tangentbasis);
   Nb = normalize((Nb+IN.WorldNormal)*0.5);

   Nb2.xy = Nb2.xy * 2.07 - 1.0; //1.2
   Nb2.z = sqrt(1.0 - dot(Nb2.xy, Nb2.xy));
   Nb2 = mul(Nb2,tangentbasis);
   Nb2 = normalize((Nb2+IN.WorldNormal)*0.5);

   //float3 Ln = normalize(IN.LightVec);
   //float3 V  = (eyePos - IN.WPos);  
   //float3 Vn  = normalize(-V); // PE: eyePos not set ? normally use (V)
   //float3 Hn = normalize(Vn+Ln);
   lighting = lit( dot(Ln,Nb),dot(Hn,Nb),24);
   //lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor); // PE: not set in this shader.
   lighting.y = clamp(lighting.y,0.8,1.0); 
             
   //float2 textureCoord = (fromcs(IN.ReflectionCords) + dudv.rg );
   //textureCoord = clamp(textureCoord, 0.001, 0.999);
   //float3 Reflection = tex2D(NormalSampler,IN.TexCoord.xy + dudv.rg).xyz;

   float3 MaterialSpecularColor = float3(0.35,0.35,0.35);
   float3 E = normalize(IN.WPos.xyz-eyePos.xyz); // eye , facing the camera.
   float3 R = reflect(Ln,Nb); // reflect direction
   float cosAlpha = clamp(dot(E,R),0.0,1.0);
   cosAlpha = pow(cosAlpha,6.0); // 6.0 shinness
   float3 spec = MaterialSpecularColor * cosAlpha;  //Specular

   //fade away lighting.z to remove more patterns.
   lighting.z = spec.z - clamp((cameraPos.z/48500.0)-0.40,0.0,spec.z);
   
   float3 WaterColor2 = (WaterCol + lighting.z ) * lighting.y * 0.75;
   
   float3 sparckle = (clamp(dot(Ln,Nb2),0.80,1.0)*1.0605)  + (lighting.z * 0.41 ); //0.35
   sparckle = clamp( sparckle - 0.980 , 0.0,0.020 ) * 40.0;

   sparckle.r = sparckle.r - clamp((cameraPos.z/3000.0)-0.2,0.0,sparckle.r);
            
   WaterColor2 = WaterColor2 * genericwave;

   //Reflection could be a cubemap.
   //float3 WaterColor = lerp( WaterColor2 , Reflection , WaterReflection );
   float3 WaterColor = WaterColor2;
#ifdef USEREFLECTIONSPARKLE
   WaterColor = lerp( WaterColor , WaterColor*reflectionSparkleIntensity , sparckle.r );
#else
   WaterColor = lerp( WaterColor , float3(WaterSparkleCol) , sparckle.r );
#endif
   diffuseContrib = float4(WaterColor,WaterTransparancy);
   diffusemap.a = diffuseContrib.a;
   //(AmbiColor * AmbiColorOverride)    
   float4 result = diffuseContrib + (AmbiColor * AmbiColorOverride) + dynamicContrib;   

#endif

#ifndef USEWATERMAPPING   
   #ifdef PEROJECTLIGHT
    float4 result = (diffuseContrib + ambContrib + dynamicContrib) * SpecularOverride;
   #else
    float4 result = diffuseContrib + ambContrib + dynamicContrib;
   #endif
#endif 
   //calculate hud pixel-fog
   float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
   // original entity diffuse alpha with override
   hudfogresult.a = diffusemap.a * alphaoverride;    
   
   // entity effect control can slice alpha based on a world Y position
   #ifndef NOCLIPPINGPLANE
   float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
   hudfogresult.a = hudfogresult.a * alphaslice;
   if( alphaslice < 0.4f ) 
   {
       clip(-1);
   }
   #endif
   
   // final pixel color
   #ifdef USECUBEMAPPINGAMBIENCE
    return float4(hudfogresult.rgb,1.0);
   #else
    return hudfogresult;
   #endif
}

float4 mainPS_lowest(vertexOutput_low IN) : COLOR
{
    // clip
    clip(IN.clip);
   
    // paint
    float4 diffusemap = DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy);
    #ifdef ALPHACLIP
     if( diffusemap.a < ALPHACLIP ) 
     {
       clip(-1);
     }
	 #ifndef PRESERVETRANSPARENCY
      diffusemap.a = 1; // defeat transparent edge issue
	 #endif
    #endif
   
	#ifdef WITHCHARACTERCREATORMASK
	 diffusemap = CharacterCreatorDiffuse(diffusemap,IN.TexCoord.xy);
	#endif
   
    // lighting
    float3 Ln = normalize(IN.LightVec);
    float3 V  = (eyePos.xyz - IN.WPos.xyz);  
    float3 Vn  = normalize(V); 
    float3 Hn = normalize(Vn+Ln);
    float4 lighting = lit(dot(Ln,IN.WorldNormal),dot(Hn,IN.WorldNormal),24);
    lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor);

    // CHEAPEST flash light system (flash light control carried in SpotFlashColor.w )
    float4 viewspacePos = mul(IN.WPos, View);
    float flashlight = (1.0f-min(1,viewspacePos.z/300.0f)) * SpotFlashColor.w;   

   float4 diffuseContrib = SurfColor * diffusemap * lighting.y * GlobalSurfaceIntensity;
   diffuseContrib.xyz = diffuseContrib.xyz + (diffusemap.xyz*float3(flashlight,flashlight,flashlight));

   // spherical ambience
   float fSkyFloorRatio = (1+dot(IN.WorldNormal.xyz,float3(0,1,0)))/2;
   float3 bouncelightcolor = lerp(FloorColor.xyz,SkyColor.xyz,fSkyFloorRatio) * diffusemap.xyz * 0.8;
   bouncelightcolor = bouncelightcolor + (diffusemap.xyz * 0.2);
   float4 ambContrib = float4(bouncelightcolor,1) * AmbiColor * AmbiColorOverride * 2;

   float4 cameraPos = mul(IN.WPos, View);

#ifdef USEWATERMAPPING   
                                         
   float3 Tn;  
   float3 Bn;
   float3 c1 = cross(IN.WorldNormal, float3(0.0, 0.0, 1.0)); 
   float3 c2 = cross(IN.WorldNormal, float3(0.0, 1.0, 0.0)); 
   if (length(c1) > length(c2)) {
     Tn = c1;   
   } else {
     Tn = c2;   
   }
   Tn = normalize(Tn); 
   Bn = normalize(cross(IN.WorldNormal, Tn));
   
   float4 watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+(timer/WaterSpeed1));
   float genericwave = (watermap.b-0.5*2.0)+0.5;
   
   float3 dudv = normalize( watermap.rgb * 2.0 - 1.0) * distortion;
   float3 dudv2 = normalize( watermap.rgb * 2.0 - 1.0) * distortion2;

   watermap = DiffuseMap.Sample(SampleWrap, IN.TexCoord.xy+dudv2.rg+(timer/WaterSpeed2) );
   // pattern removal.
   watermap = lerp( watermap , DiffuseMap.Sample(SampleWrap, (IN.TexCoord.xy/4.0)+dudv2.rg+(timer/WaterSpeed2) ) , clamp( (cameraPos.z/9500.0)-0.1,0.0,0.50) );

   genericwave = clamp(  (((watermap.b-0.75)*2.80)+0.1),genericwaveIntensity,1.0);
   genericwave = ( genericwave + clamp((watermap.a+0.4)+(cameraPos.z/7000.0),genericwaveIntensity,1.0) ) * 0.5;
   float halfinvInt = (1.0-genericwaveIntensity)*0.5;
   genericwave = clamp( (1.0-genericwave)+genericwaveIntensity+(halfinvInt*0.5)  , genericwaveIntensity , 1.0+halfinvInt); // genericwaveIntensity

   float3x3 tangentbasis = float3x3( 2*normalize(Tn), 2*normalize(Bn), IN.WorldNormal );
   float3 Nb = watermap.xyz;
   float3 Nb2 = watermap.xyz;

   Nb.xy = Nb.xy * 2.0 - 1.0;
   Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
   Nb = mul(Nb,tangentbasis);
   Nb = normalize((Nb+IN.WorldNormal)*0.5);

   Nb2.xy = Nb2.xy * 2.07 - 1.0; //1.2
   Nb2.z = sqrt(1.0 - dot(Nb2.xy, Nb2.xy));
   Nb2 = mul(Nb2,tangentbasis);
   Nb2 = normalize((Nb2+IN.WorldNormal)*0.5);

   //float3 Ln = normalize(IN.LightVec);
   //float3 V  = (eyePos - IN.WPos);  
   //float3 Vn  = normalize(-V); // PE: eyePos not set ? normally use (V)
   //float3 Hn = normalize(Vn+Ln);
   lighting = lit( dot(Ln,Nb),dot(Hn,Nb),24);
   //lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor); // PE: not set in this shader.
   lighting.y = clamp(lighting.y,0.8,1.0); 
             
   //float2 textureCoord = (fromcs(IN.ReflectionCords) + dudv.rg );
   //textureCoord = clamp(textureCoord, 0.001, 0.999);
   //float3 Reflection = tex2D(NormalSampler,IN.TexCoord.xy + dudv.rg).xyz;

   float3 MaterialSpecularColor = float3(0.35,0.35,0.35);
   float3 E = normalize(IN.WPos.xyz-eyePos.xyz); // eye , facing the camera.
   float3 R = reflect(Ln,Nb); // reflect direction
   float cosAlpha = clamp(dot(E,R),0.0,1.0);
   cosAlpha = pow(cosAlpha,6.0); // 6.0 shinness
   float3 spec = MaterialSpecularColor * cosAlpha;  //Specular

   //fade away lighting.z to remove more patterns.
   lighting.z = spec.z - clamp((cameraPos.z/48500.0)-0.40,0.0,spec.z);
   
   float3 WaterColor2 = (WaterCol + lighting.z ) * lighting.y * 0.75;
   
   float3 sparckle = (clamp(dot(Ln,Nb2),0.80,1.0)*1.0605)  + (lighting.z * 0.41 ); //0.35
   sparckle = clamp( sparckle - 0.980 , 0.0,0.020 ) * 40.0;

   sparckle.r = sparckle.r - clamp((cameraPos.z/3000.0)-0.2,0.0,sparckle.r);
            
   WaterColor2 = WaterColor2 * genericwave;

   //Reflection could be a cubemap.
   //float3 WaterColor = lerp( WaterColor2 , Reflection , WaterReflection );
   float3 WaterColor = WaterColor2;
#ifdef USEREFLECTIONSPARKLE
   WaterColor = lerp( WaterColor , WaterColor*reflectionSparkleIntensity , sparckle.r );
#else
   WaterColor = lerp( WaterColor , float3(WaterSparkleCol) , sparckle.r );
#endif
   diffuseContrib = float4(WaterColor,WaterTransparancy);
   diffusemap.a = diffuseContrib.a;
   //(AmbiColor * AmbiColorOverride)    
   float4 result = diffuseContrib + (AmbiColor * AmbiColorOverride);   

#endif
#ifndef USEWATERMAPPING
#ifdef PEROJECTLIGHT
   float4 result = (diffuseContrib + ambContrib) * SpecularOverride;
#else
   float4 result = diffuseContrib + ambContrib;
  #endif
#endif
   //calculate hud pixel-fog
    float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
    float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),hudfogfactor*HudFogColor.w);
   
   // original entity diffuse alpha with override
    hudfogresult.a = diffusemap.a * alphaoverride;    
   
   // entity effect control can slice alpha based on a world Y position
   float alphaslice = 1.0f - min(1,max(0,IN.WPos.y - EntityEffectControl.x)/50.0f);
   hudfogresult.a = hudfogresult.a * alphaslice;
   
   // final pixel color
   #ifdef USECUBEMAPPINGAMBIENCE
    return float4(hudfogresult.rgb,1.0);
   #else
    return hudfogresult;
   #endif
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
   clip(IN.clip);
#ifndef USEWATERMAPPING   
   #ifndef USECUBEMAPPINGAMBIENCE
	#ifdef SHADOWALPHACLIP
     if( DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy).a < SHADOWALPHACLIP ) 
     {
       clip(-1);
	   return float4(0,0,0,1);
     }
	#else
     #ifdef ALPHACLIP
      if( DiffuseMap.Sample(SampleWrap,IN.TexCoord.xy).a < ALPHACLIP ) 
      {
       clip(-1);
	   return float4(0,0,0,1);
      }
     #endif
    #endif
   #endif
#endif
   return float4(0,0,0,1);
}

#ifdef ALPHATOCOVERAGE
BlendState CoverageAdd
{
    AlphaToCoverageEnable = TRUE;
    BlendEnable[0] = TRUE;
};
#endif

technique11 Highest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_highest()));
        SetGeometryShader(NULL);
		#ifdef ALPHATOCOVERAGE
		 SetBlendState(CoverageAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		#endif
    }
}

technique11 Medium
{
    pass MainPass
    {
		//PE: We dont really have any difference now, speed is controlled in code by lowering cascades.
        SetVertexShader(CompileShader(vs_5_0, mainVS_highest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_highest()));
        //SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        //SetPixelShader(CompileShader(ps_5_0, mainPS_medium()));
        SetGeometryShader(NULL);
		#ifdef ALPHATOCOVERAGE
		 SetBlendState(CoverageAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		#endif
    }
}

technique11 Lowest
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS_lowest()));
        SetPixelShader(CompileShader(ps_5_0, mainPS_lowest()));
        SetGeometryShader(NULL);
		#ifdef ALPHATOCOVERAGE
		 SetBlendState(CoverageAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		#endif
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
