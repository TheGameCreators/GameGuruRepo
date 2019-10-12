-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Zone

function plrinzone_init(e)
end

function plrinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlaySound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
