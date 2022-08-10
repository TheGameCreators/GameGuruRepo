#include "constantbuffers.fx"
#include "settings.fx"
//#define SHOW_SSAO
#define BLUR_SSAO

//#define LUMINANCE_CONSIDERATION
extern float luminosity_threshold = 0.6;
#define FAR_PLANE_Z 1000.0
static const float3 gOffsetVectors[14] =
{
   float3(1.0f, 1.0f, 1.0f),
   float3(-1.0f, -1.0f, -1.0f),
   float3(-1.0f, 1.0f, 1.0f),
   float3(1.0f, -1.0f, -1.0f),
   float3(1.0f, 1.0f, -1.0f),
   float3(-1.0f, -1.0f, 1.0f),
   float3(-1.0f, 1.0f, -1.0f),
   float3(1.0f, -1.0f, 1.0f),
   float3(-1.0f, 0.0f, 0.0f),
   float3(1.0f, 0.0f, 0.0f),
   float3(0.0f, -1.0f, 0.0f),
   float3(0.0f, 1.0f, 0.0f),
   float3(0.0f, 0.0f, -1.0f),
   float3(0.0f, 0.0f, 1.0f),
};
float gWeights[11] = { 0.015f, 0.035f, 0.25f, 0.4f, 0.8f, 1.0f, 0.8f, 0.4f, 0.25f, 0.035f, 0.015f };
                 
float2 ViewSize : ViewSize;
float deltatime : deltatime;
float time: Time;
float4 ScrollScaleUV;
float SpecularOverride;
static float2 PixelSize = float2((1.0 / ViewSize.x), (1.0 / ViewSize.y));
#ifdef CHROMATICABBERATIONLUA
   float4 ChromaticAbberation = float4(0,0,0,0);
#endif

#ifdef UNDERWATERWAVE
   float4 UnderWaterSettings = float4( 0.0 , 75.0, 0.015 , 0.0); // Active,SpeedX,Distortion
#endif

#ifdef HBAO
   static float2 FocalLength = float2(tan(0.5f*radians(75.0f)) / PixelSize.x * PixelSize.y, tan(0.5f*radians(75.0f)));
   #define HBAOSamples         9      //[7 to 36] Higher better, but less FPS.
   #define HBAOAtt              0.02    //[0.001 to 0.2] 
#endif

#define USE_LUT             
#define LUT_COLOURHEIGHT    16 
#define LUT_COLOURWIDTH     256 

#define USE_SATURATION_AND_SEPIA

//#define AMD_CAS //Contrast Adaptive Sharpening

   // LICENSE
   // =======
   // Copyright (c) 2017-2019 Advanced Micro Devices, Inc. All rights reserved.
   // -------
   // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
   // files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
   // modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
   // Software is furnished to do so, subject to the following conditions:
   // -------
   // The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
   // Software.
   // -------
   // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
   // WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
   // COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   // ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

   //------------------------------------------------------------------------------------------------------------------------------

   // SUGGESTIONS FOR INTEGRATION
   // ===========================
   // Best for performance, run CAS in sharpen-only mode, choose a video mode to have scan-out or the display scale.
   //  - Sharpen-only mode is faster, and provides a better quality sharpening.
   // The scaling support in CAS was designed for when the application wants to do Dynamic Resolution Scaling (DRS).
   //  - With DRS, the render resolution can change per frame.
   //  - Use CAS to sharpen and upsample to the fixed output resolution, then composite the full resolution UI over CAS output.
   //  - This can all happen in one compute dispatch.
   // It is likely better to reduce the amount of film grain which happens before CAS (as CAS will amplify grain).
   //  - An alternative would be to add grain after CAS.
   // It is best to run CAS after tonemapping.
   //  - CAS needs to have input value 1.0 at the peak of the display output.
   // It is ok to run CAS after compositing UI (it won't harm the UI).
   //------------------------------------------------------------------------------------------------------------------------------

   uniform float Sharpness = 0.0; // 0.5; //0.0 default

   float3 min3rgb(float3 x, float3 y, float3 z)
   {
      return min(x, min(y, z));
   }
   float3 max3rgb(float3 x, float3 y, float3 z)
   {
      return max(x, max(y, z));
   }


float4x4 g_ProjectionInv : ProjectionInverse;
float4x4 ViewProjection : ViewProjection;
float4x4 g_CameraViewMatrix : ViewProjectionTranspose; // it passes the camera zero view matrix (post process view matrix is the quad, not the camera zero view)

// For Poisson Disk Gather Technique for Depth Of Field (crude but effective)
static const float2 PoissonSamples[64] =
{
    float2(-0.5119625f, -0.4827938f),
    float2(-0.2171264f, -0.4768726f),
    float2(-0.7552931f, -0.2426507f),
    float2(-0.7136765f, -0.4496614f),
    float2(-0.5938849f, -0.6895654f),
    float2(-0.3148003f, -0.7047654f),
    float2(-0.42215f, -0.2024607f),
    float2(-0.9466816f, -0.2014508f),
    float2(-0.8409063f, -0.03465778f),
    float2(-0.6517572f, -0.07476326f),
    float2(-0.1041822f, -0.02521214f),
    float2(-0.3042712f, -0.02195431f),
    float2(-0.5082307f, 0.1079806f),
    float2(-0.08429877f, -0.2316298f),
    float2(-0.9879128f, 0.1113683f),
    float2(-0.3859636f, 0.3363545f),
    float2(-0.1925334f, 0.1787288f),
    float2(0.003256182f, 0.138135f),
    float2(-0.8706837f, 0.3010679f),
    float2(-0.6982038f, 0.1904326f),
    float2(0.1975043f, 0.2221317f),
    float2(0.1507788f, 0.4204168f),
    float2(0.3514056f, 0.09865579f),
    float2(0.1558783f, -0.08460935f),
    float2(-0.0684978f, 0.4461993f),
    float2(0.3780522f, 0.3478679f),
    float2(0.3956799f, -0.1469177f),
    float2(0.5838975f, 0.1054943f),
    float2(0.6155105f, 0.3245716f),
    float2(0.3928624f, -0.4417621f),
    float2(0.1749884f, -0.4202175f),
    float2(0.6813727f, -0.2424808f),
    float2(-0.6707711f, 0.4912741f),
    float2(0.0005130528f, -0.8058334f),
    float2(0.02703013f, -0.6010728f),
    float2(-0.1658188f, -0.9695674f),
    float2(0.4060591f, -0.7100726f),
    float2(0.7713396f, -0.4713659f),
    float2(0.573212f, -0.51544f),
    float2(-0.3448896f, -0.9046497f),
    float2(0.1268544f, -0.9874692f),
    float2(0.7418533f, -0.6667366f),
    float2(0.3492522f, 0.5924662f),
    float2(0.5679897f, 0.5343465f),
    float2(0.5663417f, 0.7708698f),
    float2(0.7375497f, 0.6691415f),
    float2(0.2271994f, -0.6163502f),
    float2(0.2312844f, 0.8725659f),
    float2(0.4216993f, 0.9002838f),
    float2(0.4262091f, -0.9013284f),
    float2(0.2001408f, -0.808381f),
    float2(0.149394f, 0.6650763f),
    float2(-0.09640376f, 0.9843736f),
    float2(0.7682328f, -0.07273844f),
    float2(0.04146584f, 0.8313184f),
    float2(0.9705266f, -0.1143304f),
    float2(0.9670017f, 0.1293385f),
    float2(0.9015037f, -0.3306949f),
    float2(-0.5085648f, 0.7534177f),
    float2(0.9055501f, 0.3758393f),
    float2(0.7599946f, 0.1809109f),
    float2(-0.2483695f, 0.7942952f),
    float2(-0.4241052f, 0.5581087f),
    float2(-0.1020106f, 0.6724468f),
};

float PreBloomBoost 
<
   string UIWidget = "slider";
   float UIMax = 4.0;
   float UIMin = 0.0;
   float UIStep = 0.1;
> = 2.0f;

float BloomThreshold 
<
   string UIWidget = "slider";
   float UIMax = 1.0;
   float UIMin = 0.0;
   float UIStep = 0.05;
> = 0.9;

float PostContrast
<
   string UIWidget = "slider";
   float UIMax = 5.0;
   float UIMin = 0.0;
   float UIStep = 0.001;
> = 2.0f;

float PostBrightness
<
   string UIWidget = "slider";
   float UIMax = 1.0;
   float UIMin = 0.0;
   float UIStep = 0.001;
> = 0.4f;

float4 ScreenColor
<   string UIType = "Screen Color Effect";
> = {0.0f, 0.0f, 0.0f, 0.0f};

float4 OverallColor
<   string UIType = "Overall Color Effect";
> = {1.0f, 1.0f, 1.0f, 1.0f};

float4 Vignette
<   string UIType = "Vignette";
> = {0.0f, 0.0f, 0.0f, 0.0f};

float4 Motion
<   string UIType = "Motion";
> = {0.0f, 0.0f, 0.0f, 0.0f};

