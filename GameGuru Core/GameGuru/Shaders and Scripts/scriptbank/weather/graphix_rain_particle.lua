-- LUA Script - precede every function and global member with lowercase name of script + '_main'
local emitterList = {}

local namedEmitter = 
{ 
    rain = 
		  { animSpd = 1, SR = 0, freq = 16, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ			  
			offset = {  -150,  100, -150,  150, 100,  150 },
			speed  = {     0, -0.8,    0,    0,  -1.2,  0},
			rotate = {               0,              0 },-- currently only Z rotate processed
                     -- startMin startMax endMin endMax			
			scale  = {   150,      250,    200,     300 },
			alpha  = {    40,       50,     10,      25 }
		  },	
	splash = 
		  { animSpd = 1/8, SR = 0, freq = 16, lifeMin = 2000, lifeMax = 2500,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = { -180,   -10,  -180,  180,   -10, 180 },
			speed  = {    0,    0,    0,    0,    0,     0 },
			rotate = {                0,                0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {     5,     15,    5,    15 },
			alpha  = {     100,     180,   0,    10}
		  }
}

-- caller must load image
local function PE_CreateNamedEmitter( name, image, frames, entity )
			
	local et = namedEmitter[ name ]
	
	if et == nil then return end
	
	local emitter = ParticlesGetFreeEmitter()
	
	if emitter == -1 then return end
	
	local pos, spd, scl = et.offset, et.speed, et.scale
	
	if entity == nil then
		ParticlesAddEmitterEx( emitter, et.animSpd, et.SR, 
		                     pos[1], pos[2], pos[3], pos[4], pos[5], pos[6],
							 scl[1], scl[2], scl[3], scl[4],
							 spd[1], spd[2], spd[3], spd[4], spd[5], spd[6],
							 et.rotate[1], et.rotate[2], et.lifeMin, et.lifeMax,
							 et.alpha[1], et.alpha[2], et.alpha[3], et.alpha[4],
							 et.freq, -1, 0, image, frames )
	else
		ParticlesAddEmitterEx( emitter, et.animSpd, et.SR, 
		                     pos[1], pos[2], pos[3], pos[4], pos[5], pos[6],
							 scl[1], scl[2], scl[3], scl[4],
							 spd[1], spd[2], spd[3], spd[4], spd[5], spd[6],
							 et.rotate[1], et.rotate[2], et.lifeMin, et.lifeMax,
							 et.alpha[1], et.alpha[2], et.alpha[3], et.alpha[4],
							 et.freq, entity, 0, image, frames ) 
	end
	
	return emitter
end
								
function graphix_rain_particle_init(e)
end

local splashImg  = splashImg  or ParticlesLoadImage( "effectbank\\particles\\weather\\splash.dds", 8 )
local rainImg = rainImg or ParticlesLoadImage( "effectbank\\particles\\weather\\rain.dds", 9 )

function graphix_rain_particle_main(e)

	local emitter = emitterList[e]

	if emitter == nil then
		emitterList[e] = { used = false }
		
	else	
		
		if not emitter.used then 

						
			local plonk = PE_CreateNamedEmitter( 'splash',  splashImg, 36)		
			local wet = PE_CreateNamedEmitter( 'rain', rainImg, 64)
						
			emitter.used = true
			
					
			ParticlesSetWindVector( 0, 0 )
		end
	end
	
end
