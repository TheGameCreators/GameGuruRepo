-- SaveGame page script

SAVEGAME_SLOT_FIRST = 1
SAVEGAME_SLOT_LAST = 8
SAVEGAME_BACK = 9
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgBackdrop = 0
g_sprBackdrop = 0
g_posBackdropAngle = 0
g_imgHeading = 0
g_sprHeading = 0
g_imgButton = {}
g_imgButtonH = {}
g_sprButton = {}
g_posButton = {}
g_imgCursor = 0
g_sprCursor = 0
g_sprCursorPtrX = 50
g_sprCursorPtrY = 90
g_sprCursorPtrClick = 0

g_strSlotNames = {}
g_iGameNameNumber = 0

function savegame_init()
 -- determine style folder we are in
 file = io.open("titlesbank\\style.txt", "r")
 if file ~= nil then
  io.input(file)
  g_strStyleFolder = io.read()
  io.close(file)
 end
 -- determine current game save number
 file = io.open("savegames\\gamenamenumber.dat", "r")
 if file ~= nil then
  io.input(file)
  g_iGameNameNumber = tonumber(io.read())
  io.close(file)
 else
  g_iGameNameNumber = 0
 end
 -- choose ideal resolution
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- backdrop
 g_imgBackdrop = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. g_strBestResolution .. "\\loading.png")
 g_sprBackdrop = CreateSprite ( g_imgBackdrop )
 SetSpritePosition ( g_sprBackdrop, 0, 0 )
 SetSpriteSize ( g_sprBackdrop, 110, 110 )
 SetSpriteOffset ( g_sprBackdrop, 5, 5 )
 -- heading
 g_imgHeading = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\save-game-hover.png")
 g_sprHeading = CreateSprite ( g_imgHeading ) 
 SetSpriteOffset ( g_sprHeading, GetImageWidth(g_imgHeading)/2, 0 )
 SetSpritePosition ( g_sprHeading, 50, 8 )
 -- buttons
 for i = 1, SAVEGAME_BACK, 1
 do  
  if i==SAVEGAME_BACK then
   strButtonName = "back"
   g_posButton[i] = 90
  else
   strButtonName = "blank"
   g_posButton[i] = 20+(i*7)
  end
  g_imgButton[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
  g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-hover.png")
  g_sprButton[i] = CreateSprite ( g_imgButton[i] ) 
  SetSpriteOffset ( g_sprButton[i], GetImageWidth(g_imgButton[i])/2, 0 )
  SetSpritePosition ( g_sprButton[i], 50, g_posButton[i] )
 end
 -- cursor
 g_imgCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprCursor = CreateSprite ( g_imgCursor ) 
 ActivateMouse()
 DrawSpritesFirst()
 -- populate slot names
 fillgameslots = require "titlesbank\\fillgameslots"
 fillgameslots.fill()
end

function savegame_main()
 -- redraw header
 PasteSprite ( g_sprHeading )
 -- control menus
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgButton[SAVEGAME_BACK])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgButton[SAVEGAME_BACK])/2) then
  for i = 1, SAVEGAME_BACK, 1
  do
   if g_sprCursorPtrY > g_posButton[i] and g_sprCursorPtrY <= g_posButton[i]+GetImageHeight(g_imgButton[i]) then
    iHighlightButton = i
   end
  end
 end
 for i = 1, SAVEGAME_BACK, 1
 do
  PasteSprite ( g_sprButton[i] )
  if i < SAVEGAME_BACK then
   if g_strSlotNames[i] ~= nil then
    TextCenterOnX ( 50, g_posButton[i] + 0.5 + (GetImageHeight(g_imgButton[i])/2), 3, g_strSlotNames[i])
   end
  end
  if iHighlightButton == i then
   SetSpriteImage ( g_sprButton[i], g_imgButtonH[i] )
  else
   SetSpriteImage ( g_sprButton[i], g_imgButton[i] )
  end
 end
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton>=SAVEGAME_SLOT_FIRST and iHighlightButton<=SAVEGAME_SLOT_LAST then
   -- manage game number and name
   g_iGameNameNumber = g_iGameNameNumber + 1
   strNewSlotName = g_iGameNameNumber .. ": " .. os.date()
   g_strSlotNames[iHighlightButton] = strNewSlotName
   -- save slot file
   gamedata = require "titlesbank\\gamedata"
   gamedata.save(iHighlightButton,strNewSlotName)
   -- back to game
   ResetFade()
   TriggerFadeIn()
   ResumeGame()
  end
  if iHighlightButton==SAVEGAME_BACK then
   SwitchPageBack()
  end
 end
 -- move backdrop
 SetSpritePosition ( g_sprBackdrop, math.cos(g_posBackdropAngle* 0.0174533)*5, math.sin(g_posBackdropAngle*0.0174533)*5 )
 g_posBackdropAngle = g_posBackdropAngle + 0.002
 -- cursor
 PasteSprite ( g_sprCursor )
end

function savegame_free()
 -- store current game save number
 file = io.open("savegames\\gamenamenumber.dat", "w")
 io.output(file)
 io.write(g_iGameNameNumber .. "\n")
 io.close(file)  
 -- free resources
 for i = 1, SAVEGAME_BACK, 1
 do  
  DeleteSprite ( g_sprButton[i] )
  DeleteImage ( g_imgButton[i] )
  DeleteImage ( g_imgButtonH[i] )
 end
 DeleteSprite ( g_sprBackdrop )
 DeleteImage ( g_imgBackdrop )
 DeleteSprite ( g_sprHeading )
 DeleteImage ( g_imgHeading )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
 DrawSpritesLast()
end
