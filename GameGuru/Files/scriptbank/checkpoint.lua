-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Checkpoint Zone

function checkpoint_init(e)
end

function checkpoint_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlaySound(e,0)
  Checkpoint(e)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
