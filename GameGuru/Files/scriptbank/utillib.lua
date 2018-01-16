-- Utilities module for GameGuru
-- Written by Chris Stapleton

local U = {_G = _G}

-- import section: modules
local rad   = math.rad
local deg   = math.deg
local cos	= math.cos
local sin	= math.sin
local asin  = math.asin 
local abs	= math.abs
local atan  = math.atan2
local sqrt  = math.sqrt
local pi	= math.pi
local rand  = math.random
local sort  = table.sort
local pairs = pairs

-- GG functions/Tables
local ducked  = GetGamePlayerStatePlayerDucking
local gEnt    = g_Entity
local RayCast = IntersectAll

-- include these for debugging
local Prompt  = Prompt
local PromptE =PromptLocal

-- If using any of the functions generating random positions
-- make sure random number generator is seeded by putting the
-- following lines in your main script initialisation:
-- math.randomseed(os.time())
---math.random(); math.random(); math.random()

_ENV = U 

local function getPosX() return _G.g_PlayerPosX end
local function getPosZ() return _G.g_PlayerPosZ end
local function getPos2() return _G.g_PlayerPosX, _G.g_PlayerPosZ end
local function getPos3() return _G.g_PlayerPosX, _G.g_PlayerPosY, _G.g_PlayerPosZ end
local function getAngX() return _G.g_PlayerAngX end
local function getAngY() return _G.g_PlayerAngY end
local function getAng3() return _G.g_PlayerAngX, _G.g_PlayerAngY, _G.g_PlayerAngZ end
local function huge()    return _G.math.huge end
------------------------------------------------
-- Function to rotate a point by Euler angles --
-- Useful for positioning one entity or decal --
-- with respect to another.                   --
--                                            --
-- angles are in radians!                     --
--                                            --
-- example of use; say you have a head model  --
-- and a glowing eye decal.  Calculate the x, --
-- y,z offsets from the heads origin to the   --
-- eye position, pass these into the function --
-- along with the rotation angles of the head --
-- model and the returned values can then be  --
-- added to the head origin to give the       --
-- placement position for the decal.           --
------------------------------------------------
function U.Rotate3D( x, y, z, xrot, yrot, zrot ) 

	local function RotatePoint2D( x, y, Ang )  -- Ang in radians
		local Sa, Ca = sin( Ang ), cos( Ang )
		return x*Ca - y*Sa, x*Sa + y*Ca
	end
 
	local NX, NY, NZ = x, y, z

	-- X
	NZ, NY = RotatePoint2D( NZ, NY, -xrot )
	
	-- Y
	NX, NZ = RotatePoint2D( NX, NZ, -yrot )
	
	-- Z
	NY, NX = RotatePoint2D( NY, NX, -zrot )

	return NX, NY, NZ
end

--------------------------------------------------
-- Function to return Object in players eyeline --
--                                              --
-- Uses GG's ray casting command.               --
-- parameters:                                  --
--      dist : (optional) length of ray in game --
--                        units                 --
--      ignore : (optional) object id to ignore --
--                                              --
-- returns object id or 0 if none found         --                         
--------------------------------------------------
function U.ObjectPlayerLookingAt( dist, ignore )
	local dist   = dist or 2000
	local ignore = ignore or 0

	local pxp, pyp, pzp = getPos3()
	local pxa, pya, pza = getAng3()

	if ducked() == 1 then pyp = pyp + 10 else pyp = pyp + 31 end

	local rayX, rayY, rayZ = 
		Rotate3D ( 0, 0, dist, rad( pxa ), rad( pya ), rad( pza ) )

	return RayCast( pxp, pyp, pzp, pxp + rayX, pyp + rayY, pzp + rayZ, ignore )
end

------------------------------------------------------
-- Function to detect if players eyeline intercects --
-- a given object.                                  --
-- parameters:                                      --
--      obj : object id                             --
--      dist : (optional) length of ray in game     --
--                        units                     --
--      ignore : (optional) object id to ignore     --
--                                                  --
-- Returns true or false                            --
------------------------------------------------------
function U.PlayerLookingAtObj( obj, dist, ignore )		
	local vobj = ObjectPlayerLookingAt( dist, ignore )
	
	return vobj and vobj == obj
