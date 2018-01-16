-- Resolutions module

local fillgameslots = {}

SAVEGAME_SLOT_FIRST = 1
SAVEGAME_SLOT_LAST = 8

g_strSlotNames = {}
g_strLevelFilenames = {}

function fillgameslots.fill()
 -- populate slot names
 for i = 1, SAVEGAME_SLOT_LAST, 1
 do   
  file = io.open("savegames\\gameslot" .. i .. ".dat", "r")
  if file ~= nil then
   io.input(file)
   iMagicNumber = tonumber(io.read())
   iGameNameNumber = tonumber(io.read())
   strUniqueName = io.read()
   strLevelFilename = io.read()
   iReserved = tonumber(io.read())
   g_strSlotNames[i] = strUniqueName
   g_strLevelFilenames[i] = strLevelFilename   
   io.close(file)
  else
   g_strSlotNames[i] = "EMPTY PROGRESS SLOT"
  end
 end
end

return fillgameslots
