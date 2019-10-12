-- About page script

ABOUT_BACK = 1
g_strStyleFolder = ""
g_strBestResolution = ""
g_imgAboutBackdrop = 0
g_sprAboutBackdrop = 0
g_posAboutBackdropAngle = 0
g_imgAboutButton = {}
g_imgAboutButtonH = {}
g_sprAboutButton = {}
g_posAboutButton = {}
g_imgAboutCursor = 0
g_sprAboutCursor = 0
g_sprCursorPtrX = 50
g_sprCursorPtrY = 33
g_sprCursorPtrClick = 0

function about_init()
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
 g_imgAboutBackdrop = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. g_strBestResolution .. "\\about.png")
 g_sprAboutBackdrop = CreateSprite ( g_imgAboutBackdrop )
 SetSpritePosition ( g_sprAboutBackdrop, 0, 0 )
 SetSpriteSize ( g_sprAboutBackdrop, 110, 110 )
 SetSpriteOffset ( g_sprAboutBackdrop, 5, 5 )
 -- buttons
 strButtonName = "back"
 g_imgAboutButton[ABOUT_BACK] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. ".png")
 g_imgAboutButtonH[ABOUT_BACK] = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. strButtonName .. "-hover.png")
 g_sprAboutButton[ABOUT_BACK] = CreateSprite ( g_imgAboutButton[ABOUT_BACK] ) 
 SetSpriteOffset ( g_sprAboutButton[ABOUT_BACK], GetImageWidth(g_imgAboutButton[ABOUT_BACK])/2, 0 )
 g_posAboutButton[ABOUT_BACK] = 80
 SetSpritePosition ( g_sprAboutButton[ABOUT_BACK], 50, g_posAboutButton[ABOUT_BACK] )
 -- cursor
 g_imgAboutCursor = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\cursor.png")
 g_sprAboutCursor = CreateSprite ( g_imgAboutCursor ) 
 ActivateMouse()
end

function about_main()
 -- control menus
 cursorControl = require "titlesbank\\cursorcontrol"
 g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick = cursorControl.getinput(g_sprCursorPtrX,g_sprCursorPtrY,g_sprCursorPtrClick)
 SetSpritePosition ( g_sprAboutCursor, g_sprCursorPtrX, g_sprCursorPtrY )
 iHighlightButton = 0
 if g_sprCursorPtrX > 50-(GetImageWidth(g_imgAboutButton[ABOUT_BACK])/2) and g_sprCursorPtrX <= 50+(GetImageWidth(g_imgAboutButton[ABOUT_BACK])/2) then
  if g_sprCursorPtrY > g_posAboutButton[ABOUT_BACK] and g_sprCursorPtrY <= g_posAboutButton[ABOUT_BACK]+GetImageHeight(g_imgAboutButton[ABOUT_BACK]) then
   iHighlightButton = ABOUT_BACK
  end
 end
 if iHighlightButton == ABOUT_BACK then
  SetSpriteImage ( g_sprAboutButton[ABOUT_BACK], g_imgAboutButtonH[ABOUT_BACK] )
 else
  SetSpriteImage ( g_sprAboutButton[ABOUT_BACK], g_imgAboutButton[ABOUT_BACK] )
 end
 if g_sprCursorPtrClick == 1 then
  if iHighlightButton==ABOUT_BACK then
   SwitchPageBack()
  end
 end
 -- move backdrop
 SetSpritePosition ( g_sprAboutBackdrop, math.cos(g_posAboutBackdropAngle* 0.0174533)*5, math.sin(g_posAboutBackdropAngle*0.0174533)*5 )
 g_posAboutBackdropAngle = g_posAboutBackdropAngle + 0.002
end

function about_free()
 -- free resources
 DeleteSprite ( g_sprAboutButton[ABOUT_BACK] )
 DeleteImage ( g_imgAboutButton[ABOUT_BACK] )
 DeleteImage ( g_imgAboutButtonH[ABOUT_BACK] )
 DeleteSprite ( g_sprAboutBackdrop )
 DeleteImage ( g_imgAboutBackdrop )
 DeleteSprite ( g_sprAboutCursor )
 DeleteImage ( g_imgAboutCursor )
 DeactivateMouse()
end