end

------------------------------------------------------
-- Function to detect if players eyeline intercects --
-- a given entity.                                  --
-- parameters:                                      --
--      e : entity id                               --
--      dist : (optional) length of ray in game     --
--                        units                     --
--      ignore : (optional) object id to ignore     --
--                                                  --
-- Returns true or false                            --
------------------------------------------------------
function U.PlayerLookingAt( e, dist, ignore )
	local Ent = gEnt[ e ]
	
	if Ent == nil then return false end
	
	return PlayerLookingAtObj( Ent.obj, dist, ignore )
end

------------------------------------------------------
-- Function to test whether two specified positions --
-- are within a given distance of each other        --
--                                                  --
-- Returns true or false                            --
------------------------------------------------------
function U.CloserThan( x1, y1, z1, x2, y2, z2, dist )
	local dx, dy, dz = x1 - x2, y1 - y2, z1 - z2
	return ( dx*dx+dy*dy+dz*dz ) < dist*dist
end

------------------------------------------------------------
-- Function to detect if player is looking in the general --
-- direction with a certain angle of a specified entity.  --
-- note: that entity might not actually be visible        --
-- parameters:                                            --
--      e : entity id                                     --
--      dist : (optional) distance to check, game units   --
--      fov : (optional) angle to check                   --
--                                                        --
-- Returns true or false                                  --
------------------------------------------------------------
function U.PlayerLookingNear( e, dist, fov )
	
	local Ent = gEnt[ e ]
	
	if Ent == nil then return false end

	local dist = dist or 2000
	local fov  = fov  or 20
	
	local pxp, pyp, pzp = getPos3()
	
	if not CloserThan( pxp, pyp, pzp, Ent.x, Ent.y, Ent.z, dist ) then return false end
	
	local function limitAngle ( Angle )
		while Angle < 0 do
			Angle = 360 + Angle
		end
		
		while Angle > 360 do
			Angle = Angle - 360
		end
		return Angle
	end

	local angle = limitAngle( atan( Ent.x - pxp, Ent.z - pzp ) * ( 180.0 / pi ) )
	
	local pAng  = limitAngle( getAngY() )

	local L = limitAngle( angle - fov / 2 )
	local R = limitAngle( angle + fov / 2 )
	
	if ( L < R and ( pAng > L and pAng < R ) ) or
	   ( L > R and ( pAng > L or  pAng < R ) ) then

		local PlayerVertAngle = getAngX()
		
		if PlayerVertAngle < 280 then 
			PlayerVertAngle = -PlayerVertAngle
		else
			PlayerVertAngle = 2 * ( 360 - PlayerVertAngle )
		end
		
		if ducked() == 1 then pyp = pyp + 10 else pyp = pyp + 31 end
		
		local dx, dy, dz = Ent.x - pxp, Ent.y - pyp, Ent.z - pzp
		
		local sqrDist = dx*dx + dy*dy + dz*dz

		local EntAng = deg( asin( dy / sqrt( sqrDist ) ) )
				
		return abs( EntAng - PlayerVertAngle ) < fov / 2
	
	else
		return false
	end
end

----------------------------------------------------
-- Function to test if player is within specified --
-- distance of a position.                        --
-- Returns true or false                          --
----------------------------------------------------
function U.PlayerCloserThanPos( x, y, z, dist )
	local pxp, pyp, pzp = getPos3()
	
	return CloserThan( pxp, pyp, pzp, x, y, z, dist )
end

-----------------------------------------------------
-- Function to test if player is within specified  --
-- distance of a given entity.                     --
-- Returns true or false                           --
-----------------------------------------------------
function U.PlayerCloserThan( e, dist )
	local Ent = gEnt[ e ]
	
	return Ent ~= nil and PlayerCloserThanPos( Ent.x, Ent.y, Ent.z, dist )
end

-------------------------------------------------------------
-- Function to return a random position within a specified --
-- distance of a given position.                           --
-- defaults to using player position                       --
-- returns x and z coordinates                             --
-------------------------------------------------------------
function U.RandomPos( dist, x, z )
	local dist = dist or 200
	local x = x or getPosX() 
	local z = z or getPosZ()
		
	local angle = rand() * 2 * pi

	return x + sin( angle ) * dist, z + cos( angle ) * dist
