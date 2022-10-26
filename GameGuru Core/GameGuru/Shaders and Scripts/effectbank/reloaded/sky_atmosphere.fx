string Description = "Procedural Sky Shader";
//cybernescence - inspired by Wicked Engine

#include "settings.fx"
#include "constantbuffers.fx"

//#define SUPERFASTSKY
//#define COLOUREDSTARS
#define LIGHTNING
#define AURORA
#define AURORA_RED
//#define AURORA2
#define DOMECLOUDS

// standard constants
float alphaoverride 			: alphaoverride;
float alphaoverrideatmos 		: alphaoverrideatmos ;
float time 					: time;
float4 clipPlane 				: ClipPlane;
float4 ScrollScaleUV;
float SpecularOverride;
float4 eyePos 					: CameraPosition;
float SurfaceSunFactor = {1.0f}; 
float4 LightSource;

Texture2D DiffuseMap 			: register( t0 );
TextureCube EnvironmentMap 	: register( t6 );

SamplerState SampleWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct appdata
{
    float3 Position 	: POSITION;
    float3 Normal		: NORMAL;
    float2 UV			: TEXCOORD0;    
};

struct vertexOutput
{
    float4 Position   	: POSITION;
    float2 TexCoord  	: TEXCOORD0;
	float clip        	: TEXCOORD1;
	float4 ObjPos     	: TEXCOORD2;
	float4 LPos       	: TEXCOORD3;
 };
 
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
float4 SkyClouds : Diffuse
<    string UIName =  "SkyClouds";    
> = {0.1, 0.1, 0.5, 1.0f};
float4 SkyColor : Diffuse
<    string UIName =  "SkyColor";    
> = {0.9, 0.9, 1.0, 1.0f};
float4 SkyScrollValues : Diffuse
<    string UIName =  "SkyScrollValues";    
> = {0.0f, 0.0f, 0.1f, 0.002f};


#define PI 3.14159265359
bool sun_enabled = true;
bool dark_enabled = false; 
inline float3 GetSunColor() { return float3(0.88,0.88,0.38); }
inline float3 GetSunDirection() { return LightSource.xyz; } 
inline float3 GetHorizonColor() { return float3(0.549, 0.619, 0.678); }
inline float3 GetZenithColor() { return float3(0.39, 0.57, 1.0); } 

// https://www.shadertoy.com/view/Xsl3Dl

const float3x3 mc = float3x3(  0.00,  0.80,  0.60,
								-0.80,  0.36, -0.48,
								-0.60, -0.48,  0.64 );
					
float3 hash_gradient_3D( float3 p ) 
{
	p = float3(dot(p, float3(127.1, 311.7, 74.7)),
		dot(p, float3(269.5, 183.3, 246.1)),
		dot(p, float3(113.5, 271.9, 124.6)));

	return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}
float noise_gradient_3D(in float3 p)
{
	float3 i = floor(p);
	float3 f = frac(p);

	float3 u = f * f * (3.0 - 2.0 * f);

	return lerp(lerp(lerp(dot(hash_gradient_3D(i + float3(0.0, 0.0, 0.0)), f - float3(0.0, 0.0, 0.0)),
		dot(hash_gradient_3D(i + float3(1.0, 0.0, 0.0)), f - float3(1.0, 0.0, 0.0)), u.x),
		lerp(dot(hash_gradient_3D(i + float3(0.0, 1.0, 0.0)), f - float3(0.0, 1.0, 0.0)),
			dot(hash_gradient_3D(i + float3(1.0, 1.0, 0.0)), f - float3(1.0, 1.0, 0.0)), u.x), u.y),
		lerp(lerp(dot(hash_gradient_3D(i + float3(0.0, 0.0, 1.0)), f - float3(0.0, 0.0, 1.0)),
			dot(hash_gradient_3D(i + float3(1.0, 0.0, 1.0)), f - float3(1.0, 0.0, 1.0)), u.x),
			lerp(dot(hash_gradient_3D(i + float3(0.0, 1.0, 1.0)), f - float3(0.0, 1.0, 1.0)),
				dot(hash_gradient_3D(i + float3(1.0, 1.0, 1.0)), f - float3(1.0, 1.0, 1.0)), u.x), u.y), u.z);
}
float Hash21(float2 p)
{
    p = frac(p*float2(123.34,456.21));
    p +=dot(p,p+45.32);
    return frac(p.x*p.y);
}

