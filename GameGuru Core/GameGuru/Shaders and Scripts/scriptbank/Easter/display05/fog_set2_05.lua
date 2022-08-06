-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function fog_set2_05_init(e)
end

function fog_set2_05_main(e)
 if defeated == 16 and skydome == 1 then
  skydome = 0
  PlaySound(e)
  SetFlashLightKeyEnabled(0)
  SetFlashLight(0)
  SetFogNearest(20)
  SetFogDistance(5000)
  SetFogRed(132)
  SetFogGreen(184)
  SetFogBlue(206)
  SetAmbienceIntensity(65)
  SetAmbienceRed(255)
  SetAmbienceGreen(255)
  SetAmbienceBlue(255)
  SetSurfaceRed(255)
  SetSurfaceGreen(255)
  SetSurfaceBlue(255)
  ActivateIfUsed(e)
  Destroy(e)
 end
end
