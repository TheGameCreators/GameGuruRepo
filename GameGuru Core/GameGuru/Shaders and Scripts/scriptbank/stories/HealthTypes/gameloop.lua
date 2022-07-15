-- LUA Script - precede every function and global member with lowercase name of script + '_main'

-- Create your own player health types using global values in your LUA scripts
myPlayerLastRealHealth = 0
myPlayerReadyForReset = 0
myPlayerHealth = 100
myPlayerShield = 200

function gameloop_init(e)
 HideHuds()
end

function gameloop_main(e)
 if g_PlayerHealth > 0 then
  if g_PlayerHealth < myPlayerLastRealHealth then
   thisDifference = myPlayerLastRealHealth - g_PlayerHealth
   if myPlayerShield > 0 then
    myPlayerShield = myPlayerShield - thisDifference
    if myPlayerShield < 0 then 
     myPlayerShield = 0
    end
   else
    if myPlayerHealth > 0 then
     myPlayerHealth = myPlayerHealth - thisDifference 
	 if myPlayerHealth < 0 then
	  myPlayerHealth =  0
	 end
    end
   end
   if myPlayerHealth==0 then
    HurtPlayer(-1,10001)
    myPlayerLastRealHealth = 0
	myPlayerReadyForReset = 1
   else
    SetPlayerHealth(10000)
    myPlayerLastRealHealth = 10000
   end
  else
   myPlayerLastRealHealth = g_PlayerHealth
   if myPlayerReadyForReset==1 then
    myPlayerHealth = 100
    myPlayerShield = 200
    myPlayerReadyForReset = 0
   end
  end
 end
 Prompt ( "My Player Health = " .. myPlayerHealth .. "   My Player Shield = " .. myPlayerShield .. "   Now Using " .. g_PlayerGunName )
end
