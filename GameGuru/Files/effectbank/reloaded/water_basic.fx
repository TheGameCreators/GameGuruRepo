string Description = "Water Shader";  
#include "settings.fx"
#include "constantbuffers.fx"
//#define OLDWATER

#ifndef OLDWATER
   // PE: new water settings.
   // At some point we might move this to settings.fx to make it more easy.
   // NOTE: Make a backup of this file before editing settings, just in case :)
   // Only one of the WaterCol lines should be active.
   // You can change the color in format RGB, divide by 256, so Color value 128 = "128.0/256.0"
   
   float2 nWaterScale = { 155.0f , 155.0f}; // PE: wave size higher = smaller waves default 155.0,155.0
   float distortion = 0.0055f; // PE: water distortion reflection default 0.0055f
   float distortion2 = 0.030f; // PE: water distortion waves default 0.030f
   float3 WaterCol = { 158.0/256.0 ,168.0/256.0 , 198.0/256.0 }; // PE: more neutral water
//   float3 WaterCol = { 112.0/256.0 ,169.0/256.0 , 185.0/256.0 }; // PE: Default blue water
//   float3 WaterCol = { 74.0/256.0 ,218.0/256.0 , 156.0/256.0 }; // PE: Green water
//   float3 WaterCol = { 220.0/256.0 ,0.0/256.0 , 0.0/256.0 }; // PE: red water
   float3 WaterSparkleCol = { 290.0/256.0 ,290.0/256.0 , 290.0/256.0 }; // PE: Higher sparkles skine more.
   float WaterTransparancy = 0.75f; // PE: Water transparancy lower more transparant, default 0.75
   float WaterReflection = 0.50f; // PE: Reflection higher = more reflection , default 0.5
   float WaterSpeed1 = 35.0f; // PE: Speed 1 lower = faster default 30.0f
   float WaterSpeed2 = 70.0f; // PE: Speed 2 lower = faster default 70.0f  
   float genericwaveIntensity = 0.75; // PE: Generic wave , default 0.75 , values 0.0-1.0       
   float2 flowdirection = {1,1}; // PE: Direction water flow. you can also use minus values.
   #define USEREFLECTIONSPARKLE // PE: Use reflection colors in sparkle.
   float reflectionSparkleIntensity = 1.90; // PE: Sparkle Intensity , higher = more bright
   // ###
   // Dont touch anything below this line.
   // ###
#endif

  //-----------------
  // un-tweaks
  //-----------------

   matrix WorldVP:WorldViewProjection; 
   float time : Time ;
   float4x4 RefrMat = {0.5,0,0,0.5,0,-0.5,0,0.5,0,0,0.5,0.5,0,0,0,1};
   float4x4 ReflMat = {0.5,0,0,0.5,0,0.5,0,0.5,0,0,0.5,0.5,0,0,0,1};
   float4 LightSource = {-1.0f, -1.0f, -1.0f, 1.0f};
   float4 eyePos : CameraPosition;
   
  //-----------------
  // tweaks
  //-----------------
   float2 WaterScale = { 10.0f , 10.0f};
   float WaterBump = 0.02f;
   float FresnelBias = 0.1f;
   float FresnelScale = 5.0f;//7.5f;
   float2 Speed1 = { -0.015, 0.0 };
   float2 Speed2 = { 0.014, -0.014 };
   float2 Speed3 = { -0.007, 0.007 };
   float4 WaterTint = { 0.7f, 0.7f, 0.7f, 1.0f };
   float SpecularOverride;
   float4 ScrollScaleUV;
   
  //HUD Fog Color
   float4 HudFogColor : Diffuse
   <   string UIName =  "Hud Fog Color";    
   > = {0.0f, 1.0f, 0.0f, 0.5f};

  //HUD Fog Distances (near,far,0,0)
   float4 HudFogDist : Diffuse
   <   string UIName =  "Hud Fog Dist";    
   > = {0.0f, 1.0f, 0.0f, 0.0f};

  // Texture
