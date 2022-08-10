-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function particles_init(e)
end

function particlesinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  StartParticleEmitter(e)
  ActivateIfUsed(e)
 else
  StopParticleEmitter(e)
 end
end
