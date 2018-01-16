-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Story Zone

function storyinzone_init(e)
end

function storyinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlaySound(e,0)
  PlayVideo(e,1)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
