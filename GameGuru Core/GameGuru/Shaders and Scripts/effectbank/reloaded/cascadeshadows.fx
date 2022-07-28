// Cascade Shadow Mapping
matrix          m_mShadow;
float4          m_vCascadeOffset[8];
float4          m_vCascadeScale[8];
int             m_nCascadeLevels;
float           m_fMinBorderPadding;     
float           m_fMaxBorderPadding;
float           m_fShadowBiasFromGUI;  
float           m_fCascadeBlendArea;
float           m_fTexelSize; // PE: looks fixed
float           m_fCascadeFrustumsEyeSpaceDepths[8];
float3          m_vLightDir;
float 			ShadowStrength = 1.0f;
float4 			LightSource = {-1.0f, -1.0f, -1.0f, 1.0f};

Texture2D DepthMapTX1 : register( t24 );
Texture2D DepthMapTX2 : register( t25 );
Texture2D DepthMapTX3 : register( t26 );
Texture2D DepthMapTX4 : register( t27 );
Texture2D DepthMapTX5 : register( t28 );
Texture2D DepthMapTX6 : register( t29 );
Texture2D DepthMapTX7 : register( t30 );
Texture2D DepthMapTX8 : register( t31 );

SamplerComparisonState cmpSampler
{
   Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
   //Filter = COMPARISON_MIN_MAG_MIP_POINT;
   AddressU = Clamp;
   AddressV = Clamp;
   ComparisonFunc = GREATER;
};

void ComputeCoordinatesTransform( in int iCascadeIndex, in float4 InterpolatedPosition, in out float4 vShadowTexCoord ) 
{
    float4 vLightDot = mul ( InterpolatedPosition, m_mShadow );
    vLightDot *= m_vCascadeScale[iCascadeIndex];
    vLightDot += m_vCascadeOffset[iCascadeIndex];
    vShadowTexCoord.xyz = vLightDot.xyz;
} 

// clever way to randomise shadow sample locked to model position xyz
// int index = int(16.0 * random(floor(attributes.position.xyz * 1000.0), ii)) % 16;
// visibility -= (1.0 / 4.0) * (1.0 - texture(u_ShadowMap, float3(fs_in.shadowCoord.xy + poissonDisk[index] / 700.0, (fs_in.shadowCoord.z - bias) / fs_in.shadowCoord.w)));

float PCF2x2(Texture2D depths, float2 uv, float compare, float projectionweight)
{
    float result = 0.0;
    for(int x=0; x<=1; x++)
	{
        for(int y=0; y<=1; y++)
		{
            float2 off = float2(x,y)*m_fTexelSize;
            result += depths.SampleCmpLevelZero( cmpSampler, uv+off, compare );
        }
    }
    return result/4.0;
}

// PE: better poisson disk spreading.
const uint poissonDisknum = 23;
static const float2 poissonDisk[23] =
   {
      float2(0.0f, 0.0f), // PCF
      float2(0.55f, 0.55f), // PCF
      float2(-0.55f, -0.55f), // PCF
      float2(0.2770745f, 0.6951455f),
      float2(0.1874257f, -0.02561589f),
      float2(-0.3381929f, 0.8713168f),
      float2(0.5867746f, 0.1087471f),
      float2(-0.3078699f, 0.188545f),
      float2(0.7993396f, 0.4595091f),
      float2(-0.09242552f, 0.5260149f),
      float2(0.3657553f, -0.5329605f),
      float2(-0.3829718f, -0.2476171f),
      float2(-0.01085108f, -0.6966301f),
      float2(0.8404155f, -0.3543923f),
      float2(-0.5186161f, -0.7624033f),
      float2(-0.8135794f, 0.2328489f),
      float2(-0.784665f, -0.2434929f),
      float2(0.9920505f, 0.0855163f),
      float2(-0.687256f, 0.6711345f),
      float2(0.0f, 0.5f), // PCF
      float2(0.5f, 0.0f), // PCF
      float2(0.0f, -0.5f), // PCF
      float2(-0.5f, 0.0f)
   };

