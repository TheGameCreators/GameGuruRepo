-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function movewithsound_init(e)
 CollisionOff(e)
end
function movewithsound_main(e)
 MoveForward(e,300.0)
 LoopSound(e,0)
end
