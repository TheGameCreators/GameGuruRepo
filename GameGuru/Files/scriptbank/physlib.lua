-- Physics module for GameGuru
-- Written by Chris Stapleton
local U = require "scriptbank\\utillib"
local Q = require "scriptbank\\quatlib"

local P = { _G = _G }

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
local getObjExists = GetObjectExist
local getColBox    = GetObjectColBox
local getScales    = GetObjectScales
local getCentre    = GetObjectCentre
local getWeight    = GetEntityWeight
local addColCheck  = AddObjectCollisionCheck
local delColCheck  = RemoveObjectCollisionCheck
local getTerColls  = GetTerrainNumCollisions
local getTerDetail = GetTerrainCollisionDetails
local getObjColls  = GetObjectNumCollisions
local getObjDetail = GetObjectCollisionDetails
local createHingeS = CreateSingleHinge
local createHingeD = CreateDoubleHinge
local createJointS = CreateSingleJoint
local createJointD = CreateDoubleJoint
local remObjConsts = RemoveObjectConstraints
local getObjPosAng = GetObjectPosAng
local ducked       = GetGamePlayerStatePlayerDucking
local physRayCast  = PhysicsRayCast
local setDamping   = SetObjectDamping

-- include these for debugging
local Prompt  = Prompt
local PromptE = PromptLocal

local gEnt = g_Entity

_ENV = P 

local function getPos3() return _G.g_PlayerPosX, _G.g_PlayerPosY, _G.g_PlayerPosZ end
local function getAng3() return _G.g_PlayerAngX, _G.g_PlayerAngY, _G.g_PlayerAngZ end

local initialised = false
local objEntity   = {}  

function P.Initialise()
	-- setup object Id to entity Id list
	for k, v in pairs( gEnt ) do
		if v.obj ~= 0 then objEntity[ v.obj ] = k end
	end
	
	initialised = true
end

local function objExists( obj ) return getObjExists( obj ) == 1 end

-----------------------------------------------
-- Function to get entity Id from object Id  --
-----------------------------------------------
function P.ObjectToEntity( obj )
	if not initialised then Initialise() end
	return objEntity[ obj ]
end