float PCF(Texture2D depths, float2 uv, float compare, float2 mwpos, float fCurrentPixelDepth, int iCurrentCascadeIndex )
{
    float result = 0.0;
    //float fTexelSize= 1.0/ 1100; //PE: larger texel for better spreading.
	float totaldist = m_fCascadeFrustumsEyeSpaceDepths[m_nCascadeLevels-1] * 0.30; //PE: 0.35 Larger range before 2 samples. PE: * 0.30 high samples only close up.
    int sruns = clamp( SHADOWQUALITY-floor( fCurrentPixelDepth/(totaldist/SHADOWQUALITY) ) , 2, SHADOWQUALITY ); //PE: for now dont go lower then 3.

	float fTexelSize = 1.0 / (1024.0 + (iCurrentCascadeIndex * 300)); //PE: more spreading when using backfaces.

    //if( sruns == SHADOWQUALITY ) return(0.0); //PE: debug see where last samples is.
    //if( sruns == 8 ) return(0.0); //PE: debug see where 8 samples is.
    //if( sruns == 2 ) return(0.0); //PE: to see when we go to 2 samples.
	//result = depths.SampleCmpLevelZero(cmpSampler, uv , compare); //Expand from this, not included in divide.

	for( int runs = 0; runs < sruns ; runs++ ) {
            float2 off = ( poissonDisk[ 4+runs ] ) * fTexelSize; //m_fTexelSize;
            result += depths.SampleCmpLevelZero( cmpSampler, uv+off, compare );
    }
    return clamp(result/sruns,0.0,1.0);
}