end		   

----------------------------------------------------------
-- Function to return a random position specified as an --
-- offset position wrt to a given entity.               --
-- offset is fraction of entity distance from the point --
-- specified.                                           --
-- e.g. 2 is half way, 3 a third etc = defaults to 2    --
-- returns x and z coordinates                          --   
----------------------------------------------------------
function U.RandomOffsetPos( e, dist, offset, x, z )
	local dist   = dist   or 200
	local offset = offset or 2
    local x = x or getPosX()
	local z = z or getPosZ()

	local Ent = gEnt[ e ]

	if Ent == nil then return 0, 0 end
	
	return RandomPos( x + ( Ent.x - x)/offset, z + (Ent.z - z)/offset, dist )
end

-- function returns squared distance between two points
local function sqrd( x1, z1, x2, z2 )
	local dx, dz = x1 - x2, z1 - z2
	return dx*dx+dz*dz
end

----------------------------------------------------
-- Function to return entity id of closest active --
-- entity to specified point withing a certain    --
-- distance.                                      --
-- Further tests would be needed to used to test  --
-- whether the entity is a soldier for example.   --
--                                                --
-- returns entity id or nil if none found         --
----------------------------------------------------
function U.ClosestEntToPos( x, z, dist )
	local dist   = dist or 2000
	local eClosest  = nil
	local sdClosest = nil 
	
	
	dist = dist * dist
	
	for k, v in pairs( gEnt ) do
		if eClosest == nil then
			local d = sqrd( x, z, v.x, v.z )
			if d < dist then
				eClosest = k
				sdClosest = sqrd( x, z, v.x, v.z )
			end
		else
			local d = sqrd( x, z, v.x, v.z )
			if d < dist and d < sdClosest then
				eClosest = k
				sdClosest = d
			end
		end
	end

	return eClosest
end

----------------------------------------------------
-- Function to return entity id of closest active --
-- entity to player within a specified distance.  --
--
-- returns entity id or nil if none found         --
----------------------------------------------------
function U.ClosestEntToPlayer( dist )
	local pxp, pzp = getPos2()
	
	return ClosestEntToPos( pxp, pzp )
end

-- iterates a list in order defined by sort function or by key order if none given
-- not really something you can use directly but see ClosestEntities function below
-- for example of use
function U.SortPairs( list, order_given )
    -- collect the keys
    local keys = {}
    for k in pairs(list) do keys[ #keys + 1 ] = k end

    if order_given then
        sort( keys, function( a, b ) return order_given( list, a, b) end )
    else
        sort( keys )
    end

    -- return the iterator function
    local i = 0
    return function()
        i = i + 1
        if keys[ i ] then
            return keys[ i ], list[ keys[ i ] ]
        end
    end
end

------------------------------------------------------------
-- Function to return a list of the entities in distance  --
-- order (i.e. closest first) from a specified point.     --
-- parameters:                                            --
--      dist : (optional) max distance to use             --
--      num : (optional) max number of entities to return --
--      x, z : (optional) position, defaults to player    --                                              --
--                                                        --
-- returns list of entity ids or empty list if none       --
-- note only adds entity to list if health > 0            --
------------------------------------------------------------
function U.ClosestEntities( dist, num, x, z )
	local dist = dist or huge()
	local num  = num or huge()
	local x = x or getPosX()
	local z = z or getPosZ()
	
	local entityList = {}
	
	-- first build a list of ranges of healthy entities within
	-- 'dist' from specified position or player
	for k, v in pairs( gEnt ) do
	    if v ~= nil and v.health > 0 then
			if CloserThan( v.x, v.y, v.z, x, v.y, z, dist ) then 
				entityList[ k ] = sqrd( x, z, v.x, v.z )
			end
		end
	end
	
	local sortedList = {}
	
	-- next sort and place in nbew list until 'num' entities cound
	for k, _ in SortPairs( entityList, function( list, a, b ) return list[ a ] < list[ b ] end ) do
		sortedList[ #sortedList + 1 ] = k
		if #sortedList == num then break end
	end
	
	return sortedList
end	
	
return U