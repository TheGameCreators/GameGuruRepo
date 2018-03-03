-- Title page script

TITLE_NEW = 1
TITLE_LOADGAME = 2
TITLE_ABOUT = 3
TITLE_QUIT = 4
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgBackdrop = 0
g_sprBackdrop = 0
g_posBackdropAngle = 0
g_imgButton = {}
g_imgButtonH = {}
g_sprButton = {}
g_posButton = {}
g_imgCursor = 0
g_sprCursor = 0
g_sprCursorPtrX = 50
g_sprCursorPtrY = 33
g_sprCursorPtrClick = 0

g_playtitlemusic = 0

function title_init()
 -- determine style folder we are in
 file = io.open("titlesbank\\style.txt", "r")
 if file ~= nil then
  io.input(file)
  g_strStyleFolder = io.read()
  io.close(file)
 end
 -- read graphics page settings
 file = io.open("savegames\\graphics.dat", "r")
 if file ~= nil then
  io.input(file)
  iGraphicChoice = tonumber(io.read())
  SetGameQuality(iGraphicChoice)
  io.close(file)
 end
 -- read graphics page settings
 file = io.open("savegames\\sounds.dat", "r")
 if file ~= nil then
  io.input(file)
  iSoundChoice = tonumber(io.read())
  iMusicChoice = tonumber(io.read())
  SetGameSoundVolume ( iSoundChoice )
  SetGameMusicVolume ( iMusicChoice )
  io.close(file)
 end
 -- load title music if any
 if g_playtitlemusic == 0 then
  LoadGlobalSound ( "titlesbank\\default\\title.ogg", 1 )
  LoopGlobalSound ( 1 )
  g_playtitlemusic = 1
 end 
 -- choose ideal resolution
 BackdropOff()
 DrawSpritesLast()
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- backdrop
 g_imgBackdrop = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. g_strBestResolution .. "\\title.png")
 g_sprBackdrop = CreateSprite ( g_imgBackdrop )
 SetSpritePosition ( g_sprBackdrop, 0, 0 )
 SetSpriteSize ( g_sprBackdrop, 110, 110 )
 SetSpriteOffset ( g_sprBackdrop, 5, 5 )
 -- buttons
 for i = 1, TITLE_QUIT, 1
 do
  if i == TITLE_NEW then strButtonName = "start"
  elseif i == TITLE_LOADGAME then strButtonName = "load-game"
  elseif i == TITLE_ABOUT then strButtonName = "about"
  elseif i == TITLE_QUIT then strButtonName = "quit-game"
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

function title_main()
 -- control menus
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgButton[1])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgButton[1])/2) then
  for i = 1, TITLE_QUIT, 1
  do
   if g_sprCursorPtrY > g_posButton[i] and g_sprCursorPtrY <= g_posButton[i]+GetImageHeight(g_imgButton[i]) then
    iHighlightButton = i
   end
  end
 end
 for i = 1, TITLE_QUIT, 1
 do
  if iHighlightButton == i then
   SetSpriteImage ( g_sprButton[i], g_imgButtonH[i] )
  else
   SetSpriteImage ( g_sprButton[i], g_imgButton[i] )
  end
 end
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton==TITLE_NEW then
   StopGlobalSound ( 1 )
   SwitchPage("")
  end
  if iHighlightButton==TITLE_LOADGAME then
   SwitchPage("loadgame")
  end
  if iHighlightButton==TITLE_ABOUT then
   SwitchPage("about")
  end
  if iHighlightButton==TITLE_QUIT then
   StopGlobalSound ( 1 )
   QuitGame()
  end
 end
 -- move backdrop
 SetSpritePosition ( g_sprBackdrop, math.cos(g_posBackdropAngle* 0.0174533)*5, math.sin(g_posBackdropAngle*0.0174533)*5 )
 g_posBackdropAngle = g_posBackdropAngle + 0.002
end

function title_free()
 -- free resources
 for i = 1, TITLE_QUIT, 1
 do
  DeleteSprite ( g_sprButton[i] )
  DeleteImage ( g_imgButton[i] )
  DeleteImage ( g_imgButtonH[i] )
 end
 DeleteSprite ( g_sprBackdrop )
 DeleteImage ( g_imgBackdrop )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
end
