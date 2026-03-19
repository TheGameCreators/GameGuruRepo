-- LUA Script - precede every function and global member with lowercase name of script + '_main'
--by T-Bone

function lutzone_init(e)

--Starting LUT
SetLutTo("none")
end

function lutzone_init_name(e,name)
 end

function lutzone_main(e)
 if g_Entity[e]['plrinzone'] == 1  then

--Change LUT to
SetLutTo("ps_night1")

Destroy(e)
 end
end