void CalculatePCFPercentLit ( in float fCurrentPixelDepth , in int iCurrentCascadeIndex, in float sbias, in float fRemoveProjectionBias, in float4 vShadowTexCoord, in float2 mwpos, out float fPercentLit ) 
{
   // offset z depth value with sbias to defeat distance rendering
//   vShadowTexCoord.z -= sbias;
//   vShadowTexCoord.z -= sbias;

   vShadowTexCoord.z += sbias;

   // Use PCF to sample the depth map and return a percent lit value.
   fPercentLit = 0.0f;
   Texture2D depthTexture;
   if ( iCurrentCascadeIndex==0 )
   {
	  depthTexture = DepthMapTX1;
      #ifdef BETTERSHADOWS
       //fPercentLit = PCF2x2(depthTexture, vShadowTexCoord.xy, vShadowTexCoord.z, fRemoveProjectionBias );
       fPercentLit = PCF(depthTexture, float2(vShadowTexCoord.x,vShadowTexCoord.y) , vShadowTexCoord.z,mwpos , fCurrentPixelDepth , iCurrentCascadeIndex);
      #else
       fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
      #endif
   }
   else if ( iCurrentCascadeIndex==1 )
   {
	  depthTexture = DepthMapTX2;
      #ifdef BETTERSHADOWS
       //fPercentLit = PCF2x2(depthTexture, vShadowTexCoord.xy , vShadowTexCoord.z, fRemoveProjectionBias );
       fPercentLit = PCF(depthTexture, float2(vShadowTexCoord.x,vShadowTexCoord.y) , vShadowTexCoord.z,mwpos , fCurrentPixelDepth, iCurrentCascadeIndex);
      #else
       fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
      #endif
   }
   else if ( iCurrentCascadeIndex==2 )
   {
	  depthTexture = DepthMapTX3;
      #ifdef BETTERSHADOWS
       //fPercentLit = PCF2x2(depthTexture, vShadowTexCoord.xy , vShadowTexCoord.z, fRemoveProjectionBias );
       fPercentLit = PCF(depthTexture, float2(vShadowTexCoord.x,vShadowTexCoord.y) , vShadowTexCoord.z,mwpos , fCurrentPixelDepth, iCurrentCascadeIndex);
      #else
       fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
	  #endif
   }
   else if ( iCurrentCascadeIndex==3 )
   {
	  depthTexture = DepthMapTX4;
      #ifdef BETTERSHADOWS
       //fPercentLit = PCF2x2(depthTexture, vShadowTexCoord.xy , vShadowTexCoord.z, fRemoveProjectionBias );
       fPercentLit = PCF(depthTexture, float2(vShadowTexCoord.x,vShadowTexCoord.y) , vShadowTexCoord.z,mwpos , fCurrentPixelDepth, iCurrentCascadeIndex);
      #else
       fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
	  #endif
   }
   else if ( iCurrentCascadeIndex==4 )
   {
	  depthTexture = DepthMapTX5;
      fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
   }
   else if ( iCurrentCascadeIndex==5 )
   {
	  depthTexture = DepthMapTX6;
      fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
   }
   else if ( iCurrentCascadeIndex==6 )
   {
	  depthTexture = DepthMapTX7;
      fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
   }
   else if ( iCurrentCascadeIndex==7 && vShadowTexCoord.z<1.0 )
   {
	  depthTexture = DepthMapTX8;
      fPercentLit += depthTexture.SampleCmpLevelZero( cmpSampler, float2(vShadowTexCoord.x,vShadowTexCoord.y), vShadowTexCoord.z );
   }
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

float GetShadowCore ( int iCurrentCascadeIndex, float fCurrentPixelDepth, float4 finalwpos, float3 WorldNormal, float3 Ln, int iBlendWithNext )
{
	// value to return
    float fShadow = 0.0f;	
	
	// determine if surface can 'catch' shadow cast
	float fCropForWhenLightParallelToSurfaceNormal = 0.0f; // 0.3f - good for building walls
	float fCanCatchShadow = max(0,dot(WorldNormal,Ln)-fCropForWhenLightParallelToSurfaceNormal);
	//fCanCatchShadow = 1.0;
    // shadow mapping code
    float4 vShadowMapTextureCoord = 0.0f;
    if ( ShadowStrength > 0.0f && fCanCatchShadow > 0.0f )
    {
      // reset shadow vars
      float fPercentLit = 0.0f;

	  // repeat text coord calculations for the next cascade the next cascade index is used for blurring between maps
      int iNextCascadeIndex = 1;
      iNextCascadeIndex = min ( m_nCascadeLevels - 1, iCurrentCascadeIndex + 1 ); // PE: make sure we can adjust cascades in setup.ini
      float fBlendBetweenCascadesAmount = 1.0f;
      float fCurrentPixelsBlendBandLocation = 1.0f;
      CalculateBlendAmountForInterval ( iCurrentCascadeIndex, fCurrentPixelDepth, 
         fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );

      // offset shadow pixel depth with surface bias and distance bias
	  // now done by offsetting all geometry rendered to shadow map in light direction (DepthMap technique)
//      float sbias = 0.0001f + (fCurrentPixelDepth/12000000.0f); //PE: (frontfaces) better when FAR away.
	  float sbias = 0.0f; //-0.000075f; //PE:(backfaces)
      //float sbias = -(fCurrentPixelDepth/12000000.0f); //PE: (frontfaces) better when FAR away.

	  // projection aliasing should reduce offsets of PCF 'and' adjust bias for depth compare
//	  float fRemoveProjectionBias = max(0,dot(WorldNormal,Ln)-0.4f)*1.666f;
	  float fRemoveProjectionBias = max(0, dot(WorldNormal, -Ln) - 0.4f)*1.666f;
	  //sbias -= max(0,(0.001f - (fRemoveProjectionBias*0.004761f))); // reduces full bias to zero when fRemoveProjectionBias is 0.21f
	  //sbias -= max(0,(0.0001f - (fRemoveProjectionBias*0.0004761f))); // reduces full bias to zero when fRemoveProjectionBias is 0.21f

      // work out texture coordinate into specified shadow map
      ComputeCoordinatesTransform( iCurrentCascadeIndex, finalwpos, vShadowMapTextureCoord );    

      // work out how much shadow
      CalculatePCFPercentLit ( fCurrentPixelDepth,iCurrentCascadeIndex, sbias, fRemoveProjectionBias, vShadowMapTextureCoord, finalwpos.xz, fShadow );
      if( fCurrentPixelsBlendBandLocation < m_fCascadeBlendArea && iBlendWithNext == 1 ) 
      {  
         // the current pixel is within the blend band.
         // Repeat text coord calculations for the next cascade. 
         // The next cascade index is used for blurring between maps.
         float4 vShadowMapTextureCoord_blend = 0.0f;
         ComputeCoordinatesTransform( iNextCascadeIndex, finalwpos, vShadowMapTextureCoord_blend );  
      
         // the current pixel is within the blend band.
         float fPercentLit_blend = 0.0f;
         CalculatePCFPercentLit ( fCurrentPixelDepth,iNextCascadeIndex, sbias, fRemoveProjectionBias, vShadowMapTextureCoord_blend, finalwpos.xz, fPercentLit_blend );
                           
         // Blend the two calculated shadows by the blend amount.
         fShadow = lerp( fPercentLit_blend, fShadow, fBlendBetweenCascadesAmount ); 
      }
 	  fShadow = fShadow * fCanCatchShadow;
    }
	
	// fade out at end of cascade range
	float fFinalFadeOut = 1.0f;
	if ( fCurrentPixelDepth > m_fCascadeFrustumsEyeSpaceDepths[m_nCascadeLevels-1] )
	{
	  float fDistFromEnd = fCurrentPixelDepth - m_fCascadeFrustumsEyeSpaceDepths[m_nCascadeLevels-1];
	  fFinalFadeOut = 1.0f - min( 1.0f, fDistFromEnd/500.0f );
	}
	fShadow = fShadow * fFinalFadeOut;
	
    // finally modulate shadow with strength
//    fShadow = min ( (fShadow * 4.0f * ShadowStrength), 1.0f );
    fShadow = min ( (fShadow * 3.0f * ShadowStrength), 1.0f ); //PE: Shadow can now go darker , so extent the range.

	// return final shadow value
	return fShadow;
}

float GetShadow ( float fCurrentPixelDepth, float4 finalwpos, float3 WorldNormal, float3 Ln, out int iCurrentCascadeIndex )
{
    // the interval based selection technique compares the pixel's depth against the frustum's cascade divisions
    if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[0] ) 
	{
		iCurrentCascadeIndex = 0;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[1] ) 
	{
		iCurrentCascadeIndex = 1;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[2] ) 
	{
		iCurrentCascadeIndex = 2;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[3] ) 
	{
		iCurrentCascadeIndex = 3;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[4] ) 
	{
		iCurrentCascadeIndex = 4;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[5] ) 
	{
		iCurrentCascadeIndex = 5;
	}
	else if ( fCurrentPixelDepth < m_fCascadeFrustumsEyeSpaceDepths[6] ) 
	{
		iCurrentCascadeIndex = 6;
	}
	else
	{
		iCurrentCascadeIndex = m_nCascadeLevels-1; //7; PE: make sure we can adjust cascades in setup.ini
	}
	return GetShadowCore ( iCurrentCascadeIndex, fCurrentPixelDepth, finalwpos, WorldNormal, Ln, 1 );
}

