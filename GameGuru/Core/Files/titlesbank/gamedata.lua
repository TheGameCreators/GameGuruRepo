-- gamedata module version 0.01.8
--  added table saving
--   minor optimosation (0.01.09)

local gamedata = {}

function gamedata.save(slotnumber,uniquename)
	-- save slot file
	file = io.open("savegames\\gameslot" .. slotnumber .. ".dat", "w")
	io.output(file)
	-- header for game ID (untouched for compatablity )
	io.write(123 .. "\n")
	io.write(g_iGameNameNumber .. "\n")
	io.write(uniquename .. "\n")
	io.write(g_LevelFilename .. "\n")
	io.write(0 .. "\n")
-- try to only use save from now on. Any changes to data save format will then only have to be made once

	-- player stats
	save("iPlayerPosX",g_PlayerPosX)
	save("iPlayerPosY",g_PlayerPosY)
	save("iPlayerPosZ",g_PlayerPosZ)

	save("iPlayerAngX",g_PlayerAngX)
	save("iPlayerAngY",g_PlayerAngY)
	save("iPlayerAngZ",g_PlayerAngZ)

	save("iPlayerHealth",g_PlayerHealth)
	save("iPlayerLives",g_PlayerLives)
	save("strPlayerGunName",g_PlayerGunName)

	-- weapon stats
	UpdateWeaponStats() -- make sure weapon info at latest

	for e = 1, g_EntityElementMax, 1 do
		if g_Entity[e] ~= nil then
			save("g_Entity[" .. e .. "]['x']", g_Entity[e]['x'])
			save("g_Entity[" .. e .. "]['y']", g_Entity[e]['y'])
			save("g_Entity[" .. e .. "]['z']", g_Entity[e]['z'])

			save("g_Entity[" .. e .. "]['anglex']", GetEntityAngleX(e))
			save("g_Entity[" .. e .. "]['angley']", GetEntityAngleY(e))
			save("g_Entity[" .. e .. "]['anglez']", GetEntityAngleZ(e))

			save("g_Entity[" .. e .. "]['active']", GetEntityActive(e))
 			save("g_Entity[" .. e .. "]['activated']",g_Entity[e]['activated'])

   		save("g_Entity[" .. e .. "]['collected']", g_Entity[e]['collected'])
   		save("g_Entity[" .. e .. "]['haskey']", g_Entity[e]['haskey'])
   		save("g_Entity[" .. e .. "]['health']", g_Entity[e]['health'])
   		save("g_Entity[" .. e .. "]['frame']", g_Entity[e]['frame'])

			save("g_EntityExtra[" .. e .. "]['visible']", GetEntityVisibility(e))
			save("g_EntityExtra[" .. e .. "]['spawnatstart']", GetEntitySpawnAtStart(e))
		end
	end

