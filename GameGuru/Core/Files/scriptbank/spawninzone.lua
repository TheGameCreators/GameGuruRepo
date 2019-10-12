-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function spawninzone_init(e)
end

function spawninzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlaySound(e,0)
  SpawnIfUsed(e)
 end
end
