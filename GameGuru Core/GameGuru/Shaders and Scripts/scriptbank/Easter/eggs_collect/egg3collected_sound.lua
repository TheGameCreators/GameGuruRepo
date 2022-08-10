-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays sound when all eggs are collected

Egg3 = 0

function egg3collected_sound_init(e)
end

function egg3collected_sound_main(e)
 if g_Egg3Count ~= nil then
  if g_Egg3Count==20 and Egg3==0 then
   PlaySound(e,0)
   Egg2 = 3
   Destroy(e)
  else
   StopSound(e)
  end
 end
end
