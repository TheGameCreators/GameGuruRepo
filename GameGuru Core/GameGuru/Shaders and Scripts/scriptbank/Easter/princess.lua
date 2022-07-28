-- LUA Script - precede every function and global member with lowercase name of script + '_main'

princess_rescued=0

function princess_init(e)
end

function princess_main(e)
 if GetPlayerDistance(e) < 100 then
princess_rescued=1
 end
end
