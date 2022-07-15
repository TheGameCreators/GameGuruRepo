-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays sound when all eggs are collected

Egg2 = 0

function egg2collected_sound_init(e)
end

function egg2collected_sound_main(e)
 if g_Egg2Count ~= nil then
  if g_Egg2Count==20 and Egg2==0 then
   PlaySound(e,0)
   Egg2 = 1
   Destroy(e)
  else
   StopSound(e)
  end
 end
end
