-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- 
-- pickuppable script
-- ==================
--
-- Assign script to all items to be manipulated by the player
--
--------------------------
-- user editable values --
--------------------------
local maxWeight = 800	-- default maximum weight of pickuppable
                        -- or stack of pickuppables
local maxSize   = 400   -- default maximum size of pickuppable 
local minSpeed  = 0.05  -- speed of movement at max weight

local carryOfst = 0     -- x offset of carrying position \
local carryHght = 27    -- y offset of carrying position  | relative to player
local carryDist = 60    -- z offset of carrying position /
local levelAng  = 20    -- number of degrees off horizontal still considered 
                        -- as 'level' for stacking purposes
local handSize  = 15    -- size of hand icon (screen %)

local throwEnabled = false   
local pushEnabled  = false 

--------------------------

local U = require "scriptbank\\utillib"
local Q = require "scriptbank\\quatlib"
local P = require "scriptbank\\physlib"

local modf = math.modf
local deg  = math.deg
local rad  = math.rad
local max  = math.max
local min  = math.min
local abs  = math.abs
local sin  = math.sin
local cos  = math.cos
local atan = math.atan2
local sqrt = math.sqrt
local log  = math.log
local rnd  = math.random

-- return a copy of a table/list (move to utils when tested)
local function deepcopy( orig )
    local orig_type = type( orig )
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[ deepcopy( orig_key ) ] = deepcopy( orig_value )
        end
        setmetatable( copy, deepcopy( getmetatable( orig ) ) )
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

-- ============================== --
-- global variables for save/load --
-- ============================== --
g_pickuppables_list = g_pickuppables_list or {}

-- fill g_pickuppables_list with saved version for situation
-- where saving at a checkpoint
if g_pickuppables_list == {} and 
   g_pickuppables_snapshot ~= nil then

	g_pickuppables_list = deepcopy( g_pickuppables_snapshot )
end
-- ============================== --

-- control lists
local dimensions = {}
local names = {}
local pEnt = {}

