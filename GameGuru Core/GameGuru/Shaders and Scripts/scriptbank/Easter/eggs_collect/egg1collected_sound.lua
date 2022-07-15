-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays sound when all eggs are collected

Egg1 = 0

function egg1collected_sound_init(e)
end

function egg1collected_sound_main(e)
 if g_Egg1Count ~= nil then
  if g_Egg1Count==20 and Egg1==0 then
   PlaySound(e,0)
   Egg1 = 1
   Destroy(e)
  else
   StopSound(e)
  end
 end
end