//lightning
float rand(float x)
{
    return frac(sin(x)*75154.32912);
}
float rand3d(float3 x)
{
    return frac(375.10297 * sin(dot(x, float3(103.0139,227.0595,31.05914))));
}

float noise(float x)
{
    float i = floor(x);
    float a = rand(i), b = rand(i+1.0);
    float f = x - i;
    return lerp(a,b,f);
}
float perlin(float x)
{
    float r=0.0,s=1.0,w=1.0;
    for (int i=0; i<6; i++) {
        s *= 2.0;
        w *= 0.5;
        r += w * noise(s*x);
    }
    return r;
}

float f(float y)
{
    float w = 0.25; // width of strike
    return w * (perlin(2.0 * y) - 0.5);
}
float plot(float2 p, float d, bool thicker)
{
    if (thicker) d += 5. * abs(f(p.y + 0.001) - f(p.y));
    return smoothstep(d, 0., abs(f(p.y) - p.x));
}

//AURORA
float2x2 mm2(in float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2x2(c,s,-s,c);
}
float2x2 m2 = float2x2(0.95534, 0.29552, -0.29552, 0.95534);
float tri(in float x)
{
    return clamp(abs(frac(x)-.5),0.01,0.49);
}
float2 tri2(in float2 p)
{
    return float2(tri(p.x)+tri(p.y),tri(p.y+tri(p.x)));
}
float triNoise2d(in float2 p, float spd)
{
    float z=1.8;
    float z2=2.5;
	float rz = 0.;
	p = mul(p,mm2(p.x*0.06));
    float2 bp = p;
	for (float i=0.; i<5.; i++ )
	{
        float2 dg = tri2(bp*1.85)*.75;
 		dg = mul(dg,mm2(time*spd));
        p -= dg/z2;

        bp *= 1.3;
        z2 *= 1.45;
        z *= .42;
		p *= 1.21 + (rz-1.0)*.02;
        
        rz += tri(p.x+tri(p.y))*z;
       // p*= -m2;
		p = mul(p,-m2);
		
	}
    return clamp(1./pow(rz*29., 1.3),0.,.55);
}
float hash21(in float2 n){ return frac(sin(dot(n, float2(12.9898, 4.1414))) * 43758.5453); }
float4 aurora(float3 ro, float3 rd, float2 uv)
{
    float4 col = float4(0,0,0,0);
    float4 avgCol = float4(0,0,0,0);
    float mt = 1.0;//10.;
    for(float i=0.0; i<50.0; i++) 
    {
        float of = 0.006*hash21(uv.xy)*smoothstep(0.0,15.0, i*mt);
        float pt = ((0.8+pow((i*mt),1.4)*0.002)-ro.y)/(rd.y*2.0+0.4);
        pt -= of;
    	float3 bpos = (ro) + pt*rd;
        float2 p = bpos.zx;
        
        float rzt = triNoise2d(p, 0.06);
        float4 col2 = float4(0,0,0, rzt);
        col2.rgb = (sin(1.0-float3(2.15,-0.5, 1.2)+(i*mt)*0.043)*(0.5*mt)+0.5)*rzt;
        avgCol = lerp(avgCol, col2, 0.5);
        col += avgCol*exp2((-i*mt)*0.065 - 2.5)*smoothstep(0.0,5.0,i*mt);
    }
    col *= (clamp(rd.y*15.0+0.4,0.0,1.0));
    return col*1.8;
}
//AURORA 2

const float cHashM = 43758.54;
float2 Hashv2v2 (float2 p)
{
  float2 cHashVA2 = float2 (37.0, 39.0);
  return frac (sin (float2 (dot (p, cHashVA2), dot (p + float2 (1.0, 0.0), cHashVA2))) * cHashM);
}

