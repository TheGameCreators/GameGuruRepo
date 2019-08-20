-- LUA Script - GraPhiX Explosion Script - Thank you to AmenMoses for the scripts he creates without them this would not have been possible.

local U  = require "scriptbank\\utillib"
local P  = require "scriptbank\\physlib"

local min = math.min

local emitterList    = {}
local explosion_name = {}

-- blastRadius set this to the distance you want explosions to be felt default is 300 units
local blastRadius    = 300

local maxDamage      = 150
local maxPlayerPush  = 10
local maxEntityPush  = 200

-- triggerHealth this setting adds to the entity strength.
-- the default setting in entities properties panel is 25 which equates to 1 shot changing to 50 2 shots etc..
local triggerHealth  = 1000

-- Emitter list the emitters control position and scale of each particle the emiiter name 
-- i.e boom1, boom2 is assinged in your image_List.
local namedEmitter = 
{  
   
	boom1 = 
		  { animSpd = 1/2, SR = 0, freq = 64, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = {    0,    50,    0,    0,   80,    0 },
			speed  = {    0,    0,    0,    0,    0,     0 },
			rotate = {                0,                0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {   200,     300,    100,   200 },
			alpha  = {   100,     255,   0,    0}
		  },
	boom2 = 
		  { animSpd = 1/2, SR = 0, freq = 64, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = {    0,    50,    0,    0,   80,    0 },
			speed  = {    0,    0,    0,    0,    0,     0 },
			rotate = {                0,                0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {   300,     400,    100,   300 },
			alpha  = {   100,     200,   0,    0}
		  },
	boom3 = 
		  { animSpd = 1/2, SR = 0, freq = 64, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = {    0,    90,    0,    0,   90,    0 },
			speed  = {    0,    0,    0,    0,    0,     0 },
			rotate = {                0,                0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {   200,     300,    200,   300 },
			alpha  = {   100,     200,   0,    50}
		  },
	boom4 = 
		  { animSpd = 1/2, SR = 0, freq = 64, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = {    0,   20,    0,    0,   20,    0 },
			speed  = {    0,    0,    0,    0,    0,   0 },
			rotate = {                0,               0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {   200,     300,    100,   200 },
			alpha  = {   100,     200,   0,    0}
		  },
	boom5 = 
		  { animSpd = 1/2, SR = 0, freq = 64, lifeMin = 2000, lifeMax = 2000,
                     -- minX  minY  minZ  maxX  maxY  maxZ		  
			offset = {    0,    0,    0,    0,    0,   0 },
			speed  = {    0,    0,    0,    0,    0,   0 },
			rotate = {                0,               0 }, -- currently only Z rotate processed
                     -- startMin startMax endMin endMax	
			scale  = {      0,       0,      0,     0 },
			alpha  = {      0,       0,      0,     0 }
		  }
}

-- image_List this is the list of your decals/particle sheets keep the name the same has your image file name i.e. exp1 = exp1.dds
-- also the full path is required i.e. "effectbank\\explosion\\graphix\\exp1.dds"
-- emitName = the particle parametes set above 
-- imgFrames = the number of frames in your sprite sheets
-- imageNum must be a unique value some are reserved so just use 10 - 20
-- imploder is a boolean value false for a standard explosion if you have designed a implosion sprite sheet set it to true
-- PLEASE NOTE THIS VALUE DOES NOT CONVERT AN EXPLOSION TO AN IMPLOSION USE A SPRITE SHEET FOR THAT
local image_List = 
{   
	exp1 = { path     = "effectbank\\explosion\\graphix\\exp1.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 10,
			 imploder = false },
	exp2 = { path = "effectbank\\explosion\\graphix\\exp2.dds", 
	         emitName = 'boom3',
			 imgFrames = 64,
	         imageNum = 11,
			 imploder = false },
	exp3 = { path = "effectbank\\explosion\\graphix\\exp3.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 12,
			 imploder = false },
	exp4 = { path = "effectbank\\explosion\\graphix\\exp4.dds",
             emitName = 'boom1',
			 imgFrames = 64,			 
	         imageNum = 13,
			 imploder = false },
	exp5 = { path     = "effectbank\\explosion\\graphix\\exp5.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 14,
			 imploder = false },
	exp6 = { path     = "effectbank\\explosion\\graphix\\exp6.dds",
             emitName = 'boom3',
			 imgFrames = 64,
	         imageNum = 15,
			 imploder = false },
	exp7 = { path     = "effectbank\\explosion\\graphix\\exp7.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 16,
			 imploder = false },
	exp8 = { path     = "effectbank\\explosion\\graphix\\exp8.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 17,
			 imploder = false },
	exp9 = { path     = "effectbank\\explosion\\graphix\\exp9.dds",
             emitName = 'boom4',
			 imgFrames = 64,
	         imageNum = 18,
			 imploder = false },
	exp10 = { path     = "effectbank\\explosion\\graphix\\exp10.dds",
             emitName = 'boom1',
			 imgFrames = 64,
	         imageNum = 19,
			 imploder = false },
	exp11 = { path     = "effectbank\\explosion\\graphix\\exp11.dds",
             emitName = 'boom4',
			 imgFrames = 64,
	         imageNum = 20,
			 imploder = false },
	imp1 = { path     = "effectbank\\explosion\\graphix\\imp1.dds",
             emitName = 'boom3',
			 imgFrames = 64,
	         imageNum = 21,
			 imploder = true },
	imp2 = { path     = "effectbank\\explosion\\graphix\\imp2.dds",
             emitName = 'boom4',
			 imgFrames = 64,
	         imageNum = 22,
			 imploder = true },
	imp3 = { path     = "effectbank\\explosion\\graphix\\imp3.dds",
             emitName = 'boom4',
			 imgFrames = 64,
	         imageNum = 23,
			 imploder = true },
	imp4 = { path     = "effectbank\\explosion\\graphix\\imp4.dds",
             emitName = 'boom4',
			 imgFrames = 64,
	         imageNum = 24,
			 imploder = true }
}

-- Unless you know what you are doing please do not alter any of the below script.

-- caller must load image. some clever stuff Amenmoses made
local function PE_CreateNamedEmitter( name, image, frames, entity )
			
	local et = namedEmitter[ name ]
	
	if et == nil then return end
	
	local emitter = ParticlesGetFreeEmitter()
	
	if emitter == -1 then return end
	
	local pos, spd, scl = et.offset, et.speed, et.scale
-- this bit makes a cup of tea
	if entity == nil then
		ParticlesAddEmitterEx( emitter, et.animSpd, et.SR, 
		                     pos[1], pos[2], pos[3], pos[4], pos[5], pos[6],
							 scl[1], scl[2], scl[3], scl[4],
							 spd[1], spd[2], spd[3], spd[4], spd[5], spd[6],
							 et.rotate[1], et.rotate[2], et.lifeMin, et.lifeMax,
							 et.alpha[1], et.alpha[2], et.alpha[3], et.alpha[4],
							 et.freq, -1, 0, image, frames )
	else
-- this bit makes coffee
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
								
function graphix_explosion_init_name(e, name)
	Include( "utillib.lua" )
	Include( "physlib.lua" )
		
	explosion_name[e] = string.match(name,"_(%w+)")
	SetEntityHealth( e, g_Entity[e].health + triggerHealth )

end

local rnd = math.random

local function processExplosion( e, ex, ey, ez, imploder )
	
	-- get entity position
	local Ent = g_Entity[e]
	
	-- calculate distance from centre of explosion
	local dx, dy, dz = Ent.x - ex, Ent.y - ey, Ent.z - ez
	local dist = math.sqrt( dx*dx + dz*dz )

	-- calculate modifier based on distance
	local distMod = 1 - ( dist / blastRadius )
	
	-- handle physics push/pull
	local dims = P.GetObjectDimensions( Ent.obj )
	local force = min( maxEntityPush, dims.m * maxEntityPush * 100 * ( 100 / GetEntityWeight( e ) ) ) * distMod
	local vx, vy, vz = dx / dist, dy / dist, dz / dist
	-- suck it in 
	if imploder then 
		vx, vy, vz = -vx, 0, -vz
	end
	
	if ai_bot_state[ e ] == nil then
	 -- shove it out the way
		PushObject( Ent.obj, vx * force, vy * force , vz * force, rnd(), rnd(), rnd() )
	end
	
	-- inflict damage on entity
	if explosion_name[e] ~= nil then
	    -- trigger chain reaction with like minded entities
		SetEntityHealth( e, triggerHealth )
		
	elseif
	   ai_bot_state[ e ] ~= nil then
		-- remove health from NPCs based on how close to explosion
		local newHealth = Ent.health - math.modf( maxDamage * distMod )
		if newHealth > 0 then
			if HurtEntity ~= nil then
				HurtEntity( e, newHealth, 200, Ent.x - ex, Ent.y - ey, Ent.z - ez, vx * force, vy * force , vz * force )
			else
				SetEntityHealth( e, newHealth )
			end
		else
			if KillEntity ~= nil then
				KillEntity( e, 200, vx * force, vy * force , vz * force )
			else
				SetEntityHealth( e, 0 )
			end
		end
	end
	
end


function graphix_explosion_main(e)
	-- if no sprite sheet do nothing
	if explosion_name[e] == nil then return end

	local Ent = g_Entity[e]

	if Ent.health > triggerHealth then return end
	-- put all those lovely images to use
	local emitter = emitterList[e]

	if emitter == nil then
		emitterList[e] = { used      = false,
		                   boomImg   = ParticlesLoadImage( image_List[ explosion_name[e] ].path, 
						                                  image_List[ explosion_name[e] ].imageNum ),
                           emitNam   = image_List[ explosion_name[e] ].emitName,
                           imploder  = image_List[ explosion_name[e] ].imploder,
						   imgFrames = image_List[ explosion_name[e] ].imgFrames
						 }
		return
	end
	-- only use an emiiter if need to
	if not emitter.used then 						
		emitter.blowup = PE_CreateNamedEmitter( emitter.emitNam, emitter.boomImg, emitter.imgFrames, e)								
		emitter.used = true
		ParticlesSetLife( emitter.blowup, 10, 2000, 2000, 0 , 0 )
		return
	end
		
	if emitter.timer == nil then
		ParticlesSetLife( emitter.blowup, 10, 2000, 2000, 1, 0 )
		emitter.timer = g_Time + 100
		
		-- 1.5 second delay to allow particle to fire
		StartTimer( e )
		-- play the sound assigned to sound0 in entity properties
		PlaySound( e, 0 )
		-- hide entity while we work out whats around it within range of blast
		Hide( e )	
			
		for _, v in pairs( U.ClosestEntities( blastRadius, 100, Ent.x, Ent.z) ) do
			if v ~= e then
				processExplosion( v, Ent.x, Ent.y, Ent.z, emitter.imploder )
			end
		end 
		
	elseif 
		-- run emitter effect
	   g_Time > emitter.timer then
		ParticlesSetLife( emitter.blowup, 10, 2000, 2000, 0 , 0 )
		emitter.timer = math.huge
		-- assign damage to player if in range of blast	
		if U.PlayerCloserThanPos( Ent.x, Ent.y, Ent.z, blastRadius ) then
			--AmenMoses funky math stuff i wont pretend to know what it is
			local dx, dz = g_PlayerPosX - Ent.x, g_PlayerPosZ - Ent.z
			local angle = math.deg( math.atan2( dx, dz ) )
			local dist = math.sqrt( dx*dx + dz*dz )
			
			local distMod = 1 - ( dist / blastRadius )

			local damage = maxDamage
			-- if not an explosion limit damage to player		
			if emitter.imploder then 
				angle = WrapAngle( -angle )
				damage = damage / 2
			end
			-- push the player away	
			ForcePlayer( angle, maxPlayerPush * distMod )
			-- damage to player based on how close to explosion		
			SetPlayerHealth( g_PlayerHealth - math.modf( damage * distMod ) )
		end
				
	else
		-- delete the emitter then destroy the entity
		if GetTimer( e ) > 1500 then
			ParticlesDeleteEmitter( emitter.blowup )
			Destroy( e )	
		end
	end
end