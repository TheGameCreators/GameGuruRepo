-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Checkpoint Zone

function checkpoint_init(e)
end

function checkpoint_main(e)
 if g_Entity[e]['plrinzone']==1 then
  Checkpoint(e)
  PlaySound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
