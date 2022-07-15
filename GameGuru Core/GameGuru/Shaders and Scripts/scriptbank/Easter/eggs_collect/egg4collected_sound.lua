-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays sound when all eggs are collected

Egg4 = 0

function egg4collected_sound_init(e)
end

function egg4collected_sound_main(e)
 if g_Egg4Count ~= nil then
  if g_Egg4Count==20 and Egg4==0 then
   PlaySound(e,0)
   Egg4 = 1
   Destroy(e)
  else
   StopSound(e)
  end
 end
end
