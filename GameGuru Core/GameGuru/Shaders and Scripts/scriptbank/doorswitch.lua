-- LUA Script - precede every function and global member with lowercase name of script + '_main'

g_doorswitch = {}

function doorswitch_init(e)
 g_doorswitch[e] = "closed"
end

function doorswitch_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 and g_PlayerHealth > 0 then
  if g_doorswitch[e]=="closed" then
   Prompt("Press E to open remote door" )
   if g_KeyPressE == 1 then
    g_doorswitch[e] = "opening"
	ActivateIfUsed(e)
	StartTimer(e)
   end
  end
  if g_doorswitch[e]=="open" then
   Prompt("Press E to close remote door" )
   if g_KeyPressE == 1 then
    g_doorswitch[e] = "closing"
	ActivateIfUsed(e)
	StartTimer(e)
   end
  end
 end
 if g_KeyPressE == 0 and GetTimer(e) > 2000 then
  if g_doorswitch[e]=="opening" then
   g_doorswitch[e] = "open"
  end
  if g_doorswitch[e]=="closing" then
   g_doorswitch[e] = "closed"
  end
 end
end
