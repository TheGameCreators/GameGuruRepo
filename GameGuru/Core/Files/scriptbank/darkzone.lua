-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function darkzone_init(e)
end

function darkzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  SetFogNearest(500)
  SetFogDistance(2000)
  SetFogRed(32)
  SetFogGreen(32)
  SetFogBlue(64)
  SetAmbienceIntensity(65)
  SetAmbienceRed(64)
  SetAmbienceGreen(64)
  SetAmbienceBlue(128)
  SetSurfaceRed(64)
  SetSurfaceGreen(64)
  SetSurfaceBlue(128)
  ActivateIfUsed(e)
 end
end
