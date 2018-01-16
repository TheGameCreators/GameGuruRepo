-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function jetpack_init(e)
end

function jetpack_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
	SourceAngle = g_PlayerAngY
	while SourceAngle < 0.0 do
		SourceAngle = SourceAngle + 360.0
	end	
	while SourceAngle > 340.0 do
		SourceAngle = SourceAngle - 360.0
	end
    PlayerDX = (g_Entity[e]['x'] - g_PlayerPosX)
    PlayerDZ = (g_Entity[e]['z'] - g_PlayerPosZ)
	DestAngle = math.atan2( PlayerDZ , PlayerDX )
	DestAngle = (DestAngle * 57.2957795) - 90.0
	Result = math.abs(math.abs(SourceAngle)-math.abs(DestAngle))
	if Result > 180 then
		Result = 0
	end	
	if Result < 20.0 then
	    if GetGamePlayerStateXBOX() == 1 then
         PromptLocalForVR(e,"Press Y button to pick up the jetpack")
	    else
		 PromptLocalForVR(e,"Press E To pick up the jetpack")
		end
		if g_KeyPressE == 1 then
		   PromptLocalForVR(e,"Collected the jetpack")
		   PlaySound(e,0)
		   AddPlayerJetPack(e,500)
		   Destroy(e)
		   ActivateIfUsed(e)
		end
   end
 end
end
