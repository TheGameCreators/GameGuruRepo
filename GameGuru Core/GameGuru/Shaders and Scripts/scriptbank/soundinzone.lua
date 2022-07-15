-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Sound Zone

function soundinzone_init(e)
end

function soundinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlaySound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
