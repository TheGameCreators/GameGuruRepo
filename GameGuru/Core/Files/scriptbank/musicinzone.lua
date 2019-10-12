-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Music Zone

function musicinzone_init(e)
end

function musicinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopSound(e,0)
  ActivateIfUsed(e)
 end
end