float4 DepthOfField
<   string UIType = "DepthOfField";
> = {0.0f, 0.0f, 0.0f, 0.0f};

// SAOSettings = SAORadius, SAOIntensity, SAOQuality, LensFlare
float4 SAOSettings
<   string UIType = "SAOSettings";
> = {0.25f, 0.5f, 1.0f, 0.5f};

float4 HudFogDistAndAlpha : Diffuse
<   string UIName =  "Hud Fog Dist And Alpha";    
> = {0.0f, 0.0f, 0.0f, 0.0f};

//9 sample gauss filter, declare in pixel offsets convert to texel offsets in PS
float4 GaussFilter[9] =
{
    { -1,  -1, 0,  0.0625 },
    { -1,   1, 0,  0.0625 },
    {  1,  -1, 0,  0.0625 },
    {  1,   1, 0,  0.0625 },
    { -1,   0, 0,  0.125  },
    {  1,   0, 0,  0.125  },
    {  0,  -1, 0,  0.125 },
    {  0,   1, 0,  0.125 },
    {  0,   0, 0,  0.25 },
};

// RENDERCOLORTARGET only used to inform that a part of this uses dynamic render targets
//texture frame
//< 
//   string ResourceName = "";
//   float2 ViewportRatio = {1.0,1.0 };
//>;
Texture2D frame : register( t0 );
Texture2D underWaterWaveNormal : register( t1 );
Texture2D LUT : register(t2);
Texture2D NOISE : register(t3);

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
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
SamplerState SampleLUT
{
   Filter = MIN_MAG_MIP_LINEAR;
   AddressU = Clamp;
   AddressV = Clamp;
};
SamplerState SampleNOISE
{
   Filter = MIN_MAG_MIP_LINEAR;
   //AddressU = Clamp;
   //AddressV = Clamp;
   AddressU = Wrap;
   AddressV = Wrap;
};


Texture2D DepthTex;

Texture2D buffer1Img : RENDERCOLORTARGET 
< 
   string ResourceName = ""; 
>;

Texture2D buffer2Img : RENDERCOLORTARGET 
< 
   string ResourceName = ""; 
>;

Texture2D combineFrameImg : RENDERCOLORTARGET 
< 
   string ResourceName = ""; 
>;

//2x2 average luminosity texture
Texture2D AvgLum2x2Img : RENDERCOLORTARGET 
< 
   string ResourceName = ""; 
   int width = 2;
   int height = 2;
>;
//texture2D AvgLum2x2ImgSamp = sampler_state 
//{
   // Texture = < AvgLum2x2Img >;
    //MinFilter = Point; MagFilter = Point; MipFilter = Point;
    //AddressU = Clamp; AddressV = Clamp;
//};

//Average scene luminosity stored in 1x1 texture 
Texture2D AvgLumFinal : RENDERCOLORTARGET 
< 
   string ResourceName = ""; 
   int width = 1;
   int height = 1;
>;
//texture2D AvgLumFinal = sampler_state 
//{
    //Texture = < AvgLumFinal >;
    //MinFilter = Point; MagFilter = Point; MipFilter = Point;
    //AddressU = Clamp; AddressV = Clamp;
//};

//reduce image to 1/8 size for brightpass
Texture2D BrightpassImg : RENDERCOLORTARGET
< 
   string ResourceName = "";
   int width = 512;
   int height = 384;
   
>;
//texture2D BrightpassImg = sampler_state 
//{
   // Texture = < BrightpassImg >;
    //MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
    //AddressU = Clamp; AddressV = Clamp;
//};

//blur texture 1
Texture2D Blur1Img : RENDERCOLORTARGET
< 
   string ResourceName = "";
   int width = 512;
   int height = 384;
   
>;
//texture2D Blur1Img = sampler_state 
//{
   // Texture = < Blur1Img >;
    //MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
    //AddressU = Clamp; AddressV = Clamp;
//};

//blur texture 2
Texture2D Blur2Img : RENDERCOLORTARGET
< 
   string ResourceName = "";
   int width = 512;
   int height = 384;
   
>;
//texture2D Blur2Img = sampler_state 
//{
  //  Texture = < Blur2Img >;
    //MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
    //AddressU = Clamp; AddressV = Clamp;
//};

//depth of field texture
Texture2D DepthOfFieldImg : RENDERCOLORTARGET
< 
   string ResourceName = "";
>;
//texture2D DepthOfFieldImg = sampler_state 
//{
  //  Texture = < DepthOfFieldImg >;
    //MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
    //AddressU = Clamp; AddressV = Clamp;
//};

struct input 
{
   float3 pos   : POSITION;
   float3 normal: NORMAL;
   float2 uv   : TEXCOORD0;    
};
 
struct output 
{
   float4 pos: POSITION;
   float2 uv: TEXCOORD0;
};

output VS( input IN ) 
{
   output OUT;
   float4 oPos = float4( IN.pos.xy,0.0,1.0 );
   OUT.pos = oPos;
   float2 uv = (IN.pos.xy + 1.0) / 2.0;
   uv.y = 1 - uv.y; 
   OUT.uv = uv;
   return OUT;   
}

struct VSIN
{
   float3 vertPos   : POSITION0;
   float3 normal    : NORMAL;
   float2 UVCoord   : TEXCOORD0;    
};

struct VSOUT
{
   float4 vertPos : POSITION0;
   float2 UVCoord : TEXCOORD0;
};

VSOUT FrameVS(VSIN IN) 
{
   VSOUT OUT = (VSOUT)0.0f;
   float4 oPos = float4( IN.vertPos.xy,0.0,1.0 );
   OUT.vertPos = oPos;
   float2 uv = (IN.vertPos.xy + 1.0) / 2.0;
   uv.y = 1 - uv.y; 
   OUT.UVCoord = uv;   
   return OUT;
}

float3 VSPositionFromDepth(float2 vTexCoord)
{
    // Get the depth value for this pixel
    float z = DepthTex.SampleLevel(SampleBorder, vTexCoord, 0).r;  
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    // Transform by the inverse projection matrix
    float4 vPositionVS = mul(vProjectedPos, g_ProjectionInv);  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;  
}

float3 reconstructCSFaceNormal(float3 C)
{
    return normalize(cross(ddx(C),ddy(C)));
}

float OcclusionFunction(float distZ)
{
    // Determines how much the sample point q occludes the point p as a function of distZ.
    // If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
    // depth(q) and depth(p) are sufficiently close, then we also assume q cannot
    // occlude p because q needs to be in front of p by Epsilon to occlude p.
    //
    // We use the following function to determine the occlusion.  
    // 
    //
    //       1.0     -------------\
    //               |           |  \
    //               |           |    \
    //               |           |      \ 
    //               |           |        \
    //               |           |          \
    //               |           |            \
    //  ------|------|-----------|-------------|---------|--> zv
    //        0     Eps          z0            z1        
    //
    
    float occlusion = 0.0f;
    float gOcclusionFadeStart = 0.8f;
    float gOcclusionFadeEnd = 2.0f;
    float gSurfaceEpsilon = 0.2f;
    if(distZ > gSurfaceEpsilon)
    {
        float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
        occlusion = saturate( (gOcclusionFadeEnd-distZ)/fadeLength );
    }
    
    return occlusion;    
}

float NdcDepthToViewDepth(float z_ndc)
{
    float viewZ = Projection[3][2] / (z_ndc - Projection[2][2]);
    return viewZ;
}

#ifdef HBAO

