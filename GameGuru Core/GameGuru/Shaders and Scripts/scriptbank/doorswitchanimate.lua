-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Assumes the entity associated as two animations specified ON and OFF

g_doorswitchanimate = {}

function doorswitchanimate_init(e)
 g_doorswitchanimate[e] = "closed"
end

function doorswitchanimate_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 and g_PlayerHealth > 0 then
  if g_doorswitchanimate[e]=="closed" then
   Prompt("Press E to open remote door" )
   if g_KeyPressE == 1 then
    g_doorswitchanimate[e] = "opening"
	SetAnimation(0)
	PlayAnimation(e)
	ActivateIfUsed(e)
	StartTimer(e)
   end
  end
  if g_doorswitchanimate[e]=="open" then
   Prompt("Press E to close remote door" )
   if g_KeyPressE == 1 then
    g_doorswitchanimate[e] = "closing"
	SetAnimation(1)
	PlayAnimation(e)
	ActivateIfUsed(e)
	StartTimer(e)
   end
  end
 end
 if g_KeyPressE == 0 and GetTimer(e) > 2000 then
  if g_doorswitchanimate[e]=="opening" then
   g_doorswitchanimate[e] = "open"
  end
  if g_doorswitchanimate[e]=="closing" then
   g_doorswitchanimate[e] = "closed"
  end
 end
end
