-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function goindoors_init(e)
end

function goindoors_main(e)
 if g_Entity[e]['plrinzone']==1 then
  HideTerrain()
  HideWater()
  ActivateIfUsed(e)
 end
end