float2 GetRandomR(float2 co)
{
   float noiseX = (frac(sin(dot(co, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
   float noiseY = sqrt(1 - noiseX * noiseX);
   return float2(noiseX, noiseY);
}

float2 getRandom(in float2 uv)
{
   float2 random_size = float2(64, 64); //256.0;
   float2 g_screen_size = ViewSize * 2;
   return saturate(NOISE.SampleLevel(SampleNOISE, g_screen_size * uv / random_size, 0).xy * 2.0f - 1.0f);
}

float3 GetEyePosition(in float2 uv, in float eye_z)
{
   uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
   float3 pos = 0;
   pos.xy = (uv * FocalLength) * eye_z;
   pos.z = eye_z;
   return pos;
}

float GetLinearDepth(float2 projQ)
{
   float zFarPlane = 7000; //70000;
   float zNearPlane = 0.1;
   float depth = (DepthTex.SampleLevel(SampleClamp, projQ.xy, 0.0f).x - 0.95) * 20;
   return  1 / ((depth * ((zFarPlane - zNearPlane) / (-zFarPlane * zNearPlane)) + zFarPlane / (zFarPlane * zNearPlane)));
}

float4 SSAOCalculate3(VSOUT IN) : SV_Target
{
   float4 output = float4(1,1,1,1);
   float3 viewSpacePos = VSPositionFromDepth(IN.UVCoord);
   bool earlyOut = viewSpacePos.z > 2000.0f; //5000.0f; // || viewSpacePos.z < 25.0f;
   [branch]
   if (earlyOut)
   {
      return output;
   }

   float depth = GetLinearDepth(IN.UVCoord.xy);
   if (depth > 5.0) return float4(1.0,1.0,1.0,1.0);

   float2 sample_offset[8] =
   {
      float2(1, 0),
      float2(0.7071f, 0.7071f),
      float2(0, 1),
      float2(-0.7071f, 0.7071f),
      float2(-1, 0),
      float2(-0.7071f, -0.7071f),
      float2(0, -1),
      float2(0.7071f, -0.7071f)
   };

   float3 pos = GetEyePosition(IN.UVCoord.xy, depth);
   float3 dx = ddx(pos);
   float3 dy = ddy(pos);
   float3 norm = normalize(cross(dx,dy));

   float sample_depth = 0;
   float3 sample_pos = 0;

   float ao = 0;
   float s = 0.0;

   //float HBAORange = SAOSettings.x * 5.0;
   float radius = (((SAOSettings.x - 0) * (1.0 - 0.5)) / (1.0 - 0.0)) + 0.5;
   float HBAORange = radius * 5.0;

   //float2 rand_vec = GetRandomR(IN.UVCoord.xy); //uses formula
   float2 rand_vec = getRandom(IN.UVCoord); //uses texture

   float2 sample_vec_divisor = FocalLength * depth / (HBAORange*PixelSize.xy);
   float2 sample_center = IN.UVCoord.xy;

   for (float i = 0; i < 8; i++)
   {
      float theta,temp_theta,temp_ao,curr_ao = 0;
      float3 occlusion_vector = 0.0;

      float2 sample_vec = reflect(sample_offset[i], rand_vec);
      sample_vec /= sample_vec_divisor;
      float2 sample_coords = (sample_vec*float2(1,(float)ViewSize.x / (float)ViewSize.y)) / HBAOSamples;

      for (float k = 1; k <= HBAOSamples; k++)
      {
         float2 GLD = 0;
         GLD.xy = sample_center + sample_coords * (k - 0.5 * (i % 2));
         sample_depth = GetLinearDepth(GLD);

         sample_pos = GetEyePosition(sample_center + sample_coords * (k - 0.5*(i % 2)), sample_depth);
         occlusion_vector = sample_pos - pos;
         temp_theta = dot(norm, normalize(occlusion_vector));

         if (temp_theta > theta)
         {
            theta = temp_theta;
            temp_ao = 1 - sqrt(1 - theta * theta);
            ao += (1 / (1 + HBAOAtt * pow(length(occlusion_vector) / HBAORange * 5000,2)))*(temp_ao - curr_ao);
            curr_ao = temp_ao;
         }
      }
      s += 1;
   }

   ao /= max(0.00001,s);
   ao = 1.0 - ao * (10.0f*SAOSettings.y);
   ao = clamp(ao,0,1);

   return float4(ao.xxx, 1.0);
}

#endif


float4 SSAOCalculate(VSOUT IN) : COLOR0
{
    float4 output = float4(1,1,1,1);
    float3 viewSpacePos = VSPositionFromDepth(IN.UVCoord);
    float4 worldSpacePos = mul(float4(viewSpacePos,1), ViewInv);
    float3 viewSpaceNormal = reconstructCSFaceNormal(viewSpacePos);   
   
    bool earlyOut = viewSpacePos.z > FAR_PLANE_Z || viewSpacePos.z < 3.0f;
    [branch]
    if(earlyOut)
    {
        return output;
    }
   
   // fade in SAO when emerge from distance
   float fFadeIn = min(max((FAR_PLANE_Z - viewSpacePos.z)/100.0f,0.0f),1.0f);
   
    // p -- the point we are computing the ambient occlusion for.
    // n -- normal vector at p.
    // q -- a random offset from p.
    // r -- a potential occluder that might occlude p.

    // Get viewspace normal and z-coord of this pixel.  The tex-coords for
    // the fullscreen quad we drew are already in uv-space.
    float4 normalDepth = float4(viewSpaceNormal.xyz,viewSpacePos.z);
    float3 n = normalDepth.xyz;
    float pz = normalDepth.w;
   
   float3 worldNormal = mul(float4(n,0),ViewInv).xyz;

    // Reconstruct full view space position (x,y,z).
    // Find t such that p = t*pin.ToFarPlane.
    // p.z = t*pin.ToFarPlane.z
    // t = p.z / pin.ToFarPlane.z
    float3 p = viewSpacePos;
    
    // Sample neighboring points about p in the hemisphere oriented by n.
   int gSampleCount = 20;
    float occlusionSum = 0.0f;
    [unroll]
    for(int i = 0; i < gSampleCount; ++i)
    {
      // lets make a random half-hemisphere pointing into scene
      float3 offset = normalize(float3(-0.5f+((float)i/20.0f),PoissonSamples[i].y*0.5f,1.0f-((float)i/30.0f)));

      // rotate it to normal direction facing away from surface
        float3 c1 = cross(n, float3(0.0, 0.0, 1.0)); 
        float3 c2 = cross(n, float3(0.0, 1.0, 0.0)); 
       float3 tangent; 
       float3 bitangent; 
        if (length(c1) > length(c2)) {
       tangent = normalize(c1);   
       } else {
       tangent = normalize(c2);   
       }
        bitangent = normalize(cross(tangent,n));   
      float3x3 nrotate = float3x3(tangent, bitangent, n); 
      offset = mul(offset,nrotate).xyz;
   
        // Flip offset vector if it is behind the plane defined by (p, n).
        float flip = sign( dot(offset, n) );
        
        // Sample a point near p within the occlusion radius.
      float gOcclusionRadius  = 2.0f * SAOSettings.x;
        float3 q = p + flip * gOcclusionRadius * offset;
      
        // Project q and generate projective tex-coords.  
        float4 projQ = mul(float4(q, 1.0f), Projection);
        projQ /= projQ.w;
      projQ.x = 0.5f + (projQ.x*0.5f);
      projQ.y = 0.5f + (projQ.y*-0.5f);
    
        // Find the nearest depth value along the ray from the eye to q (this is not
        // the depth of q, as q is just an arbitrary point near p and might
        // occupy empty space).  To find the nearest depth we look it up in the depthmap.
        float rz = DepthTex.SampleLevel(SampleBorder, projQ.xy, 0.0f).x;
      rz = NdcDepthToViewDepth(rz);
      
        // Reconstruct full view space position r = (rx,ry,rz).  We know r
        // lies on the ray of q, so there exists a t such that r = t*q.
        // r.z = t*q.z ==> t = r.z / q.z
        float3 r = (rz / q.z) * q;
              
        // Test whether r occludes p.
        //   * The product dot(n, normalize(r - p)) measures how much in front
        //     of the plane(p,n) the occluder point r is.  The more in front it is, the
        //     more occlusion weight we give it.  This also prevents self shadowing where 
        //     a point r on an angled plane (p,n) could give a false occlusion since they
        //     have different depth values with respect to the eye.
        //   * The weight of the occlusion is scaled based on how far the occluder is from
        //     the point we are computing the occlusion of.  If the occluder r is far away
        //     from p, then it does not occlude it.
        // 
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);
        float occlusion = dp * OcclusionFunction(distZ);
        occlusionSum += occlusion;
    }
    occlusionSum /= gSampleCount;
    
   // reduce SAO effect on geometry very close to camera, including weapons
   float fFadeOutWhenClose = 1.0f * min(max(viewSpacePos.z-10.0f,0),30)/30.0f;
   occlusionSum *= fFadeOutWhenClose;
   
    // Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
    float access = max(0,1.0f - (occlusionSum*fFadeIn));   
    return saturate(pow(access, SAOSettings.y));
}

float4 BlurBL(VSOUT IN, uniform texture2D bufferTexSamp, uniform bool isBlurHorizontal) : COLOR0
{
    float4 output = float4(1,1,1,1);
   
   float2 texOffset;
  float gTexelWidth = (1 / ViewSize.x)*2.0f;
   float gTexelHeight = (1 / ViewSize.y)*2.0f;
    if(isBlurHorizontal)
    {
        texOffset = float2(gTexelWidth, 0.0f);
    }
    else
    {
        texOffset = float2(0.0f, gTexelHeight);
    }

    // The center value always contributes to the sum
   float gBlurRadius = 5;
    float4 color      = gWeights[5]*bufferTexSamp.SampleLevel(SampleBorder, IN.UVCoord, 0.0);
    float totalWeight = gWeights[5];
     
    float centerNormalDepth = DepthTex.SampleLevel(SampleBorder, IN.UVCoord, 0.0f).x;
    for(float i = -gBlurRadius; i <=gBlurRadius; ++i)
    {
        // We already added in the center weight.
        if( i == 0 )
            continue;

        float2 tex = IN.UVCoord + i*texOffset;
        float neighborNormalDepth = DepthTex.SampleLevel(SampleBorder, tex, 0.0f).x;

        // If the center value and neighbor values differ too much (either in 
        // normal or depth), then we assume we are sampling across a discontinuity.
        // We discard such samples from the blur.
        if( abs(neighborNormalDepth - centerNormalDepth) <= 0.2f )
        {
            float weight = gWeights[i+gBlurRadius];
            color += weight*bufferTexSamp.SampleLevel(SampleBorder, tex, 0.0);
            totalWeight += weight;
        }
    }

    // Compensate for discarded samples by making total weights sum to 1.
    return color / totalWeight;
}

float4 combine( VSOUT IN, uniform texture2D oldFrameSamp, uniform texture2D bufferTexSamp ) : COLOR0 
{
   float4 final = float4(oldFrameSamp.Sample(SampleWrap, IN.UVCoord).xyz,1);
   float ao = bufferTexSamp.Sample(SampleWrap, IN.UVCoord).r;
   
   #ifdef SHOW_SSAO
    return float4(ao,ao,ao,1);
   #endif
   #ifdef LUMINANCE_CONSIDERATION
    float luminance = (color.r*0.2125f)+(color.g*0.7154f)+(color.b*0.0721f);
    float white = 1.0f;
    float black = 0.0f;
    luminance = clamp(max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold)+max(black,luminance-luminosity_threshold),0.0,1.0);
    ao = lerp(ao,white,luminance);
   #endif   
   final.rgb *= ao;
   return final;
}

//takes original frame image and outputs to 2x2
float4 PSReduce( output IN, uniform texture2D srcTex ) : COLOR
{
    float4 color = float4(srcTex.Sample(SampleWrap, IN.uv ).xyz,1);
    return color;    
}

//-----------------computes average luminosity for scene-----------------------------
float4 PSGlareAmount( output IN, uniform texture2D srcTex ) : COLOR
{
    float4 GlareAmount = 0;
    
    //sample texture 4 times with offset texture coordinates
    float4 color1= srcTex.Sample(SampleWrap, IN.uv + float2(-0.5, -0.5) );
    float4 color2= srcTex.Sample(SampleWrap, IN.uv + float2(-0.5, 0.5) );
    float4 color3= srcTex.Sample(SampleWrap, IN.uv + float2(0.5, -0.5) );
    float4 color4= srcTex.Sample(SampleWrap, IN.uv + float2(0.5, 0.5) );
    
    //average these samples
    float3 AvgColor = saturate(color1.xyz * 0.25 + color2.xyz * 0.25 + color3.xyz * 0.25 + color4.xyz * 0.25);
    
    //convert to luminance
    AvgColor = dot(float3(0.3,0.59,0.11), AvgColor);
    GlareAmount.xyz = pow(abs(AvgColor),2);
    
    //interpolation value to blend with previous frames
    GlareAmount.w = deltatime * 2;
       
    return GlareAmount;    
}

#ifdef LENSFLARE
// PE: generated in agk. static const 
int flarecolr[64] = {255,254,250,241,233,223,215,202,182,160,
135,114,98,138,157,121,79,56,51,55,
49,42,38,34,30,26,23,19,16,13,
12,12,22,43,78,135,208,251,254,255,
255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,
241,154,119,86};
int flarecolg[64] = {255,253,247,236,222,202,186,164,150,141,
140,140,139,154,123,86,52,35,30,32,
27,21,16,10,6,2,2,1,1,1,
1,1,0,0,4,22,51,62,72,87,
105,125,146,166,185,201,215,225,234,235,
205,154,97,49,22,38,95,127,118,96,
70,56,62,67};
int flarecolb[64] = {255,254,249,241,233,224,217,207,197,187,
178,169,161,165,158,148,137,126,106,85,
73,72,77,84,88,87,79,69,60,51,
45,39,36,32,25,17,15,7,2,0,
0,0,0,0,0,0,0,0,0,6,
28,53,79,99,108,81,25,0,21,67,
121,162,178,193};
        
float4 lensflarefirst( output IN, uniform texture2D lfsrcTex ) : COLOR
{
   float3 lfBias = float3 TRIGGERLENSFLARE;
   float3 lfScale = float3( 10.0,10.0,10.0);
   float3 color = lfsrcTex.Sample(SampleWrap,IN.uv).xyz - lfBias;
   return float4( max( float3(0.0,0.0,0.0) , color ) * lfScale * SAOSettings.w, 1.0 );
}

float uDispersal = 0.35; //0.35;
float uHaloWidth = 0.50; //0.55;
float uDistortion = 4.5; //4.5

float4 textureDistorted(texture2D tex,float2 texcoord,float2 direction,float3 distortion ) 
{
   return float4(
   tex.Sample(SampleWrap, texcoord + direction * distortion.r).r,
   tex.Sample(SampleWrap, texcoord + direction * distortion.g).g,
   tex.Sample(SampleWrap, texcoord + direction * distortion.b).b,1.0);
}

float4 lensflare( output IN, uniform texture2D srcTex ) : COLOR
{
   float2 texelSize = 1.0 / ViewSize;
   float2 texcoord = -IN.uv + float2(1.0,1.0); // flip texcoords
   float2 ghostVec = (float2(0.5,0.5) - texcoord) * uDispersal;
   float2 haloVec = (normalize(ghostVec) * uHaloWidth) ;
   float3 distortion = float3(-texelSize.x * uDistortion, 0.0 , texelSize.x * uDistortion);
   
   // sample ghosts
   float2 offset;
   float weight;
   float4 color = float4(0.0, 0.0, 0.0, 0.0);
              
   for (int i = 0; i < 3; ++i) 
   {
      offset = frac(texcoord + ghostVec * float(i));
      weight = length(float2(0.5,0.5) - offset) / length(float2(0.5,0.5));
      weight = pow( abs(1.0 - weight), 10.0);
      color += srcTex.Sample(SampleWrap, offset)* weight;
   }

   offset = frac(texcoord + ghostVec * 4.0);
   weight = length(float2(0.5,0.5) - offset) / length(float2(0.5,0.5));
   weight = pow( abs(1.0 - weight) , 10.0);
   color += srcTex.Sample(SampleWrap, offset)* weight;

   float distance2Center = length(float2(0.5,0.5) - texcoord) / length(float2(0.5,0.5));
   float2 uvLensColor = frac( float2(distance2Center, 1) );

   int idx = abs( clamp( uvLensColor.x*64 , 0 , 63 )); 
   float4 lcol = float4( (flarecolr[idx])/256.0 , (flarecolg[idx])/256.0  , (flarecolb[idx])/256.0 , 1.0);
   color *= lcol;

#ifndef DISABLELENSFLAREHALO
   //sample halo:
   weight = length(float2(0.5,0.5) - frac(texcoord + haloVec)) / length(float2(0.5,0.5));
   weight = pow( abs(1.0 - weight), 5.0);
   color += (textureDistorted(srcTex, frac(texcoord + haloVec),normalize(ghostVec),distortion) * weight)*0.5;
#endif
   return float4( color.rgb ,1.0);  
}
#endif
                
float4 PSBrightpass( output IN, uniform texture2D srcTex, uniform texture2D srcTex2  ) : COLOR
{
    //remove low luminance pixels, keeping only brightest
    float4 screen = srcTex.Sample(SampleWrap, IN.uv);  //original screen texture;
    float4 glaretex = srcTex2.Sample(SampleWrap, IN.uv);  //glareamount from 1x1 in previous pass
    float3 Brightest = saturate(screen.xyz - BloomThreshold);

#ifdef LENSFLARE

#ifdef HQLENSFLARE
    Brightest.xyz = pow( abs(Brightest.xyz) ,2) * (1+glaretex.xyz) * PreBloomBoost;
#else
    Brightest.xyz = pow( abs(Brightest.xyz) ,2) * (1+glaretex.xyz) * clamp(PreBloomBoost-1.15,0.0,4.0);
#endif
#else
    Brightest.xyz = pow( abs(Brightest.xyz) ,2) * (1+glaretex.xyz) * PreBloomBoost;
#endif
    float4 color = float4(Brightest.xyz, 1);
    return color;    
}

float4 PSBlur( output IN, uniform texture2D srcTex ) : COLOR
{           
    float4 color = float4(0,0,0,0);
    
    float2 PixelOffsets [ 6 ];
    float BlurWeights [ 6 ];

    //linear sampling. using middle coordinate between the two texel centers
    // reduced to 5 samples for same quality as normal.
    PixelOffsets[ 0 ] = float2( 0.0, 0 );
    PixelOffsets[ 1 ] = float2( 1.3846153846, 1.3846153846 );
    PixelOffsets[ 2 ] = float2( 3.2307692308, 3.2307692308 );
    PixelOffsets[ 3 ] = float2( -1.3846153846, -1.3846153846 );
    PixelOffsets[ 4 ] = float2( -3.2307692308, -3.2307692308 );
    BlurWeights [ 0 ] = 0.2270270270;
    BlurWeights [ 1 ] = 0.3162162162;
    BlurWeights [ 2 ] = 0.0702702703;
    BlurWeights [ 3 ] = 0.3162162162;
    BlurWeights [ 4 ] = 0.0702702703;

    float2 scale = float2( 1/ViewSize.x,1/ViewSize.y);
     for (int i = 0; i < 5; i++) {   
      //convert pixel offsets into texel offsets via the inverse view values. 
      color +=  srcTex.Sample(SampleWrap, IN.uv + PixelOffsets[i].xy*scale ) * BlurWeights[i];
    }   
    return color;
}

//PE TODO: Better to do it Vertical and then Hortizental in 2 runs.
float4 BlurV( output IN, uniform texture2D srcTex ) : COLOR
{
    float4 color = float4(0,0,0,0);
    float2 PixelOffsets [ 6 ];
    float BlurWeights [ 6 ];
    //linear sampling. using middle coordinate between the two texel centers
    PixelOffsets[ 0 ] = float2( 0.0, 0 );
    PixelOffsets[ 1 ] = float2( 1.3846153846, 0 );
    PixelOffsets[ 2 ] = float2( 3.2307692308, 0 );
    PixelOffsets[ 3 ] = float2( -1.3846153846, 0 );
    PixelOffsets[ 4 ] = float2( -3.2307692308, 0 );
    BlurWeights [ 0 ] = 0.2270270270;
    BlurWeights [ 1 ] = 0.3162162162;
    BlurWeights [ 2 ] = 0.0702702703;
    BlurWeights [ 3 ] = 0.3162162162;
    BlurWeights [ 4 ] = 0.0702702703;
    float2 scale = float2( 1/ViewSize.x,1/ViewSize.y);
    for (int i = 0; i < 5; i++) {   
      //convert pixel offsets into texel offsets via the inverse view values. 
      color += srcTex.Sample(SampleWrap, IN.uv + PixelOffsets[i].xy*scale ) * BlurWeights[i];
    }   
    return color;
}
float4 BlurH( output IN, uniform texture2D srcTex ) : COLOR
{
    float4 color = float4(0,0,0,0);
    float2 PixelOffsets [ 6 ];
    float BlurWeights [ 6 ];
    //linear sampling. using middle coordinate between the two texel centers
    PixelOffsets[ 0 ] = float2( 0.0, 0 );
    PixelOffsets[ 1 ] = float2( 1.3846153846, 0 );
    PixelOffsets[ 2 ] = float2( 3.2307692308, 0 );
    PixelOffsets[ 3 ] = float2( -1.3846153846, 0 );
    PixelOffsets[ 4 ] = float2( -3.2307692308, 0 );
    BlurWeights [ 0 ] = 0.2270270270;
    BlurWeights [ 1 ] = 0.3162162162;
    BlurWeights [ 2 ] = 0.0702702703;
    BlurWeights [ 3 ] = 0.3162162162;
    BlurWeights [ 4 ] = 0.0702702703;
    float2 scale = float2( 1/ViewSize.x,1/ViewSize.y);
    for (int i = 0; i < 5; i++) {   
      //convert pixel offsets into texel offsets via the inverse view values. 
      color += srcTex.Sample(SampleWrap, IN.uv + PixelOffsets[i].yx*scale ) * BlurWeights[i];
    }   
    return color;
}

float4 PSDepthOfField( output IN, uniform texture2D srcTex ) : COLOR
{             
   float3 color = srcTex.Sample(SampleWrap, IN.uv).xyz;
   if( DepthOfField.y >= 0.05 ) 
   {
      float2 ViewInv = float2( 1/ViewSize.x,1/ViewSize.y);
      float fZDepth = (DepthTex.Sample(SampleWrap, IN.uv).r-0.8f)*5;
      float fCircleOfConfusion = min(max(0,(fZDepth-DepthOfField.x)*DepthOfField.y*10.0),10.0);
      float fContributions = 1;
      for(int p=0; p<64; p+=4)
      {
         float2 offsetuv = PoissonSamples[p] * ViewInv * fCircleOfConfusion;
         float3 currentColor = srcTex.Sample(SampleWrap, IN.uv+offsetuv).xyz;  
         float fZNeighborDepth = (DepthTex.Sample(SampleWrap, IN.uv+offsetuv).r-0.8f)*5;
         float weight = 1.0 - saturate(max(fZDepth - fZNeighborDepth,0)*50000);
         color += (currentColor*weight);
         fContributions += weight;
      }
      color = color / fContributions;
   }
   return float4(color,1);
}


//@https://www.opengl.org/discussion_boards/showthread.php/184192-GLSL-FXAA-rendering-off-screen
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

float4 fxaa(uniform texture2D sampler0,float2 fcoords, float4 curcolor )
{
   float2 resolution = ViewSize; //float2(1280.0,768.0); // ViewSize
   float3 rgbNW = sampler0.Sample(SampleWrap, (fcoords.xy + (float2(-1.0,-1.0)/resolution)) ).xyz;
   float3 rgbNE = sampler0.Sample(SampleWrap, (fcoords.xy + (float2(1.0,-1.0)/resolution)) ).xyz;
   float3 rgbSW = sampler0.Sample(SampleWrap, (fcoords.xy + (float2(-1.0,1.0)/resolution)) ).xyz;
   float3 rgbSE = sampler0.Sample(SampleWrap, (fcoords.xy + (float2(1.0,1.0)/resolution)) ).xyz;
   float3 rgbM  = curcolor.xyz;
 
   float3 luma = float3(0.299, 0.587, 0.114);
   float lumaNW = dot(rgbNW, luma);
   float lumaNE = dot(rgbNE, luma);
   float lumaSW = dot(rgbSW, luma);
   float lumaSE = dot(rgbSE, luma);
   float lumaM  = dot(rgbM,  luma);
   float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
   float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 
   float2 dir;
   dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
   dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
   float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),FXAA_REDUCE_MIN);
   float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
   dir = min(float2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),dir * rcpDirMin)) / resolution;
   float3 rgbA = 0.5 * (sampler0.Sample(SampleWrap,   fcoords.xy  + dir * (1.0/3.0 - 0.5)).xyz + sampler0.Sample(SampleWrap,   fcoords.xy  + dir * (2.0/3.0 - 0.5)).xyz);
   float3 rgbB = rgbA * 0.5 + 0.25 * (sampler0.Sample(SampleWrap,  fcoords.xy  + dir *  - 0.5).xyz + sampler0.Sample(SampleWrap,  fcoords.xy + dir * 0.5).xyz);
   float lumaB = dot(rgbB, luma);
   float3 ori;
   if((lumaB < lumaMin) || (lumaB > lumaMax)) {
      ori = rgbA;
   } else {
      ori=rgbB;
   }

