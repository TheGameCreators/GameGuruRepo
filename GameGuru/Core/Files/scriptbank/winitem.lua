-- LUA Script - precede every function and global member with lowercase name of script

function winitem_init(e)
end

function winitem_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
  PlaySound(e,0)
  JumpToLevelIfUsed(e)
  Destroy(e)
  ActivateIfUsed(e)
 end
end

