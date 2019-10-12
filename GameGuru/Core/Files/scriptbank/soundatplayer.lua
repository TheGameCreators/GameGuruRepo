-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Sound Zone

function soundatplayer_init(e)
end

function soundatplayer_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlayNon3DSound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