float Noisefv2 (float2 p)
{
  float2 t, ip, fp;
  ip = floor (p);  
  fp = frac (p);
  fp = fp * fp * (3.0 - 2.0 * fp);
  t = lerp (Hashv2v2 (ip), Hashv2v2 (ip + float2 (0.0, 1.0)), fp.y);
  return lerp (t.x, t.y, fp.x);
}
float WaveHt (float2 p)
{
  float tWav = 0.8 * time * SkyClouds.w;
  float2x2 qRot = float2x2 (0.8, -0.6, 0.6, 0.8);
  float4 t4, v4;
  float2 t;
  float wFreq, wAmp, ht;
  wFreq = 1.0;
  wAmp = 1.0;
  ht = 0.0;
  for (int j = 0; j < 3; j ++) {
 	p = mul(p,qRot);
    t = tWav * float2 (1.0, -1.0);
    t4 = (p.xyxy + t.xxyy) * wFreq;
    t = float2 (Noisefv2 (t4.xy), Noisefv2 (t4.zw));
    t4 += 2. * t.xxyy - 1.0;
    v4 = (1.0 - abs (sin (t4))) * (abs (sin (t4)) + abs (cos (t4)));
    ht += wAmp * dot (pow (1.0 - sqrt (v4.xz * v4.yw), float2 (8.0,8.0)), float2 (1.0,1.0));
    wFreq *= 2.;
    wAmp *= 0.5;
  }
  return ht;
}