float GetShadowCascade ( int iCurrentCascadeIndex, float4 finalwpos, float3 WorldNormal, float3 Ln )
{
	//PE: This is only used from old non PBR shaders , make sure they always works, and use the most distance cascade.
	if( iCurrentCascadeIndex > (m_nCascadeLevels-1) ) iCurrentCascadeIndex = m_nCascadeLevels-1;
	return GetShadowCore ( iCurrentCascadeIndex, m_fCascadeFrustumsEyeSpaceDepths[iCurrentCascadeIndex], finalwpos, WorldNormal, Ln, 0 );
}

float4 TintDebugShadow ( in int iCurrentCascadeIndex, in float4 finalColor )
{

	if( iCurrentCascadeIndex == 0 ) { finalColor = float4( finalColor.r, finalColor.g*0.35, finalColor.b*0.35, finalColor.a ); }
	if( iCurrentCascadeIndex == 1 ) { finalColor = float4( finalColor.r*0.35, finalColor.g, finalColor.b*0.35, finalColor.a ); }
	if( iCurrentCascadeIndex == 2 ) { finalColor = float4( finalColor.r*0.35, finalColor.g*0.35, finalColor.b, finalColor.a ); }
	if( iCurrentCascadeIndex == 3 ) { finalColor = float4( finalColor.r, finalColor.g, finalColor.b*0.35, finalColor.a ); }
	if( iCurrentCascadeIndex == 4 ) { finalColor = float4( finalColor.r*0.35, finalColor.g, finalColor.b, finalColor.a ); }
	if( iCurrentCascadeIndex == 5 ) { finalColor = float4( finalColor.r, finalColor.g, finalColor.b, finalColor.a ); }
	if( iCurrentCascadeIndex == 6 ) { finalColor = float4( finalColor.r*0.5, finalColor.g*0.1, finalColor.b*0.1, finalColor.a ); }
	if( iCurrentCascadeIndex == 7 ) { finalColor = float4( finalColor.r*0.1, finalColor.g*0.5, finalColor.b*0.1, finalColor.a ); }

	return finalColor;
}
