string Description = "Water Entity Shader";
#define USEWATERMAPPING
   float2 WaterScale = { 155.0f , 155.0f}; // PE: wave size higher = smaller waves default 155.0,155.0
   float distortion = 0.0055f; // PE: water distortion reflection default 0.0055f
   float distortion2 = 0.030f; // PE: water distortion waves default 0.030f
//   float3 WaterCol = { 158.0/256.0 ,168.0/256.0 , 198.0/256.0 }; // PE: more neutral water
   float3 WaterCol = { 112.0/256.0 ,129.0/256.0 , 145.0/256.0 }; // PE: Default more grey , blue water
//   float3 WaterCol = { 112.0/256.0 ,169.0/256.0 , 185.0/256.0 }; // PE: Default blue water
//   float3 WaterCol = { 74.0/256.0 ,218.0/256.0 , 156.0/256.0 }; // PE: Green water
//   float3 WaterCol = { 220.0/256.0 ,0.0/256.0 , 0.0/256.0 }; // PE: red water
   float3 WaterSparkleCol = { 290.0/256.0 ,290.0/256.0 , 290.0/256.0 }; // PE: Higher sparkles skine more.
   float WaterTransparancy = 0.75f; // PE: Water transparancy lower more transparant, default 0.75
   float WaterReflection = 0.50f; // PE: Reflection higher = more reflection , default 0.5
   float WaterSpeed1 = 30.0f; // PE: Speed 1 lower = faster default 30.0f
   float WaterSpeed2 = 70.0f; // PE: Speed 2 lower = faster default 30.0f  
   float genericwaveIntensity = 0.75; // PE: Generic wave , default 0.75 , values 0.0-1.0       
#define USEREFLECTIONSPARKLE // PE: Use reflection colors in sparkle.
   float reflectionSparkleIntensity = 1.80; // PE: Sparkle Intensity , higher = more bright
#define PRESERVETRANSPARENCY
#include "entity_core.fx"
