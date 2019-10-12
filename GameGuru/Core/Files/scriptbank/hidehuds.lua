-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function hidehuds_init(e)
end

function hidehuds_main(e)
 if g_Entity[e]['plrinzone']==1 then
  HideHuds()
  Destroy(e)
 end
end
