-- Graphics page script

GRAPHICS_LOW = 1
GRAPHICS_MEDIUM = 2
GRAPHICS_HIGH = 3
GRAPHICS_FOV = 4
GRAPHICS_FOVSLIDER = 5
GRAPHICS_BACK = 6
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgHeading = 0
g_sprHeading = 0
g_imgButton = {}
g_imgButtonH = {}
g_imgButtonS = {}
g_sprButton = {}
g_posButton = {}
g_sprSlider = 0
g_imgCursor = 0
g_sprCursor = 0
g_iGraphicChoice = 2
g_sprCursorPtrX = 50
g_sprCursorPtrY = 90
g_sprCursorPtrClick = 0

function graphics_init()
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
  g_iGraphicChoice = tonumber(io.read())
  io.close(file)
 end
 SetGameQuality(g_iGraphicChoice)
 SetPlayerFOV(g_PlayerFOV)
 -- choose ideal resolution
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- heading
 g_imgHeading = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\graphics-settings-title.png")
 g_sprHeading = CreateSprite ( g_imgHeading ) 
 SetSpriteOffset ( g_sprHeading, GetImageWidth(g_imgHeading)/2, 0 )
 SetSpritePosition ( g_sprHeading, 50, 8 )
 -- buttons
 for i = 1, GRAPHICS_BACK, 1
 do
  if i == GRAPHICS_LOW then strButtonName = "lowest"
  elseif i == GRAPHICS_MEDIUM then strButtonName = "medium"
  elseif i == GRAPHICS_HIGH then strButtonName = "highest"
  elseif i == GRAPHICS_FOV then strButtonName = "fov"
  elseif i == GRAPHICS_FOVSLIDER then strButtonName = "slider-bar-empty"
  elseif i == GRAPHICS_BACK then strButtonName = "back"
  else strButtonName = ""
  end	
  g_imgButton[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
  if i == GRAPHICS_FOV or i == GRAPHICS_FOVSLIDER then
   -- ignore hover feature
   g_imgButtonH[i] = -1
  else
   g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-hover.png")
  end
  if i == GRAPHICS_LOW or i == GRAPHICS_MEDIUM or i == GRAPHICS_HIGH then
   g_imgButtonS[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-selected.png")
  else
   g_imgButtonS[i] = -1
  end
  if i == GRAPHICS_FOVSLIDER then
   g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\slider-bar-full.png")
  end
  if i == g_iGraphicChoice then
   g_sprButton[i] = CreateSprite ( g_imgButtonS[i] ) 
  else
   g_sprButton[i] = CreateSprite ( g_imgButton[i] ) 
  end
  SetSpriteOffset ( g_sprButton[i], GetImageWidth(g_imgButton[i])/2, 0 )
  g_posButton[i] = 20+(i*10)
  SetSpritePosition ( g_sprButton[i], 50, g_posButton[i] )
 end
 -- slider overlay
 g_sprSlider = CreateSprite ( g_imgButtonH[GRAPHICS_FOVSLIDER] ) 
 SetSpriteOffset ( g_sprSlider, GetImageWidth(g_imgButton[GRAPHICS_FOVSLIDER])/2, 0 )
 SetSpritePosition ( g_sprSlider, 50, g_posButton[GRAPHICS_FOVSLIDER] )
 -- cursor
 g_imgCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprCursor = CreateSprite ( g_imgCursor ) 
 ActivateMouse()
end

function graphics_main()
 -- control menus
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgButton[1])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgButton[1])/2) then
  for i = 1, GRAPHICS_BACK, 1
  do
   if g_sprCursorPtrY > g_posButton[i] and g_sprCursorPtrY <= g_posButton[i]+GetImageHeight(g_imgButton[i]) then
    iHighlightButton = i
   end
  end
 end
 for i = 1, GRAPHICS_BACK, 1
 do
  if i == g_iGraphicChoice then
   SetSpriteImage ( g_sprButton[i], g_imgButtonS[i] )
  else
   if i == GRAPHICS_FOVSLIDER then
    SetSpriteImage ( g_sprButton[i], g_imgButton[i] )
   else
    if iHighlightButton == i and g_imgButtonH[i] ~= -1 then
     SetSpriteImage ( g_sprButton[i], g_imgButtonH[i] )
    else
     SetSpriteImage ( g_sprButton[i], g_imgButton[i] )
    end
   end
  end
 end
 iPercentageWidth = GetImageWidth(g_imgButton[1])
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton==GRAPHICS_LOW then
   g_iGraphicChoice = 1
   SetGameQuality(g_iGraphicChoice)
  end
  if iHighlightButton==GRAPHICS_MEDIUM then
   g_iGraphicChoice = 2
   SetGameQuality(g_iGraphicChoice)
  end
  if iHighlightButton==GRAPHICS_HIGH then
   g_iGraphicChoice = 3
   SetGameQuality(g_iGraphicChoice)
  end
  if iHighlightButton==GRAPHICS_FOVSLIDER then
   g_PlayerFOV = ((g_sprCursorPtrX - (50-(iPercentageWidth/2)))/iPercentageWidth)*100
   if g_PlayerFOV < 0 then 
    g_PlayerFOV = 0
   end
   if g_PlayerFOV > 100 then 
    g_PlayerFOV = 100
   end
   SetPlayerFOV(g_PlayerFOV)
  end
  if iHighlightButton==GRAPHICS_BACK then
   SwitchPageBack()
  end
 end
 SetSpriteSize ( g_sprSlider, (g_PlayerFOV/100)*iPercentageWidth, GetImageHeight(g_imgButtonH[GRAPHICS_FOVSLIDER]) )
end

function graphics_free()
 -- write graphics page settings
 file = io.open("savegames\\graphics.dat", "w")
 io.output(file)
 io.write(g_iGraphicChoice .. "\n")
 io.close(file)  
 -- free resources
 for i = 1, GRAPHICS_BACK, 1
 do
  DeleteSprite ( g_sprButton[i] )
  DeleteImage ( g_imgButton[i] )
  if g_imgButtonH[i] ~= -1 then
   DeleteImage ( g_imgButtonH[i] )
  end
  if g_imgButtonS[i] ~= -1 then
   DeleteImage ( g_imgButtonS[i] )
  end
 end
 DeleteSprite ( g_sprSlider )
 DeleteSprite ( g_sprHeading )
 DeleteImage ( g_imgHeading )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
end
