-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays sound when all eggs are collected

Egg5 = 0

function egg5collected_sound_init(e)
end

function egg5collected_sound_main(e)
 if g_Egg5Count ~= nil then
  if g_Egg5Count==20 and Egg5==0 then
   PlaySound(e,0)
   Egg5 = 1
   Destroy(e)
  else
   StopSound(e)
  end
 end
end
