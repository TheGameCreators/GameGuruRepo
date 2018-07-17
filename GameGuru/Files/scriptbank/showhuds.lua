-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function showhuds_init(e)
end

function showhuds_main(e)
 if g_Entity[e]['plrinzone']==1 then
  ShowHuds()
  Destroy(e)
 end
end
