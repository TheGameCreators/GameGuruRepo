-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function poison_dust_init(e)
end

function poison_dust_main(e)
 RotateToCamera(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 250  then
  Show(e)
  LoopSound(e,0)
  else
  StopSound(e,0)
  Hide(e)
 end
if g_Entity[e]['activated'] == 1 then
StopSound(e,0)
Destroy(e)
end
end