#ifdef LUMASHARPEN
   //float3 sharp_strength_luma = ( float3(0.2126, 0.7152, 0.0722) * LUMASHARPEN) *= 1.5; // sRBG luma coefficient ( monitor/HD tv).(0.299, 0.587, 0.114) = non HD to test.
   float3 blur_ori;
   //PE: reuse fxaa samples.
   blur_ori  = rgbNW; // North West
   blur_ori += rgbSE; // South East
   blur_ori += rgbNE; // North East
   blur_ori += rgbSW; // South West
   //PE: Add more samples to produce better result.
   blur_ori *= 0.25;  //Divide by the number of texture fetches
   //sharp_strength_luma *= 1.5;
   float3 sharp = ori - blur_ori;  //subtract blurred image from the original.
   //float sharp_luma = dot(sharp,  ( float3(0.2126, 0.7152, 0.0722) * LUMASHARPEN) * 1.5  );
   float sharpen = clamp( dot(sharp,  ( float3(0.2126, 0.7152, 0.0722) * LUMASHARPEN) * 1.5  )  , -0.035, 0.035); // 0.035 = to prevent artifacts.
#ifdef SPLITTESTFXAACVLS
   if( frac(fcoords.x) > 0.5 ) {
      ori.rgb = saturate(ori + sharpen);
   }