local carryingEnt = nil   -- 'pickuppable' actually in hand (i.e. bottom of any 'Stack'
local droppedEnt  = nil   -- 'pickuppable' last to be in hand (i.e. last to be dropped)
local promptsOn   = true  -- whether to show Prompts to player or not
local thrownObj   = nil   -- details of thrown objects for collision purposes

-- ================================= --
-- start of global functions section --
-- ================================= --

function PU_EnableThrowing()
	throwEnabled = true
	g_pickuppables_list.throwEnabled = throwEnabled
end

function PU_CanThrow() return throwEnabled end

function PU_DisableThrowing()
	throwEnabled = false
	g_pickuppables_list.throwEnabled = throwEnabled
end

function PU_EnablePushPull()
	pushEnabled = true
	g_pickuppables_list.pushEnabled = pushEnabled
end

function PU_CanPush() return pushEnabled end

function PU_DisablePushPull()
	pushEnabled = false
	g_pickuppables_list.pushEnabled = pushEnabled
end

-- global functions to read/set max weight we can lift
function PU_GetMaxCarryWeight()
	return maxWeight
end

function PU_SetMaxCarryWeight( value )
	maxWeight = value
	g_pickuppables_list.maxWeight = maxWeight
end

-- global functions to read/set max size of item we can lift
function PU_GetMaxCarrySize()
	return maxSize
end

function PU_SetMaxCarrySize( value )
	maxSize = value
	g_pickuppables_list.maxSize = maxSize
end

function PU_DisablePrompts()
	promptsOn = false
end

-- stops entity specified from being 'pickuppable'
function PU_RemoveEntity( e )
	local remList = g_pickuppables_list.removed
	
	if remList == nil then
		g_pickuppables_list.removed = {}
		remList = g_pickuppables_list.removed
	end

	remList[ e ] = true

	dimensions[ e ] = nil
	
	pEnt[ e ] = nil
end
	
-- returns nil or entity id of item being carried
function PU_GetEntityCarried()
	return carryingEnt
end

-- returns nil or entity id of last dropped item
function PU_LastDropped()
	return droppedEnt
end

-- returns empty list or list of all carried items
function PU_GetCarriedList()
	local tempList = {}
	
	if carryingEnt == nil then return tempList end

	templist[ #templist + 1 ] = carryingEnt
	
	local function getStackItems( p )

		if p.stackList == nil then return end
		
		for k, v in pairs( p.stackItems ) do
			templist[ #templist + 1 ] = k
			getStackItems( pEnt[ k ] )
		end
	end
	
	getStackItems( pEnt[ carryingEnt ] )
	
	return tempList
end

-- returns a list of carried items by name
function PU_CarriedNames()
	local tempList = {}
	
	for _, v in pairs( PU_GetCarriedList() ) do
		templist[ #templist + 1 ] = names[ v ]
	end
	
	return templist
end

-- creates a copy of the current state for save/load purposes
function PU_SnapshotGlobalList()
	g_pickuppables_snapshot = deepcopy( g_pickuppables_list )
end

-- End of global functions --
-- ======================= --

local mouse1Clicked = false
local mouse2Clicked = false
local mouseTimer    = math.huge

-- sprite 'pointers'
local Sprites = {}
local handShowing = 'none'

local function HideHand( hand )
	SetSpritePosition( Sprites[ hand ],  200, 200)
	handShowing = 'none'
end

local function ShowHand( hand )
	SetSpritePosition( Sprites[ hand ],   50,  50)
	handShowing = hand
end

local function stackWeight( p )
	local totWeight = 0
	
	if p.stackList ~= nil then
		for k, p in pairs( p.stackList ) do
			totWeight = totWeight + ( dimensions[ p.obj ].m or 0 ) + stackWeight( pEnt[ k ] )
		end
	end
	return totWeight
end

local function ObjectPickuppable( obj )
	if obj == nil or obj == 0 then return false end 
	
	for _,v in pairs( pEnt ) do
		if v.obj == obj then return true end
	end
	return false
end

local function getDims( obj )
	dimensions[ obj ] = P.GetObjectDimensions( obj )						
end

-------------------------------------------
-- Init routine - called once per entity --
-- at spawn time                         --
-------------------------------------------
function pickupsimple_init_name( e, name )
	-- These includes ensure that the 'make standalone' process
	-- copies the libararies to the standalone folder
	Include( "utillib.lua" )
	Include( "quatlib.lua" )
	Include( "physlib.lua" )

	-- save name to local list
	names[ e ] = name
	
	-- null the list entries for this entity so that when 
	-- main routine called for the first time they will be
	-- correctly initialised
	dimensions[ g_Entity[e].obj ] = nil
	pEnt[ e ] = nil
	
	-- load sprites if not already loaded
	if Sprites[ 'hand1' ] == nil then 
		Sprites[ 'hand1' ] = CreateSprite(LoadImage ( "scriptbank\\pickuppable\\hand1.png" ) )
		Sprites[ 'hand2' ] = CreateSprite(LoadImage ( "scriptbank\\pickuppable\\hand2.png" ) )
	
		local s = Sprites[ 'hand1' ]
		SetSpriteOffset( s, -1 , handSize / 2 )
		SetSpriteSize  ( s, -1 , handSize )
		SetSpriteDepth ( s, 0 )
		HideHand( 'hand1' )
		
		s = Sprites[ 'hand2' ]
		SetSpriteOffset( s, -1 , handSize / 2 )
		SetSpriteSize  ( s, -1 , handSize )
		SetSpriteDepth ( s, 0 )
		HideHand( 'hand2' )
	end
	
	-- if there is global maximum weight/size use it to initialise the
	-- local copy
	if g_pickuppables_list.maxWeight ~= nil then
		maxWeight = g_pickuppables_list.maxWeight
	end
	if g_pickuppables_list.maxSize ~= nil then
		maxSize = g_pickuppables_list.maxSize
	end
	if g_pickuppables_list.pushEnabled ~= nil then
		pushEnabled = g_pickuppables_list.pushEnabled
	end
	if g_pickuppables_list.throwEnabled ~= nil then
		throwEnabled = g_pickuppables_list.throwEnabled
	end
end

local function getRealWorldYangle( xa, ya, za )  -- Euler in degrees, returns radians 
	-- Tricky to explain but basically g_PlayerAngY is the real-world
	-- angle, i.e. 0-359 degrees, whereas the Euler angle for an entity
    -- returned by GetObjectPosAng is not.
	-- In order to get the 'real-world' equivalent we need to do some math.

	-- first rotate a unit 'facing forward' vector by the Euler angles
	local xv, _, zv = U.Rotate3D( 0, 0, 1,  rad( xa ), rad( ya ), rad ( za ) )
	
	-- now work out the angle from the x and z components of the result
	return atan( xv, zv )
end	

local function isLevelIsh( xa, ya, za )  -- Euler in degrees, returns true/false
	local maxAng =  180 - levelAng
	local minAng = -180 + levelAng
	
	return ( xa < minAng or xa > maxAng or 
	         ( xa < levelAng and xa > -levelAng ) ) and
		   ( za < minAng or za > maxAng or 
	         ( za < levelAng and za > -levelAng ) ) 
end


local RayCast = IntersectAll

-- given angles and origin position return centre position
local function getCentre( p, x, y, z, xa, ya, za )
	-- first rotate offsets 
	local dims = dimensions[ p.obj ]
	local xo, yo, zo = U.Rotate3D( dims.cx, dims.cy, dims.cz, xa, ya, za )

	-- now calculate centre of object
	return x + xo, y + yo, z + zo
end

local function checkIfSatOn( pb, bpx, bpy, bpz, bax, bay, baz,
                             pt, tpx, tpy, tpz, tax, tay, taz )
		
	-- can't be stacked if its origin is lower than the base
	-- objects origin no matter what the relative orientations are
	if tpy < bpy then return false end
	
	-- send a ray trace straight down from the centre of the top object

	-- first calculate centre of object
	local tcx, tcy, tcz = getCentre( pt, tpx, tpy, tpz, tax, tay, taz )
	
	local lowestY = tcy - ( dimensions[ pt.obj ].h / 2 + 5 )
	
	-- now cast rays from around centre point to try and find the base object
	local bObj = RayCast( tcx, tcy, tcz, tcx, lowestY , tcz, pt.obj )
	
	if bObj ~= pb.obj then
		bObj = RayCast( tcx + 1, tcy, tcz + 1, tcx + 1, lowestY, tcz + 1, pt.obj )
	end
	if bObj ~= pb.obj then
		bObj = RayCast( tcx + 1, tcy, tcz - 1, tcx + 1, lowestY, tcz - 1, pt.obj )
	end
	if bObj ~= pb.obj then
		bObj = RayCast( tcx - 1, tcy, tcz + 1, tcx + 1, lowestY, tcz + 1, pt.obj )
	end
	if bObj ~= pb.obj then
		bObj = RayCast( tcx - 1, tcy, tcz - 1, tcx + 1, lowestY, tcz - 1, pt.obj )
	end
	
	if bObj ~= pb.obj then return false end
	
	local q1 = Q.Conjugate( Q.FromEuler( bax, bay, baz ) )

	local xo, yo, zo = U.Rotate3D( tpx - bpx, tpy - bpy, tpz - bpz, Q.ToEuler( q1 ) )
	
	return true, xo, yo, zo, Q.Mul( q1 , Q.FromEuler( tax, tay, taz ) )
end

local function buildStackList( p, base_obj )
	-- basically we have to find all pickuppable objects
	-- that are sitting on this one and bung them in a 
	-- separate list, storing their positions relative to
	-- the base entity and rotations relative to it
	-- we have to do this before we first move the object
	-- so we can also move all the others at the same time
	local stacked = "No"
	
	-- first check if object is roughly level
	local pbx, pby, pbz, abx, aby, abz = GetObjectPosAng( p.obj )

	-- for now only allow stacking on 'level(ish)' object
	if p.obj == base_obj and not isLevelIsh( abx, aby, abz ) then 
		p.stackList = nil
		return 
	end
	
	-- roughly level so lets find out who is sat on us; 
	-- start by creating an empty list, we can delete
	-- it if we don't find any 'stackable' items
	p.stackList = {}
	
	for k, v in pairs( pEnt ) do
		-- don't process ourselves!
		if v ~= p then
			-- only process entities within range
			local tpx, tpy, tpz, tax, tay, taz = GetObjectPosAng( v.obj )
			
			if U.CloserThan( tpx, tpy, tpz, pbx, pby, pbz, maxSize * 2 ) then
			
				local isOn, ox, oy, oz, q = checkIfSatOn( p, pbx, pby, pbz, rad(abx), rad(aby), rad(abz),
                                                          v, tpx, tpy, tpz, rad(tax), rad(tay), rad(taz), 
													      dimensions[ k ] );
				if isOn then 
					-- offset values are from the centres not origin!
					p.stackList[ k ] = { obj = v.obj, x = ox, y = oy, z = oz, q = q }
					stacked = "Yes"				
				end
			end
		end
	end	
	
	if stacked == "Yes" then
		for k, v in pairs( p.stackList ) do
			-- now recurse
			buildStackList( pEnt[ k ] )
		end
	else -- didn't find any so delete the stack 
		p.stackList = nil
	end
end

local function processRotation( e, p, plQ )

	local yaw   = -( g_MouseX - 50 ) / 1000
	local pitch = -( g_MouseY - 50 ) / 1000

	if abs( yaw )   < 0.01 then yaw   = 0 end
	
	if p.stackList ~= nil or abs( pitch ) < 0.01 then pitch = 0 end 
			
	if promptsOn then PromptLocalForVR( e, "Use mouse to rotate " .. names[ e ], 3 ) end		
			
	if roty ~= 0 or rotx ~= 0 then
	    local quat = Q.FromEuler( pitch, yaw, 0 )
		
		p.quat = Q.Mul( quat, p.quat )
	end
end

local function clearStacks( p )
	if p.stackList ~= nil then
		for k, _ in pairs( p.stackList ) do
			clearStacks( pEnt[ k ] )
			pEnt[ k ].stackList = nil
		end
	end
end

local topSpeed = 0
local dropTimer = math.huge

local function dropObject( p, e )
	droppedEnt = e
	
	mouse1Clicked = true
	carryingEnt  = nil
	mouseTimer   = g_Time + 200

	-- reset quat ready for next time we pick this entity up
	-- stacking will use its own calculations
	p.quat = nil
				
	-- clear stackList(s)
	clearStacks( p )
	
	p.stackList = nil
	
	SetGamePlayerControlTopspeed( topSpeed )
	
	dropTimer = g_Time + 1000
end

local corners = {}

local function adjustForTerrainCollision( nx, ny, nz, dims, ax, ay, az )
	-- We need to work out the coordinates of each 'corner' and check if
	-- any of them would be under the terrain, if so we need to raise y
	-- by the difference plus a little bit
	-- ( could be expanded to add a raycast from corner to corner to check
    --	 for a terrain collision, but that's difficult ;-) )
	-- TODO - replace this with the new terrain collision detection commands
	--        at some point

	local hw, hh, hl = dims.w / 2, dims.h / 2, dims.l / 2
	
	-- rotate offsets 
	local ofx, ofy, ofz = U.Rotate3D( dims.cxs, dims.cys,dims.czs, ax, ay, az );
	
	-- create our own collision box
	corners = { { xo =  hw, yo =  hh, zo =  hl },
	            { xo =  hw, yo =  hh, zo = -hl },
			    { xo =  hw, yo = -hh, zo =  hl },
				{ xo =  hw, yo = -hh, zo = -hl },
				{ xo = -hw, yo =  hh, zo =  hl },
				{ xo = -hw, yo =  hh, zo = -hl },
				{ xo = -hw, yo = -hh, zo =  hl },
				{ xo = -hw, yo = -hh, zo = -hl }
			  };

	local heightDiff = 0
	
	-- now check all the corners to see if any are below
	-- the terrain
	for k, v in pairs( corners ) do
		-- rotate offsets to entity angle
		local ox, oy, oz = U.Rotate3D( v.xo, v.yo, v.zo, ax, ay, az );
									   
		local th = GetTerrainHeight( nx + ox + ofx , nz + oz + ofz )

		local yh = ny + oy + ofy
		
		if th > yh and th - yh > heightDiff then 
			heightDiff = th - yh
		end
	end
	
	return ny + heightDiff
end

local function rePositionEnt( e, obj, x, y, z, ax, ay, az )		

	-- Gentler movement of entity
	GravityOff(e)
	SetPosition ( e, x, y, z )
	GravityOn(e)

	--This force causes objects to go inside other entities
	--CollisionOff( e )
	--PositionObject( obj , x, y, z )
	--RotateObject( obj, deg( ax ), deg( ay ), deg( az ) )
	--CollisionOn( e )
end

local function playerIsOn( p, ppx, ppy, ppz )
	return p.obj == RayCast( ppx, ppy, ppz, ppx, ppy - 200, ppz, 0 )
end

local function repositionStack( p, nquat, nx, ny, nz, ppx, ppy, ppz )

	if p.stackList ~= nil then
		
		if ppy > nx + 20 or playerIsOn( p, ppx, ppy, ppz ) then
			dropObject( pEnt[ carryingEnt ], carryingEnt )
			return
		end
		-- quaternion of base item 
		local xa, ya, za = Q.ToEuler( nquat )

		
		for k, v in pairs( p.stackList ) do
			
			-- calculate relative rotation
			local pquat = Q.Mul( nquat, v.q )
						
			-- rotate centre offsets to match new angle
			local xo, yo, zo = U.Rotate3D( v.x, v.y, v.z, xa, ya, za )

			local px, py, pz = nx + xo, ny + yo, nz + zo 
			
			local nxa, nya, nza = Q.ToEuler( pquat )
			
			rePositionEnt( k, v.obj, px, py, pz, nxa, nya, nza )

			repositionStack( pEnt[ k ], pquat, px, py, pz, ppx, ppy, ppz )
			
			if carryingEnt == nil then return end
		end
	end
end

local SEPTT_timer = 0

local function stopEntsPunchingThroughTerrain()
	-- or more accurately rescue them when they have been punched through
	if g_Time > SEPTT_timer then
		-- check 5 times a second
		SEPTT_timer = g_Time + 200
		
		for k, v in pairs( pEnt ) do
			
			if k ~= carryingEnt then
				
				local  x, y, z, xa, ya, za = GetObjectPosAng( v.obj )
				
				-- check if the centre of the object is below terrain height
				local cx, cy, za = getCentre( v, x, y, z, xa, ya, za )
				
				local th = GetTerrainHeight( x, z )
				
				if cy < th - 30 then
					ya = getRealWorldYangle( xa, ya, za )
					rePositionEnt( k, v.obj, x, th + 1, z, 0, ya, 0 )
				end
			end
		end
	end
end

local numFrames = 7

local function positionHandler( e, p, rq, ppx, ppy, ppz )

	-- So we need this to run every frame and do the following:
	if playerIsOn( p, ppx, ppy, ppz ) then
		dropObject( p, e )
		return
	end
	
	-- calculate current object position and rotation : cp, cq
	local cpx, cpy, cpz, cax, cay, caz = GetObjectPosAng( p.obj )
	
	local cq = Q.FromEuler( rad( cax ), rad( cay ), rad( caz ) )
	
	-- calculate difference between cp and required position : rp
	local xtg, ytg, ztg = p.rx - cpx, p.ry - cpy, p.rz - cpz  
	
	-- if difference greater that some amount work out how far we need 
	-- to move and calculate a per frame vector to apply which will depend
	-- on maximum movement allowable
	
	local dtg = xtg*xtg + ytg*ytg + ztg*ztg 

	if dtg < 1 then 
		rePositionEnt( e, p.obj, p.rx, p.ry, p.rz, Q.ToEuler( rq ) )

		repositionStack( p, rq, p.rx, p.ry, p.rz, ppx, ppy, ppz )
	else
		
		xtg, ytg, ztg = xtg / numFrames, ytg / numFrames, ztg / numFrames
	
		-- calculate per frame rotation (SLERP) : sq (same rules as for position) 
		local sq = Q.NLerp( cq, rq, 1 / numFrames ) 
		
		local nx, ny, nz = cpx + xtg, cpy + ytg, cpz + ztg
		
		rePositionEnt( e, p.obj, nx, ny, nz, Q.ToEuler( sq ) )

		-- now position stacked ents
		repositionStack( p, sq, nx, ny, nz, ppx, ppy, ppz )
	end
end

local RayCast = IntersectAll
 
local throwTimer = 0

local hitObject = {}

local function CheckThrowCollision()

	if throwTimer == 0 then throwTimer = g_Time + 1000 end
	
	local colList = P.GetObjectCollisionDetails( thrownObj )
	
	if colList ~= nil then
		for i, v in ipairs( colList ) do
			if v.obj ~= 0 then
				local force = v.f * 1000
				local e = P.ObjectToEntity( v.obj )
				local Ent = g_Entity[ e ]
				if Ent ~= nil then
					SetEntityHealth( e, g_Entity[ e ].health - force )
				end
				if PU_ShowDecal ~= nil then
					PU_ShowDecal( "PU Decal", v.x, v.y, v.z, force * 5 )
				end
			end
		end
	
		-- remove health from thrown object causing it to
		-- explode ( if explodable )
		SetEntityHealth( P.ObjectToEntity( thrownObj ), 0)
		
	elseif g_Time < throwTimer then
		-- didn't hit anything yet so give it more time
		return
	end
	
	-- done with this object so remove from checking list 
	RemoveObjectCollisionCheck( thrownObj )
	thrownObj = nil
	throwTimer = 0	
end

local function HandleTerrainCollisions()

	local colList = P.GetTerrainCollisionDetails( thrownObj )
	
	if colList == nil then return end
	
	for _, v in ipairs( colList ) do
		if v.x ~= 0 then
			if PU_ShowDecal ~= nil then
				PU_ShowDecal( "PU Decal2", v.x, v.y, v.z )
			end
		end
	end
end

local currSpeed     = 0
local colCheckTimer = 0 

-----------------------------------------------------
-- Start of main entry routine - called very frame --
-----------------------------------------------------
function pickupsimple_main(e)

	if throwEnabled and P.ObjectToEntity( thrownObj ) == e then
		if g_Time > colCheckTimer then
			HandleTerrainCollisions()
			CheckThrowCollision()
			colCheckTimer = g_Time + 100 -- 1/10 second-
		end
	end
	
	if g_pickuppables_list.removed ~= nil and
	   g_pickuppables_list.removed[ e ] then 
	   return
	end
	     
	local Ent = g_Entity[ e ]
	
	if Ent == nil then return end
		
	-- first time though calculate dimensions of entity
	local dims = dimensions[ Ent.obj ]
	
	if dims == nil then
		getDims( Ent.obj )

		return
	end
								
	local p = pEnt[ e ]
	
	if p == nil then
		-- store the object id for future use
		pEnt[ e ] = { obj = Ent.obj }
		return
	end
	
	if carryingEnt == e or droppedEnt == e then 
		stopEntsPunchingThroughTerrain()

		-- delayed re-enabling of jump mode
		if g_Time > dropTimer then
			SetGamePlayerControlJumpMode( 0 )
			dropTimer = math.huge
		end
	end
	
	local pObj, objX, objY, objZ = 0, 0, 0, 0

	if carryingEnt == nil then
		if not U.PlayerCloserThan( e, 200 ) then return end
		
		pObj, objX, objY, objZ = P.ObjectPlayerLookingAt( 100 )
		
		if pObj == p.obj then 
					
			if handShowing == 'none' then ShowHand( 'hand1' ) end
				
		elseif not ObjectPickuppable( pObj ) then
			if handShowing ~= 'none' then 
				HideHand( handShowing )
			end
			return
		else
			return
		end
		
	elseif carryingEnt == e then	
	
		if g_PlrKeySPACE == 1 then
			SetGamePlayerControlTopspeed( minSpeed )
		else
			SetGamePlayerControlTopspeed( currSpeed )
		end
		
		-- get player position and angles
		local ppX, ppY, ppZ = g_PlayerPosX, g_PlayerPosY, g_PlayerPosZ
		local paX, paY, paZ = rad( g_PlayerAngX ), 
			                  rad( g_PlayerAngY ),
						      rad( g_PlayerAngZ );
							  
		-- player quaternion
		local plQ = Q.FromEuler( 0, paY, paZ )
				
		-- we are carrying this entity
		if handShowing ~= 'none' then 
			HideHand( handShowing )
		end
		if promptsOn then 
			PromptLocalForVR( e, "Can drop the object", 3 )
		end
		if g_MouseClick == 1 or GetPlayerWeaponID() ~= 0 then
			if not mouse1Clicked then

				dropObject( p, e )
				
				if not throwEnabled then return end
				
				-- handle throwing
				if g_KeyPressE == 1 and g_MouseClick == 1 then 
				    local force = min( maxWeight / 6, dims.m )
					
					vx, vy, vz = U.Rotate3D ( 0, 0, 1, paX, paY, paZ )
					
					if names[ e ] == 'saw blade' then 
					    force = force * 20
						PushObject( p.obj, vx * force, vy * force, vz * force, 
										  rnd()/10 , 0, rnd()/10 )
					else
						PushObject( p.obj, vx * force, vy * force, vz * force, 
										   rnd()/100, rnd()/100, rnd()/100 )
					end
					
					-- test new collision testing command
					thrownObj = p.obj
					
					AddObjectCollisionCheck( p.obj )

					-- do initial check in case enemy really close!
					CheckThrowCollision()
				end
				return
			end

		elseif g_MouseClick == 2 then
			if not mouse2Clicked then
				mouse2Clicked = true
				ActivateMouse()
			end
			
			--processRotation( e, p, plQ )
			
		elseif mouse2Clicked or (mouse1Clicked and g_Time > mouseTimer) then
			DeactivateMouse()
			mouse1Clicked = false
			mouse2Clicked = false
			mouseTimer    = math.huge
		end	
				
		-- move entity with player
		-- (may need to handle collision as entity will probably be
		--  outside player capsule) 
		if p.quat == nil then
					
			-- we've just picked it up so initialise:
			-- work out offset position of entity when held by player
			local size = max( dims.w, dims.h, dims.l )
			
			-- need to ensure entity is offset far enough to rotate
			p.xo, p.yo, p.zo = carryOfst, carryHght, carryDist + size / 2

			-- get entity angles
			local _,_,_, eaX, eaY, eaZ = GetObjectPosAng( p.obj )
			
			-- need to work out real world angle of entity
			rwY = getRealWorldYangle( eaX, eaY, eaZ )
			
			-- store angle as quaternion
			p.quat = Q.FromEuler( 0, rwY - paY, 0 ) 
		end
			

		-- work out offset position of entity when held by player
		-- first taking account of player angles
		local xo, yo, zo = U.Rotate3D( p.xo, p.yo, p.zo, paX, paY, paZ )

		-- work out rotation quaternion of entity
		local enQ = Q.Mul( plQ, p.quat )
		
		-- then object offsets, i.e to centre it properly
		local oAx, oAy, oAz = Q.ToEuler( enQ )

		local oxo, oyo, ozo = U.Rotate3D( dims.cx, dims.cy, dims.cz, oAx, oAy, oAz )

		-- calculate new position for entity
		p.rx, p.ry, p.rz = ppX + xo - oxo, ppY + yo - oyo, ppZ + zo - ozo 

		-- make sure we don't sink the entity into the terrain	
		p.ry = adjustForTerrainCollision( p.rx, p.ry, p.rz, dims, oAx, oAy, oAz )

		positionHandler( e, p, enQ, ppX, ppY, ppZ ) 
		
		return
	else
		-- something being carried and it isn't us so leave
		return
	end
	
	if GetPlayerWeaponID() ~= 0 then
		if promptsOn then PromptLocalForVR( e, "Can't manipulate without your hands free!", 3 ) end
		if handShowing == 'hand1' then
			HideHand( handShowing )
			ShowHand( 'hand2' )
		end
		mouseTimer = g_Time + 200
		return
	end
	
	-- if we've got here we can potentially pick up the item
	-- check how heavy it is
	if max( dims.w, dims.h, dims.l ) > maxSize then
		if promptsOn then PromptLocalForVR( e, "Too large to carry!", 3 ) end
		if handShowing == 'hand1' then
			HideHand( handShowing )
			ShowHand( 'hand2' )
		end

	elseif dims.m > maxWeight then
		if promptsOn then PromptLocalForVR( e, "Too heavy to lift!", 3 ) end
		if handShowing == 'hand1' then
			HideHand( handShowing )
			ShowHand( 'hand2' )
		end
		
	else
		if handShowing == 'hand2' and 
		   g_Time > mouseTimer then
			HideHand( handShowing )
			ShowHand( 'hand1' )
		end
		
		if mouse1Clicked then
			if g_Time > mouseTimer then
				mouse1Clicked = false
				mouseTimer = math.huge
			end
			return
		end
		
		if promptsOn then
			PromptLocalForVR( e, "Can pick up object", 3 )
		end
		local pushX, pushY, pushZ = P.GetViewVector()
		-- PromptLocal( e, pushX .. "," .. pushY .. "," .. pushZ .. " : " ..
		--                objX  .. "," .. objY  .. "," .. objZ )
		if g_MouseClick == 1 then
			if pushEnabled then
				-- handle pushing
				if g_KeyPressE == 1 or g_KeyPressR == 1 then
					-- local pushX, pushY, pushZ = P.GetViewVector()
				
					-- get object position and angle
					local ex, ey, ez, xa, ya, za = GetObjectPosAng( p.obj )
			
					-- work out centre of object
					local ox, oy, oz = U.Rotate3D( dims.cx, dims.cy, dims.cz, xa, ya, za );
					local cx, cy, cz = ex + ox, ey + oy, ez + oz
			
					-- finally work out contact point offsets from centre
					ox, oy, oz = objX - cx, objY - cy, objZ - cz
			
					-- push force less than throw force
					local force
					if g_KeyPressE == 1 then
						force = min( maxWeight / 20, dims.m / 4 )
				
						-- and push!
						PushObject( p.obj, pushX*force, pushY*force, pushZ*force, ox, oy, oz )
					else
						-- pull force less than push force
						local force = min( maxWeight / 60, dims.m / 6 )

						-- and pull!
						PushObject( p.obj, -pushX*force, -pushY*force, -pushZ*force, ox, oy, oz )
					end
					return
				end
			end
			
			mouse1Clicked = true
			
			-- build stack list
			buildStackList( p, p.obj )

			-- check how heavy the stack is
			local currWeight = stackWeight( p ) + dims.m
			
			if currWeight > maxWeight then
				if promptsOn then 
					PromptLocalForVR( e, "Stack is too heavy!", 3 )
				end
				if handShowing == 'hand1' then
					HideHand( handShowing )
					ShowHand( 'hand2' )
				end

				mouseTimer = g_Time + 2000
			else
				carryingEnt   = e
				
				mouseTimer    = g_Time + 200
				
				topSpeed = GetGamePlayerControlTopspeed()
				
				currSpeed = minSpeed + ( ( maxWeight - currWeight ) / 
				                           ----------------------
				                                 maxWeight ) * ( topSpeed - minSpeed )
												 
				SetGamePlayerControlTopspeed( currSpeed )
				SetGamePlayerControlJumpMode( 3 )
			end
		end
	end
end