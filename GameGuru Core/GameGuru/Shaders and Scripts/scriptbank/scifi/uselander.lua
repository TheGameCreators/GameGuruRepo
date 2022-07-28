-- LUA Script - precede every function and global member with lowercase name of script + '_main'
uselander_endsequence = 0

function uselander_init(e)
end
function uselander_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 200 and g_PlayerHealth > 0 and uselander_endsequence == 0 then
  Prompt ( "Press E to transport ship to crash site and rescue spaceman" )
  if g_KeyPressE == 1 then
   Hide(e)
   PlayNon3DSound(e,0)
   TransportToIfUsed(e)
   uselander_endsequence = 1
  end
 end
 if uselander_endsequence > 0 and uselander_endsequence < 2000 then
  uselander_endsequence = uselander_endsequence + 1
  if uselander_endsequence == 10 then
   --FreezePlayer()
  end
  if uselander_endsequence > 500 then
   FinishLevel()
  end
  if uselander_endsequence > 600 then
   --UnFreezePlayer()
   uselander_endsequence = 2000
  end
 end
end