#else
      ori.rgb = saturate(ori + sharpen);
#endif
#endif

   return float4(ori,1.0);
}

float4 presentfxaa(output IN, uniform texture2D srcTex ) : COLOR
{
    float2 texCoord = IN.uv;

#ifdef PIXELLATEUV
    int ir; 
    ir = round(texCoord.x * PIXELLATEUVx );
    texCoord.x = float(ir / PIXELLATEUVx );
    ir = round(texCoord.y * PIXELLATEUVy);
    texCoord.y = float(ir / PIXELLATEUVy);
#endif

#ifdef UNDERWATERWAVE
   //PE: Underwater wave look.
   if( UnderWaterSettings.x > 0 && texCoord.x >= 0.011 && texCoord.x <= 0.989 && texCoord.y >= 0.011 && texCoord.y <= 0.989 ) {
      float2 waveuv = texCoord * UnderWaterSettings.w; // scale normalmap up.
      waveuv.y += (time/UnderWaterSettings.y); // UnderWaterSettings.y
      float4 normalmap = underWaterWaveNormal.Sample(SampleWrap, waveuv );
      float3 dudv = ( normalmap.rgb * 2.0 - 1.0) * UnderWaterSettings.z;
      texCoord =  clamp( texCoord + dudv.rg ,0.0,1.0 );
   }
#endif

    float4 ScreenMap = srcTex.Sample(SampleWrap, texCoord);

#ifdef PIXELLATEUV
   //PE: Instead of fxaa, blur image to stop flicker.

   float4 fxaatex = ScreenMap;
   float2 PixelOffsets [ 6 ];
   //reduced to 5 samples for same quality as normal.
   PixelOffsets[ 0 ] = float2( 1.3846153846, 0.0 );
   PixelOffsets[ 1 ] = float2( 1.3846153846, 1.3846153846 );
   PixelOffsets[ 2 ] = float2( 3.2307692308, 3.2307692308 );
   PixelOffsets[ 3 ] = float2( -1.3846153846, -1.3846153846 );
   PixelOffsets[ 4 ] = float2( -3.2307692308, -3.2307692308 );

   float2 scale = float2( 1/ViewSize.x,1/ViewSize.y);
   for (int i = 0; i < 5; i++) {
      fxaatex +=  srcTex.Sample(SampleWrap, texCoord + PixelOffsets[i].xy*scale );
   }
   fxaatex /= 6.0;
   
#else
    float4 fxaatex = fxaa( srcTex , texCoord , ScreenMap );
#endif


#ifdef COLORVIBRANCE

#ifdef SPLITTESTFXAACVLS
   if( frac(texCoord.x) > 0.5 ) {
#endif
   float luma = dot(float3(0.2126, 0.7152, 0.0722), fxaatex.rgb);
   float max_color = max(fxaatex.r, max(fxaatex.g, fxaatex.b)); // Find the strongest color
   float min_color = min(fxaatex.r, min(fxaatex.g, fxaatex.b)); // Find the weakest color
   float color_saturation = max_color - min_color; // The difference between the two is the saturation
   float3 coeffVibrance = float3(COLORVIBRANCE,COLORVIBRANCE,COLORVIBRANCE);
   fxaatex.rgb = lerp(luma, fxaatex.rgb, 1.0 + (coeffVibrance * (1.0 - (sign(coeffVibrance) * color_saturation))));
#ifdef SPLITTESTFXAACVLS
   }
   if( frac(texCoord.x) > 0.499 && frac(texCoord.x) < 0.501) {
      fxaatex.r = 0.0;
      fxaatex.g = 0.0;
      fxaatex.b = 0.0;
   }
#endif
   
#endif


#ifdef PIXELLATEUV
     // no dither on this effect.
     ir = round(fxaatex.r * PIXELLATECOLOR );
     fxaatex.r = float(ir / PIXELLATECOLOR );
     ir = round(fxaatex.g * PIXELLATECOLOR );
     fxaatex.g = float(ir / PIXELLATECOLOR );
     ir = round(fxaatex.b * PIXELLATECOLOR );
     fxaatex.b = float(ir / PIXELLATECOLOR );
#endif


    return fxaatex;
}

