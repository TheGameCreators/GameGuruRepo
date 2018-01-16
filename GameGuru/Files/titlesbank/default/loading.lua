-- Loading page script

g_strStyleFolder = ""
g_strBestResolution = ""
g_imgBackdrop = 0
g_sprBackdrop = 0
g_imgHeading = 0
g_sprHeading = 0
g_imgProgressB = 0
g_sprProgressB = 0
g_imgProgressF = 0
g_sprProgressF = 0
g_iLoadingCountdown = 0
g_iGraphicChoice = 3

function loading_init()
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
 g_imgBackdrop = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\" .. g_strBestResolution .. "\\loading.png")
 g_sprBackdrop = CreateSprite ( g_imgBackdrop )
 SetSpritePosition ( g_sprBackdrop, 0, 0 )
 SetSpriteSize ( g_sprBackdrop, 100, 100 )
 -- loading header and progress
 g_imgHeading = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\loading-text.png")
 g_sprHeading = CreateSprite ( g_imgHeading ) 
 SetSpritePosition ( g_sprHeading, 50, 80 )
 SetSpriteOffset ( g_sprHeading, GetImageWidth(g_imgHeading)/2, 0 )
 g_imgProgressB = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\loading-bar-empty.png")
 g_sprProgressB = CreateSprite ( g_imgProgressB ) 
 g_imgProgressF = LoadImage("titlesbank\\" .. g_strStyleFolder .. "\\loading-bar-full.png")
 g_sprProgressF = CreateSprite ( g_imgProgressF ) 
 SetSpritePosition ( g_sprProgressB, 50, 90 )
 SetSpriteOffset ( g_sprProgressB, GetImageWidth(g_imgProgressB)/2, 0 )
 SetSpritePosition ( g_sprProgressF, 50, 90 )
 SetSpriteOffset ( g_sprProgressF, GetImageWidth(g_imgProgressB)/2, 0 )
 -- loading page logic
 g_iLoadingCountdown = 10
end

function loading_main()
 -- loading page loop 
 g_iLoadingCountdown = g_iLoadingCountdown - 1
 if g_iLoadingCountdown <= 0 then
  -- Loading Resource specifies sprites used outside of LUA system - all sprites are force drawn in reverse order
  SetLoadingResource(0,4) -- number of sprites up to 63
  SetLoadingResource(1,g_sprProgressF) -- sprite size width controlled by progress percentage (always [1])
  SetLoadingResource(2,g_sprProgressB) -- used as a guide to the total width of progress at 100% (always [2])
  SetLoadingResource(3,g_sprHeading) -- the LOADING text
  SetLoadingResource(4,g_sprBackdrop) -- the overwriting backdrop to hide 3D elements
  -- read graphics settings
  file = io.open("savegames\\graphics.dat", "r")
  if file ~= nil then
   io.input(file)
   g_iGraphicChoice = tonumber(io.read())
   io.close(file)
  else
   g_iGraphicChoice = 3
  end
  SetGameQuality(g_iGraphicChoice)
  StartGame()
 end
end

function loading_free()
 -- resources specified above deleted in engine
 -- as sprites need to survive deletion of this LUA script
end
