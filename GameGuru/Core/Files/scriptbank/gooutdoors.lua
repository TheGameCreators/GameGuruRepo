-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function gooutdoors_init(e)
end

function gooutdoors_main(e)
 if g_Entity[e]['plrinzone']==1 then
  ShowTerrain()
  ShowWater()
  ActivateIfUsed(e)
 end
end