float threshold( float thr1,  float thr2 ,  float val) {
 if (val < thr1) {return 0.0;}
 if (val > thr2) {return 1.0;}
 return val;
}

float avg_intensity( float4 pix) {
 return (pix.r + pix.g + pix.b)/3.0;
}

float4 get_pixel( float2 coords,  float dx,  float dy,uniform texture2D srcTex) {
 return srcTex.Sample(SampleWrap,coords + float2(dx, dy));
}

float IsEdge( float2 coords,uniform texture2D srcTex){
  float dxtex = 1.0 / ViewSize.x; //512.0 /*image width*/;
  float dytex = 1.0 / ViewSize.y; //512.0 /*image height*/;
  float pix[9];
  int k = -1;
  float delta;

  pix[0] = avg_intensity(get_pixel(coords,float(-1)*dxtex,float(-1)*dytex,srcTex));
  pix[1] = avg_intensity(get_pixel(coords,float(-1)*dxtex,float(0)*dytex,srcTex));
  pix[2] = avg_intensity(get_pixel(coords,float(-1)*dxtex,float(1)*dytex,srcTex));

  pix[3] = avg_intensity(get_pixel(coords,float(0)*dxtex,float(-1)*dytex,srcTex));
  pix[4] = avg_intensity(get_pixel(coords,float(0)*dxtex,float(0)*dytex,srcTex));
  pix[5] = avg_intensity(get_pixel(coords,float(0)*dxtex,float(1)*dytex,srcTex));

  pix[6] = avg_intensity(get_pixel(coords,float(1)*dxtex,float(-1)*dytex,srcTex));
  pix[7] = avg_intensity(get_pixel(coords,float(1)*dxtex,float(0)*dytex,srcTex));
  pix[8] = avg_intensity(get_pixel(coords,float(1)*dxtex,float(1)*dytex,srcTex));

  // average color differences around neighboring pixels
  delta = (abs(pix[1]-pix[7])+abs(pix[5]-pix[3]) +abs(pix[0]-pix[8])+abs(pix[2]-pix[6]) )/4.0;
  return clamp( CARTOONIMPACT *delta,0.0,1.0);
}

float4 cartoon(output IN, float4 color,uniform texture2D srcTex)
{
   color = lerp( color , float4(0.0,0.0,0.0,1.0) , IsEdge(IN.uv,srcTex) );
   return color;
}

float retroluma(float3 color) {
  return dot(color, float3(0.299, 0.587, 0.114) );
}
float retrodither4a(float2 uvpos, float brightness) {
  int x = int(fmod( uvpos.x, 4.0));
  int y = int(fmod( uvpos.y, 4.0));
  int index = x + y * 4;
  float limit = 0.0;
  if (x < 8) {
    if (index == 0) limit = 0.0625;
    if (index == 1) limit = 0.5625;
    if (index == 2) limit = 0.1875;
    if (index == 3) limit = 0.6875;
    if (index == 4) limit = 0.8125;
    if (index == 5) limit = 0.3125;
    if (index == 6) limit = 0.9375;
    if (index == 7) limit = 0.4375;
    if (index == 8) limit = 0.25;
    if (index == 9) limit = 0.75;
    if (index == 10) limit = 0.125;
    if (index == 11) limit = 0.625;
    if (index == 12) limit = 1.0;
    if (index == 13) limit = 0.5;
    if (index == 14) limit = 0.875;
    if (index == 15) limit = 0.375;
  }
  return brightness < limit ? 0.0 : 1.0;
}
float3 retrodither4(float2 uvpos, float3 color) {
  return color.rgb * retrodither4a(uvpos, retroluma(color.rgb));
}

// @ https://www.c64-wiki.com/index.php/Color
float3 bestcolorc64( float3 oldcolor )
{
   float3 c64col[27];
   // map colors.  
   // pure 128 255 colors seams to work better.
   c64col[0] = float3(0.0,0.0,0.0);
   c64col[1] = float3(0.0,0.0,128.0);
   c64col[2] = float3(0.0,0.0,255.0);
   c64col[3] = float3(0.0,128.0,0.0);
   c64col[4] = float3(0.0,128.0,128.0);
   c64col[5] = float3(0.0,128.0,255.0);
   c64col[6] = float3(0.0,255.0,0.0);
   c64col[7] = float3(0.0,255.0,128.0);
   c64col[8] = float3(0.0,255.0,255.0);
   c64col[9] = float3(128.0,0.0,0.0);
   c64col[10] = float3(128.0,0.0,128.0);
   c64col[11] = float3(128.0,0.0,255.0);
   c64col[12] = float3(128.0,128.0,0.0);
   c64col[13] = float3(128.0,128.0,128.0);
   c64col[14] = float3(128.0,128.0,255.0);
   c64col[15] = float3(128.0,255.0,0.0);
   c64col[16] = float3(128.0,255.0,128.0);
   c64col[17] = float3(128.0,255.0,255.0);
   c64col[18] = float3(255.0,0.0,0.0);
   c64col[19] = float3(255.0,0.0,128.0);
   c64col[20] = float3(255.0,0.0,255.0);
   c64col[21] = float3(255.0,128.0,0.0);
   c64col[22] = float3(255.0,128.0,128.0);
   c64col[23] = float3(255.0,128.0,255.0);
   c64col[24] = float3(255.0,255.0,0.0);
   c64col[25] = float3(255.0,255.0,128.0);
   c64col[26] = float3(255.0,255.0,255.0);
   float3 match = float3(0.0,0.0,0.0);
   float best_dot = 999999.0;
   // PE: Stupid way slow but works.
   // perhaps i generic postp color conversion texture ? would be faster.
   for (int c=26;c>=0;c--) {
      float this_dot = distance((c64col[c]/255.0),oldcolor);
      if (this_dot<best_dot) {
         best_dot=this_dot;
         match=c64col[c];
      }
   }
   return match/255.0;
}  

#ifdef USE_SATURATION_AND_SEPIA 
float3 saturationMatrix(float VibranceI, float3 colorI)
{
   float3 color = colorI;
   float  mean = (colorI.r + colorI.g + colorI.b) * 0.33333333;
   float3 dev = colorI - mean;
   color.rgb = mean + dev * VibranceI;

   float3 sepia;
   sepia.r = dot(color.rgb, float3(0.393, 0.769, 0.189));
   sepia.g = dot(color.rgb, float3(0.349, 0.686, 0.168));
   sepia.b = dot(color.rgb, float3(0.272, 0.534, 0.131));
   color.rgb = lerp(color.rgb, sepia.rgb, DepthOfField.z);

   return color;
}
#endif

