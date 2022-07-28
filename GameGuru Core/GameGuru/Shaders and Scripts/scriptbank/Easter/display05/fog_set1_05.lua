-- LUA Script - precede every function and global member with lowercase name of script + '_main'

skydome = 0

function fog_set1_05_init(e)
end

function fog_set1_05_main(e)
 if g_Entity[e]['plrinzone']==1 then
  skydome = 1
  SetFlashLightKeyEnabled(1)
  SetFlashLight(1)
  SetFogNearest(10)
  SetFogDistance(1000)
  SetFogRed(32)
  SetFogGreen(32)
  SetFogBlue(40)
  SetAmbienceIntensity(65)
  SetAmbienceRed(64)
  SetAmbienceGreen(64)
  SetAmbienceBlue(90)
  SetSurfaceRed(64)
  SetSurfaceGreen(64)
  SetSurfaceBlue(90)
  ActivateIfUsed(e)
  Destroy(e)
 end
end