-- radarobjectives array
	if radarObjectives ~= nil then
		save ("radarObjectives", radarObjectives)
	end

	-- globals
	for n in pairs(_G) do
		if string.find(tostring(_G[n]), "function:") == nil then
			local dumpout = 1
			if n == "file" then dumpout = 0 end
			if n == "_G" then dumpout = 0 end
			if n == "io" then dumpout = 0 end
			if n == "math" then dumpout = 0 end
			if string.sub(n,1,2) == "g_" then -- Other globals
				-- filter out Suff we don't want to save
				-- choped it into chunks to cut down number of string compars per iteration
				if n == "g_Entity" or n == "g_EntityExtra" then -- (0.01.09) these take up the most slots so get out early saves time
					dumpout = 0
				else
					if string.sub(n,1,8) == "g_Player" then -- no Player stuff as taken care of else where (saves 22)
						if n == "g_PlayerPosX" then dumpout = 0  -- restore target changed to iPlayerPosX
						elseif n == "g_PlayerPosY" then dumpout = 0  -- restore target changed to iPlayerPosY
						elseif n == "g_PlayerPosZ" then dumpout = 0  -- restore target changed to iPlayerPosZ
						elseif n == "g_PlayerAngX" then dumpout = 0  -- restore target changed to iPlayerAngX
						elseif n == "g_PlayerAngY" then dumpout = 0  -- restore target changed to iPlayerAngY
						elseif n == "g_PlayerAngZ" then dumpout = 0  -- restore target changed to iPlayerAngZ
						elseif n == "g_PlayerHealth" then dumpout = 0  -- restore target changed to iPlayerHealth
						elseif n == "g_PlayerLives" then dumpout = 0  -- restore target changed to iPlayerLives
						elseif n == "g_PlayerGunName" then dumpout = 0  -- restore target changed to strPlayerGunName
						elseif n == "g_PlayerGunZoomed" then dumpout = 0
						elseif n == "g_PlayerGunID" then dumpout = 0
						elseif n == "g_PlayerGunFired" then dumpout = 0	
						elseif n == "g_PlayerGunMode" then dumpout = 0
						elseif n == "g_PlayerGunClipCount" then dumpout = 0
						elseif n == "g_PlayerGunAmmoCount" then dumpout = 0
						elseif n == "g_PlayerGunCount" then dumpout = 0
						elseif n == "g_PlayerFlashlight" then dumpout = 0
						elseif n == "g_PlayerLastHitTime" then dumpout = 0						
						elseif n == "g_PlayerObjNo" then dumpout = 0
						elseif n == "g_PlayerDeadTime" then dumpout = 0
						elseif n == "g_PlayerThirdPerson" then dumpout = 0
						elseif n == "g_PlayerController" then dumpout = 0
						elseif n == "g_PlayerFOV" then dumpout = 0
						end
					end
	
					if string.sub(n,1,7) == "g_Mouse" then -- no mouse stuff (saves 3)
						if n == "g_MouseX" then dumpout = 0
							elseif n == "g_MouseY" then dumpout = 0
							elseif n == "g_MouseClick" then dumpout = 0
							elseif n == "g_MouseWheel" then dumpout = 0
						end
					end
	
					if string.sub(n,1,11) == "g_gameloop_" then -- no game control stuff (saves 3)
						if n == "g_gameloop_RegenRate" then dumpout = 0
						elseif n == "g_gameloop_RegenSpeed" then dumpout = 0
						elseif n == "g_gameloop_RegenDelay" then dumpout = 0
						elseif n == "g_gameloop_StartHealth" then dumpout = 0
						end
					end
	
					if string.sub(n,1,3) == "g_i" then -- (saves 18)
						if string.sub(n,1,5) == "g_img" then -- no standard images ?? (saves 10)
							if n == "g_imgCursor" then dumpout = 0
							elseif n == "g_imgBackdrop" then dumpout = 0
							elseif n == "g_imgHeading" then dumpout = 0
							elseif n == "g_imgAboutBackdrop" then dumpout = 0
							elseif n == "g_imgAboutCursor" then dumpout = 0
							elseif n == "g_imgProgressB" then dumpout = 0
							elseif n == "g_imgProgressF" then dumpout = 0
							elseif n == "g_imgAboutButton" then dumpout = 0 -- not in original
							elseif n == "g_imgAboutButtonH" then dumpout = 0 -- not in original
							elseif n == "g_imgButton" then dumpout = 0  -- not in original
							elseif n == "g_imgButtonS" then dumpout = 0 -- not in original
							elseif n == "g_imgButtonH" then dumpout = 0 -- not in original
							end
						elseif n == "g_iGameNameNumber" then dumpout = 0 -- no standard integers
						elseif n == "g_iGraphicChoice" then dumpout = 0
						elseif n == "g_iLoadingCountdown" then dumpout = 0
						elseif n == "g_iMusicChoice" then dumpout = 0
						elseif n == "g_iSoundChoice" then dumpout = 0
						end
					end
	
					if string.sub(n,1,5) == "g_spr" then -- no standard sprites ?? (saves 11)
						if n == "g_sprHeading" then dumpout = 0
						elseif n == "g_sprSlider" then dumpout = 0
						elseif n == "g_sprSliderM" then dumpout = 0
						elseif n == "g_sprProgressF" then dumpout = 0
						elseif n == "g_sprAboutBackdrop" then dumpout = 0
						elseif n == "g_sprCursor" then dumpout = 0
						elseif n == "g_sprSliderS" then dumpout = 0
						elseif n == "g_sprAboutCursor" then dumpout = 0
						elseif n == "g_sprProgressB" then dumpout = 0
						elseif n == "g_sprBackdrop" then dumpout = 0
						elseif n == "g_sprButton" then dumpout = 0 -- not in original
						elseif n == "g_sprAboutButton" then dumpout = 0 -- not in original
						end
					end
	
					if string.sub(n,1,10) == "g_KeyPress" then -- no keystates (saves 9)
						if n == "g_KeyPressC" then dumpout = 0
						elseif n == "g_KeyPressA" then dumpout = 0
						elseif n == "g_KeyPressR" then dumpout = 0
						elseif n == "g_KeyPressF" then dumpout = 0
						elseif n == "g_KeyPressS" then dumpout = 0
						elseif n == "g_KeyPressD" then dumpout = 0
						elseif n == "g_KeyPressW" then dumpout = 0
						elseif n == "g_KeyPressE" then dumpout = 0
						elseif n == "g_KeyPressSHIFT" then dumpout = 0
						elseif n == "g_KeyPressSPACE" then dumpout = 0
						end
					end
	
					if string.sub(n,1,5) == "g_str" then -- nostandard strings (saves 3)
						if n == "g_strStyleFolder" then dumpout = 0
						elseif n == "g_strBestResolution" then dumpout = 0
						elseif n == "g_strLevelFilenames" then dumpout = 0 -- not in original
						elseif n == "g_strSlotNames" then dumpout = 0 -- not in original
						end
					end
	
					if string.sub(n,1,5) == "g_pos" then -- no standard positioning info (saves 3)
						if n == "g_posButton" then dumpout = 0 -- not in original
						elseif n == "g_posAboutButton" then dumpout = 0 -- not in original
						elseif n == "g_posBackdropAngle" then dumpout = 0
						elseif n == "g_posAboutBackdropAngle" then dumpout = 0
						end
					end
	
					-- oddes and ends
					if n == "g_InKey" then dumpout = 0
					elseif n == "g_Time" then dumpout = 0
					elseif n == "g_TimeElapsed" then dumpout = 0
					elseif n == "g_GameStateChange" then dumpout = 0
					elseif n == "g_LevelFilename" then dumpout = 0
					elseif n == "g_DebugStringPeek" then dumpout = 0
					elseif n == "g_Scancode" then dumpout = 0
					end
				end

				if dumpout == 1 then
					save (n, _G[n])
				end
			end
		end
	end

	io.write("successful=1\n") -- will mean load returning successful means whole file read
	io.close(file)-- end of file

