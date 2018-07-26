-- GameMenu page script

GAMEMENU_MAINMENU = 1
GAMEMENU_LOADGAME = 2
GAMEMENU_SAVEGAME = 3
GAMEMENU_GRAPHICS = 4
GAMEMENU_SOUND = 5
GAMEMENU_RESUME = 6
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgHeading = 0
g_sprHeading = 0
g_imgButton = {}
g_imgButtonH = {}
g_sprButton = {}
g_posButton = {}
g_imgCursor = 0
g_sprCursor = 0
g_sprCursorPtrX = 50
g_sprCursorPtrY = 33
g_sprCursorPtrClick = 0

function gamemenu_init()
 -- determine style folder we are in
 file = io.open("titlesbank\\style.txt", "r")
 if file ~= nil then
  io.input(file)
  g_strStyleFolder = io.read()
  io.close(file)
 end
 -- choose ideal resolution
 DrawSpritesLast()
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- heading
 g_imgHeading = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\game-paused-title.png")
 g_sprHeading = CreateSprite ( g_imgHeading ) 
 SetSpriteOffset ( g_sprHeading, GetImageWidth(g_imgHeading)/2, 0 )
 SetSpritePosition ( g_sprHeading, 50, 8 )
 -- buttons
 for i = 1, GAMEMENU_RESUME, 1
 do
  if i == GAMEMENU_MAINMENU then strButtonName = "main-menu"
  elseif i == GAMEMENU_LOADGAME then strButtonName = "load-game"
  elseif i == GAMEMENU_SAVEGAME then strButtonName = "save-game"
  elseif i == GAMEMENU_GRAPHICS then strButtonName = "graphics-settings"
  elseif i == GAMEMENU_SOUND then strButtonName = "sound-levels"
  elseif i == GAMEMENU_RESUME then strButtonName = "resume-game"
  else strButtonName = ""
  end	
  g_imgButton[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
  g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-hover.png")
  g_sprButton[i] = CreateSprite ( g_imgButton[i] ) 
  SetSpriteOffset ( g_sprButton[i], GetImageWidth(g_imgButton[i])/2, 0 )
  g_posButton[i] = 20+(i*10)
  SetSpritePosition ( g_sprButton[i], 50, g_posButton[i] )
 end
 -- cursor
 g_imgCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprCursor = CreateSprite ( g_imgCursor ) 
 ActivateMouse()
end

function gamemenu_main()
 -- control menus
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgButton[1])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgButton[1])/2) then
  for i = 1, GAMEMENU_RESUME, 1
  do
   if g_sprCursorPtrY > g_posButton[i] and g_sprCursorPtrY <= g_posButton[i]+GetImageHeight(g_imgButton[i]) then
    iHighlightButton = i
   end
  end
 end
 for i = 1, GAMEMENU_RESUME, 1
 do
  if iHighlightButton == i then
   SetSpriteImage ( g_sprButton[i], g_imgButtonH[i] )
  else
   SetSpriteImage ( g_sprButton[i], g_imgButton[i] )
  end
 end
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton==GAMEMENU_MAINMENU then
   LeaveGame()
  end
  if iHighlightButton==GAMEMENU_LOADGAME then
   SwitchPage("loadgame")
  end
  if iHighlightButton==GAMEMENU_SAVEGAME then
   SwitchPage("savegame")
  end
  if iHighlightButton==GAMEMENU_GRAPHICS then
   SwitchPage("graphics")
  end
  if iHighlightButton==GAMEMENU_SOUND then
   SwitchPage("sounds")
  end
  if iHighlightButton==GAMEMENU_RESUME then
   ResumeGame()
  end
 end
 -- debug
 --Prompt ( "g_MouseClick=" .. g_MouseClick .. " g_strBestResolution=" .. g_strBestResolution )
end

function gamemenu_free()
 -- free resources
 for i = 1, GAMEMENU_RESUME, 1
 do
  DeleteSprite ( g_sprButton[i] )
  DeleteImage ( g_imgButton[i] )
  DeleteImage ( g_imgButtonH[i] )
 end
 DeleteSprite ( g_sprHeading )
 DeleteImage ( g_imgHeading )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
end
