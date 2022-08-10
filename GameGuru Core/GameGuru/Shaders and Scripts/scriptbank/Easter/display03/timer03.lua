local Delay = 1
local g_Seconds = 59
local g_Minutes = 14
local TimeStamp = 0
local TimePassed = 0
local Time1 = 0
local Time2 = 0
local LastTime = 15
Timesup = 0

function timer03_init(e)
end

function timer03_main(e)
if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 then
Destroy(e)
end 
if g_FirstRun == 1 then
if TimeStamp == 0 then
Time1 = g_Time
TimeStamp = 1
end
Time2 = g_Time
TimePassed = (Time2 - Time1) * 0.001
if TimePassed >= Delay then
g_Seconds = g_Seconds - 1
TimeStamp = 0
if g_Seconds == 0 then
g_Minutes = g_Minutes - 1
g_Seconds = 59
if g_Minutes == 0 or g_Seconds > 0 then
LastTime = LastTime - 1
end
end
end
if LastTime>0 then
TextCenterOnX(47.5,3,4,":")
TextCenterOnX(50,3,4,""..g_Seconds.. " ")
TextCenterOnX(45,3,4," "..g_Minutes.. " ")
else
Timesup = 1
TextCenterOnX(47.5,3,4,":")
TextCenterOnX(50,3,4,"0")
TextCenterOnX(45,3,4,"0")
FreezePlayer()
TextCenterOnX(50,20,4,"Time is over! You need to try again!")
Prompt ("Press E to repeat the level" )
if g_KeyPressE == 1 then
 UnFreezePlayer()
 JumpToLevel("level3")
 Destroy(e)
 end
end
end
end
