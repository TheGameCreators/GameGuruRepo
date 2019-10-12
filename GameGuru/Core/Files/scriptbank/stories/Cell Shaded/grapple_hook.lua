
--how fast the hook moves *too high will cause it to pass through objects*
local hook_move_speed = 2000
--how fast the hook gains/loses altitude *set this in relation to the move speed*
local hook_height_gain = 37
--how far the hook will travel *also effected by move speed so this is not a set value*
local max_range = 3000
--only 1 hook can be fired at a time but this is an extra optional delay if you want the hook to have extra delay (but it has already stopped moving)
local fire_delay = 2000
--the key press required to fire the grapple (hopefully will be possible to make it a real gun shot in future)
local fire_key = "q"
--if the "hook" starts too low or high then change this
local ymodifier = -66
--how far away from the targets the hook can connect from
local hook_connect_range = 80

local hook_moved = 0
local player_moved = 0
local dist = 0
local x = {}
local y = {}
local z = {}
local target = {}
local targets = 0
local new_move_delay = move_delay
local hit_check = 10
local state = "init"
local move_delay = 1



function grapple_hook_init(e)
CollisionOff(e)
Scale(e,150)
Hide(e)
StartParticleEmitter(e)
end


function grapple_hook_main(e)

if targets == 0 then
for a = 1, 19999 do
if g_Entity[a] ~= nil then
if weapon_name[a] == "hook target" then
targets = targets + 1
target[targets] = a
end
end
end
else
if grappleid == nil then
grappleid = g_PlayerGunID
else

if state == "init" then
if g_PlayerGunID == grappleid then
state = "fire?"
end
elseif state == "fire?" then
Hide(e)
if GetTimer(e) >= fire_delay then
if GetInKey() == fire_key and g_PlayerGunID == grappleid then
dist = 0
CollisionOff(e)
SetPosition(e,g_PlayerPosX,g_PlayerPosY + ymodifier,g_PlayerPosZ)
SetRotation(e,g_PlayerAngX,g_PlayerAngY,g_PlayerAngZ)
MoveForward(e,100)
state = "firing"
hook_moved = 0
player_moved = 0
new_move_delay = GetTimer(e) + move_delay
hit_check = GetTimer(e) + 50
end
end

if g_PlayerGunID ~= grappleid then
state = "init"
end

elseif state == "firing" then

Show(e)
if GetPlayerFlatDistance(e) < max_range then
CollisionOff(e)
MoveForward(e,hook_move_speed)
MoveUp(e,g_PlayerAngX * -hook_height_gain)
hook_moved = hook_moved + 1
x[hook_moved] = g_Entity[e]['x']
y[hook_moved] = g_Entity[e]['y']
z[hook_moved] = g_Entity[e]['z']
else
state = "fire?"

end

if g_PlayerAngX > 5 and hook_moved > 3 then
if g_Entity[e]['y'] >= g_PlayerPosY - 66 then
state = "fire?"

end
end

if g_PlayerAngX < -10 then
collision_mod = 1
else
collision_mod = 0.02
end

if GetTimer(e) > hit_check then
hit_check = GetTimer(e) + 1
if hook_moved > 5 then
for a = 1, targets do
if GetDistance(e,target[a]) <= hook_connect_range then
state = "hit"
break
end
end
end
if GetPlayerFlatDistance(e) <= dist - collision_mod then
state = "fire?"
else
dist = GetPlayerFlatDistance(e)
end
end

elseif state == "hit" then
Hide(e)
CollisionOff(e)
SetPosition(e,g_PlayerPosX,g_PlayerPosY + ymodifier,g_PlayerPosZ)
state = "move player"

elseif state == "move player" then
if player_moved < hook_moved then
if GetTimer(e) >= new_move_delay then
CollisionOff(e)
TransportToIfUsed(e)
Hide(e)
if player_moved + 2 < hook_moved then
player_moved = player_moved + 2
else
player_moved = hook_moved
end
SetPosition(e,x[player_moved],y[player_moved],z[player_moved])
new_move_delay = GetTimer(e) + move_delay
end
else
state = "fire?"
end
end --state

GravityOn(e)
if GetPlayerFlatDistance(e) > 80 then
CollisionOn(e)
end

--Prompt(grappleid.. "  " ..g_PlayerGunID)
end
end
end --main


function grapple_hook_exit(e)

end


function GetPlayerFlatDistance(e)
  local tPlayerDX = (g_Entity[e]['x'] - g_PlayerPosX)
  local tPlayerDZ = (g_Entity[e]['z'] - g_PlayerPosZ)
  return math.sqrt(math.abs(tPlayerDX*tPlayerDX)+math.abs(tPlayerDZ*tPlayerDZ));
end

function GetDistance(e,v)
if g_Entity[e] ~= nil and g_Entity[e] ~= 0 and g_Entity[v] ~= nil then
local disx = g_Entity[e]['x'] - g_Entity[v]['x']
local disy = g_Entity[e]['y'] - g_Entity[v]['y']
local disz = g_Entity[e]['z'] - g_Entity[v]['z']
return math.sqrt(disx^2 + disy^2 + disz^2)
end
end