-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Jet Pack Fuel Tank



function jetpackfuel_init(e)

jetpackfuel = 0

end

function jetpackfuel_main(e)
--Prompt ("Jetpack Fuel: "..jetpackfuel)
--jetpackfuel = GetGamePlayerControlJetpackFuel ( ) 
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 60 and g_PlayerHealth > 0 and g_PlayerThirdPerson == 0 then
   Prompt("Collected jetpack fuel")
   PlaySound(e,0)
   jetpackfuel = GetGamePlayerControlJetpackFuel ()
   SetGamePlayerControlJetpackFuel ( jetpackfuel + 500 )
   Destroy(e)
   ActivateIfUsed(e)
 end
end
