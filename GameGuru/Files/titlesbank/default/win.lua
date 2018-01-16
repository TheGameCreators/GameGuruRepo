-- Win page script

GAME_CONTINUE = 1
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

function win_init()
 -- determine style folder we are in
 file = io.open("titlesbank\\style.txt", "r")
 if file ~= nil then
  io.input(file)
  g_strStyleFolder = io.read()
  io.close(file)
 end
 -- choose ideal resolution
 resolutions = require "titlesbank\\resolutions"
 g_strBestResolution = resolutions.findclosest()
 -- backdrop
 g_imgBackdrop = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. g_strBestResolution .. "\\end.png")
 g_sprBackdrop = CreateSprite ( g_imgBackdrop )
 SetSpritePosition ( g_sprBackdrop, 0, 0 )
 SetSpriteSize ( g_sprBackdrop, 110, 110 )
 SetSpriteOffset ( g_sprBackdrop, 5, 5 )
 -- buttons
 strButtonName = "continue"
 g_imgButton[GAME_CONTINUE] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
 g_imgButtonH[GAME_CONTINUE] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-hover.png")
 g_sprButton[GAME_CONTINUE] = CreateSprite ( g_imgButton[GAME_CONTINUE] ) 
 SetSpriteOffset ( g_sprButton[GAME_CONTINUE], GetImageWidth(g_imgButton[GAME_CONTINUE])/2, 0 )
 g_posButton[GAME_CONTINUE] = 80
 SetSpritePosition ( g_sprButton[GAME_CONTINUE], 50, g_posButton[GAME_CONTINUE] )
 -- cursor
 g_imgCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprCursor = CreateSprite ( g_imgCursor ) 
 DrawSpritesLast()
 ActivateMouse()
end

function win_main()
 -- control menus
 SetSpritePosition ( g_sprCursor, g_MouseX, g_MouseY )
 iHighlightButton = 0
 if g_MouseX > 50-(GetImageWidth(g_imgButton[GAME_CONTINUE])/2) and g_MouseX <= 50+(GetImageWidth(g_imgButton[GAME_CONTINUE])/2) then
  if g_MouseY > g_posButton[GAME_CONTINUE] and g_MouseY <= g_posButton[GAME_CONTINUE]+GetImageHeight(g_imgButton[GAME_CONTINUE]) then
   iHighlightButton = GAME_CONTINUE
  end
 end
 if iHighlightButton == GAME_CONTINUE then
  SetSpriteImage ( g_sprButton[GAME_CONTINUE], g_imgButtonH[GAME_CONTINUE] )
 else
  SetSpriteImage ( g_sprButton[GAME_CONTINUE], g_imgButton[GAME_CONTINUE] )
 end
 if g_MouseClick == 1 then
  if iHighlightButton==GAME_CONTINUE then
   SwitchPage("")
  end
 end
 -- move backdrop
 SetSpritePosition ( g_sprBackdrop, math.cos(g_posBackdropAngle* 0.0174533)*5, math.sin(g_posBackdropAngle*0.0174533)*5 )
 g_posBackdropAngle = g_posBackdropAngle + 0.002
end

function win_free()
 -- free resources
 DeleteSprite ( g_sprButton[GAME_CONTINUE] )
 DeleteImage ( g_imgButton[GAME_CONTINUE] )
 DeleteImage ( g_imgButtonH[GAME_CONTINUE] )
 DeleteSprite ( g_sprBackdrop )
 DeleteImage ( g_imgBackdrop )
 DeleteSprite ( g_sprCursor )
 DeleteImage ( g_imgCursor )
 DeactivateMouse()
end
