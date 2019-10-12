-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function decal_facing_init(e)
end

function decal_facing_main(e)
 RotateToCamera(e)
 if g_Entity[e]['activated'] == 1 then
  Destroy(e)
 end
end
