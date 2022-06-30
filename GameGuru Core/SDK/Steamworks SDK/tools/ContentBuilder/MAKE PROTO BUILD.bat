"F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Media List Builder.exe"
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\FPSC-Proto-Game.exe" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\FPSC-Proto-Game.exe" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Files\mapbank\multiplayerlevel.fpm" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\Files\mapbank\multiplayerlevel.fpm" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Files\scriptbank\global.lua" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\Files\scriptbank\global.lua" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Files\scriptbank\multiplayer_first_to_10.lua" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\Files\scriptbank\multiplayer_first_to_10.lua" /Y
builder\steamcmd.exe +login daveravey cabbagexr137 +run_app_build ..\scripts\app_build_266310.vdf