Texture2D WaterbumpTX : register( t0 );
Texture2D WaterrefractTX : register( t1 );
Texture2D WaterreflectTX : register( t2 );
Texture2D WatermaskTX : register( t3 );
 
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

  //-----------------
  // structs 
  //-----------------
   struct input
     {
    float3 Pos    : POSITION;
    float3 Normal      : NORMAL;
    float2 UV          : TEXCOORD0;
     };
   struct output
   {
    float4 OPos:POSITION; 
    float2 Tex0:TEXCOORD0; 
    float2 Tex1:TEXCOORD1; 
    float2 Tex2:TEXCOORD2; 
    float2 Tex3:TEXCOORD3; 
    float3 ViewVec:TEXCOORD4;  
    float4 RefrProj:TEXCOORD5;
    float4 ReflProj:TEXCOORD6;
    float4 WPos:TEXCOORD7;
#ifndef OLDWATER    
    float3 WorldNormal:TEXCOORD8;
    float3 Tn:TEXCOORD9;
    float3 Bn:TEXCOORD10;
#endif
   };


  //-----------------
  // vertex shader
  //-----------------
   output VS(input IN) 
   {
    output OUT;
   OUT.OPos = mul(float4(IN.Pos,1),WorldVP); 
   OUT.Tex0 = IN.UV;
#ifdef OLDWATER   
   OUT.Tex1 = IN.UV*WaterScale/0.8+(time*Speed1);
   OUT.Tex2 = IN.UV*WaterScale+(time*Speed2);
   OUT.Tex3 = IN.UV*WaterScale*0.8+(time*Speed3);
#else
   OUT.Tex1 = IN.UV*nWaterScale/0.8+(time*Speed1);
   OUT.Tex2 = IN.UV*nWaterScale+(time*Speed2);
   OUT.Tex3 = IN.UV*nWaterScale*0.8+(time*Speed3);
   OUT.WorldNormal = normalize(mul(IN.Normal, (float3x3)WorldVP).xyz);
   float3 c1 = cross(OUT.WorldNormal, float3(0.0, 0.0, 1.0)); 
   float3 c2 = cross(OUT.WorldNormal, float3(0.0, 1.0, 0.0)); 
   if (length(c1) > length(c2)) {
      OUT.Tn = c1;   
   } else {
      OUT.Tn = c2;   
   }
   OUT.Tn = normalize( OUT.Tn);
   OUT.Bn = normalize(cross(OUT.WorldNormal, OUT.Tn));
#endif
   float4 WPos= mul(float4(IN.Pos,1),World);  
   float3 VP = ViewInv[3].xyz-WPos.xyz; 
    OUT.ViewVec = -VP/(FresnelScale*VP.y); 
     OUT.RefrProj = mul(RefrMat,OUT.OPos);
     OUT.ReflProj = mul(ReflMat,OUT.OPos);
    OUT.WPos = WPos; 
   return OUT;
   }

// pixel shaders

float4 PS(output IN) : COLOR 
{
   float4 diffuse=WaterbumpTX.Sample(SampleWrap,IN.Tex0);
   return float4(diffuse.xyz,0.5f);
}