#ifdef LENSFLARE
float4 PSPresent( output IN, uniform texture2D srcTex, uniform texture2D srcTex2, uniform texture2D srcTex3, uniform texture2D srcTex4 ) : COLOR
#else
float4 PSPresent( output IN, uniform texture2D srcTex, uniform texture2D srcTex2, uniform texture2D srcTex3 ) : COLOR
#endif  
{
    
   #ifdef AMD_CAS //Contrast Adaptive Sharpening

      float pixelX = PixelSize.x;
      float pixelY = PixelSize.y;
      float3 a = srcTex.Sample(SampleWrap, IN.uv + float2(-pixelX, -pixelY)).rgb;
      float3 b = srcTex.Sample(SampleWrap, IN.uv + float2(0.0, -pixelY)).rgb;
      float3 c = srcTex.Sample(SampleWrap, IN.uv + float2(pixelX, -pixelY)).rgb;
      float3 d = srcTex.Sample(SampleWrap, IN.uv + float2(-pixelX, 0.0)).rgb;
      float4 allpixel = srcTex.Sample(SampleWrap, IN.uv).rgba;
      float3 e = allpixel.rgb;
      float allpixel_alpha = allpixel.a;
      float3 f = srcTex.Sample(SampleWrap, IN.uv + float2(pixelX, 0.0)).rgb;
      float3 g = srcTex.Sample(SampleWrap, IN.uv + float2(-pixelX, pixelY)).rgb;
      float3 h = srcTex.Sample(SampleWrap, IN.uv + float2(0.0, pixelY)).rgb;
      float3 i = srcTex.Sample(SampleWrap, IN.uv + float2(pixelX, pixelY)).rgb;
      float3 mnRGB = min3rgb(min3rgb(d, e, f), b, h);
      float3 mnRGB2 = min3rgb(min3rgb(mnRGB, a, c), g, i);
      mnRGB += mnRGB2;
      float3 mxRGB = max3rgb(max3rgb(d, e, f), b, h);
      float3 mxRGB2 = max3rgb(max3rgb(mxRGB, a, c), g, i);
      mxRGB += mxRGB2;
      float3 rcpMRGB = rcp(mxRGB);
      float3 ampRGB = saturate(min(mnRGB, 2.0 - mxRGB) * rcpMRGB);
      ampRGB = sqrt(ampRGB);
      float peak = -rcp(lerp(8.0, 5.0, saturate(Sharpness)));
      float3 wRGB = ampRGB * peak;
      float3 rcpWeightRGB = rcp(1.0 + 4.0 * wRGB);
      float3 window = (b + d) + (f + h);
      float3 outColor = saturate((window * wRGB + e) * rcpWeightRGB);
   #endif
   
   // sample screen texture with supersampled UV's
    float4 BloomMap=srcTex2.Sample(SampleWrap, IN.uv );
    #ifdef LENSFLARE
     float4 LensMap = srcTex4.Sample(SampleWrap, IN.uv ) * LENSFLAREINTENSITY;
    #endif
    float4 AmtMap=srcTex3.Sample(SampleWrap, IN.uv );
   
    // as scene gets brighter, reduce brightness effects of post process
    float ToneLuminance = dot(AmtMap.xyz, float3(0.3, 0.59, 0.11)) * 1.5;
   
    // stores world pos and view Z 0-7000
    float3 vPositionVS = VSPositionFromDepth(IN.uv);
    float4 worldSpacePos = mul(float4(vPositionVS,1), ViewInv);
    worldSpacePos.w = vPositionVS.z;
   
    #ifdef MOTIONBLUR
    // Motion Blur effect calculation
#ifdef AMD_CAS
   float4 ScreenMap = float4(outColor.rgb, allpixel_alpha);
#else
   float4 ScreenMap = srcTex.Sample(SampleWrap, IN.uv);
#endif
    if( Motion.y >= 0.05 ) 
    {
      float4 currentPos = mul(float4(worldSpacePos.xyz,1),ViewProjectionMatrix);
      float4 previousPos = mul(float4(worldSpacePos.xyz,1),PreviousViewProjectionMatrix);
      currentPos /= currentPos.w;
      previousPos /= previousPos.w;
      
      // this corrects the issue of reversed spiral syndrome
      float2 velocity = (currentPos.xy - previousPos.xy);
      velocity.y = -velocity.y;
     
     // lessen velocity if pixel close (controlled by distance slider)
     velocity.xy *= lerp(1.0f,1.0f-min(1,1.0f/(worldSpacePos.w/500.0f)),Motion.x);
     
     // no motion blur for HUD guns
     if ( worldSpacePos.w < 275.0f ) { velocity.xy = float2(0,0); }
     worldSpacePos.w = max(0,worldSpacePos.w-275.0f)/500.0f;
     
      // blur velocity modulated by real distance
      float fAllowMB = min(worldSpacePos.w,1);
      velocity = velocity * fAllowMB * Motion.y;
      
      // Motion Blur - collect (10 samples)
      float2 texCoord = IN.uv;
      float4 color = ScreenMap;  
      velocity = velocity / 10; 
      velocity = velocity / 5; // solve fore/back judder
      texCoord += velocity;
      float fContributions = 1;
     [unroll]
      for(int i = 1; i < 10; ++i, texCoord -= velocity)
      {  
        // Sample the color buffer along the velocity vector
      float3 vPositionVS = VSPositionFromDepth(texCoord);
        float fAllowMB = max(vPositionVS.z-275.0f,1);
      float4 currentColor = srcTex.Sample(SampleWrap, texCoord);
      color += (currentColor*fAllowMB);
      fContributions += fAllowMB;
      }  
      
      // Average all of the samples to get the final blur color
      ScreenMap = color / fContributions;
    }
    #else
     float2 texCoord = IN.uv;
#ifdef AMD_CAS
    float4 ScreenMap = float4(outColor.rgb, allpixel_alpha);
#else
    float4 ScreenMap = srcTex.Sample(SampleWrap, texCoord);
#endif
    #endif

    // add results based scene pixel brightness
    #ifdef LENSFLARE
     float4 MaxAmount = ScreenMap + BloomMap + LensMap;
    #else
     float4 MaxAmount = ScreenMap + BloomMap;
    #endif

#ifdef CHROMATICABBERATION

#ifdef CHROMATICABBERATIONLUA
   //ChromaticAbberation.rgb split values , .w enable.
   if ( ChromaticAbberation.w > 0 )
   {
   float3 rgbsplit;
   rgbsplit = ChromaticAbberation.rgb;
#else
   float3 rgbsplit = float3 CHROMATICABBERATION;
#endif
   float2 stexelSize = 1.0 / ViewSize;

    float2 CAxy = ((IN.uv*2)-1) * 1.0;
    float CAborderrem = 1.0 - clamp((1.0-1.0) + (1.0)*pow( abs((CAxy.x+1.0)*(CAxy.y+1.0)*(CAxy.x-1.0)*(CAxy.y-1.0)), 0.25 ) * 0.5 ,0.0,1.0);

    float CAintensity = 1.0- clamp( (worldSpacePos.w/450.0) ,0.25,0.75);

    float3 rightRed = srcTex.Sample(SampleWrap, IN.uv - (stexelSize * rgbsplit.r * CAintensity) ).rgb;
    float3 leftGreen = srcTex.Sample(SampleWrap, IN.uv + (stexelSize * rgbsplit.g * CAintensity) ).rgb;
   //PE: Dont split blue just keep it at center to make it faster. wip.
    //float3 diagBlue = srcTex.Sample(SampleWrap, IN.uv + (stexelSize * rgbsplit.b * CAintensity) ).rgb;

    ScreenMap.r = lerp( rightRed.r , ScreenMap.r , CAborderrem);
    ScreenMap.g = lerp( leftGreen.g , ScreenMap.g , CAborderrem);
    //ScreenMap.b = lerp( diagBlue.b , ScreenMap.g , CAborderrem);

    #ifdef LENSFLARE
     MaxAmount = ScreenMap + BloomMap + LensMap;
    #else
     MaxAmount = ScreenMap + BloomMap;
    #endif
#ifdef CHROMATICABBERATIONLUA
   }
#endif
#endif

    float3 final = lerp(MaxAmount.xyz, ScreenMap.xyz, ToneLuminance);

#ifdef USE_LUT

   if (Motion.w > 0)
   {
      float2 Lut_Size = float2(LUT_COLOURWIDTH, LUT_COLOURHEIGHT);
      float2 Lut_pSize = 1.0 / Lut_Size;
      float4 Lut_UV;
      float3 colorIN = saturate(final.xyz) * (Lut_Size.y - 1.0);

      Lut_UV.w = floor(colorIN.b);
      Lut_UV.xy = (colorIN.rg + 0.5) * Lut_pSize;
      Lut_UV.x += Lut_UV.w * Lut_pSize.y;
      Lut_UV.z = Lut_UV.x + Lut_pSize.y;
      final.xyz = lerp(LUT.SampleLevel(SampleLUT, Lut_UV.xy, 0).rgb, LUT.SampleLevel(SampleLUT, Lut_UV.zy, 0).rgb, colorIN.b - Lut_UV.w);
   }

#endif

    // add any screen color effect
    #ifndef NOCOLOREFFECT
     final.xyz += ScreenColor.xyz;
    #endif

    // overall modulation control
    final.xyz *= OverallColor.xyz;
   
    // Apply contrast
    final.rgb = ((final.rgb - 0.5f) * max(PostContrast, 0)) + 0.5f;

    // Apply brightness
    final.rgb += PostBrightness;
   
    // Vignetting   
    float2 xy = ((IN.uv*2)-1) * Vignette.x;
    final *= (1.0-Vignette.y) + (Vignette.y)*pow( abs((xy.x+1.0)*(xy.y+1.0)*(xy.x-1.0)*(xy.y-1.0)), 0.25 );
   
    // final pixel color
    #ifdef CARTOON
     final = cartoon(IN,float4(final,1.0),srcTex).rgb;
    #endif

    // find screenpos using ViewSize.
    #ifdef RETRODITHER
     final = retrodither4( (IN.uv.xy*ViewSize), final);
    #endif

    #ifdef RETRODITHER64
     #ifndef RETRODITHER
      final = retrodither4( (IN.uv.xy*ViewSize), final);
     #endif
     // convert to nearest c64 color.
     final = bestcolorc64(final);
    #endif

   #ifdef USE_SATURATION_AND_SEPIA
      float saturationlevel; 
      saturationlevel = Motion.z * 3;
      final.xyz = saturationMatrix(saturationlevel, final.xyz);
   #endif

    return float4(final,1);
}

