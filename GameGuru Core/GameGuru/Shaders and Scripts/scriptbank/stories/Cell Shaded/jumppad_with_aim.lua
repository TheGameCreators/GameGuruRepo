
local use_key = "e"
local use_pad_range = 180
local travel_speed = 500
local climb_speed = 100


local state = {}
local missile = {}
local target = {}
local misx = {}
local misy = {}
local misz = {}
local start_distance = 0

function jumppad_with_aim_init_name(e,name)
weapon_name[e] = name
state[e] = "wait"
end


function jumppad_with_aim_main(e)
if missile[e] == nil then
Prompt("no missile found for pad " ..weapon_name[e])
for a = e - 50, e + 50 do
if g_Entity[a] ~= nil then
if a ~= e then
if weapon_name[a] == "jumppad missile" then
if GetDistance(e,a) < 200 then
CollisionOff(a)
--SetPosition(a,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z'])
misx[e] = g_Entity[a]['x']
misy[e] = g_Entity[a]['y']
misz[e] = g_Entity[a]['z']
missile[e] = a
break
end
end
end
end
end
--missile found
else

if target[e] == nil then
Prompt("no target found for pad " ..weapon_name[e])
for a = e - 50, e + 50 do
if g_Entity[a] ~= nil then
if a ~= e then
if weapon_name[a] == weapon_name[e] then
RotateToEntity(missile[e],a)
target[e] = a 
break
end
end
end
end
--target found
else

--main firing code
if state[e] == "wait" then
if GetPlayerDistance(e) < use_pad_range then
CollisionOff(e)
SetRotation(e,0,g_PlayerAngY,0)
SetRotation(missile[e],0,g_PlayerAngY,0)
Prompt("Press " ..use_key.. " to use the pad")
if GetInKey() == use_key then
PlayAnimation(e)
TransportToIfUsed(e)
start_distance = GetPlayerFlatDistance(target[e])
state[e] = "firing"
end
end
elseif state[e] == "firing" then

if GetPlayerFlatDistance(e) < start_distance / 2 then
MoveUp(missile[e],climb_speed)
MoveForward(missile[e],travel_speed)
TransportToIfUsed(e)
elseif GetPlayerFlatDistance(e) < start_distance then
MoveUp(missile[e],-climb_speed)
MoveForward(missile[e],travel_speed)
TransportToIfUsed(e)
else
SetPosition(missile[e],misx[e],misy[e],misz[e])
CollisionOn(e)
state[e] = "wait"
end
end --state
end --target
end --missile
end --main


function GetDistance(e,v)
if g_Entity[e] ~= nil and g_Entity[e] ~= 0 and g_Entity[v] ~= nil then
local disx = g_Entity[e]['x'] - g_Entity[v]['x']
local disy = g_Entity[e]['y'] - g_Entity[v]['y']
local disz = g_Entity[e]['z'] - g_Entity[v]['z']
return math.sqrt(disx^2 + disy^2 + disz^2)
end
end

function GetPlayerFlatDistance(e)
if g_Entity[e] ~= nil then
local disx = g_Entity[e]['x'] - g_PlayerPosX
local disz = g_Entity[e]['z'] - g_PlayerPosZ
return math.sqrt(disx^2 + disz^2)
end
end

function RotateToEntity(e,v)
if g_Entity[e] ~= nil and g_Entity[e] ~= 0 and g_Entity[v] ~= nil and g_Entity[v] ~= 0 then
local x = g_Entity[v]['x'] - g_Entity[e]['x']
local z = g_Entity[v]['z'] - g_Entity[e]['z']
local angle = math.atan2(x,z)
angle = angle * (180.0 / math.pi)
if angle < 0 then
angle = 360 + angle
elseif angle > 360 then
angle = angle - 360
end
SetRotation(e,0,angle,0)
return angle
end
end