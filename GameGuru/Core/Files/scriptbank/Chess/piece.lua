-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- 
local pieces = {}
local sub = string.sub
local rad = math.rad
local sin = math.sin
local cos = math.cos
local deg = math.deg

local mouse_sprite = nil
local hand_sprite  = nil

local Q = require "scriptbank\\quatlib"
local U = require "scriptbank\\utillib"

selectedPiece = ''
currentMove = ''
 
function piece_init_name(e, name)
	Include("quatlib.lua")
	Include("utillib.lua")

	pieces[e] = {name = name, onBoard = false, sq = ''}	
	
	mouse_sprite = CreateSprite(LoadImage ( "scriptbank\\chess\\mouse_cursor.png"))
	SetSpriteOffset(mouse_sprite, -1 , 1.5)
	SetSpriteSize  (mouse_sprite, -1 , 3)
	SetSpriteDepth (mouse_sprite, 0)
	SetSpritePosition (mouse_sprite,  200, 200)

	hand_sprite = CreateSprite(LoadImage ( "scriptbank\\chess\\hand.png"))
	SetSpriteOffset(hand_sprite, -1 , 1.5)
	SetSpriteSize  (hand_sprite, -1 , 3)
	SetSpriteDepth (hand_sprite, 0)
	SetSpritePosition (hand_sprite,  200, 200)
end

function GetPiece(sq)
	for k, v in pairs(pieces) do
		if v.sq == sq and v.onBoard then
			return v.name, k
		end
	end
	return '', nil
end

function PutPiece(name, sq, x, y, z, wQuat, bQuat)
	for k, v in pairs(pieces) do
		if v.name == name and not v.onBoard then
			pieces[k] = {name = name, onBoard = true, sq = sq}
			CollisionOff(k)
			SetPosition(k, x, y, z)
			
			if sub(name,1,5) == 'White' then
				local xA, yA, zA = Q.ToEuler(wQuat)
				SetRotation(k, deg(xA), deg(yA), deg(zA))
			else
				local xA, yA, zA = Q.ToEuler(bQuat)
				SetRotation(k, deg(xA), deg(yA), deg(zA))
			end
			CollisionOn(k)
			Show(k)
			return
		end
	end
end

function ResetPieces()
	for k, v in pairs(pieces) do
		if v.name == "White Queen(P)" then 
			pieces[k].name = "White Pawn" 
		elseif v.name == "Black Queen(P)" then
			pieces[k].name = "Black Pawn"
		end
		pieces[k].onBoard = false
	end		
	SetSpritePosition (mouse_sprite, 200, 200)
	SetSpritePosition (hand_sprite,  200, 200)
end

local function HidePiece(sq)
	for k, v in pairs(pieces) do
		if v.sq == sq and v.onBoard then
			CollisionOff(k)
			Hide(k)
			pieces[k].onBoard = false
			return
		end
	end
end
	
function MovePiece(fromsq, tosq)
	local name, k = GetPiece(fromsq)

	-- check for castling
	if name == 'White King' and fromsq == 'e1' then
		-- might be castling 
		if tosq == 'g1' then
			MovePiece('h1', 'f1')
		elseif tosq == 'c1' then
			MovePiece('a1', 'd1')
		end
	elseif name == 'Black King' and fromsq == 'e8' then
		-- might be castling 
		if tosq == 'g8' then
			MovePiece('h8', 'f8')
		elseif tosq == 'c8' then
			MovePiece('a8', 'd8')
		end
	end
	
	-- check for enpassant
	if name == 'White Pawn' and sub(fromsq, 2, 2) == '5' then
		-- might be enpassant
		if sub(fromsq, 1, 1) ~= sub(tosq, 1, 1) then
			local name, k = GetPiece(tosq)
			if k == nil then
				HidePiece(sub(tosq, 1, 1) .. '5')
			end
		end
	elseif name == 'Black Pawn' and sub(fromsq, 2, 2) == '4' then
		-- might be enpassant
		if sub(fromsq, 1, 1) ~= sub(tosq, 1, 1) then
			local name, k = GetPiece(tosq)
			if k == nil then
				HidePiece(sub(tosq, 1, 1) .. '4')
			end
		end
	end
	
	HidePiece(tosq)
		
	CollisionOff(k)
	local x, y, z, wQuat, bQuat = GCGetPos(tosq)
	SetPosition(k, x, y, z)
	if sub(name,1,5) == 'White' then
		local xA, yA, zA = Q.ToEuler(wQuat)
		SetRotation(k, deg(xA), deg(yA), deg(zA))		
	else
		local xA, yA, zA = Q.ToEuler(bQuat)
		SetRotation(k, deg(xA), deg(yA), deg(zA))
	end
	CollisionOn(k)
		
	pieces[k].sq = tosq
end

local function PlayerLookingAtPiece()
		
	local yOff = 31
		
	if GetGamePlayerStatePlayerDucking() == 1 then yOff = 10 end
		
	local x, y, z = g_PlayerPosX, g_PlayerPosY + yOff, g_PlayerPosZ
	
	local paX, paY, paZ = rad(g_PlayerAngX), rad(g_PlayerAngY), rad(g_PlayerAngZ)

	local rayX, rayY, rayZ = U.Rotate3D(0,0,2000,paX, paY, paZ)
	
	return IntersectAll(x, y, z, x + rayX, y + rayY, z + rayZ, 0)