float3 HsvToRgb (float3 c)
{
	float3 p;
	p = abs (frac (c.xxx + float3 (1.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
	return c.z * lerp (float3 (1.0,1.0,1.0), clamp (p - 1.0, 0.0, 1.0), c.y);

}
float4 AurCol (float3 ro, float3 rd)
{
  float4 col, mCol;
  float3 p, dp;
  float ar;
  dp = rd / rd.y;
  p = ro + (40.0 - ro.y) * dp;
  col = float4 (0.0,0.0,0.0,0.0);
  mCol = float4 (0.0,0.0,0.0,0.0);
  for (float ns = 0.0; ns < 50.0; ns ++) {
    p += dp;
    ar = 0.05 - clamp (0.06 * WaveHt (0.01 * p.xz), 0.0, 0.04);
    mCol = lerp (mCol, ar * float4 (HsvToRgb (float3 (0.34 + 0.007 * ns, 1., 1. - 0.02 * ns)), 1.0), 0.5);
    col += mCol;
  }
  return col;
}

//AURORA 3

float hash(float2 co) { return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453); }
float hash(float x, float y) { return hash(float2(x, y)); }

float shash(float2 co)
{
	float x = co.x;
	float y = co.y;
	
	float corners = (hash(x-1.0, y-1.0) + hash(x+1.0, y-1.0) + hash(x-1.0, y+1.0) + hash(x+1.0, y+1.0))/16.0;
	float sides   = (hash(x-1.0, y) + hash(x+1.0, y) + hash(x, y-1.0) + hash(x, y+1.0))/8.0;
	float center  = hash(co) / 4.0;
	
	return corners + sides + center;
}

float noise(float2 co)
{
	float2 pos  = floor(co);
	float2 fpos = co - pos;
	
	fpos = (3.0 - 2.0*fpos)*fpos*fpos;
	
	float c1 = shash(pos);
	float c2 = shash(pos + float2(0.0, 1.0));
	float c3 = shash(pos + float2(1.0, 0.0));
	float c4 = shash(pos + float2(1.0, 1.0));
	
	float s1 = lerp(c1, c3, fpos.x);
	float s2 = lerp(c2, c4, fpos.x);
	
	return lerp(s1, s2, fpos.y);
}

float pnoise(float2 co, int oct)
{
	float total = 0.0;
	float m = 0.0;
	
	if(oct == 1)
	{
		
		int i = 0;
		
		float freq = pow(2.0, float(i));
		float amp  = pow(0.5, float(i));
		
		total += noise(freq * co) * amp;
		m += amp;
	
	}
	else
	{
	
	for(int i=0; i<oct; i++)
	{
		float freq = pow(2.0, float(i));
		float amp  = pow(0.5, float(i));
		
		total += noise(freq * co) * amp;
		m += amp;
	}
	
	}
	
	return total/m;
}

float2 fbm(float2 p, int oct)
{
	return float2(pnoise(p + float2(time, 0.0), oct), pnoise(p + float2(-time, 0.0), oct));
}

float fbm2(float2 p, int oct)
{
	return pnoise(p + 10.0*fbm(p, oct) + float2(0.0, time), oct);
}

float3 lights(float2 co)
{
	float d,r,g,b,h;
	float3 rc;
	
	// Red 
	r = fbm2(co * float2(1.0, 0.5), 1);
	d = pnoise(2.0*co+float2(0.3*time,0.3*time), 1);
	rc = float3(1, 0.0, 0.0) * r * smoothstep(0.0, 2.5+d*r, co.y) * smoothstep(-0.1, 1.0, 4.0-co.y-2.0*d);	
	return rc;
}

#define FLATTEN 0.30 //.17
float2 IntoCartesian(float3 dir)
{
	float2 uv;
	dir.y /= FLATTEN;//'height' of clouds
	dir = normalize(dir);
	uv.x = dir.x;
	uv.y = dir.z;
	uv = 0.5 + (0.5 * uv);
	//uv.y = 1.0 - uv.y;
	return uv;
}

const float2x2 m = float2x2( 1.6,  1.2, -1.2,  1.6 );

float2 hashc(float2 p)
{
	p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
	return -1.0 + 2.0*frac(sin(p)*43758.5453123);
}

float noisec(in float2 p)
{
	const float K1 = 0.366025404; 
	const float K2 = 0.211324865; 
	float2 i = floor(p + (p.x + p.y)*K1);
	float2 a = p - i + (i.x + i.y)*K2;
	float2 o = (a.x > a.y) ? float2(1.0, 0.0) : float2(0.0, 1.0); 
	float2 b = a - o + K2;
	float2 c = a - 1.0 + 2.0*K2;
	float3 h = max(0.5 - float3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	float3 n = h * h*h*h*float3(dot(a, hashc(i + 0.0)), dot(b, hashc(i + o)), dot(c, hashc(i + 1.0)));
	return dot(n, float3(70.0, 70.0, 70.0));
}

float fbmc(float2 n) 
{
	float total = 0.0, amplitude = 0.1;
	for (int i = 0; i < 7; i++) 
	{
		total += noisec(n) * amplitude;
		n = mul(n, m); 
		amplitude *= 0.4;
	}
	return total;
}


vertexOutput mainVS(appdata IN)   
{                   
    vertexOutput OUT;
    float4 worldSpacePos = mul(float4(IN.Position,1), World);
    OUT.Position = mul(float4(IN.Position,1), World);
    OUT.Position = mul(OUT.Position, View);
    OUT.Position = mul(OUT.Position, Projection);
    OUT.clip = dot(worldSpacePos, clipPlane);                                                                      
    OUT.ObjPos = float4(IN.Position,1);
    OUT.LPos = worldSpacePos;
    OUT.TexCoord = IN.UV; 
    return OUT;
}

float4 mainPS(vertexOutput IN) : COLOR
{
	float4 result = float4(1,0,0,1);
	
	float distantfogfactor = 1.0 - (HudFogDist.y / 50000.0f);
	float hudfogfactor = 1.0; //distantfogfactor;
	distantfogfactor = 1.0 - (HudFogDist.y / 200000.0f);
	hudfogfactor = clamp(hudfogfactor*HudFogColor.w*distantfogfactor,0.0,1.0);  

	//TODO - when all shaders refactored for GI
	//float3 horizonskycolor = EnvironmentMap.SampleLevel(SampleWrap, float3(0.0, 0.0, 1.0), 100).rgb; //horizon
	//float3 horizonskycolor = EnvironmentMap.SampleLevel(SampleWrap, float3(0.0, 1.0, 0.0), 100).rgb; //up
	float3 horizonskycolor = GetHorizonColor();  
	horizonskycolor.rgb = horizonskycolor.rgb * HudFogColor.xyz;
	
	#ifdef SUPERFASTSKY
		result.rgb = lerp(GetHorizonColor(), GetZenithColor(), saturate(IN.ObjPos.y * 0.5f + 0.5f)) * 1.0;
	#else
		//custom			
		float3 sunDirection = GetSunDirection();
		float3 sunColor = GetSunColor();
	
		float3 V; 
		float3 eyeraw = eyePos.xyz - IN.LPos.xyz;
		V = normalize(-eyeraw);
	
		const float3 skyColor = GetZenithColor();
		const bool sunPresent = any(sunColor);
		const bool sunDisc = sun_enabled;

		const float zenith = V.y + 0.05; //added 0.05 to move horizon down slightly // how much is above (0: horizon, 1: directly above)
		const float sunScatter = saturate(sunDirection.y + 0.1f); // how much the sun is directly above. Even if sun is at horizon, we add a constant scattering amount so that light still scatters at horizon

		const float atmosphereDensity = 0.5 + 0.0; //0.1;//0.2; //g_xFrame_FogHeightSky; // constant of air density, or "fog height" as interpreted here (bigger is more obstruction of sun)
		const float zenithDensity = atmosphereDensity / pow(max(0.000001f, zenith), 0.75f);
		const float sunScatterDensity = atmosphereDensity / pow(max(0.000001f, sunScatter), 0.75f);

		const float3 aberration = float3(0.39, 0.57, 1.0); // the chromatic aberration effect on the horizon-zenith fade line
		const float3 skyAbsorption = saturate(exp2(aberration * -zenithDensity) * 2.0f); // gradient on horizon
		const float3 sunAbsorption = sunPresent ? saturate(sunColor * exp2(aberration * -sunScatterDensity) * 2.0f) : 1; // gradient of sun when it's getting below horizon

		const float sunAmount = distance(V, sunDirection); // sun falloff descreasing from mid point
		const float rayleigh = sunPresent ? 1.0 + pow(1.0 - saturate(sunAmount), 2.0) * PI * 0.5 : 1;
		const float mie_disk = saturate(1.0 - pow(sunAmount, 0.1));//0.1
		const float3 mie = mie_disk * mie_disk * (3.0 - 2.0 * mie_disk) * 2.0 * PI * sunAbsorption;

		float3 totalColor = lerp(GetHorizonColor(), GetZenithColor() * zenithDensity * rayleigh, skyAbsorption);
		totalColor = lerp(totalColor * skyAbsorption, totalColor, sunScatter); // when sun goes below horizon, absorb sky color
		//if (sunDisc)
		//{
			const float3 sunl = smoothstep(0.03, 0.026, sunAmount) * sunColor * 50.0 * skyAbsorption; // sun disc
			totalColor += sunl;
			totalColor += mie;
		//}
		totalColor *= (sunAbsorption + length(sunAbsorption)) * 0.5f; // when sun goes below horizon, fade out whole sky
		totalColor *= 0.95; //0.25; // exposure level

		if (dark_enabled)
		{
			totalColor = max(pow(saturate(dot(sunDirection, V)), 64) * sunColor, 0) * skyAbsorption;
		}
		
		//stars
		if (SkyClouds.z>0) 
		{
			float3 stars_direction = mul(V, mc); //TODO rotate slowly?
			float stars_visibility = pow(saturate(1 - sunDirection.y), 2);
			float starDensity = SkyClouds.z; 
			float stars_density_at_maximum = lerp(18, 8, starDensity);
			float stars_threshold = lerp(32, stars_density_at_maximum, stars_visibility); // modifies the number of stars that are visible
			float stars_exposure = lerp(0, 512, stars_visibility); // modifies the overall strength of the stars
			float stars = saturate(pow(abs(noise_gradient_3D(stars_direction * 300)), stars_threshold)) * stars_exposure;
			stars *= lerp(0.4, 1.4, noise_gradient_3D(stars_direction * 256 + time)); // time based flickering
			
			#ifdef COLOUREDSTARS
				//float n = Hash21(stars_direction.xy); 
				float n = Hash21(IN.TexCoord); 
				float3 starColor = sin(float3(0.2,0.3,0.9)*frac(n*2345.2)*123.2)*0.5+0.5;
				float c_intensity = sin(time*3.+n*6.2831)*0.5 + 2.0;
				starColor *= c_intensity;
				starColor *= stars;
				totalColor.rgb += starColor;
			#else
				totalColor.rgb += stars;
			#endif
		}
		
		#ifdef LIGHTNING //https://www.shadertoy.com/view/fsdGWf
			
		if (SkyClouds.w>0.5) 
		{	
			float x = time + 0.1;
			float m = 0.25; // max duration of strike
			float i = floor(x/m);
			float f = x/m - i;
			float k = 1.0 - (((SkyClouds.w * (1.00 - 0.50)) / 1.0) + 0.5); //0.4; // frequency of strikes
			float n = noise(i);
			float t = ceil(n-k); // occurrence
			float d = max(0.0, n-k) / (1.0 - k); // duration
			float o = ceil(t - f - (1.0 - d)); // occurrence with duration
			float gt = 0.1; // glare duration
			float go = ceil(t - f - (1.0 - gt)); // glare occurrence
		
			float lightning = 0.0;
			float light = 0.0;
			float glare = 0.0;
			
			float2 uv = V.xy;
			if (o == 1.0) 
			{
				float2 uv2 = V.xy;
				uv2.y += i * 2.0; 
				float p = (noise(i+10.0) - 0.5) * 2.0; 
				uv2.x -= p;
			
				float strike = plot(uv2, 0.01, true);
				float glow = plot(uv2, 0.04, false);
				float glow2 = plot(uv2, 1.5, false);

				lightning = strike * 0.4 + glow * 0.15;

				float h = noise(i+5.0); // height
				lightning *= smoothstep(h, h+0.05, uv.y + perlin(1.2*uv.x + 4.0*h)*0.03);
				lightning += glow2 * 0.3;
				light = smoothstep(5.0, 0.0, abs(uv.x - p));
				glare = go * light;
			}
			
			totalColor.rgb += lightning; //(lightning+glare);
		}
		
		#endif	
			
		#ifdef AURORA
		
		if (SkyClouds.w>0 && SkyClouds.w<0.5) 
		{	
			float aurora_visibility = saturate(0.06 - sunDirection.y); 
			float2 q = V.xy;
			float2 p = q + 0.05;//0.10;
					
			float3 O = float3(0.0, 0.0, (SkyClouds.w+0.5) * -10.0);
			float3 D = normalize(float3(p,V.z));
			float4 aur = float4(0.0,0.0,0.0,1.0);
			
			#ifdef AURORA2
				float fade = smoothstep(0.0,0.01,abs(D.y))*0.1+0.9;
				aur = smoothstep(0.0,1.5,aurora(O,D,V.xy)) * fade * aurora_visibility;
			#endif
			
			float4 aCol = float4(0.0,0.0,0.0,1.0);
			if (V.y > -0.1)
			{
				//O = float3 (0.0, 10.0, -100.0);
				O = float3 (0.0, 2.0, -4.0);
				aCol = AurCol (O, D);
				aCol *= aurora_visibility;
				float mix = clamp(pow(abs(V.y+0.30),2),0,1.0);
				mix = lerp(mix,0,V.y+0.1);
				float amix = smoothstep(0.0,0.2,V.y);
				aCol = lerp(0,aCol,amix);
				totalColor.rgb = totalColor.rgb * (1.0-aCol.a) + aCol.rgb;
				
				#ifdef AURORA2
					totalColor.rgb = totalColor.rgb * (1.0-aur.a) + aur.rgb;
				#endif
				
				// Aurora Red
				#ifdef AURORA_RED
					float2 uv = V.xy;
					float2 co = V.xy;
					float3 col = float3(0.0,0.0,0.0);
					float s = 0.1*sin(time);
					float f = 0.1+0.4*pnoise(float2(5.0*uv.x, 0.3*time),1);
					float2 aco = co;
					aco.y -= f;
					aco *= 10.0*uv.x+15.0*SkyClouds.w;
					col += 1.5*lights(aco)
						* (smoothstep(0.3, 0.6, pnoise(float2(10.0*uv.x, 0.3*time),1))
						+  0.5*smoothstep(0.5, 0.7, pnoise(float2(10.0*uv.x,time),1)));
					col *= 2;
					float armix = smoothstep(0.0,0.1,V.y);
					col = lerp(0,col,armix);
					col *= aurora_visibility;
					totalColor.rgb += col.rgb;
				#endif
			}
				
		}
		
		#endif
		
		#ifdef DOMECLOUDS
		
			const float cloudscale = 10;//15;//1.1;
			const float clouddark = 0.5;
			const float cloudlight = 0.3;
		
			float speed = SkyScrollValues.w;
			float cloudcover = -clamp(0.5 - SkyScrollValues.z,-1,1); 
			float cloudalpha = (1 - SkyClouds.y) * -1;
			float skytint = SkyClouds.x; 
	
			float2 p = IntoCartesian(V);//V.xy;
			float2 uv = p;    
			float ctime = time * speed;
			float q = fbmc(uv * cloudscale * 0.5);
    
			float r = 0.0;
			uv *= cloudscale;
			uv -= q - ctime;
			float weight = 0.8;
			for (int i=0; i<8; i++)
			{
				r += abs(weight*noise( uv ));
				uv = mul(uv, m) + ctime;
				weight *= 0.7;
			}
    
			float f = 0.0;
			uv = p;
			uv *= cloudscale;
			uv -= q - ctime;
			weight = 0.7;
			for (int i2=0; i2<8; i2++)
			{
				f += weight*noise( uv );
				uv = mul(uv, m) + ctime;
				weight *= 0.6;
			}
    
			f *= r + f;
    	
			float c = 0.0;
			ctime = time * speed * 2.0;
			uv = p;
			uv *= cloudscale*2.0;
			uv -= q - ctime;
			weight = 0.4;
			for (int i3=0; i3<7; i3++)
			{
				c += weight*noise( uv );
				uv = mul(uv, m) + ctime;
				weight *= 0.6;
			}
    	
			float c1 = 0.0;
			ctime = time * speed * 3.0;
			uv = p;
			uv *= cloudscale*3.0;
			uv -= q - ctime;
			weight = 0.4;
			for (int i4=0; i4<7; i4++)
			{
				c1 += abs(weight*noise( uv ));
				uv = mul(uv, m) + ctime;
				weight *= 0.6;
			}
			c += c1;
			
			float3 skycolour = totalColor.rgb;
			float3 cloudcolour = float3(1.1, 1.1, 0.9) * clamp((skytint + cloudlight*c), 0.0, 1.0);
			f = cloudcover + cloudalpha*f*r;
			float dmix = smoothstep(0.0,0.2,V.y);
			cloudcolour = lerp(0,cloudcolour, dmix);
			
			totalColor.rgb = lerp(skycolour, clamp(0.5 * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));
    		
		#endif
		
		result.xyz = totalColor.rgb;
			
	#endif
			
	//add fog to simulated sky
	result = lerp(result,float4(horizonskycolor.rgb,0),hudfogfactor); 
	
	return float4(result.xyz,1);//alphaoverrideatmos); //alphaoverride);
	
}

float4 blackPS(vertexOutput IN) : SV_TARGET
{
   return float4(0,0,0,1);
}

technique11 Main
{
    pass MainPass
    {
        SetVertexShader(CompileShader(vs_5_0, mainVS()));
        SetPixelShader(CompileShader(ps_5_0, mainPS()));
        SetGeometryShader(NULL);
    }
}

technique11 blacktextured 
{
   pass P0
    {
		SetVertexShader(CompileShader(vs_5_0, mainVS()));
		SetPixelShader(CompileShader(ps_5_0,blackPS()));
		SetGeometryShader(NULL);
    }
}
