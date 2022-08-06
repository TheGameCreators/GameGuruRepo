-- LUA Script - precede every function and global member with lowercase name of script + '_main'
musicswapinzone_onlyplayonesound = 0
musicswapinzone_volumecontrol = 0

function musicswapinzone_init(e)
end
function musicswapinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopSound(e,0)
  ActivateIfUsed(e)
  if musicswapinzone_onlyplayonesound ~= e then
   musicswapinzone_onlyplayonesound = e
   musicswapinzone_volumecontrol = 750
  end
 else
  if musicswapinzone_onlyplayonesound > 0 and musicswapinzone_onlyplayonesound ~= e then
   StopSound(e,0)
  end
 end
 if musicswapinzone_volumecontrol > 0 and musicswapinzone_volumecontrol < 1000 then
  musicswapinzone_volumecontrol = musicswapinzone_volumecontrol + 1
  if musicswapinzone_volumecontrol > 1000 then
   musicswapinzone_volumecontrol = 1000
  end
  SetSoundVolume ( musicswapinzone_volumecontrol/10 )
 end
end