end

local function PlayerLookingAtSquare()
	local yOff = 31
		
	if GetGamePlayerStatePlayerDucking() == 1 then yOff = 10 end
		
	local x, y, z = g_PlayerPosX, g_PlayerPosY + yOff, g_PlayerPosZ
	
	local paX, paY, paZ = rad(g_PlayerAngX), rad(g_PlayerAngY), rad(g_PlayerAngZ)

	local rayX, rayY, rayZ = U.Rotate3D(0,0,2000,paX, paY, paZ)
	
	local obj = IntersectAll(x, y, z, x + rayX, y + rayY, z + rayZ, 0)
		
	if obj ~= g_boardObject then
		-- try again ignoring object
		obj = IntersectAll(x, y, z, x + rayX, y + rayY, z + rayZ, obj)
	end
	
	if obj == g_boardObject then
		return GCGetSquare(GetIntersectCollisionX(),
		                   GetIntersectCollisionY(),
						   GetIntersectCollisionZ());
	else
		return '??'
	end
end

local objects = {}

local function ShowSprite(spr)
	if spr == hand_sprite then
		SetSpritePosition (mouse_sprite, 200, 200)
		SetSpritePosition (hand_sprite,   50,  50)
	else
		SetSpritePosition (mouse_sprite,  50,  50)
		SetSpritePosition (hand_sprite,  200, 200)
	end
end

local choosePiece  = nil
local mousePressed = false
local frameCounter = 0
local flasherFlag  = false
local lastPiece    = nil

local function ProcessPlayer(e)
	
	local fp = false
	
	if choosePiece == nil then
		local obj = PlayerLookingAtPiece()
	
		for k, v in pairs(objects) do
			if v.obj == obj then
				if pieces[k].onBoard then
					selectedPiece = pieces[k].name
					ShowSprite(hand_sprite)
					if g_MouseClick == 1 then
						if not mousePressed then
							choosePiece = k
							mousePressed = true
						end
					else
						mousePressed = false
					end
				end
				return
			end
		end
		selectedPiece = ''
		ShowSprite(mouse_sprite)
	else
		frameCounter = frameCounter + 1
		if frameCounter > 25 then 
			frameCounter = 0
			flasherFlag = not flasherFlag
		end
		
		local fromsq = pieces[choosePiece].sq
		
		local tosq = PlayerLookingAtSquare()
		
		if tosq ~= '??' and tosq ~= fromsq then
			
			CollisionOff(choosePiece)
			
			local valid, move = GCValidMove(fromsq, tosq)
			if valid then
				currentMove = fromsq .. tosq
				local x, y, z, ang = GCGetPos(tosq)
				SetPosition(choosePiece, x, y, z)
				
				local _,otherPiece = GetPiece(tosq)
				if otherPiece ~= nil then
					if lastPiece and lastPiece ~= otherPiece then
						Show(lastPiece)
					end
					lastPiece = otherPiece
					if not flasherFlag then 
						Hide(otherPiece)
					else
						Show(otherPiece)
					end
				else
					if lastPiece ~= nil then
						Show(lastPiece)
					end
				end
			end
		else
			currentMove = ''
			if lastPiece ~= nil then
				Show(lastPiece)
			end
			CollisionOff(choosePiece)
			local x, y, z, ang = GCGetPos(fromsq)
			SetPosition(choosePiece, x, y, z)
			CollisionOn(choosePiece)
		end
		
		if flasherFlag then 
			Hide(choosePiece)
		else
			Show(choosePiece)
		end
		
		if g_MouseClick ~= 1 then	
			mousePressed = false
			
			local x, y, z, ang = GCGetPos(fromsq)
			SetPosition(choosePiece, x, y, z)
			CollisionOn(choosePiece)
			Show(choosePiece)
			
			if tosq ~= '??' then
				if lastPiece ~= nil then
					Show(lastPiece)
					lastPiece = nil
				end
				local valid, move = GCValidMove(fromsq, tosq)
				if valid then
					GCMakeMove(fromsq, tosq, move)
					
					-- handle promotion
					if pieces[choosePiece].name == 'White Pawn' then
						if sub(tosq,2,2) == '8' then
							pieces[choosePiece].name = 'White Queen(P)'
						end
					elseif pieces[choosePiece].name == 'Black Pawn' then
						if sub(tosq,2,2) == '1' then
							pieces[choosePiece].name = 'Black Queen(P)'
						end
					end
				end	
			end
			
			choosePiece = nil
			currentMove = ''
		end		
	end
end

local Trigger_Ent = nil

function piece_main(e)

	local Ent = g_Entity[e]
	
	local thisObj = objects[e]
	
	if thisObj == nil then
		objects[e] = {obj = Ent.obj, state = 'steady'}
					  
		if Trigger_Ent == nil then Trigger_Ent = e end
		
		return
	end
	
	if Trigger_Ent == e and GCPlay() then
		ProcessPlayer(e)
	end
end