end

function gamedata.load(slotnumber)
	-- load game data
	successful = 0
	local file = assert(io.open("savegames\\gameslot" .. slotnumber .. ".dat", "r"))
	if file ~= nil then
		-- header for game ID
		iMagicNumber = tonumber(file:read())
		iGameNameNumber = tonumber(file:read())
		uniquename = file:read()
		strLevelFilename = file:read()
		iReserved = tonumber(file:read())
		-- get game data
		strField = file:read()

		while strField ~= nil do -- get each data item  optimised (0.01.09)
			if string.find(strField,"{}$") == nil and string.find(strField, "=") then -- crashes  if {} ?  and need  = so skip
				strsub = string.match(strField,"%w-%[(%d%d-)%]") -- new approach (0.01.09)
				if strsub then
					i = tonumber(strsub) 
					if  (string.sub(strField,1,8) == "g_Entity" and g_Entity[i] == nil) or (string.sub(strField,1,16) == "radarObjectives[" and radarObjectives[i] == nil) then
						ldvar = nil
					else
						ldvar = load(strField)
					end
				else
						ldvar = load(strField) -- no numerc subscript
				end
				if ldvar and type(ldvar) == "function" then ldvar() end -- skip bad lines as bad lines crash gg also check for bug reported in 2002 re incorrect type returned from load

			end
			strField = file:read()
		end
		-- end of file
		file:close()
	end

	return successful --successful = 1 success set in file
end

function bS (o) -- covert to safe strings
  if type(o) ==  "number" then
    return tostring(o)
  else   -- assume it is a string
    return string.format("%q", o) -- using %q rather than %s to cater for embended stuff
  end
end

function save (name, value, saved) -- based on example in manual
	saved = saved or {}       -- initial value
	io.write(name, "=")
	if type(value) == "number" or type(value) == "string" then
		io.write(bS(value), "\n")
	elseif type(value) == "table" then
		if saved[value] then    -- value already saved?
			io.write(saved[value], "\n")  -- use the previous name
		else
			saved[value] = name   -- save name use next time
			io.write("{}\n")     -- create new table
			for k,v in pairs(value) do      -- save its fields
				local fieldname = string.format("%s[%s]", name, bS(k))
				save(fieldname, v, saved) -- recursion be very careful
			end
		end
	else
		io.write("Error ".. name," -- cannot save a " .. type(value).."\n") -- this will fail as bad line on load
	end
end

return gamedata
