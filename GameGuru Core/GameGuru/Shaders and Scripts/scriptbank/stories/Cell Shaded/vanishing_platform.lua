
local state = {}
local flash_time = 2000
local flash_delay = {}
local active_range = 80
local reset_time = 5000

local flash_time_new = {}
local flash_delay_new = {}
local shown = {}
function vanishing_platform_init(e)
shown[e] = 1
flash_delay[e] = flash_time / 50
state[e] = "wait"
end

function vanishing_platform_main(e)
if state[e] == "wait" then
if GetPlayerDistance(e) <= active_range then
state[e] = "active"
flash_time_new[e] = GetTimer(e) + flash_time
flash_delay_new[e] = GetTimer(e) + flash_delay[e]
end
elseif state[e] == "active" then
if GetTimer(e) < flash_time_new[e] then
if GetTimer(e) > flash_delay_new[e] then
flash_delay_new[e] = GetTimer(e) + flash_delay[e]
flash_delay[e] = flash_delay[e] * 0.75
if shown[e] == 1 then
Hide(e)
shown[e] = 0
elseif shown[e] == 0 then
Show(e)
shown[e] = 1
end
end
else
Hide(e)
shown[e] = 0
CollisionOff(e)
flash_time_new[e] = GetTimer(e) + reset_time
state[e] = "reset"
end
elseif state[e] == "reset" then
if GetTimer(e) > flash_time_new[e] then
CollisionOn(e)
flash_delay[e] = flash_time / 50
Show(e)
shown[e] = 1
state[e] = "wait"
end
end



end --main