-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function alientransportinzone_init(e)
end

function alientransportinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlayNon3DSound(e,0)
  TransportToIfUsed(e)
 end
end
