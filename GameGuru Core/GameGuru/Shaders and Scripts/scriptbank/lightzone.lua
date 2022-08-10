-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function lightzone_init(e)
end

function lightzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  SetFogNearest(500)
  SetFogDistance(50000)
  SetFogRed(140)
  SetFogGreen(158)
  SetFogBlue(173)
  SetAmbienceIntensity(65)
  SetAmbienceRed(255)
  SetAmbienceGreen(255)
  SetAmbienceBlue(255)
  SetSurfaceRed(255)
  SetSurfaceGreen(255)
  SetSurfaceBlue(255)
  ActivateIfUsed(e)
 end
end
