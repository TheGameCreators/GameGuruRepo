-- LUA Script - precede every function and global member with lowercase name of script

function winzone_init(e)
local winzone_name = ""
end

function winzone_init_name(e,name)
winzone_name = name
end

function winzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
   if (winzone_name == "Win Zone" or g_Entity[e]['activated'] == 1) then
    JumpToLevelIfUsed(e)
   end
 end
end

