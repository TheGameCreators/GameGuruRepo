-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Weapon

weapon_therecanbeonlyone = 0

function weapon_init_name(e,name)
	weapon_name[e] = name
end

function weapon_main(e)
 if weapon_therecanbeonlyone==-1 then
	if g_KeyPressE == 0 and g_InKey == ""  then
		weapon_therecanbeonlyone = 0
	end
 end
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 and g_PlayerHealth > 0 and g_PlayerThirdPerson==0 then

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
	-- Convert to degrees
	DestAngle = (DestAngle * 57.2957795) - 90.0
	
	Result = math.abs(math.abs(SourceAngle)-math.abs(DestAngle))
	if Result > 180 then
		Result = 0
	end	
	
	if Result < 20.0 and weapon_therecanbeonlyone==0 then
		weapon_therecanbeonlyone = e
	end
	if Result >= 20.0 and weapon_therecanbeonlyone==e then
		weapon_therecanbeonlyone = 0
	end
	
	if Result < 20.0 and weapon_therecanbeonlyone==e then
		if g_PlayerGunCount < 9 then
			if g_PlayerGunID > 0 then
				if g_PlayerController==0 then
					Prompt("Press E to pick up the " .. weapon_name[e] .. " or T to replace" )
				else
					Prompt("Press Y Button to pick up the " .. weapon_name[e] )
				end
			else
				if g_PlayerController==0 then
					Prompt("Press E To pick up the " .. weapon_name[e] )
				else
					Prompt("Press Y Button to pick up the " .. weapon_name[e] )
				end
			end
			if g_KeyPressE == 1 then
			   Prompt("Collected the " .. weapon_name[e])
			   PlaySound(e,0)
			   AddPlayerWeapon(e)
			   Destroy(e)
			   ActivateIfUsed(e)
			   weapon_therecanbeonlyone = -1
			end
		else
			if g_PlayerController==0 then
				if g_PlayerGunID > 0 then
					Prompt("Press T to replace with " .. weapon_name[e] )
				else
					Prompt("Cannot collect any more weapons" )
				end
			else
				Prompt("Cannot collect any more weapons" )
			end
		end
		if g_InKey == "t" and g_PlayerGunID > 0 then
		   Prompt("Replaced with " .. weapon_name[e])
		   PlaySound(e,0)
		   ReplacePlayerWeapon(e)
		   Destroy(e)
		   ActivateIfUsed(e)
		   weapon_therecanbeonlyone = -1
		end
   end
 else
	if weapon_therecanbeonlyone==e then
		weapon_therecanbeonlyone = 0
	end
 end
end
