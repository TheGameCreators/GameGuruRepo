--smallg
--script to read a note/paper found in game
--image must be in folder "scriptbank\images\note" & named "000.png" / "001.png" / "002.png" etc
 
--how far away from note it can be read
local use_range = 120
--key press to read note
local use_key = "e"
--maximum notes to display (use key will cycle through)
local max_page = 1
 
 
local pressed = 0
local reading = {}
 
function note010_init(e)
LoadImages("cellshaded",0)
reading[e] = 0
end
 
 
function note010_main(e)
if GetPlayerDistance(e) <= use_range then
if reading[e] == 0 then
PromptLocal(e,"Read note? *" ..use_key.."*")

if GetInKey() == use_key and pressed == 0 then
PlaySound(e,0)
Hide(e)
FreezePlayer()
pressed = 1
reading[e] = 1
HideImage(0)
SetImageAlignment(0)
SetImagePosition(50,50)
ShowImage(16)
end
elseif reading[e] >= 1 then
Prompt("Close note? *" ..use_key.."*")
if GetInKey() == use_key and pressed == 0 then
pressed = 1
HideImage(reading[e] - 1)
reading[e] = reading[e] + 1 
if reading[e] > max_page then
reading[e] = 0 
Show(e)
UnFreezePlayer()
else
ShowImage(reading[e] - 1)
end
end
end
 
else
if reading[e] > 0 then
HideImage(reading[e] - 1)
reading[e] = 0 
Show(e)
end
end
if pressed == 1 and GetInKey() == "" then
pressed = 0
end

 
end
 
function note_exit(e)
 
end