-------------------------------------------------
-- Function to get object dimensions.          --
-- returns { w, h, l, m, cx, cy, cz )          --
-- w = width  ('x' dimension of collision box) --
-- h = height ('y' dimension of collision box) --
-- l = length ('z' dimension of collision box) --
-- m = physics mass of object                  --
-- cx, cy, cz = offsets of origin from centre  --
-- (unscaled for repositioning)                --
-- cxs, cys, czs = offsets from centre scaled  --
-- (use for calculating physics shape position --
-------------------------------------------------
function P.GetObjectDimensions( obj )
	if not objExists( obj ) then return end
	
	local xmin, ymin, zmin, xmax, ymax, zmax = getColBox( obj ) 
	
	-- get scale factors for object
	local sx, sy, sz = getScales( obj )
	
	-- now work out width, height, length and mass
	local w, h, l = (xmax - xmin) * sx, (ymax - ymin) * sy, (zmax - zmin) * sz
	
	-- and get offsets from centre (note these are not scaled)
	local cx, cy, cz = getCentre( obj )
	
	-- get entity 'weight' (actually a mass modifier value!)
	local massmod = getWeight( ObjectToEntity( obj ) ) / 100
	
	return { w = w, h = h, l = l, 
			 -- mass calculation based on GG internal calculation
			 -- if Physics re-write changes that then this will need 
			 -- to be altered to match (assumes collision box)
			 -- (note: actual mass capped at 400)
			 m = ( w * h * l ) / 50 * massmod,
			 cx = cx, cy = cy, cz = cz,
			 cxs = cx * sx, cys = cy * sy, czs = cz * sz }		
end

----------------------------------------------------------
-- Function to Add an entity to the collision checking  --
-- system.  Doing this allows the collisions between    --
-- this entity and other objects/terrain to be queried  --
-- and acted upon by Lua scripts.                       --                         
----------------------------------------------------------
function P.AddEntityCollisionCheck( e )
	if e == nil then return end
	addColCheck( gEnt[ e ].obj )
end

----------------------------------------------------------
-- Function to Remove an entity from the collision      --
-- checking system when no longer interested in it,     --
-- e.g. because it no longer exists.                    --                         
----------------------------------------------------------
function P.RemoveEntityCollisionCheck( e )
	if e == nil then return end
	delColCheck( gEnt[ e ].obj )
end

----------------------------------------------------------
-- Function to get the number of terrain collisions     --
-- stored by the collision checking system for a        --
-- specified entity.                                    --
----------------------------------------------------------
function P.GetEntityTerrainNumCollisions( e )
	if e == nil then return 0 end
	return getTerColls( gEnt[ e ].obj )
end

----------------------------------------------------------
-- Functions to get the details of terrain collisions   --
-- from the collision checking system for a specified   --
-- object/entity.                                       --
-- Returns nil if none detected                         --
-- Returns a table containing the world location of the --
-- collisions detected in order oldest first            --
----------------------------------------------------------
function P.GetTerrainCollisionDetails( obj )
	if obj == nil then return end
	
	local numCols = getTerColls( obj )
	
	if numCols == 0 then return end
	
	local colList = {}
	local latestCol, x, y, z
	
	for i = 1, numCols do
		latestCol, x, y, z = getTerDetail( obj, i )
		
		colList[ i ] = { x = x, y = y, z = z }
	end
	
	if latestCol == numCols then return colList end
	
	-- sort collisions in order, oldest first
	local sortedList = {}
	
	for i = latestCol + 1, numCols do
		sortedList[ #sortedList + 1 ] = colList[ i ]
	end
	for i = 1, latestCol do
		sortedList[ #sortedList + 1 ] = colList[ i ]
	end
	return sortedList
end

function P.GetEntityTerrainCollisionDetails( e )
	local obj = gEnt[ e ].obj
	return GetTerrainCollisionDetails( obj )
end

----------------------------------------------------------
-- Function to get the number of object-on-object       --
-- collisions stored by the collision checking system   -- 
-- for a specified entity.                              --
----------------------------------------------------------
function P.GetEntityObjectNumCollisions( e )
	if e == nil then return 0 end
	return getObjColls( gEnt[ e ].obj )
end

----------------------------------------------------------
-- Functions to get the details of object-on-object     --
-- collisions from the collision checking system for a  --
-- specified  object/entity.                            --
-- Returns nil if none detected  OR                     --
-- Returns a table containing the object id of the      --
-- collided with object, the world location of the      --
-- contact point and the 'force'* of the contact in     --
-- order oldest first.                                  --
-- (*not really proper force but a useful indication of --
--  the severity of the impact good enough for applying --
--  damage to objects)                                  --
----------------------------------------------------------
function P.GetObjectCollisionDetails( obj )
	if obj == nil or not objExists( obj ) then return end
	
	local numCols = getObjColls( obj )
	
	if numCols == 0 then return end
	
	local colList = {}
	
	for i = 1, numCols do
		local objB, x, y, z, f = getObjDetail( obj, i )
		
		colList[ i ] = { obj = objB, x = x, y = y, z = z, f = f }
	end
	
	return colList 
end

function P.GetEntityObjectCollisionDetails( e )
	local obj = gEnt[ e ].obj
	return GetObjectCollisionDetails( obj )
end

local hingeValues =
 { [ 'TopLeft'        ] = { w = -0.5, h =  0.5, l =  0,   t = 3 },
   [ 'TopRight'       ] = { w =  0.5, h =  0.5, l =  0,   t = 3 },
   [ 'TopFront'       ] = { w =  0,   h =  0.5, l = -0.5, t = 1 },
   [ 'TopBack'        ] = { w =  0,   h =  0.5, l =  0.5, t = 1 },
   [ 'TopCentreW'     ] = { w =  0,   h =  0.5, l =  0,   t = 1 },
   [ 'TopCentreL'     ] = { w =  0,   h =  0.5, l =  0,   t = 3 },
   [ 'BottomLeft'     ] = { w = -0.5, h = -0.5, l =  0,   t = 3 },
   [ 'BottomRight'    ] = { w =  0.5, h = -0.5, l =  0,   t = 3 },
   [ 'BottomFront'    ] = { w =  0,   h = -0.5, l = -0.5, t = 1 },
   [ 'BottomBack'     ] = { w =  0,   h = -0.5, l =  0.5, t = 1 },
   [ 'BottomCentreW'  ] = { w =  0,   h = -0.5, l =  0,   t = 1 },
   [ 'BottomCentreL'  ] = { w =  0,   h = -0.5, l =  0,   t = 3 },
   [ 'LeftBack'       ] = { w = -0.5, h =  0,   l =  0.5, t = 2 },
   [ 'LeftFront'      ] = { w = -0.5, h =  0,   l = -0.5, t = 2 },
   [ 'LeftCentre'     ] = { w = -0.5, h =  0,   l =  0,   t = 2 },
   [ 'FrontCentre'    ] = { w =  0,   h =  0,   l = -0.5, t = 1 },
   [ 'BackCentre'     ] = { w =  0,   h =  0,   l =  0.5, t = 1 },   
   [ 'RightBack'      ] = { w =  0.5, h =  0,   l =  0,   t = 2 },
   [ 'RightFront'     ] = { w =  0.5, h =  0,   l = -0.5, t = 2 },
   [ 'RightCentre'    ] = { w =  0.5, h =  0,   l =  0,   t = 2 },
   [ 'CentreVertical' ] = { w =  0,   h =  0,   l =  0,   t = 2 },
   [ 'CentreWidth'    ] = { w =  0,   h =  0,   l =  0,   t = 1 },
   [ 'CentreLength'   ] = { w =  0,   h =  0,   l =  0,   t = 3 },
   [ 'CentreFront'    ] = { w =  0,   h =  0,   l = -0.5, t = 2 },
   [ 'CentreBack'     ] = { w =  0,   h =  0,   l =  0.5, t = 2 }
 }
   
local function getHingeValues( dims, hingeName, spacing )
	local hv = hingeValues[ hingeName ]
	if hv == nil then return end
	spacing = spacing or { w = 0, h = 0, l = 0 }
	
	return ( dims.w + spacing.w ) * hv.w,
    	   ( dims.h + spacing.h ) * hv.h,
 		   ( dims.l + spacing.l ) * hv.l, hv.t
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

----------------------------------------------------------
-- Function to add a named hinge to an object           --
-- e.g. P.AddSingleHinge( Ent.obj, "LeftCentre" )       --
-- would probably be good for a standard door hinge.    --
-- swingAng is the total angle the door is allowed to   --
-- swing through.                                       --
-- offset is the % offset of the start point of the     --
-- swing, default is to start in the centre (swing door --
-- style)                                               --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddObjectSingleHinge( obj, hingeName, swingAng, offset, spacing )
	swingAng = swingAng or 180  -- default to swing door style
	offset   = offset   or  50  -- default to swing door style
	
	-- sanity check values
	if swingAng > 360 then swingAng = 360 end
	if swingAng <   0 then swingAng =   0 end
	if offset   > 100 then offset   = 100 end
	if offset   <   0 then offset   =   0 end
	
	if obj == nil or not objExists( obj ) then return end
	
	local dims = GetObjectDimensions( obj )
	
	if dims ~= nil then
		local x, y, z, hingeType = getHingeValues( dims, hingeName, spacing )
		
		if x == nil then return end
		
		local _, _, _, aX, aY, aZ = getObjPosAng( obj )

		local minAng, maxAng = 0, 0
		
		if hingeType == 2 then   -- rotates around Y axis
			-- We assume that the door starts in its centre position 
			local midAng = deg( getRealWorldYangle( aX, aY, aZ ) )	
			minAng = -( midAng + swingAng * offset / 100 )
			maxAng = -( midAng - swingAng * ( 1 - offset / 100 ) )
		end

		-- TBD work out what angles mean for X,Z axis rotations!
		-- (currently the 0 defaults will mean that no angular values will
		--  be set in the engine for this hinge )
		
		return createHingeS( obj, x, y, z, hingeType, rad( minAng ), rad( maxAng ) )
	end
end

----------------------------------------------------------
-- Function to add a named hinge to an entity           --
-- e.g. P.AddObjectSingleHinge( e, "LeftCentre" )       --
-- would probably be good for a standard door hinge.    --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddEntitySingleHinge( e, hingeName, swingAng, offset )
	if e == nil then return end
	return AddObjectSingleHinge( gEnt[ e ].obj, hingeName, swingAng, offset )
end

----------------------------------------------------------
-- Function to add a named hinge connecting two objects --
-- e.g. P.AddObjectDoubleHinge( A, B, hingeA, hingeB )  --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddObjectDoubleHinge( objA, objB, hingeNameA, hingeNameB, spacing, noCols )
	if objA == nil or not objExists( objA ) then return end
	if objB == nil or not objExists( objB ) then return end
	coCols = noCols or 0
	
	local dimsA = GetObjectDimensions( objA )
	local dimsB = GetObjectDimensions( objB )
	
	if dimsA ~= nil and dimsB ~= nil then
		local xa, ya, za, hingeTypeA = getHingeValues( dimsA, hingeNameA, spacing )
		if xa == nil then return end
		local xb, yb, zb, hingeTypeB = getHingeValues( dimsB, hingeNameB, spacing )
		if xb == nil then return end
		
		return createHingeD( objA, objB, xa, ya, za, xb, yb, zb, hingeTypeA, hingeTypeB, noCols )
	end
end
	
----------------------------------------------------------
-- Function to add a named hinge connecting 2 entities  --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddEntityDoubleHinge( e1, e2, hingeName1, hingeName2, spacing, noCols )
	if e1 == nil or e2 == nil then return end
	
	return AddObjectDoubleHinge( gEnt[ e1 ].obj, gEnt[ e2 ].obj, 
	                             hingeName1, hingeName2, spacing, noCols )
end

----------------------------------------------------------
-- Function to add a named joint to an object           --
-- e.g. P.AddObjectSingleJoint( Ent.obj, "TopCentreW" ) --                                            --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!)                   --
----------------------------------------------------------
function P.AddObjectSingleJoint( obj, jointName, spacing )
	
	if obj == nil or not objExists( obj ) then return end
	
	local dims = GetObjectDimensions( obj )
	
	if dims ~= nil then
		local x, y, z = getHingeValues( dims, jointName, spacing )
		
		if x == nil then return end

		return createJointS( obj, x, y, z )
	end
end
----------------------------------------------------------
-- Function to add a named joint to an entity           --
-- e.g. P.AddObjectSingleJoint( e, "TopCentreW" )       --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddEntitySingleJoint( e, jointName, spacing )
	if e == nil then return end
	return AddObjectSingleJoint( gEnt[ e ].obj, jointName, spacing )
end

----------------------------------------------------------
-- Function to add a named joint connecting two objects --
-- e.g. P.AddObjectDoubleJoint( A, B, jointA, jointB )  --
-- returns nil if either object doesn't exist           --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddObjectDoubleJoint( objA, objB, jointNameA, jointNameB, spacing, noCols )
	if objA == nil or not objExists( objA ) then return end
	if objB == nil or not objExists( objB ) then return end
	coCols = noCols or 0
	
	local dimsA = GetObjectDimensions( objA )
	local dimsB = GetObjectDimensions( objB )
	
	if dimsA ~= nil and dimsB ~= nil then
		local xa, ya, za = getHingeValues( dimsA, jointNameA, spacing )
		if xa == nil then return end
		local xb, yb, zb = getHingeValues( dimsB, jointNameB, spacing )
		if xb == nil then return end
		
		return createJointD( objA, objB, xa, ya, za, xb, yb, zb, noCols )
	end
end
	
----------------------------------------------------------
-- Function to add a named Joint connecting 2 entities  --
-- returns -1 if joint cannot be created                --
-- otherwise returns the index of the constraint        --
-- (note C++ value so counts from 0!                    --
----------------------------------------------------------
function P.AddEntityDoubleJoint( e1, e2, jointNameA, jointNameB, spacing, noCols )
	if e1 == nil or e2 == nil then return end
	
	return AddObjectDoubleJoint( gEnt[ e1 ].obj, gEnt[ e2 ].obj, 
	                             jointNameA, jointNameB, spacing, noCols )
end

---------------------------------------------
-- Function to remove all constraints from --
-- an entity                               --
---------------------------------------------
function P.RemoveEntityConstraints( e )
	if e == nil then return end
	
	remObjConsts( gEnt[ e ].obj )
end

---------------------------------------------
-- Function to remove all constraints from --
-- an entity if dead ( health <= 0 )       --
---------------------------------------------
function P.RemoveEntityConstraintsIfDead( e )
	if e == nil then return end
	
	if gEnt[ e ].health <= 0 then
		remObjConsts( gEnt[ e ].obj )
	end
end

---------------------------------------------
-- Function to set entity damping factors --
---------------------------------------------
function P.SetEntityDamping( e, damping, angle )
	if e == nil then return end

	setDamping( gEnt[ e ].obj, damping, angle )
end
	
--------------------------------------------------
-- Function to return Physics Object in players --
-- eye-line, differs from utillib version in    --
-- that only dynamic objects are reported as    --
-- 'hits'.                                      --
-- parameters:                                  --
--      dist : (optional) length of ray in game --
--                        units                 --
--      force : (optional) force to apply to    --
--              object hit                      --
-- returns object id or 0 if none found         --
-- if found also returns coordinates of ray     --
-- 'hit' point                                  --                         
--------------------------------------------------
-- vector representing direct player is looking
local rayX, rayY, rayZ

function P.GetViewVector()
	return rayX, rayY, rayZ
end

function P.ObjectPlayerLookingAt( dist, force )
	local dist  = dist  or 2000
	local force = force or 0

	local pxp, pyp, pzp = getPos3()
	local pxa, pya, pza = getAng3()
	
	if ducked == nil or ducked() == 0 then 
		pyp = pyp + 31 
	else 
		pyp = pyp + 10 
	end

	rayX, rayY, rayZ = U.Rotate3D ( 0, 0, 1, rad( pxa ), rad( pya ), rad( pza ) )

	return physRayCast( pxp, pyp, pzp, 
						pxp + rayX * dist, 
					    pyp + rayY * dist, 
					    pzp + rayZ * dist, force )
end		

return P