// technique passes

technique11 Main
<
   //specify where we want the original image to be put
   string RenderColorTarget = "frame";
>
{
#ifndef NOSAO
   pass SAOp1
   <
      string RenderColorTarget = "buffer1Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, FrameVS()));
#ifdef HBAO
     SetPixelShader(CompileShader(ps_5_0, SSAOCalculate3()));
#else
      SetPixelShader(CompileShader(ps_5_0, SSAOCalculate()));
#endif
      SetGeometryShader(NULL);
   }
#ifdef BLUR_SSAO
   pass SAOp2
   <
      string RenderColorTarget = "buffer2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, FrameVS()));
      SetPixelShader(CompileShader(ps_5_0, BlurBL(buffer1Img,true)));
      SetGeometryShader(NULL);
   }
   pass SAOp3
   <
      string RenderColorTarget = "buffer1Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, FrameVS()));
      SetPixelShader(CompileShader(ps_5_0, BlurBL(buffer2Img,false)));
      SetGeometryShader(NULL);
   }
#endif
   pass SAOp4
   <
      string RenderColorTarget = "combineFrameImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, FrameVS()));
      SetPixelShader(CompileShader(ps_5_0, combine(frame,buffer1Img)));
      SetGeometryShader(NULL);
   }
#endif

   //1. first reduce to 2x2 and save in AvgLum2x2Img
   pass Reduce2x2
   <
      string RenderColorTarget = "AvgLum2x2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      #ifndef NOSAO
       SetPixelShader(CompileShader(ps_5_0, PSReduce( combineFrameImg )));
      #else
       SetPixelShader(CompileShader(ps_5_0, PSReduce( frame )));
      #endif
      SetGeometryShader(NULL);
   }
   
   //2. reduce to 1x1 and save in AvgLumFinalImg, using alpha blending to blend with previous frames
   pass Reduce1x1
   <
      string RenderColorTarget = "AvgLumFinal";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, PSGlareAmount( AvgLum2x2Img )));
      SetGeometryShader(NULL);
   }
   
   //3. remove low luminance pixels keeping only brightest for blurring in next pass
   pass Brightpass
   <
      string RenderColorTarget = "BrightpassImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      #ifndef NOSAO
       SetPixelShader(CompileShader(ps_5_0, PSBrightpass( combineFrameImg, AvgLumFinal )));
      #else
       SetPixelShader(CompileShader(ps_5_0, PSBrightpass( frame, AvgLumFinal )));
      #endif
      SetGeometryShader(NULL);
   }
   
   //4. blur texture and save in Blur1Img
   pass Blur1
   <
      string RenderColorTarget = "Blur1Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, BlurV( BrightpassImg )));
      SetGeometryShader(NULL);
   }
   
   //5. repeat blur texture and save in Blur2Img
   pass Blur2
   <
      string RenderColorTarget = "Blur2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, BlurH( Blur1Img )));
      SetGeometryShader(NULL);
   }
   
   //6. repeat blur again
   pass Blur3
   <
      string RenderColorTarget = "Blur1Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, PSBlur( Blur2Img )));
      SetGeometryShader(NULL);
   }

#ifdef LENSFLARE

#ifdef HQLENSFLARE
   // get only brighest values for lens flare.
   pass lensflarefirst
   <
      string RenderColorTarget = "BrightpassImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      #ifndef NOSAO
       SetPixelShader(CompileShader(ps_5_0, lensflarefirst( combineFrameImg )));
      #else
       SetPixelShader(CompileShader(ps_5_0, lensflarefirst( frame )));
      #endif
      SetGeometryShader(NULL);
   }

   pass lensflareBlur1H
   <
      string RenderColorTarget = "Blur2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, BlurH( BrightpassImg )));
      SetGeometryShader(NULL);
   }

   pass lensflareBlur1V
   <
      string RenderColorTarget = "BrightpassImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, BlurV( Blur2Img )));
      SetGeometryShader(NULL);
   }

   pass lfBlur3
   <
      string RenderColorTarget = "Blur2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, PSBlur( BrightpassImg )));
      SetGeometryShader(NULL);
   }

   pass lfBlur4
   <
      string RenderColorTarget = "BrightpassImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, PSBlur( Blur2Img )));
      SetGeometryShader(NULL);
   }

   // run the lens flare.
   pass lensflareblur
   <
      string RenderColorTarget = "Blur2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, lensflare( BrightpassImg )));
      SetGeometryShader(NULL);
   }

#else
   pass Lensflarepass
   <
      string RenderColorTarget = "Blur2Img";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, lensflare( Blur1Img )));
      SetGeometryShader(NULL);
   }
#endif

#endif
   
   #ifdef DOF
   pass DepthOfField
   <
      string RenderColorTarget = "DepthOfFieldImg";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      #ifndef NOSAO
       SetPixelShader(CompileShader(ps_5_0, PSDepthOfField( combineFrameImg )));
      #else
       SetPixelShader(CompileShader(ps_5_0, PSDepthOfField( frame )));
      #endif
      SetGeometryShader(NULL);
   }
   #endif
   
   pass Present
   <
      #ifdef FXAA
       string RenderColorTarget = "buffer1Img";
      #else
       string RenderColorTarget = "";
      #endif
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      #ifndef NOSAO
       #ifdef LENSFLARE
        #ifdef DOF
         SetPixelShader(CompileShader(ps_5_0, PSPresent( DepthOfFieldImg, Blur1Img, AvgLumFinal,Blur2Img )));
        #else
         SetPixelShader(CompileShader(ps_5_0, PSPresent( combineFrameImg, Blur1Img, AvgLumFinal,Blur2Img )));
        #endif
       #else
        #ifdef DOF
         SetPixelShader(CompileShader(ps_5_0, PSPresent( DepthOfFieldImg, Blur1Img, AvgLumFinal )));
        #else
         SetPixelShader(CompileShader(ps_5_0, PSPresent( combineFrameImg, Blur1Img, AvgLumFinal )));
        #endif
       #endif
      #else
       #ifdef LENSFLARE
        #ifdef DOF
         SetPixelShader(CompileShader(ps_5_0, PSPresent( DepthOfFieldImg, Blur1Img, AvgLumFinal,Blur2Img )));
        #else
         SetPixelShader(CompileShader(ps_5_0, PSPresent( frame, Blur1Img, AvgLumFinal,Blur2Img )));
        #endif
       #else
        #ifdef DOF
         SetPixelShader(CompileShader(ps_5_0, PSPresent( DepthOfFieldImg, Blur1Img, AvgLumFinal )));
        #else
         SetPixelShader(CompileShader(ps_5_0, PSPresent( frame, Blur1Img, AvgLumFinal )));
        #endif
       #endif
      #endif
      SetGeometryShader(NULL);
   }
   
   #ifdef FXAA
   pass fxaapresent
   <
      string RenderColorTarget = "";
   >
   {
      SetVertexShader(CompileShader(vs_5_0, VS()));
      SetPixelShader(CompileShader(ps_5_0, presentfxaa( buffer1Img )));
      SetGeometryShader(NULL);
   }
   #endif
}
