-- Sounds page script

SOUNDS_SLABEL = 1
SOUNDS_SOUND = 2
SOUNDS_MLABEL = 3
SOUNDS_MUSIC = 4
SOUNDS_BACK = 5
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgHeading = 0
g_sprHeading = 0
g_imgButton = {}
g_imgButtonH = {}
g_sprButton = {}
g_posButton = {}
g_sprSliderS = 0
g_sprSliderM = 0
g_imgCursor = 0
g_sprCursor = 0
g_iSoundChoice = 100
g_iMusicChoice = 100
g_sprCursorPtrX = 50
g_sprCursorPtrY = 33
g_sprCursorPtrClick = 0

function sounds_init()
 -- determine style folder we are in
 file = io.open("titlesbank\\style.txt", "r")
 if file ~= nil then
  io.input(file)
  g_strStyleFolder = io.read()
  io.close(file)
 end
 -- read graphics page settings
 file = io.open("savegames\\sounds.dat", "r")
 if file ~= nil then
  io.input(file)
  g_iSoundChoice = tonumber(io.read())
  g_iMusicChoice = tonumber(io.read())
  io.close(file)
 end
 SetGameSoundVolume ( g_iSoundChoice )
 SetGameMusicVolume ( g_iMusicChoice )
 -- choose ideal resolution
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- heading
 g_imgHeading = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\sound-levels-title.png")
 g_sprHeading = CreateSprite ( g_imgHeading ) 
 SetSpriteOffset ( g_sprHeading, GetImageWidth(g_imgHeading)/2, 0 )
 SetSpritePosition ( g_sprHeading, 50, 8 )
 -- buttons
 for i = 1, SOUNDS_BACK, 1
 do
  if i == SOUNDS_SLABEL then strButtonName = "sound-effects-label"
  elseif i == SOUNDS_SOUND then strButtonName = "slider-bar-empty"
  elseif i == SOUNDS_MLABEL then strButtonName = "music-label"
  elseif i == SOUNDS_MUSIC then strButtonName = "slider-bar-empty"
  elseif i == SOUNDS_BACK then strButtonName = "back"
  else strButtonName = ""
  end	
  g_imgButton[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
  if i == SOUNDS_SOUND or i == SOUNDS_MUSIC then
   g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\slider-bar-full.png")
  else
   if i == SOUNDS_BACK then
    g_imgButtonH[i] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\back-hover.png")
   else 
    g_imgButtonH[i] = -1
   end
  end
  g_sprButton[i] = CreateSprite ( g_imgButton[i] ) 
  SetSpriteOffset ( g_sprButton[i], GetImageWidth(g_imgButton[i])/2, 0 )
  g_posButton[i] = 20+(i*10)
  SetSpritePosition ( g_sprButton[i], 50, g_posButton[i] )
 end
 -- slider overlays
 g_sprSliderS = CreateSprite ( g_imgButtonH[SOUNDS_SOUND] ) 
 SetSpriteOffset ( g_sprSliderS, GetImageWidth(g_imgButton[SOUNDS_SOUND])/2, 0 )
 SetSpritePosition ( g_sprSliderS, 50, g_posButton[SOUNDS_SOUND] )
 g_sprSliderM = CreateSprite ( g_imgButtonH[SOUNDS_MUSIC] ) 
 SetSpriteOffset ( g_sprSliderM, GetImageWidth(g_imgButton[SOUNDS_MUSIC])/2, 0 )
 SetSpritePosition ( g_sprSliderM, 50, g_posButton[SOUNDS_MUSIC] )
 -- cursor
 g_imgCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprCursor = CreateSprite ( g_imgCursor ) 
 ActivateMouse()
end

function sounds_main()
 -- control cursor pos
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 -- find highlighted button
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgButton[1])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgButton[1])/2) then
  for i = 1, SOUNDS_BACK, 1
  do
   if g_sprCursorPtrY > g_posButton[i] and g_sprCursorPtrY <= g_posButton[i]+GetImageHeight(g_imgButton[i]) then
    iHighlightButton = i
   end
  end
 end
 -- highlight back button
 if iHighlightButton == SOUNDS_BACK then
  SetSpriteImage ( g_sprButton[SOUNDS_BACK], g_imgButtonH[SOUNDS_BACK] )
 else
  SetSpriteImage ( g_sprButton[SOUNDS_BACK], g_imgButton[SOUNDS_BACK] )
 end
 -- control slider button
 iPercentageWidth = GetImageWidth(g_imgButton[1])
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton==SOUNDS_SOUND then
   g_iSoundChoice = ((g_sprCursorPtrX - (50-(iPercentageWidth/2)))/iPercentageWidth)*100
   if g_iSoundChoice < 0 then 
    g_iSoundChoice = 0
   end
   if g_iSoundChoice > 100 then 
    g_iSoundChoice = 100
   end
   SetGameSoundVolume ( g_iSoundChoice )
  end
  if iHighlightButton==SOUNDS_MUSIC then
   g_iMusicChoice = ((g_sprCursorPtrX - (50-(iPercentageWidth/2)))/iPercentageWidth)*100
   if g_iMusicChoice < 0 then 
    g_iMusicChoice = 0
   end
   if g_iMusicChoice > 100 then 
    g_iMusicChoice = 100
   end
   SetGameMusicVolume ( g_iMusicChoice )
  end
  if iHighlightButton==SOUNDS_BACK then
   SwitchPageBack()
  end
 end
 SetSpriteSize ( g_sprSliderS, (g_iSoundChoice/100)*iPercentageWidth, GetImageHeight(g_imgButtonH[SOUNDS_SOUND]) )
 SetSpriteSize ( g_sprSliderM, (g_iMusicChoice/100)*iPercentageWidth, GetImageHeight(g_imgButtonH[SOUNDS_MUSIC]) )
end

function sounds_free()
 -- write sound page settings
 file = io.open("savegames\\sounds.dat", "w")
 io.output(file)
 io.write(g_iSoundChoice .. "\n")
 io.write(g_iMusicChoice .. "\n")
 io.close(file)  
 -- free resources
 for i = 1, SOUNDS_BACK, 1
 do
  DeleteSprite ( g_sprButton[i] )
  DeleteImage ( g_imgButton[i] )
  if g_imgButtonH[i] ~= -1 then
   DeleteImage ( g_imgButtonH[i] )
  end
 end
 DeleteSprite ( g_sprSliderS )
 DeleteSprite ( g_sprSliderM )
 DeleteSprite ( g_sprHeading )
 DeleteImage ( g_imgHeading )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
end
