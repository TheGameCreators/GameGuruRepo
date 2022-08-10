-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function transportinzone_init(e)
end

function transportinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlayNon3DSound(e,0)
  TransportToIfUsed(e)
 end
end
