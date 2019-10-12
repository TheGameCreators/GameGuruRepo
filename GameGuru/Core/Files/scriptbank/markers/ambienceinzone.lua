-- LUA Script - precede every function and global member with lowercase name of script + '_main'

g_ambienceinzone_soundlooping = 0

function ambienceinzone_init(e)
end

function ambienceinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if g_ambienceinzone_soundlooping ~= e then
   StopSound(g_ambienceinzone_soundlooping,0)
   LoopNon3DSound(e,0)
   ActivateIfUsed(e)
   g_ambienceinzone_soundlooping = e
  end
 end
end
