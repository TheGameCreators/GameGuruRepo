"F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Media List Builder.exe"
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\changelog.txt" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\changelog.txt" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\version.ini" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\version.ini" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Guru-MapEditor.exe" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\Guru-MapEditor.exe" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\Guru-Lightmapper.exe" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\Guru-Lightmapper.exe" /Y
xcopy "F:\TGCSHARED\fpsc-reloaded\FPS Creator Files\GameGuru.exe" "F:\TGCSHARED\fpsc-reloaded\Reloaded MP\Steamworks SDK\tools\ContentBuilder\content\GameGuru.exe" /Y
builder\steamcmd.exe +login daveravey cabbagexr137 +run_app_build ..\scripts\app_build_266310.vdf