float4 Fresnel_Reflect_Core(output IN,float nosky)
{
   #ifdef USEWATERFORFLOOR
   float2 refluv = IN.ReflProj.xy / IN.ReflProj.w;
   return float4( WaterreflectTX.Sample(SampleClamp,refluv).xyz,0.10);
   #else
#ifdef OLDWATER    
   float Mask=WatermaskTX.Sample(SampleClamp,IN.Tex0).x;
   float RippleFactor = WatermaskTX.Sample(SampleClamp,IN.Tex0).y * 0.01f;
   float4 Distort = (WaterbumpTX.Sample(SampleWrap, IN.Tex1)*2-1)*0.3;
   Distort = Distort + (WaterbumpTX.Sample(SampleWrap, IN.Tex2)*2-1)*0.3;
   Distort = Distort + (WaterbumpTX.Sample(SampleWrap, IN.Tex3)*2-1)*0.3;
   float  Fresnel = 1-saturate(dot(IN.ViewVec,IN.ViewVec));
   Fresnel = FresnelBias+Fresnel*(1-FresnelBias);
   Fresnel = Fresnel * 0.4f;
   Distort = (Distort*(IN.RefrProj.z*WaterBump)) * RippleFactor;
   //float3 Reflection = tex2Dproj(WaterReflect,IN.ReflProj+Distort);
   float2 refluv = IN.ReflProj.xy / IN.ReflProj.w;
   float3 Reflection = WaterreflectTX.Sample(SampleClamp,refluv+Distort.xy).xyz;
   float3 WaterColor = (Reflection*WaterTint.xyz);
   float4 cameraPos = mul(IN.WPos, View);
   float hudfogfactor = saturate((cameraPos.z - HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   float fMaskValue = 0.1f+((Mask*(1.0f-Fresnel))*0.9f);
   float4 result = float4(WaterColor,fMaskValue);
   float finalFactor = hudfogfactor*HudFogColor.w;
   float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),finalFactor);
   finalFactor += cameraPos.z/2500.0; // PE: Hide terrain lod. 
   return float4(hudfogresult.xyz,clamp(fMaskValue+finalFactor,0.0,1.0));
#else

   float3 Nn = normalize(IN.WorldNormal);     
   float4 cameraPos = mul(IN.WPos, View);

   float4 normalmap = WaterbumpTX.Sample(SampleWrap, IN.Tex1+ ( (time/WaterSpeed1)*flowdirection) );
   float genericwave = (normalmap.b-0.5*2.0)+0.5;
   
   float3 dudv = normalize( normalmap.rgb * 2.0 - 1.0) * distortion;
   float3 dudv2 = normalize( normalmap.rgb * 2.0 - 1.0) * distortion2;

   normalmap = WaterbumpTX.Sample( SampleWrap, IN.Tex1+dudv2.rg+ ((time/WaterSpeed2)*flowdirection) );
   // pattern removal.
   normalmap = lerp( normalmap , WaterbumpTX.Sample( SampleWrap, (IN.Tex1/4.0)+dudv2.rg+((time/WaterSpeed2)*flowdirection) ) , clamp( (cameraPos.z/9500.0)-0.1,0.0,0.50) );

   genericwave = clamp(  (((normalmap.b-0.75)*2.80)+0.1),genericwaveIntensity,1.0);
   genericwave = ( genericwave + clamp((normalmap.a+0.4)+(cameraPos.z/7000.0),genericwaveIntensity,1.0) ) * 0.5;
   float halfinvInt = (1.0-genericwaveIntensity)*0.5;
   genericwave = clamp( (1.0-genericwave)+genericwaveIntensity+(halfinvInt*0.5)  , genericwaveIntensity , 1.0+halfinvInt); // genericwaveIntensity

   float3x3 tangentbasis = float3x3( 2*normalize(IN.Tn), 2*normalize(IN.Bn), Nn );
   float3 Nb = normalmap.xyz;
   float3 Nb2 = normalmap.xyz;

   Nb.xy = Nb.xy * 2.0 - 1.0;
   Nb.z = sqrt(1.0 - dot(Nb.xy, Nb.xy));
   Nb = mul(Nb,tangentbasis);
   Nb = normalize((Nb+IN.WorldNormal)*0.5);

   Nb2.xy = Nb2.xy * 2.07 - 1.0; //1.2
   Nb2.z = sqrt(1.0 - dot(Nb2.xy, Nb2.xy));
   Nb2 = mul(Nb2,tangentbasis);
   Nb2 = normalize((Nb2+IN.WorldNormal)*0.5);

   float3 Ln = normalize(LightSource.xyz);
   float3 V  = (eyePos.xyz - IN.WPos.xyz);  
   float3 Vn  = normalize(-V); // PE: eyePos not set ? normally use (V)
   float3 Hn = normalize(Vn+Ln);
   float4 lighting = lit( dot(Ln,Nb),dot(Hn,Nb),24);
   //lighting.y = lerp(0.65,lighting.y,SurfaceSunFactor); // PE: not set in this shader.
   lighting.y = clamp(lighting.y,0.8,1.0); 
             
   float2 refluv = IN.ReflProj.xy / IN.ReflProj.w;
   float3 Reflection = WaterreflectTX.Sample(SampleClamp,refluv+dudv.rg).xyz * nosky;
   //float3 Reflection = (ReflectionTex*WaterTint.xyz);

   float3 MaterialSpecularColor = float3(0.35,0.35,0.35);
   float3 E = normalize(IN.WPos.xyz-eyePos.xyz); // eye , facing the camera.
   float3 R = reflect(-Ln,Nb); // reflect direction
   float cosAlpha = clamp(dot(E,R),0.0,1.0);
   cosAlpha = pow(cosAlpha,6.0);
   float3 spec = MaterialSpecularColor * cosAlpha;  //Specular

   //fade away lighting.z to remove more patterns.
   lighting.z = spec.z - clamp((cameraPos.z/48500.0)-0.40,0.0,spec.z);
   
   float3 WaterColor2 = (WaterCol + lighting.z ) * lighting.y * 0.75;
   
   float3 sparckle = (clamp(dot(Ln,Nb2),0.80,1.0)*1.0605)  + (lighting.z * 0.41 ); //0.35
   sparckle = clamp( sparckle - 0.980 , 0.0,0.020 ) * 40.0;

   sparckle.r = sparckle.r - clamp((cameraPos.z/3000.0)-0.2,0.0,sparckle.r);

   WaterColor2 = WaterColor2 * genericwave;

   float3 WaterColor = lerp( WaterColor2 , Reflection , WaterReflection );

#ifdef USEREFLECTIONSPARKLE
   WaterColor = lerp( WaterColor , WaterColor*reflectionSparkleIntensity , sparckle.r );
#else
   WaterColor = lerp( WaterColor , float3(WaterSparkleCol) , sparckle.r );
#endif

   // Adding in fog
   float hudfogfactor = saturate((cameraPos.z - HudFogDist.x)/(HudFogDist.y - HudFogDist.x));

   float4 result = float4(WaterColor,WaterTransparancy);

   float finalFactor = hudfogfactor*HudFogColor.w;
   float4 hudfogresult = lerp(result,float4(HudFogColor.xyz,0),finalFactor);
   finalFactor += cameraPos.z/2500.0; // PE: Hide terrain lod. 
   return float4(hudfogresult.xyz,clamp(WaterTransparancy+finalFactor,0.0,1.0));


#endif

   #endif
} 

