-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function portal_init(e)
end

function portal_main(e)
 if GetPlayerDistance(e) < 45 then
  PlaySound(e,0)
  TransportToIfUsed(e)
 end
end
