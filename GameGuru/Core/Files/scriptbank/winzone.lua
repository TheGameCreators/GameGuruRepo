-- LUA Script - precede every function and global member with lowercase name of script

function winzone_init(e)
end

function winzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  JumpToLevelIfUsed(e)
 end
end