float4 PS_Fresnel_Reflect(output IN) : COLOR 
{
	return Fresnel_Reflect_Core ( IN, 1.0 );
}

float4 PS_Fresnel_ReflectNoSky(output IN) : COLOR 
{
	return Fresnel_Reflect_Core ( IN, 0.0 );
}

float4 PS_Fresnel_NoReflect(output IN) : COLOR 
{
   float Mask=WatermaskTX.Sample(SampleClamp,IN.Tex0).r;
   float Fresnel = 1-saturate(dot(IN.ViewVec,IN.ViewVec));
   Fresnel = FresnelBias+Fresnel*(1-FresnelBias);
   float3 WaterColor = float3(34.0/256.0,54.0/256.0,107.0/256.0);
   float4 cameraPos = mul(IN.WPos, View);
   float hudfogfactor = saturate((cameraPos.z- HudFogDist.x)/(HudFogDist.y - HudFogDist.x));
   float fMaskValue = 0.1f+((Mask*(1.0f-Fresnel))*0.9f);
   float4 result = float4(WaterColor,fMaskValue);
   float finalFactor = hudfogfactor*HudFogColor.w;
   float3 hudfogresult = lerp(result.xyz,HudFogColor.xyz,finalFactor);
   finalFactor += cameraPos.z/2500.0; // PE: Hide terrain lod.
   return float4(hudfogresult,clamp(fMaskValue+finalFactor,0.0,1.0));
} 

// techniques   

technique11 Editor
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        SetGeometryShader(NULL);
    }
}

technique11 UseReflection
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Fresnel_Reflect()));
        SetGeometryShader(NULL);
    }
}

technique11 UseReflectionNoSky
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Fresnel_ReflectNoSky()));
        SetGeometryShader(NULL);
    }
}

technique11 NoReflection
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Fresnel_NoReflect()));
        SetGeometryShader(NULL);
    }
}
