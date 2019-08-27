-- GamePlayerControl module

local gameplayercontrol = {}

g_gunandmeleemouseheld = 0
g_meleekeynotfree = 0
g_lmbheld = 0
g_lmbheldtime = 0
g_plrkeyRheld = 0
g_jamadjust = 0
g_forcealtswap = 0
g_keyboardpress = 0
g_mousewheellast = 0
g_suspendplayercontrols = 0
g_playercontrolcooldownmode = 0
g_specialPBRDebugView = 0
g_FootFallTimer = 0

function gameplayercontrol.main()
 gameplayercontrol.jetpack()
 gameplayercontrol.weaponfire()
 gameplayercontrol.weaponselectzoom()
 gameplayercontrol.lookmove()
 gameplayercontrol.control()
 if ( g_Scancode == 87 and IsTestGame() == 1 ) then g_specialPBRDebugView = 1 end
 if ( g_specialPBRDebugView == 1 ) then gameplayercontrol.debug() end
end
	
function gameplayercontrol.jetpack()
 
	-- Jet Pack Controls
	if ( GetGamePlayerControlJetpackMode() ~= 0 ) then 
		if ( GetGamePlayerControlJetpackMode() == 2 ) then 
			if ( (bit32.band(g_MouseClickControl,2)) ~= 0 and GetGamePlayerControlJetpackFuel()>0 and GetCameraPositionY(0)<3000.0 ) then 
				-- if jetpack starting for first time
				if ( GetGamePlayerControlJetpackParticleEmitterIndex() == 0 ) then 
					-- start jet emitter
					tResult = ParticlesGetFreeEmitter()
					if ( tResult>0 ) then 
						SetGamePlayerControlJetpackParticleEmitterIndex(tResult)
						ParticlesAddEmitter(tResult,1/64.0,1,-20,-20,-20,20,20,20,5,10,90,100,-0.1,-0.9,-0.1,0.1,-0.1,0.1,-0.1,0.1,1000,2000,40,75,0,0,25)
					end
				end
				-- apply player upward thrust
				SetGamePlayerControlJetpackThrust(GetGamePlayerControlJetpackThrust()+(0.75*GetTimeElapsed()))
				if ( GetGamePlayerControlJetpackThrust()>1.0 ) then 
					SetGamePlayerControlJetpackThrust(1.0)
				end
				-- play thrust sound with volume control
				if ( RawSoundExist(GetGamePlayerControlSoundStartIndex()+18) == 1 ) then 
					if ( RawSoundPlaying(GetGamePlayerControlSoundStartIndex()+18) == 0 ) then 
						LoopRawSound ( GetGamePlayerControlSoundStartIndex()+18 )
					end
				end
				if ( RawSoundExist(GetGamePlayerControlSoundStartIndex()+18) == 1 ) then 
					SetRawSoundVolume ( GetGamePlayerControlSoundStartIndex()+18, 70+(30*GetGamePlayerControlJetpackThrust()) )
				end
				-- deduct fuel
				SetGamePlayerControlJetpackFuel(GetGamePlayerControlJetpackFuel() - (0.1*GetTimeElapsed()))
				if ( GetGamePlayerControlJetpackFuel() < 0 ) then SetGamePlayerControlJetpackFuel(0.0) end
			end
			if ( g_PlrKeyJ == 1 ) then 
				SetGamePlayerControlJetpackMode(3)
			end
		end
		if ( GetGamePlayerControlJetpackMode() == 1 ) then 
			if ( g_PlrKeyJ == 0 ) then 
				SetGamePlayerControlJetpackMode(2)
				if ( GetGamePlayerControlJetpackHidden() == 0 ) then 
					ChangePlayerWeaponID(0)
				end
			end
		end
		if ( GetGamePlayerControlJetpackMode() == 3 ) then 
			if ( g_PlrKeyJ == 0 ) then 
				SetGamePlayerControlJetpackMode(0)
			end
		end
	else
		if ( g_PlrKeyJ == 1 and GetGamePlayerControlJetpackCollected() == 1 ) then 
			SetGamePlayerControlJetpackMode(1)
		end
	end
	if ( GetGamePlayerControlJetpackThrust()>0 ) then 
		-- reduce thrust using inertia
		SetGamePlayerControlJetpackThrust(GetGamePlayerControlJetpackThrust()-(GetTimeElapsed()*0.05))
		if ( GetGamePlayerControlJetpackThrust() <= 0 ) then 
			-- jet pack switches off
			if ( RawSoundExist(GetGamePlayerControlSoundStartIndex()+18) == 1 ) then StopRawSound ( GetGamePlayerControlSoundStartIndex()+18 ) end
			SetGamePlayerControlJetpackThrust(0.0)
			-- stop particle emitter
			if ( GetGamePlayerControlJetpackParticleEmitterIndex()>0 ) then 
				ParticlesDeleteEmitter(GetGamePlayerControlJetpackParticleEmitterIndex())
				SetGamePlayerControlJetpackParticleEmitterIndex(0)
			end
		end
		-- thrust noise volume control
		if ( RawSoundExist(GetGamePlayerControlSoundStartIndex()+18) == 1 ) then 
			SetRawSoundVolume ( GetGamePlayerControlSoundStartIndex()+18, 70+(30*GetGamePlayerControlJetpackThrust()) )
		end
	end

end

function gameplayercontrol.weaponfire()
	-- carrying an entity (pickuppable.lua script) then return
	if PU_GetEntityCarried ~= nil and PU_GetEntityCarried() ~= nil then return end

	-- Mouse based control
	SetGamePlayerStateFiringMode(0)
	if ( g_PlayerHealth>0 or GetGamePlayerControlStartStrength() == 0 ) then 
		-- Melee control
		if ( g_gunandmeleemouseheld == 1 ) then 
			if ( g_MouseClick ~= 1 or GetWeaponAmmo(GetGamePlayerStateWeaponAmmoIndex()+GetGamePlayerStateAmmoOffset())>0 ) then g_gunandmeleemouseheld = 0 end
		end
		-- Melee with a key press
		if ( GetGamePlayerStateGunMeleeKey()>0 ) then 
			if ( GetKeyState(GetGamePlayerStateGunMeleeKey()) == 1 and g_meleekeynotfree == 0 and GetGamePlayerStateIsMelee() == 0 ) then 
				SetGamePlayerStateIsMelee(1) 
				g_meleekeynotfree = 1 
			end
			if ( g_meleekeynotfree == 1 and GetKeyState(GetGamePlayerStateGunMeleeKey()) == 0 ) then g_meleekeynotfree = 0 end
		else
			g_meleekeynotfree=0
		end
		if ( GetWeaponAmmo(GetGamePlayerStateWeaponAmmoIndex()+GetGamePlayerStateAmmoOffset())>0 or GetFireModeSettingsReloadQty() == 0 ) then 
			if ( (bit32.band(g_MouseClickControl,1)) == 1 and GetGamePlayerStateBlockingAction() == 0 and GetGamePlayerStateIsMelee() == 0 ) then SetGamePlayerStateFiringMode(1) end
			if ( GetFireModeSettingsIsEmpty()>0 ) then 
				if ( GetGamePlayerStateGunMode() ~= 7031 and GetGamePlayerStateGunMode() ~= 7041 ) then 
					if ( GetGamePlayerStateGunMode()<123 or GetGamePlayerStateGunMode()>124 ) then 
						if ( GetGamePlayerStateGunMode()<700 or GetGamePlayerStateGunMode()>707 ) then SetFireModeSettingsIsEmpty(0) end
					end
				end
			end
		else
			if ( GetFireModeSettingsIsEmpty() == 0 and GetFireModeSettingsHasEmpty() == 1 ) then SetFireModeSettingsIsEmpty(1) end
			if ( (bit32.band(g_MouseClickControl,1)) == 1 and g_gunandmeleemouseheld == 0 and GetGamePlayerStateBlockingAction() == 0 and GetGamePlayerStateIsMelee() == 0 ) then SetGamePlayerStateGunShootNoAmmo(1) end
		end

		-- Firing the weapon if mouseclick pressed
		if ( (bit32.band(g_MouseClickControl,1)) == 1 ) then 
			SetGamePlayerStateFiringMode(1)
		end
		
		-- Hold down fire button value (can be interrupted when weapon runs out or is reloading)
		holdDownMouseClickButton = (bit32.band(g_MouseClickControl,1))
		holdDownMouseClickButton2 = (bit32.band(g_MouseClickControl,2))
		if ( GetGamePlayerStateGunMode() <= 8 or GetGamePlayerStateGunMode() == 9999 ) then
			holdDownMouseClickButton = 0
			holdDownMouseClickButton2 = 0
		end

		-- Gun jamming Timer
		if ( holdDownMouseClickButton == 1 ) then 
			if ( g_lmbheld == 0 ) then 
				if ( GetFireModeSettingsJammed() == 1 ) then 
					-- if tried to fire, but jammed, dry fire
					if ( GetGamePlayerStateAlternate() == 0 ) then 
						tsndid = GetGamePlayerStateGunSound(3)
					else
						tsndid = GetGamePlayerStateGunAltSound(3)
					end
					if ( tsndid > 0 ) then 
						if ( RawSoundExist(tsndid) == 1 ) then 
							if ( RawSoundPlaying(tsndid) == 0 ) then 
								PlayRawSound(tsndid)
							end
						end
					end
				end
				g_jamadjust = 0
				g_lmbheld = 1
				g_lmbheldtime = Timer()
			end
		end

		-- Track mouse held times (for lmb/rmb and jam adjust control) then
		if ( holdDownMouseClickButton2 == 2 ) then 
			if ( g_rmbheld == 0 ) then 
				g_rmbheld = 1
				g_rmbheldtime = Timer()
			end
		end
		if ( holdDownMouseClickButton == 0 ) then 
			g_lmbheld = 0
			g_lmbheldtime = 0
			g_jamadjust = 0
		end
		if ( holdDownMouseClickButton2 == 0 ) then 
			g_rmbheld = 0
			g_rmbheldtime = 0
		end

		-- Weapon jammed
		if ( GetFireModeSettingsJamChance() ~= 0 ) then 
			-- amount of minimum delay before jamming can occour
			if ( GetFireModeSettingsMinTimer() ~= 0 ) then 
				tmintimer=GetFireModeSettingsMinTimer()
			else
				tmintimer=500
			end
			-- increase jam change as barrels overheat
			if ( g_lmbheld ~= 0 ) then 
				if ( math.floor((Timer()-g_lmbheldtime)/100)>(math.floor((Timer()-g_lmbheldtime)/100)) ) then 
					g_jamadjust=g_jamadjust+5
				end
			end
			if ( (GetGamePlayerStateGunMode() == 105 or GetGamePlayerStateGunMode() == 106) and (Timer()-GetFireModeSettingsAddTimer())>(tmintimer-g_jamadjust) ) then 
				-- semi-auto fire
				SetFireModeSettingsShotsFired(GetFireModeSettingsShotsFired()+1)
				SetFireModeSettingsCoolTimer(Timer())
				SetFireModeSettingsAddTimer(Timer())
			end
			if ( GetGamePlayerStateGunMode() == 104 and (Timer()-GetFireModeSettingsAddTimer())>100 ) then 
				-- full-auto fire
				SetFireModeSettingsShotsFired(GetFireModeSettingsShotsFired()+1)
				SetFireModeSettingsCoolTimer(Timer())
				SetFireModeSettingsAddTimer(Timer())
			end
			if ( GetGamePlayerStateFiringMode() == 1 and GetFireModeSettingsJammed() == 0 ) then 
				tchance=99999
				if ( Timer()>g_lmbheldtime+(GetFireModeSettingsOverheatAfter()*1000) ) then 
					-- beyond overheat range
					if ( Timer()>GetFireModeSettingsJamChanceTime() ) then 
						SetFireModeSettingsJamChanceTime(Timer()+100)
						math.randomseed ( Timer() )
						tjamtemp=100-GetFireModeSettingsJamChance()
						if ( tjamtemp<0 ) then tjamtemp = 0 end
						if ( tjamtemp == 0 or math.random(0,tjamtemp) == 0 ) then tchance = 0 end
					end
				end
				if ( GetFireModeSettingsShotsFired()>tchance and GetWeaponAmmo(GetGamePlayerStateWeaponAmmoIndex())>1 ) then 
					SetFireModeSettingsJammed(1)
					-- ensure weapon jams affect both modes if sharing ammo
					if ( GetGamePlayerStateModeShareMags() == 1 ) then 
						---g.firemodes[GetGamePlayerStateGunID()][0].settings.jammed=1
						---g.firemodes[GetGamePlayerStateGunID()][1].settings.jammed=1
						SetFireModeSettingsJammed(1)
					end
					if ( GetGamePlayerStateAlternate() == 0 ) then 
						tsndid=GetGamePlayerStateGunSound(3)
					else
						tsndid=GetGamePlayerStateGunAltSound(3)
					end
					if ( tsndid>0 ) then 
						if ( RawSoundExist(tsndid) ) then 
							PlayRawSound ( tsndid )
						end
					end
				end
			end
			if ( GetFireModeSettingsJammed() == 1 ) then 
				SetGamePlayerStateFiringMode(0)
				SetFireModeSettingsShotsFired(0)
			end
			-- jam cool-down control
			if ( GetFireModeSettingsCoolDown() ~= 0 ) then 
				ttempcool=Timer()-GetFireModeSettingsCoolTimer()
				if ( ttempcool>GetFireModeSettingsCoolDown() ) then 
					SetFireModeSettingsShotsFired(0)
				end
			end
		end

		-- Underwater weapons fire
		if ( GetFireModeSettingsNoSubmergedFire() == 1 and GetGamePlayerStateUnderwater() == 1 ) then 
			SetGamePlayerStateFiringMode(0)
		end
		if ( (bit32.band(g_MouseClickControl,2)) == 2 and GetFireModeSettingsActionBlockStart() ~= 0 and GetGamePlayerStateBlockingAction() == 0 and GetGamePlayerStateIsMelee() == 0 ) then 
			SetGamePlayerStateBlockingAction(1)  
		end

		-- Trigger Zoom (no Zoom in When Reloading or firing in simple zoom or with gun empty or when running) then
		tttriggerironsight=0
		if ( GetGamePlayerControlThirdpersonEnabled() == 0 ) then
		if ( GetGamePlayerStateRightMouseHold()>0 ) then 
			-- mode to allow toggle with RMB instead of holding it down
			if ( (bit32.band(g_MouseClickControl,2)) == 2 and GetGamePlayerStateRightMouseHold() == 1 ) then SetGamePlayerStateRightMouseHold(2) end
			if ( (bit32.band(g_MouseClickControl,2)) == 0 and GetGamePlayerStateRightMouseHold() == 2 ) then SetGamePlayerStateRightMouseHold(3) end
			if ( (bit32.band(g_MouseClickControl,2)) == 2 and GetGamePlayerStateRightMouseHold() == 3 ) then SetGamePlayerStateRightMouseHold(4) end
			if ( (bit32.band(g_MouseClickControl,2)) == 0 and GetGamePlayerStateRightMouseHold() == 4 ) then SetGamePlayerStateRightMouseHold(1) end
			if ( GetGamePlayerStateRightMouseHold() >= 2 and GetGamePlayerStateRightMouseHold() <= 4 ) then tttriggerironsight = 1 end
		else
			if ( (bit32.band(g_MouseClickControl,2)) == 2  ) then tttriggerironsight = 1 end
		end
		end
		if ( GetGamePlayerStateXBOX() == 1 ) then 
			if ( GetGamePlayerStateXBOXControllerType() == 2 ) then 
				-- Logitech F310
			else
				if ( JoystickZ() > 200  ) then
					tttriggerironsight = 1
				end
			end
		end
		if ( GetGamePlayerControlJetpackMode()>0 ) then tttriggerironsight = 0 end

		-- can now zoom even if have no bullets
		if ( tttriggerironsight == 1 and (GetGamePlayerStateGunMode() <= 100 or GetFireModeSettingsSimpleZoom() == 0) and (GetFireModeSettingsForceZoomOut() == 0 or GetGamePlayerStateGunMode() <= 100) and GetGamePlayerControlIsRunning() == 0 and GetGamePlayerStateIsMelee() == 0 ) then 
			-- Modified for Simple Zoom
			if ( GetGamePlayerStateGunZoomMode() == 0 and (GetFireModeSettingsZoomMode() ~= 0 or GetFireModeSettingsSimpleZoom() ~= 0) ) then 
				SetGamePlayerStateGunZoomMode(1)  
				SetGamePlayerStateGunZoomMag(1.0)
				if ( GetFireModeSettingsSimpleZoomAnim() ~= 0 and GetFireModeSettingsSimpleZoom() ~= 0  ) then SetGamePlayerStateGunMode(2001) end
				if ( GetGamePlayerStateAlternate() == 0 ) then 
					tsndid = GetGamePlayerStateGunSound(0)
				else 
					tsndid = GetGamePlayerStateGunAltSound(0)
				end
				if ( tsndid > 0 ) then
					if ( RawSoundPlaying ( tsndid ) == 1 ) then
						PlayRawSound(tsndid,GetCameraPositionX(0),GetCameraPositionY(0),GetCameraPositionZ(0))
					end
				end
			end
		else
			-- only UNZOOM when we have finished other gun actions
			if ( GetGamePlayerStateGunMode() ~= 106 ) then 
				if ( GetGamePlayerStateGunZoomMode() == 9 and (tttriggerironsight == 0 or GetGamePlayerControlIsRunning() ~= 0) ) then 
					SetGamePlayerStateGunZoomMode(11)
					if ( GetFireModeSettingsSimpleZoomAnim() ~= 0 and GetFireModeSettingsSimpleZoom() ~= 0  ) then SetGamePlayerStateGunMode(2003) end
				end
			end
			if ( GetGamePlayerStateGunZoomMode() == 20 ) then SetGamePlayerStateGunZoomMode(0) end
		end

		-- Reload key
		if ( g_plrkeyRheld == 1 and g_PlrKeyR == 0 ) then g_plrkeyRheld = 0 end
		ttpool=GetFireModeSettingsPoolIndex()
		if ( ttpool == 0  ) then 
			tammo = GetWeaponClipAmmo(GetGamePlayerStateWeaponAmmoIndex()+GetGamePlayerStateAmmoOffset())
		else 
			tammo = GetWeaponPoolAmmo(ttpool)
		end
		if ( tammo == 0 or GetFireModeSettingsReloadQty() == 0 ) then 
			if ( (g_PlrKeyR) == 1 and g_plrkeyRheld == 0 ) then 
				SetGamePlayerStateGunReloadNoAmmo(1)   
				g_plrkeyRheld=1
				if ( GetFireModeSettingsJammed() == 1 ) then 
					SetGamePlayerStateFiringMode(2)
				end
			end
		else
			if ( (g_PlrKeyR) == 1 and GetGamePlayerStateIsMelee() ~= 2 ) then 
				SetGamePlayerStateFiringMode(2)
				SetGamePlayerStatePlrReloading(1)
				if ( GetGamePlayerStateIsMelee() == 1 ) then SetGamePlayerStateIsMelee(0) end
			end
		end
	end

	-- ensure weapon unjam if jammed
	if ( GetGamePlayerStateGunZoomMode() ~= 0 ) then g_PlrKeyR = 0 end
	if ( GetFireModeSettingsJammed() == 1 and (g_PlrKeyR) == 1 and GetGamePlayerStateIsMelee() ~= 2 and GetFireModeSettingsSimpleZoomAnim() == 0 and GetFireModeSettingsSimpleZoom() == 0 ) then 
		SetGamePlayerStateFiringMode(2)
	end

end

function gameplayercontrol.weaponselectzoom()

	-- Key select
	tselkeystate = 0
	if ( tselkeystate == 0 ) then 
        for ti = 2, 11, 1 do
			if ( GetKeyState(ti)==1 ) then 
				tselkeystate = ti 
				break
			end
		end
	end

	-- Keyboard
	taltswapkeycalled = 0
	if ( GetGamePlayerStateGunAltSwapKey1()>-1 and GetKeyState(GetGamePlayerStateGunAltSwapKey1())==1 and (GetGamePlayerStateGunZoomMode() == 0 or (GetGamePlayerStateGunZoomMode()>=8 and GetGamePlayerStateGunZoomMode()<=10)) or g_forcealtswap == 1 ) then 
		taltswapkeycalled = 1 
		g_forcealtswap = 0 
	end
	if ( tselkeystate>0 and (GetGamePlayerStateGunZoomMode() == 0 or (GetGamePlayerStateGunZoomMode()>=8 and GetGamePlayerStateGunZoomMode()<=10)) or taltswapkeycalled == 1 ) then 
		if ( taltswapkeycalled == 1 and GetGamePlayerStateGunAltSwapKey2()>-1 and GetKeyState(GetGamePlayerStateGunAltSwapKey2())==1 or taltswapkeycalled == 1 and GetGamePlayerStateGunAltSwapKey2() == -1 or taltswapkeycalled == 0 ) then 
			if ( g_keyboardpress == 0 ) then 
				-- Change weapon - and prevent plr selecting gun if flagged
				if ( (g_PlayerHealth>0 or GetGamePlayerControlStartStrength() == 0) ) then 
					-- if in zoom mode, switch out at same time
					if ( GetGamePlayerStateGunZoomMode() == 10 ) then SetGamePlayerStateGunZoomMode(11) end
					-- Weapon select - Allow moving and switching
					twepsel=tselkeystate-1
					-- Cannot keypress until release
					g_keyboardpress=tselkeystate
					if ( tselkeystate == 11 ) then twepsel = -1 end
					if ( taltswapkeycalled == 1 ) then 
						g_keyboardpress = -2   
						twepsel = -2 
					end
					-- Actual weapon change
					if ( taltswapkeycalled == 0 ) then 
						if ( twepsel>0 ) then 
							SetGamePlayerStateWeaponKeySelection(twepsel)
							if ( GetGamePlayerStateWeaponKeySelection()>0 ) then 
								if ( GetWeaponSlotNoSelect(GetGamePlayerStateWeaponKeySelection()) == 0 ) then 
									SetGamePlayerStateWeaponIndex(GetWeaponSlotGot(GetGamePlayerStateWeaponKeySelection()))
									if ( GetGamePlayerStateWeaponIndex()>0 ) then SetGamePlayerStateCommandNewWeapon(GetGamePlayerStateWeaponIndex()) end
								end
							end
						end
						-- ensure can only put weapon away when in idle mode
						if ( twepsel == -1 ) then 
							if ( GetGamePlayerStateGunMode()<31 or GetGamePlayerStateGunMode()>35 ) then 
								if ( GetGamePlayerStateGunID() ~= 0 ) then 
									SetGamePlayerStateGunMode(31)  
									SetGamePlayerStateGunSelectionAfterHide(0)
								end
							end
						end
					end
					if ( taltswapkeycalled == 1 and GetWeaponSlotNoSelect(GetGamePlayerStateWeaponKeySelection()) == 0 ) then 
						if ( GetGamePlayerStateGunID() ~= 0 ) then 
							if ( (GetGamePlayerStateAlternateIsFlak() == 1 or GetGamePlayerStateAlternateIsRay() == 1) and GetGamePlayerStateGunMode() <= 100 ) then 
								-- reset gunburst variable so burst doesn't get confused when switching gunmodes
								if ( GetGamePlayerStateAlternate() == 1 ) then 
									SetGamePlayerStateGunMode(2009) 
									--SetGamePlayerStateAlternate(0) now set inside gunmode 2009 'after' zoom out
									SetGamePlayerStateGunBurst(0)
								else 
									SetGamePlayerStateGunMode(2007) 
									--SetGamePlayerStateAlternate(1) now set inside gunmode 2009 'after' zoom out
									SetGamePlayerStateGunBurst(0) 
								end
							end
						end
					end
				end
				taltswapkeycalled=0
			end
		end
	end
	if ( g_keyboardpress>0 ) then 
		if ( GetKeyState(g_keyboardpress) == 0 ) then g_keyboardpress = 0 end
	end
	if ( g_keyboardpress == -2 and taltswapkeycalled == 0 ) then g_keyboardpress = 0 end

	--  Mouse Wheel scrolls weapons
	ttmz = g_MouseWheel - g_mousewheellast
	g_mousewheellast = g_MouseWheel

	-- Contoller only if ready for gun swap
	if ( GetGamePlayerStateXBOX() == 1 ) then 
		if ( GetGamePlayerStateGunMode() >= 5 and GetGamePlayerStateGunMode()<31 ) then 
			if ( JoystickHatAngle(0) == 9000  ) then ttmz = 10 end
			if ( JoystickHatAngle(0) == 27000  ) then ttmz = -10 end
			if ( JoystickFireXL(4) == 1  ) then ttmz = -10 end
			if ( JoystickFireXL(5) == 1  ) then ttmz = 10 end
		end
	end

	-- Change weapon using scroll wheel
	if ( GetGamePlayerStateIsMelee() == 0 and (GetGamePlayerStateGunZoomMode() == 0 or GetGamePlayerStateGunZoomMode() == 10) ) then 
		if ( ttmz ~= 0 ) then 
			-- and prevent player changing guns with joystick
			ttokay=0  
			tttries=10
			ttweaponindex=0
			while ( ttokay == 0 and tttries>0 ) do
				if ( ttmz<0 ) then SetGamePlayerStateWeaponKeySelection(GetGamePlayerStateWeaponKeySelection()-1) end
				if ( ttmz>0 ) then SetGamePlayerStateWeaponKeySelection(GetGamePlayerStateWeaponKeySelection()+1) end
				if ( GetGamePlayerStateWeaponKeySelection()>10 ) then SetGamePlayerStateWeaponKeySelection(1) end
				if ( GetGamePlayerStateWeaponKeySelection()<1 ) then SetGamePlayerStateWeaponKeySelection(10) end
				if ( GetWeaponSlotNoSelect(GetGamePlayerStateWeaponKeySelection()) == 0 ) then 
					ttweaponindex = GetWeaponSlotGot(GetGamePlayerStateWeaponKeySelection()) 
				end
				if ( ttweaponindex>0 ) then ttokay = 1 end
				tttries = tttries - 1
			end
			if ( ttokay == 1 ) then 
				-- change up or down
				SetGamePlayerStateWeaponIndex(GetWeaponSlotGot(GetGamePlayerStateWeaponKeySelection()))
				if ( GetGamePlayerStateWeaponIndex()>0 ) then SetGamePlayerStateCommandNewWeapon(GetGamePlayerStateWeaponIndex()) end
				if ( GetGamePlayerStateGunZoomMode() == 10 ) then SetGamePlayerStateGunZoomMode(11) end
			end
		end
	else
		-- zoom magnification wheel
		if ( ttmz ~= 0 ) then 
			ttmz_f=ttmz  
			SetGamePlayerStateGunZoomMag(GetGamePlayerStateGunZoomMag()+(ttmz_f/100.0))
			if ( GetGamePlayerStateGunZoomMag()<1.0 ) then SetGamePlayerStateGunZoomMag(1.0) end
			if ( GetGamePlayerStateGunZoomMag()>10.0 ) then SetGamePlayerStateGunZoomMag(10.0) end
			SetGamePlayerStatePlrZoomInChange(1)
		end
	end

	-- Gun Zoom Control
	if ( GetGamePlayerStateGunZoomMode()>0 ) then 
		if ( GetGamePlayerStateGunZoomMode() >= 1 and GetGamePlayerStateGunZoomMode() <= 8 ) then SetGamePlayerStateGunZoomMode(GetGamePlayerStateGunZoomMode()+1) end
		if ( GetGamePlayerStateGunZoomMode() >= 11 and GetGamePlayerStateGunZoomMode() <= 19 ) then SetGamePlayerStateGunZoomMode(GetGamePlayerStateGunZoomMode()+1) end
		if ( GetGamePlayerStateGunZoomMode()<10 ) then 
			SetGamePlayerStatePlrZoomIn(GetGamePlayerStateGunZoomMode())  
			SetGamePlayerStatePlrZoomInChange(GetGamePlayerStatePlrZoomInChange()+1)
		end
		if ( GetGamePlayerStateGunZoomMode() == 10 ) then 
			-- in full zoom
		end
		if ( GetGamePlayerStateGunZoomMode()>10 ) then 
			SetGamePlayerStatePlrZoomIn(10-(GetGamePlayerStateGunZoomMode()-10))
			SetGamePlayerStatePlrZoomInChange(1)
		end
	end

	-- Handle optical effect of zoom
	if ( GetGamePlayerStatePlrZoomInChange() == 1 ) then 
		if ( GetGamePlayerStatePlrZoomIn()>1.0 ) then 
			if ( GetFireModeSettingsSimpleZoom() ~= 0 ) then 
				trealfovdegree=GetGamePlayerStateCameraFovZoomed()*((GetGamePlayerStatePlrZoomIn()*GetFireModeSettingsSimpleZoom()))
				SetGamePlayerStateRealFov(GetGamePlayerStateCameraFov()-trealfovdegree)
				if ( GetGamePlayerStateRealFov()<7 ) then SetGamePlayerStateRealFov(7) end
				SetCameraFOV ( 0, GetGamePlayerStateRealFov() )
				SetCameraFOV ( 2, GetGamePlayerStateRealFov() )
			else
				trealfovdegree=GetGamePlayerStateCameraFovZoomed()*((GetGamePlayerStatePlrZoomIn()*7)-GetGamePlayerStateGunZoomMag())
				SetGamePlayerStateRealFov(GetGamePlayerStateCameraFov()-trealfovdegree)
				if ( GetGamePlayerStateRealFov()<15 ) then SetGamePlayerStateRealFov(15) end
				SetCameraFOV ( 0, GetGamePlayerStateRealFov() )
				SetCameraFOV ( 2, GetGamePlayerStateRealFov() )
			end
		else
			trealfovdegree=GetGamePlayerStateCameraFovZoomed()
			SetGamePlayerStateRealFov(GetGamePlayerStateCameraFov()+trealfovdegree)
			if ( GetGamePlayerStateRealFov()<15 ) then SetGamePlayerStateRealFov(15) end
			SetCameraFOV ( 0, GetGamePlayerStateRealFov() )
			SetCameraFOV ( 2, GetGamePlayerStateRealFov() )
		end
	end
	SetGamePlayerStatePlrZoomInChange(0)

end

function gameplayercontrol.lookmove()

	-- determine if any head tracker in use
	ttvrheadtrackermode = 0
	if GetHeadTracker() == 1 then
 	 ttvrheadtrackermode = 1
	 if g_InKey == "p" then
	  ResetHeadTracker()
	 end
	end
	
	-- completely skip use of mousemovexy so LUA mouse system can use it for its own pointer
	if ( GetGamePlayerStateLuaActiveMouse() ~= 1 ) then
		-- No player control if dead, but use up mousemoves to prevent sudden move on respawn or if in multiplayer and respawning
		if ( g_PlayerHealth == 0 or GetGamePlayerStatePlrHasFocus() == 0 or (GetGamePlayerStateGameRunAsMultiplayer() == 1 and GetGamePlayerStateSteamWorksRespawnLeft() ~= 0) ) then 
			tcleardeltas = MouseMoveX() + MouseMoveY()
		end
		-- Tab Mode 2, low FPS screen and construction kit menus require mouse
		if ( GetGamePlayerStateTabMode()<2 and GetGamePlayerStateLowFpsWarning() ~= 1 ) then 
			-- Manipulate camera angle using mouselook
			if ( GetGamePlayerStateMouseInvert() == 1 ) then 
				tttmousemovey = MouseMoveY()*-1 
			else 
				tttmousemovey = MouseMoveY()
			end
			--if ttvrheadtrackermode == 1 then --allow mouselook up/down in VR
			--	tttmousemovey = 0
			--end			
			if ( GetGamePlayerStateSlowMotion() == 1 and GetGamePlayerStateSmoothCameraKeys() == 1 ) then 
				SetGamePlayerStateCamMouseMoveX(GetGamePlayerStateCamMouseMoveX()+(MouseMoveX()/10.0))
				SetGamePlayerStateCamMouseMoveY(GetGamePlayerStateCamMouseMoveY()+(tttmousemovey/10.0))
				SetGamePlayerStateCamMouseMoveX(GetGamePlayerStateCamMouseMoveX()*0.94)
				SetGamePlayerStateCamMouseMoveY(GetGamePlayerStateCamMouseMoveY()*0.94)
			else
				SetGamePlayerStateCamMouseMoveX(MouseMoveX())
				SetGamePlayerStateCamMouseMoveY(tttmousemovey)
			end
			if ( GetGamePlayerStatePlrHasFocus() == 1 ) then 
				PositionMouse ( GetDesktopWidth()/2,GetDesktopHeight()/2 )
			end
		else
			tnull=MouseMoveX() 
			tnull=MouseMoveY()
			SetGamePlayerStateCamMouseMoveX(0)  
			SetGamePlayerStateCamMouseMoveY(0)
		end
		if ( GetGamePlayerStateXBOX() == 1 ) then 
			if ( GetGamePlayerStateXBOXControllerType() == 2 ) then 
				-- Logitech F310 (with deadzones)
				ttjoyrotx=(JoystickZ()+0.0)/1000.0
				--if ttvrheadtrackermode == 1 then  --allow mouselook up/down in VR
				--	ttjoyroty=0
				--else
					ttjoyroty=((JoystickTwistZ()+0.0)-32768.0)/32768.0
				--end
			else
				ttjoyrotx=((JoystickTwistX()+0.0)-32768.0)/32768.0
				ttjoyroty=((JoystickTwistY()+0.0)-32768.0)/32768.0
			end
			if ( ttjoyrotx>-0.3 and ttjoyrotx<0.3 ) then ttjoyrotx = 0 end
			if ( ttjoyroty>-0.3 and ttjoyroty<0.3 ) then ttjoyroty = 0 end
			SetGamePlayerStateCamMouseMoveX(ttjoyrotx*6.0)
			SetGamePlayerStateCamMouseMoveY(ttjoyroty*6.0)
		end

		-- Modifly rotation speed by zoom amount
		ttturnspeedmodifier=1.0
		if ( GetGamePlayerStateGunID()>0 ) then 
			ttturnspeedmodifier=GetFireModeSettingsPlrTurnSpeedMod()
			if ( GetGamePlayerStatePlrZoomIn()>1 ) then 
				-- use gunspec settings to control relative speed of zoom mouselook
				ttturnspeedmodifier=ttturnspeedmodifier*GetFireModeSettingsZoomTurnSpeed()
			end
		end
		ttRotationDivider=8.0/ttturnspeedmodifier
		SetGamePlayerStateCamAngleX(GetCameraAngleX(0)+(GetGamePlayerStateCamMouseMoveY()/ttRotationDivider))
		SetGamePlayerStateCamAngleY(GetGamePlayerControlFinalCameraAngley()+(GetGamePlayerStateCamMouseMoveX()/ttRotationDivider))
	end

	-- Handle player weapon recoil
	if ( GetGamePlayerControlThirdpersonEnabled() == 0 ) then 
		SetGamePlayerStateGunRecoilAngleY(CurveValue(GetGamePlayerStateGunRecoilY(),GetGamePlayerStateGunRecoilAngleY(),2.0))
		SetGamePlayerStateGunRecoilY(CurveValue(0.0,GetGamePlayerStateGunRecoilY(),1.5))
		SetGamePlayerStateGunRecoilAngleX(CurveValue(GetGamePlayerStateGunRecoilX(),GetGamePlayerStateGunRecoilAngleX(),2.0))
		SetGamePlayerStateGunRecoilX(CurveValue(0.0,GetGamePlayerStateGunRecoilX(),1.5))
		SetGamePlayerStateGunRecoilCorrectY(CurveValue(0.0,GetGamePlayerStateGunRecoilCorrectY(),3.0))
		SetGamePlayerStateGunRecoilCorrectX(CurveValue(0.0,GetGamePlayerStateGunRecoilCorrectX(),3.0))
		SetGamePlayerStateGunRecoilCorrectAngleY(CurveValue(GetGamePlayerStateGunRecoilCorrectY(),GetGamePlayerStateGunRecoilCorrectAngleY(),4.0))
		SetGamePlayerStateGunRecoilCorrectAngleX(CurveValue(GetGamePlayerStateGunRecoilCorrectX(),GetGamePlayerStateGunRecoilCorrectAngleX(),4.0))
		SetGamePlayerStateCamAngleX(GetGamePlayerStateCamAngleX()-GetGamePlayerStateGunRecoilAngleY()+GetGamePlayerStateGunRecoilCorrectAngleY())
		SetGamePlayerStateCamAngleY(GetGamePlayerStateCamAngleY()-GetGamePlayerStateGunRecoilAngleX()+GetGamePlayerStateGunRecoilCorrectAngleX())
	end

	-- Cap look up/down angle so cannot wrap around
	if ( GetGamePlayerStateCamAngleX()<-80 ) then SetGamePlayerStateCamAngleX(-80) end
	if ( GetGamePlayerStateCamAngleX()>85 ) then SetGamePlayerStateCamAngleX(85) end

	-- Handle player ducking
	if ( g_PlrKeyC == 1 ) then 
		SetGamePlayerStatePlayerDucking(1)
	else
		if ( GetGamePlayerStatePlayerDucking() == 1 ) then SetGamePlayerStatePlayerDucking(2) end
		if ( GetCharacterControllerDucking() == 0 ) then 
			SetGamePlayerStatePlayerDucking(0)
		end
	end

	--  third person can freeze player controls if MID-anim (such as jump) then
	g_suspendplayercontrols=0
	if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
		-- if in middle of playcsi event, suspend controls until finished
		SetGamePlayerStateCharAnimIndex(GetGamePlayerControlThirdpersonCharacterIndex())
		if ( GetCharAnimStatePlayCsi(GetGamePlayerStateCharAnimIndex()) > 0 ) then 
			if ( GetCharSeqTrigger(GetCharAnimStatePlayCsi(GetGamePlayerStateCharAnimIndex())) == 0 ) then 
				g_suspendplayercontrols=1
			end
		end
	end
	
	-- Rotate player camera
	if ( g_PlayerHealth>0 and g_suspendplayercontrols == 0 ) then 
		-- when camea rotation overridden, prevent rotation
		if ( GetCameraOverride() ~= 2 and GetCameraOverride() ~= 3 ) then
			SetCameraAngle ( 0, GetGamePlayerStateCamAngleX(), GetGamePlayerStateCamAngleY(), 0 )
			SetGamePlayerControlCx(GetCameraAngleX(0))
			SetGamePlayerControlCy(GetCameraAngleY(0))
			SetGamePlayerControlCz(GetCameraAngleZ(0))
		end
		-- when camera overridden, do not allow movement
		if ( GetCameraOverride() ~= 1 and GetCameraOverride() ~= 3 ) then
			if ( GetGamePlayerStateEditModeActive() ~= 0 ) then 
				if ( GetGamePlayerStatePlrKeyShift() == 1 or GetGamePlayerStatePlrKeyShift2() == 1 ) then 
					SetGamePlayerControlBasespeed(4.0)
				else
					if ( GetGamePlayerStatePlrKeyControl() == 1 ) then 
						SetGamePlayerControlBasespeed(0.25)
					else
						SetGamePlayerControlBasespeed(1.5)
					end
				end
				SetGamePlayerControlIsRunning(0)
			else
				if ( GetGamePlayerStatePlayerDucking() == 1 ) then 
					SetGamePlayerControlBasespeed(0.5)
					SetGamePlayerControlIsRunning(0)
				else
					if ( GetGamePlayerControlCanRun() == 1 and (GetGamePlayerStatePlrKeyShift() == 1 or GetGamePlayerStatePlrKeyShift2() == 1) ) then 
						-- sprinting speed, modified by directions.
						SetGamePlayerControlBasespeed(2.0)
						if ( GetGamePlayerControlThirdpersonEnabled() == 1 and GetGamePlayerControlThirdpersonCameraFollow() == 1 ) then 
							-- WASD run speed
						else
							if ( g_PlrKeyS == 1 ) then 
								SetGamePlayerControlBasespeed(1.0)
							else
								if ( g_PlrKeyW  ==  1 ) then 
									if ( g_PlrKeyA == 1 or g_PlrKeyD == 1 ) then 
										SetGamePlayerControlBasespeed(1.75)
									end
								else
									if ( g_PlrKeyA == 1 or g_PlrKeyD == 1 ) then 
										SetGamePlayerControlBasespeed(1.5)
									end
								end
							end
						end
						SetGamePlayerControlIsRunning(1)
					else
						SetGamePlayerControlBasespeed(1.0)
						SetGamePlayerControlIsRunning(0)
					end
				end
			end
			SetGamePlayerControlMaxspeed(GetGamePlayerControlTopspeed()*GetGamePlayerControlBasespeed())
			SetGamePlayerControlMovement(0)
			if ( g_PlrKeyA == 1 ) then 
				SetGamePlayerControlMovement(1)
				SetGamePlayerControlMovey(GetGamePlayerControlCy()-90)
				if ( g_PlrKeyW == 1 ) then SetGamePlayerControlMovey(GetGamePlayerControlCy()-45)
				elseif ( g_PlrKeyS == 1 ) then SetGamePlayerControlMovey(GetGamePlayerControlCy()-45-90) end
			else
				if ( g_PlrKeyD == 1 ) then 
					SetGamePlayerControlMovement(1)
					SetGamePlayerControlMovey(GetGamePlayerControlCy()+90)
					if ( g_PlrKeyW == 1 ) then SetGamePlayerControlMovey(GetGamePlayerControlCy()+45)
					elseif ( g_PlrKeyS == 1 ) then SetGamePlayerControlMovey(GetGamePlayerControlCy()+45+90) end
				else
					if ( g_PlrKeyW == 1 and g_PlrKeyS==0 ) then 
						SetGamePlayerControlMovement(1)  
						SetGamePlayerControlMovey(GetGamePlayerControlCy())
					elseif ( g_PlrKeyS == 1 and g_PlrKeyW == 0 ) then 
						SetGamePlayerControlMovement(1)  
						SetGamePlayerControlMovey(GetGamePlayerControlCy()+180) 
					end
				end
			end
		else
			-- camera overridden players cannot move themselves
			SetGamePlayerControlMovement(0)
		end
	else
		--  dead players cannot move themselves
		SetGamePlayerControlMovement(0)
	end

end

function gameplayercontrol.control()

	-- if movement state changed, reset footfall to get first foot in frame
	if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
		if ( GetGamePlayerControlLastMovement() ~= GetGamePlayerControlMovement() ) then 
			SetGamePlayerControlFootfallCount(0)
		end
	end
	SetGamePlayerControlLastMovement(GetGamePlayerControlMovement())

	--  Control speed and walk wobble
	if ( GetGamePlayerControlMovement()>0 ) then 
		ttokay=0
		if ( GetGamePlayerControlGravityActive() == 1 ) then 
			if ( GetGamePlayerControlPlrHitFloorMaterial() ~= 0 ) then ttokay = 1 end
			if ( GetGamePlayerControlUnderwater() == 1 and GetGamePlayerStateNoWater() == 0  ) then ttokay = 1 end
			if ( GetGamePlayerControlJumpMode() == 1 and GetGamePlayerControlJumpModeCanAffectVelocityCountdown()>0 ) then ttokay = 1 end
		else
			ttokay=1
		end
		if ( ttokay == 1 ) then 
			-- increase plr speed using accel ratio
			SetGamePlayerControlSpeed(GetGamePlayerControlSpeed()+(GetGamePlayerControlAccel()*GetTimeElapsed()*0.05))
			if ( GetGamePlayerControlSpeed()>GetGamePlayerControlMaxspeed()*GetGamePlayerControlSpeedRatio() ) then 
				SetGamePlayerControlSpeed(GetGamePlayerControlMaxspeed()*GetGamePlayerControlSpeedRatio())
			end
		else
			-- with no Floor (  under player, reduce any speed for downhill and fall behavior ) then
			ttdeductspeed=(GetGamePlayerControlMaxspeed()*GetGamePlayerControlSpeedRatio())*GetTimeElapsed()*0.01
			SetGamePlayerControlSpeed(GetGamePlayerControlSpeed()-ttdeductspeed)
			if ( GetGamePlayerControlSpeed()<0.0 ) then SetGamePlayerControlSpeed(0.0) end
		end
		if ( GetGamePlayerControlGravityActive() == 1 and GetGamePlayerControlJumpMode() ~= 1 ) then 
			-- on ground
			ttWeaponMoveSpeedMod = 1.0
			if ( GetGamePlayerStateGunID() > 0 ) then
			 ttWeaponMoveSpeedMod = GetFireModeSettingsPlrMoveSpeedMod()
			 if ttWeaponMoveSpeedMod < 0.4 then ttWeaponMoveSpeedMod = 0.4 end
			end
			SetGamePlayerControlWobble(WrapValue(GetGamePlayerControlWobble()+(GetGamePlayerControlWobbleSpeed()*GetElapsedTime()*GetGamePlayerControlBasespeed()*GetGamePlayerControlSpeedRatio()*ttWeaponMoveSpeedMod)))
		else
			-- in air
			SetGamePlayerControlWobble(CurveValue(0,GetGamePlayerControlWobble(),3.0))
		end
	else
		ttdeductspeed=(GetGamePlayerControlMaxspeed()*GetGamePlayerControlSpeedRatio())*GetTimeElapsed()*0.5
		SetGamePlayerControlSpeed(GetGamePlayerControlSpeed()-ttdeductspeed)
		if ( GetGamePlayerControlSpeed()<0.0 ) then SetGamePlayerControlSpeed(0.0) end
		ttamounttostep = 25.0*GetTimeElapsed()
		if ( GetGamePlayerControlWobble() < 180 ) then
			ttnewtotal = GetGamePlayerControlWobble() - ttamounttostep
			if ttnewtotal < 0 then ttnewtotal = 0 end
			SetGamePlayerControlWobble(ttnewtotal)
		else
			ttnewtotal = GetGamePlayerControlWobble() + ttamounttostep
			if ttnewtotal >= 360 then ttnewtotal = 0 end
			SetGamePlayerControlWobble(ttnewtotal)
		end
	end

	-- Handle player jumping
	tplayerjumpnow=0.0
	if ( GetGamePlayerControlGravityActive() == 1 and g_suspendplayercontrols == 0 ) then 
		ttokay=0
		if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
			ttcharacterindex=GetGamePlayerControlThirdpersonCharacterIndex()
			entid = GetEntityElementBankIndex(GetCharAnimStateOriginalE(ttcharacterindex))
			if ( g_PlrKeySPACE == 1 and GetGamePlayerControlJumpMode() == 0 and GetEntityProfileJumpModifier(entid) > 0.0 ) then 
				ttplaycsi=GetCharAnimStatePlayCsi(ttcharacterindex)
				if ( ttplaycsi >= GetCsiStoodVault(1) and ttplaycsi <= GetCsiStoodVault(1)+2 ) then 
					--  still in jump animation action
				else
					ttokay=1
				end
			end
			ttjumpmodifier = GetEntityProfileJumpModifier(entid)
		else
			if ( g_PlrKeySPACE == 1 and GetGamePlayerControlJumpMode() == 0  ) then ttokay = 1 end
			ttjumpmodifier = 1.0
		end
		if ( ttokay == 1 ) then 
			-- player can only jump if a certain height above the waterline (i.e wading in shallows, not swimming) then
			if ( GetGamePlayerStateNoWater() ~= 0 or GetCameraPositionY(0) > GetGamePlayerStateWaterlineY() + 20 ) then 
				tplayerjumpnow=GetGamePlayerControlJumpmax()*ttjumpmodifier
				if ( GetGamePlayerStateGunID()>0 ) then tplayerjumpnow = tplayerjumpnow*GetFireModeSettingsPlrJumpSpeedMod() end
				SetGamePlayerControlJumpMode(1)
				if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
					entid = GetEntityElementBankIndex(GetGamePlayerControlThirdpersonCharactere())
					tq = GetEntityProfileStartOfAIAnim(entid) + GetCsiStoodVault(1)
					if ( GetEntityAnimStart(entid,tq) > 0 ) then
						-- only if have VAULT animation, so we use the jump animation at all
						SetCharAnimControlsLeaping(ttcharacterindex,1)
					end
					SetGamePlayerControlJumpModeCanAffectVelocityCountdown(7.0)
				else
					if ( GetGamePlayerControlMovement() == 0 ) then 
						--  only if static can have the 'burst forward' feature to get onto crates
						SetGamePlayerControlJumpModeCanAffectVelocityCountdown(7.0)
					else
						--  otherwise we can make HUGE running jumps with this
						SetGamePlayerControlJumpModeCanAffectVelocityCountdown(0)
					end
				end
			end
		end
	end
	if ( GetGamePlayerControlJumpModeCanAffectVelocityCountdown()>0 ) then 
		SetGamePlayerControlJumpModeCanAffectVelocityCountdown(GetGamePlayerControlJumpModeCanAffectVelocityCountdown()-GetTimeElapsed())
		if ( GetGamePlayerControlJumpModeCanAffectVelocityCountdown()<0 ) then 
			SetGamePlayerControlJumpModeCanAffectVelocityCountdown(0)
			if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
				ttcharacterindex=GetGamePlayerControlThirdpersonCharacterIndex()
				SetCharAnimControlsLeaping(ttcharacterindex,0)
			end
		end
	end
	if ( GetGamePlayerControlJumpMode() == 1 ) then 
		if ( GetGamePlayerControlJumpModeCanAffectVelocityCountdown() == 0 ) then 
			if ( GetPlrObjectAngleX() == 1.0 ) then 
				SetGamePlayerControlJumpMode(2)
				SetGamePlayerControlJumpModeCanAffectVelocityCountdown(0)
			end
		end
	end
	if ( GetGamePlayerControlJumpMode() == 2 ) then 
		if ( g_PlrKeySPACE == 0 ) then SetGamePlayerControlJumpMode(0) end
	end

	--  Handle player flash light control
	if ( g_PlrKeyF == 1 and GetGamePlayerStateFlashlightKeyEnabled() == 1 ) then
		if ( GetGamePlayerStateFlashlightControl() == 0.0 ) then SetGamePlayerStateFlashlightControl(0.9) end
		if ( GetGamePlayerStateFlashlightControl() == 1.0 ) then SetGamePlayerStateFlashlightControl(0.1) end
	else
		if ( GetGamePlayerStateFlashlightControl() >= 0.89 ) then SetGamePlayerStateFlashlightControl(1.0) end
		if ( GetGamePlayerStateFlashlightControl() <= 0.11 ) then SetGamePlayerStateFlashlightControl(0.0) end
	end

	-- Pass movement state to player array
	SetGamePlayerStateMoving(0)
	if ( GetGamePlayerControlMovement() == 1 ) then 
		SetGamePlayerStateMoving(1)
		if ( GetGamePlayerStatePlayerDucking() ~= 0 ) then 
			SetGamePlayerStateMoving(2)
		end
	end

	-- Prevent player leaving terrain area
	if ( GetPlrObjectPositionX()<100 ) then SetGamePlayerControlPushangle(90.0) SetGamePlayerControlPushforce(1.0) end
	if ( GetPlrObjectPositionX()>51100 ) then SetGamePlayerControlPushangle(270.0) SetGamePlayerControlPushforce(1.0) end
	if ( GetPlrObjectPositionZ()<100 ) then SetGamePlayerControlPushangle(0.0) SetGamePlayerControlPushforce(1.0) end
	if ( GetPlrObjectPositionZ()>51100 ) then SetGamePlayerControlPushangle(180.0) SetGamePlayerControlPushforce(1.0) end

	-- Reduce any player push force over time
	if ( GetGamePlayerControlPushforce()>0 ) then 
		SetGamePlayerControlPushforce(GetGamePlayerControlPushforce()-(GetTimeElapsed()/2.0))
		if ( GetGamePlayerControlPushforce()<0 ) then 
			SetGamePlayerControlPushforce(0.0)
		end
	end

	-- Get current player ground height stood on
	if ( GetGamePlayerStateTerrainID()>0 ) then 
		SetGamePlayerStateTerrainHeight(GetGroundHeight(GetPlrObjectPositionX(),GetPlrObjectPositionZ()))
	else
		SetGamePlayerStateTerrainHeight(1000.0)
	end
	
	-- System to detect when player drops below terrain - hard coded

	-- Control player using Physics character controller
	ttgotonewheight=0
	SetGamePlayerStateJetpackVerticalMove(0.0)
	if ( GetGamePlayerControlGravityActive() == 0 ) then 
		-- in air
		if ( GetGamePlayerControlLockAtHeight() == 1 ) then 
			SetGamePlayerStateJetpackVerticalMove(0)
		else
			SetGamePlayerStateJetpackVerticalMove(WrapValue(GetCameraAngleX(0)))
			if ( g_PlrKeyS == 1 ) then SetGamePlayerStateJetpackVerticalMove(GetGamePlayerStateJetpackVerticalMove()-360.0) end
			if ( g_PlrKeyA == 1 ) then SetGamePlayerStateJetpackVerticalMove(0) end
			if ( g_PlrKeyD == 1 ) then SetGamePlayerStateJetpackVerticalMove(0) end
		end
		-- extra height control with mouse wheel
		if ( GetGamePlayerControlControlHeight()>0 or GetGamePlayerControlControlHeightCooldown()>0 ) then 
			SetGamePlayerControlStoreMovey(GetGamePlayerControlMovey())
			if ( GetGamePlayerControlControlHeight() == 3 ) then 
				if ( GetGamePlayerStateTerrainID()>0 ) then 
					ttgotonewheight=1000.0+GetGamePlayerStateTerrainHeight()
					SetGamePlayerControlControlHeight(0)
				end
			else
				if ( GetGamePlayerControlControlHeightCooldown() > 0.0 ) then
					if ( g_playercontrolcooldownmode == 1 ) then 
						SetGamePlayerControlMovey(GetGamePlayerControlMovey()+180)
						SetGamePlayerStateJetpackVerticalMove(-270)
					else
						SetGamePlayerStateJetpackVerticalMove(90)
					end
				else
					if ( GetGamePlayerControlControlHeight() == 1 ) then 
						-- move up
						SetGamePlayerControlMovey(GetGamePlayerControlMovey()+180)
						SetGamePlayerStateJetpackVerticalMove(-270)
					else
						-- move down
						SetGamePlayerStateJetpackVerticalMove(90)
					end
					g_playercontrolcooldownmode = GetGamePlayerControlControlHeight()
				end
			end
			if ( GetGamePlayerControlControlHeightCooldown() > 0 ) then
				SetGamePlayerControlControlHeightCooldown(GetGamePlayerControlControlHeightCooldown() - 1.0)
				SetGamePlayerControlSpeed(0)
			else
				SetGamePlayerControlSpeed(GetGamePlayerControlMaxspeed()*GetGamePlayerControlSpeedRatio())
			end
		end
	else
		-- regular gravity
		if ( GetGamePlayerControlPlrHitFloorMaterial() == 0 and GetGamePlayerControlJumpMode() ~= 1 ) then 
			SetGamePlayerStateJetpackVerticalMove(25.0)
		end
		-- add any JETPACK device effect
		if ( GetGamePlayerControlJetpackThrust()>0 ) then 
			SetGamePlayerControlWobble(0.0)
			ttgeardirection=WrapValue(GetCameraAngleX(0))
			if ( ttgeardirection>180 ) then ttgeardirection = ttgeardirection-360.0 end
			ttgeardirection=(ttgeardirection/90.0)*2.0
			if ( g_PlrKeyW == 1 ) then ttgeardirection = ttgeardirection+1.0 end
			if ( g_PlrKeyS == 1 ) then ttgeardirection = ttgeardirection-1.0 end
			ttnewspeedneed=GetGamePlayerControlMaxspeed()*ttgeardirection*GetGamePlayerControlJetpackThrust()
			if ( GetGamePlayerControlSpeed()<ttnewspeedneed ) then 
				SetGamePlayerControlSpeed(ttnewspeedneed)
			end
			SetGamePlayerControlMovey(CurveAngle(GetCameraAngleY(0),GetGamePlayerControlMovey(),5.0))
		end
	end

	-- handle slope climb prevention
	tsteepstart=30
	tsteepslopeweight=1.0
	if ( ttgotonewheight == 0 ) then 
		-- Player direction and speed
		ttfinalplrmovey=GetGamePlayerControlMovey()
		ttfinalplrspeed=GetGamePlayerControlSpeed()*2
		-- If not in air due to jetpack thurst
		if ( GetGamePlayerControlJetpackThrust() == 0 ) then 
			-- Player speed affected by gun speed modifier and zoom mode modifier
			if ( GetGamePlayerStateGunID()>0 ) then 
				if ( GetGamePlayerStateGameRunAsMultiplayer() == 0 and GetGamePlayerStateEnablePlrSpeedMods() == 1 ) then 
					-- only for single player action - MP play is too muddy!
					if ( GetFireModeSettingsIsEmpty() == 1 ) then 
						ttfinalplrspeed=ttfinalplrspeed*GetFireModeSettingsPlrEmptySpeedMod()
					else
						ttfinalplrspeed=ttfinalplrspeed*GetFireModeSettingsPlrMoveSpeedMod()
					end
					if ( GetGamePlayerStatePlrZoomIn()>1 ) then 
						ttfinalplrspeed=ttfinalplrspeed*GetFireModeSettingsZoomWalkSpeed()
					end
					if ( (GetGamePlayerStateGunMode() >= 121 and GetGamePlayerStateGunMode() <= 126) or (GetGamePlayerStateGunMode()>= 700 and GetGamePlayerStateGunMode() <= 707) or (GetGamePlayerStateGunMode() >= 7000 and GetGamePlayerStateGunMode()<= 7099) ) then 
						ttfinalplrspeed=ttfinalplrspeed*GetFireModeSettingsPlrReloadSpeedMod()
					end
				end
			end
			-- Determine slope angle of plr direction
			if ( GetGamePlayerStateTerrainID()>0 ) then 
				ttplrx=GetPlrObjectPositionX()
				ttplry=GetPlrObjectPositionY()
				ttplrz=GetPlrObjectPositionZ()
				ttplrgroundy=ttplry-34.0
				ttplrx2=NewXValue(ttplrx,ttfinalplrmovey,20.0)
				ttplrz2=NewZValue(ttplrz,ttfinalplrmovey,20.0)
				ttplrgroundy2=GetGroundHeight(ttplrx2,ttplrz2)
				if ( ttplrgroundy2>ttplrgroundy ) then 
					ttsteepvalue=ttplrgroundy2-ttplrgroundy
					if ( ttsteepvalue>tsteepstart ) then 
						-- terrain steeper than 45 degrees starts to slow down move speed
						ttfinalplrspeed=ttfinalplrspeed-((ttsteepvalue-tsteepstart)/tsteepslopeweight)
						if ( ttfinalplrspeed<0 ) then ttfinalplrspeed = 0 end
					end
				end
			end
		end

		-- Move player using Bullet character controller
		if ( GetGamePlayerStateRiftMode()>0 ) then ttfinalplrmovey = ttfinalplrmovey + 0 end
		if ( ttvrheadtrackermode == 1 ) then ttfinalplrmovey = ttfinalplrmovey - GetHeadTrackerYaw() end
		if ( GetGamePlayerControlIsRunning() == 1 ) then SetGamePlayerStateJetpackVerticalMove(0) end
		ControlDynamicCharacterController ( ttfinalplrmovey,GetGamePlayerStateJetpackVerticalMove(),ttfinalplrspeed,tplayerjumpnow,GetGamePlayerStatePlayerDucking(),GetGamePlayerControlPushangle(),GetGamePlayerControlPushforce(),GetGamePlayerControlJetpackThrust() )
		if GetDynamicCharacterControllerDidJump() == 1 then
			ttsnd = GetGamePlayerControlSoundStartIndex()+6
			if ( RawSoundExist(ttsnd) == 1 ) then
				PlayRawSound(ttsnd)
			end
		end
		if ( GetGamePlayerControlGravityActive() == 0 ) then 
			if ( GetGamePlayerControlControlHeight()>0 ) then 
				SetGamePlayerControlMovey(GetGamePlayerControlStoreMovey())
			end
		end

		-- If player under waterline, set playercontrol.underwater flag
		SetGamePlayerStatePlayerY(GetPlrObjectPositionY())
		if ( GetGamePlayerStatePlayerDucking() ~= 0 ) then 
			ttsubtleeyeadjustment=10.0
		else
			ttsubtleeyeadjustment=30.0
		end
		--if ( GetGamePlayerStatePlayerY()<GetGamePlayerStateWaterlineY()+20 and GetGamePlayerStateNoWater() == 0 ) then 
		if ( (GetGamePlayerStatePlayerY()+ttsubtleeyeadjustment)<GetGamePlayerStateWaterlineY() and GetGamePlayerStateNoWater() == 0 ) then 
			SetGamePlayerControlUnderwater(1)
			if ( g_PlayerUnderwaterMode == 1 ) then
				ChangePlayerWeaponID(0)
			end
		else
			SetGamePlayerControlUnderwater(0)
		end

		-- Act on any responses from character controller
		ttplrhitfloormaterial=GetCharacterHitFloor()
		if ( ttplrhitfloormaterial>0 ) then 
			SetGamePlayerControlPlrHitFloorMaterial(ttplrhitfloormaterial)
			ttplrfell=GetCharacterFallDistance()
			if ( GetGamePlayerStateImmunity() == 0 ) then 
				if ( ttplrfell>0 and (g_PlayerUnderwaterMode == 0 or GetGamePlayerStatePlayerY()>GetGamePlayerStateWaterlineY()) ) then 
					-- for a small landing, make a sound
					if ( ttplrfell>75 ) then 
						ttsnd=GetGamePlayerControlSoundStartIndex()+5
						if ( RawSoundExist(ttsnd)==1 ) then
							PlayRawSound(ttsnd)
						end
					end
					-- when player lands on Floor, it returns how much 'air' the player traversed
					if ( GetGamePlayerControlHurtFall()>0 ) then 
						if ( ttplrfell>GetGamePlayerControlHurtFall() ) then 
							ttdamage=ttplrfell-GetGamePlayerControlHurtFall()  
							HurtPlayer(-1,ttdamage)
						end
					end
				end
			end
		else
			SetGamePlayerControlPlrHitFloorMaterial(0)
		end
		
	else
		-- shift player to new height position ('M' key) then
		SetFreezePosition(GetPlrObjectPositionX(),ttgotonewheight,GetPlrObjectPositionZ())
		SetFreezeAngle(0,GetCameraAngleY(0),0)
		TransportToFreezePosition()
		--SetWorldGravity (  0,0,0 )
		if ( GetCameraOverride() ~= 2 and GetCameraOverride() ~= 3 ) then
			SetCameraAngle ( 0, 90, GetCameraAngleY(0), GetCameraAngleZ(0) )
		end
	end
	
	-- Place camera at player object position (stood or ducking) then
	if ( GetGamePlayerStatePlayerDucking() ~= 0 ) then 
		ttsubtleeyeadjustment=10.0
	else
		ttsubtleeyeadjustment=30.0
	end
	SetGamePlayerStateAdjustBasedOnWobbleY((math.cos(math.rad(GetGamePlayerControlWobble()))*GetGamePlayerControlWobbleHeight()))
	if ( GetGamePlayerStateSlowMotion() == 1 and GetGamePlayerStateSmoothCameraKeys() == 1 ) then 
		SetGamePlayerStateFinalCamX(CurveValue(GetPlrObjectPositionX(),GetGamePlayerStateFinalCamX(),10.0))
		SetGamePlayerStateFinalCamY(CurveValue(ttsubtleeyeadjustment+GetPlrObjectPositionY()-GetGamePlayerStateAdjustBasedOnWobbleY(),GetGamePlayerStateFinalCamY(),10.0))
		SetGamePlayerStateFinalCamZ(CurveValue(GetPlrObjectPositionZ(),GetGamePlayerStateFinalCamZ(),10.0))
	else
		ttleanangle=GetCameraAngleY(0)+GetGamePlayerControlLeanoverAngle()
		ttleanoverx=NewXValue(0,ttleanangle,GetGamePlayerControlLeanover())
		ttleanoverz=NewZValue(0,ttleanangle,GetGamePlayerControlLeanover())
		SetGamePlayerStateFinalCamX(GetPlrObjectPositionX()+ttleanoverx)
		SetGamePlayerStateFinalCamY(ttsubtleeyeadjustment+GetPlrObjectPositionY()-GetGamePlayerStateAdjustBasedOnWobbleY())
		SetGamePlayerStateFinalCamZ(GetPlrObjectPositionZ()+ttleanoverz)
	end

	-- Apply a shake vector to camera position
	SetGamePlayerStateShakeX(0)
	SetGamePlayerStateShakeY(0)
	if ( GetGamePlayerControlCameraShake()>0.0 ) then 
		SetGamePlayerControlCameraShake(GetGamePlayerControlCameraShake()-1.0)
		ttcamshakelimit = GetGamePlayerControlCameraShake()  
		if ( ttcamshakelimit>5.0 ) then ttcamshakelimit = 5.0 end
		SetGamePlayerStateShakeX(NewXValue(0,GetCameraAngleY(0)+90,GetGamePlayerControlCameraShake()/2.0))
		SetGamePlayerStateShakeZ(NewZValue(0,GetCameraAngleY(0)+90,GetGamePlayerControlCameraShake()/2.0))
		SetGamePlayerStateShakeY(ttcamshakelimit)
		ttodd=math.floor(GetGamePlayerControlCameraShake()/2.0)*2
		if ( (math.floor(GetGamePlayerControlCameraShake())-ttodd) == 0 ) then 
			SetGamePlayerStateShakeX(GetGamePlayerStateShakeX()*-1)
			SetGamePlayerStateShakeY(GetGamePlayerStateShakeY()*-1)
			SetGamePlayerStateShakeZ(GetGamePlayerStateShakeZ()*-1)
		end
		if ( GetGamePlayerControlCameraShake()<0.0 ) then 
			SetGamePlayerControlCameraShake(0.0)
		end
	end
	
	-- record present camera angles
	SetGamePlayerControlFinalCameraAnglex(GetCameraAngleX(0))
	SetGamePlayerControlFinalCameraAngley(GetCameraAngleY(0))
	SetGamePlayerControlFinalCameraAnglez(GetCameraAngleZ(0))
	if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
		-- ensure camerafocus is restored each cycle
		SetGamePlayerControlFinalCameraAnglex(GetGamePlayerControlFinalCameraAnglex()+GetGamePlayerControlThirdpersonCameraFocus())
		-- take over control of a character in the game
		ttpersone=GetGamePlayerControlThirdpersonCharactere()  
		ttpersonobj=0
		if ( ttpersone>0 ) then 
			ttpersonobj=GetEntityElementObj(ttpersone)
			if ( GetObjectExist(ttpersonobj) == 0 ) then ttpersonobj = 0 end
		end
		if ( ttpersonobj == 0 ) then 
			return 
		end
		-- person coordinate
		ttpx=GetPlrObjectPositionX()
		ttpy=GetPlrObjectPositionY()+35
		ttpz=GetPlrObjectPositionZ()

		-- stop control if in ragdoll (dying) then
		if ( GetEntityElementRagdollified(ttpersone) ~= 0 ) then 
			return
		end
		
		-- control animation of third person character
		SetGamePlayerStateCharAnimIndex(GetGamePlayerControlThirdpersonCharacterIndex())
		CopyCharAnimState(GetGamePlayerStateCharAnimIndex(),0)
		SetCharAnimStateE(0,ttpersone)
		if ( GetGamePlayerControlMovement() ~= 0 ) then 
			if ( GetGamePlayerControlThirdpersonEnabled() == 1 and GetGamePlayerControlThirdpersonCameraFollow() == 1 and GetGamePlayerControlCamRightMouseMode() == 0 ) then 
				-- anim based on WASD run mode
				if ( GetGamePlayerControlIsRunning() == 1 ) then 
					SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),5)
				else
					SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),1)
				end
			else
				-- anim based on camera
				if ( GetGamePlayerControlIsRunning() == 1 ) then 
					if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()-45 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()-90 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()-45-90 ) then 
						SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),7)
					else
						if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()+45 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()+90 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()+45+90 ) then 
							SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),8)
						else
							if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()+180 ) then 
								SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),2)
							else
								SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),5)
							end
						end
					end
				else
					if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()-45 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()-90 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()-45-90 ) then 
						SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),3)
					else
						if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()+45 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()+90 or GetGamePlayerControlMovey() == GetGamePlayerControlCy()+45+90 ) then 
							SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),4)
						else
							if ( GetGamePlayerControlMovey() == GetGamePlayerControlCy()+180 ) then 
								SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),2)
							else
								SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),1)
							end
						end
					end
				end
			end
		else
			SetCharAnimControlsMoving(GetGamePlayerStateCharAnimIndex(),0)
		end

		-- character Animation Speed
		if ( GetObjectSpeed(GetCharAnimStateObj(0))<0 ) then
			tpolarity = -1 
		else 
			tpolarity = 1
		end
		ttfinalspeed=GetEntityElementSpeedModulator(GetCharAnimStateE(0))*GetCharAnimStateAnimationSpeed(0)*tpolarity*2.5*GetTimeElapsed()
		SetObjectSpeed ( GetCharAnimStateObj(0),ttfinalspeed )

		-- run character animation system
		RunCharLoop()

		-- ensure 'jump' animation is intercepted so anim frame controlled on decent
		fStartFrame=0
		fHoldFrame=0
		fResumeFrame=0
		fFinishFrame=0
		ttentid=GetEntityElementBankIndex(GetCharAnimStateOriginalE(0))
		tq = GetEntityProfileStartOfAIAnim(ttentid) + GetCsiStoodVault(1)
		fStartFrame = GetEntityAnimStart(ttentid,tq)
		fFinishFrame = GetEntityAnimFinish(ttentid,tq)
		if ( GetEntityProfileJumpHold(ttentid) > 0 ) then
			fHoldFrame = GetEntityProfileJumpHold(ttentid)
		else
			fHoldFrame = GetEntityAnimFinish(ttentid,tq) - 10
		end
		if ( GetEntityProfileJumpResume(ttentid) > 0 ) then
			fResumeFrame = GetEntityProfileJumpResume(ttentid)
		else
			fResumeFrame = GetEntityAnimFinish(ttentid,tq)
		end
		if ( GetObjectFrame(GetCharAnimStateObj(0)) >= fStartFrame and GetObjectFrame(GetCharAnimStateObj(0)) <= fFinishFrame ) then 
			-- until we hit terra firma, hold the last 'air' frame
			ttframeposyadjustindex=math.floor(GetObjectFrame(GetCharAnimStateObj(0)))
			ttearlyoutframe = fHoldFrame
			if ( GetGamePlayerControlJumpMode() == 1 and ttframeposyadjustindex>ttearlyoutframe ) then 
				SetObjectFrame ( GetCharAnimStateObj(0),ttearlyoutframe+1.0 )
			end
		end

		-- restore entry in character array
		SetCharAnimStateE(0,0)
		CopyCharAnimState(0,GetGamePlayerStateCharAnimIndex())

		-- transfer first person camera awareness to character
		tttargetx=GetPlrObjectPositionX()
		tttargety=GetPlrObjectPositionY()-35.0
		tttargetz=GetPlrObjectPositionZ()
		PositionObject ( ttpersonobj,tttargetx,tttargety,tttargetz )

		-- in follow mode, only rotate protagonist if moving OR shooting
		ttokay=0
		if ( GetGamePlayerControlThirdpersonCameraFollow() == 0 ) then ttokay = 1 end
		if ( GetGamePlayerControlThirdpersonCameraFollow() == 1 ) then 
			ttokay=2
			if ( GetGamePlayerControlCamRightMouseMode() == 1 ) then ttokay = 1 end
			if ( GetGamePlayerControlThirdpersonShotFired() == 1 ) then 
				SetGamePlayerControlThirdpersonShotFired(0) 
				ttokay = 1 
			end
		end
		if ( ttokay == 1 ) then 
			RotateObject (  ttpersonobj,0,GetCameraAngleY(0),0 )
		end
		if ( ttokay == 2 and (g_PlrKeyW == 1 or g_PlrKeyA == 1 or g_PlrKeyS == 1 or g_PlrKeyD == 1) ) then 
			if ( g_PlrKeyW == 1 ) then 
				if ( g_PlrKeyA == 1 ) then 
					ttadjusta=-45.0
				else
					if ( g_PlrKeyD == 1 ) then 
						ttadjusta=45.0
					else
						ttadjusta=0.0
					end
				end
			else
				if ( g_PlrKeyS == 1 ) then 
					if ( g_PlrKeyA == 1 ) then 
						ttadjusta=225.0
					else
						if ( g_PlrKeyD == 1 ) then 
							ttadjusta=135.0
						else
							ttadjusta=180.0
						end
					end
				else
					if ( g_PlrKeyA == 1 ) then ttadjusta = -90.0 end
					if ( g_PlrKeyD == 1 ) then ttadjusta = 90.0 end
				end
			end
			ttfinalrotspeed=2.0/GetTimeElapsed()
			ttfinalobjroty=CurveAngle(GetCameraAngleY(0)+ttadjusta,GetObjectAngleY(ttpersonobj),ttfinalrotspeed)
			RotateObject ( ttpersonobj,0,ttfinalobjroty,0 )
		end

		-- cap vertical angle
		ttcapverticalangle=GetGamePlayerControlFinalCameraAnglex()
		if ( ttcapverticalangle>90 ) then ttcapverticalangle = 90 end
		if ( ttcapverticalangle<-12 ) then ttcapverticalangle = -12 end

		-- no retract system (now uses new interpolation for smoother transitions below)
		ttusecamdistance=GetGamePlayerControlThirdpersonCameraDistance()--GetGamePlayerControlCamCurrentDistance()

		-- work out camera distance and height based on vertical angle (GTA) then
		if ( GetGamePlayerControlThirdpersonCameraLocked() == 1 ) then 
			ttusecamdist=GetGamePlayerControlThirdpersonCameraDistance()
			ttusecamheight=GetGamePlayerControlThirdpersonCameraHeight()
		else
			ttusecamdist=ttusecamdistance
			ttusecamdist=(ttusecamdist*((102.0-(ttcapverticalangle+12.0))/102.0))
			ttusecamheight=(GetGamePlayerControlThirdpersonCameraHeight()*2)-5.0
			ttusecamheight=25.0+(ttusecamheight*((ttcapverticalangle+12.0)/102.0))
		end

		-- the ideal final position of the camera fully extended
		if ( GetGamePlayerControlThirdpersonCameraFollow() == 1 ) then 
			tdaa=WrapValue(180-(0-GetCameraAngleY(0)))
		else
			tdaa=WrapValue(180-(0-GetObjectAngleY(ttpersonobj)))
		end
		ttadjx=(math.sin(math.rad(tdaa))*ttusecamdist)
		tffdcx=tttargetx+ttadjx
		ttadjy=ttusecamheight
		tffdcy=tttargety+ttadjy
		ttadjz=(math.cos(math.rad(tdaa))*ttusecamdist)
		tffdcz=tttargetz+ttadjz
		tffidealdist=math.sqrt(math.abs(tffdcx*tffdcx)+math.abs(tffdcy*tffdcy)+math.abs(tffdcz*tffdcz))
		tffidealx=tffdcx
		tffidealy=tffdcy
		tffidealz=tffdcz		

		-- if fixed view, get X angle before collision adjustment
		if ( GetCameraOverride() ~= 1 and GetCameraOverride() ~= 3 ) then
			if ( GetGamePlayerControlThirdpersonCameraLocked() == 1 ) then 
				PositionCamera ( 0,tffdcx,tffdcy,tffdcz )
				PointCamera ( 0,tttargetx,tttargety,tttargetz )
				ttcapverticalangle=GetCameraAngleX(0)
			end
		end

		-- special mode which ignores camera collision (for isometric style games) then
		bIgnoreCameraCollision = false
		if ( GetGamePlayerControlThirdpersonCameraHeight() > GetGamePlayerControlThirdpersonCameraDistance()/2.0 ) then
			bIgnoreCameraCollision = true
		end

		-- check if camera in collision
		tthitdist=0
		tthitvalue=0
		tthitdiffdist1=0
		if ( bIgnoreCameraCollision==false ) then
			tthitdiffy=0
			tthitvaluey=0
			if ( RayTerrain(ttpx,ttpy-20,ttpz,tffdcx,tffdcy-20,tffdcz) == 1 ) then 
				tthitvaluex=GetRayCollisionX()
				tthitvaluey=GetRayCollisionY()+20
				tthitvaluez=GetRayCollisionZ()
				tthitdiffx=tthitvaluex-ttpx
				tthitdiffy=tthitvaluey-ttpy
				tthitdiffz=tthitvaluez-ttpz
				tthitdiffdist1=math.sqrt(math.abs(tthitdiffx*tthitdiffx)+math.abs(tthitdiffy*tthitdiffy)+math.abs(tthitdiffz*tthitdiffz))
			end
			tthitvalue=IntersectStatic(ttpx,ttpy,ttpz,tffdcx,tffdcy,tffdcz,ttpersonobj)
			if ( tthitvalue > 0 ) then 
				tthitvaluex=GetIntersectCollisionX()
				tthitvaluey=GetIntersectCollisionY()
				tthitvaluez=GetIntersectCollisionZ()
				tthitdiffx=tthitvaluex-ttpx
				tthitdiffy=tthitvaluey-ttpy
				tthitdiffz=tthitvaluez-ttpz
				tthitdiffdist2=math.sqrt(math.abs(tthitdiffx*tthitdiffx)+math.abs(tthitdiffy*tthitdiffy)+math.abs(tthitdiffz*tthitdiffz))
				tthitdist=tthitdiffdist2
				if ( tthitdiffdist1 > 0 ) then
					if ( tthitdiffdist1 < tthitdiffdist2 ) then
						tthitvaluex=GetRayCollisionX()
						tthitvaluey=GetRayCollisionY()+20
						tthitvaluez=GetRayCollisionZ()
						tthitdist=tthitdiffdist1
						tthitvalue = -1
					end
				end
			else
				if ( tthitdiffdist1 ~= 0 ) then
					tthitvaluex=GetRayCollisionX()
					tthitvaluey=GetRayCollisionY()+20
					tthitvaluez=GetRayCollisionZ()
					tthitdist=tthitdiffdist1
					tthitvalue = -1
				end
			end			
			if tthitdist > 0 and tthitdist < 30 and math.abs(tthitdiffy) < 20 then
				-- minimum distance pushes camera UP to prevent head penetration
				tthitvaluey = tthitvaluey + (30-tthitdist)
			end
		end
		if ( tthitvalue ~= 0 ) then 
			-- work out new camera position a touch forward to miss wall intersection
			tffdcx=tthitvaluex
			tffdcy=tthitvaluey
			tffdcz=tthitvaluez
			ttcoldistX = ((tttargetx-tthitvaluex)/tthitdist)*2
			ttcoldistZ = ((tttargetz-tthitvaluez)/tthitdist)*2
			tffdcx=tffdcx+ttcoldistX
			tffdcz=tffdcz+ttcoldistZ
		end
			
		-- push camera away from any surface in all cases
		if ( bIgnoreCameraCollision==false ) then
			if ( IntersectStatic(tffdcx,tffdcy,tffdcz,tffdcx-8,tffdcy,tffdcz,ttpersonobj) > 0 ) then 
			 tffdcx=GetIntersectCollisionX()+(GetIntersectCollisionNX()*8.0)
			 tffdcy=GetIntersectCollisionY()+(GetIntersectCollisionNY()*8.0)
			 tffdcz=GetIntersectCollisionZ()+(GetIntersectCollisionNZ()*8.0)
			 tthitdist=1
			end
			if ( IntersectStatic(tffdcx,tffdcy,tffdcz,tffdcx+8,tffdcy,tffdcz,ttpersonobj) > 0 ) then 
			 tffdcx=GetIntersectCollisionX()+(GetIntersectCollisionNX()*8.0)
			 tffdcy=GetIntersectCollisionY()+(GetIntersectCollisionNY()*8.0)
			 tffdcz=GetIntersectCollisionZ()+(GetIntersectCollisionNZ()*8.0)
			 tthitdist=1
			end
			if ( IntersectStatic(tffdcx,tffdcy,tffdcz,tffdcx,tffdcy,tffdcz-8,ttpersonobj) > 0 ) then 
			 tffdcx=GetIntersectCollisionX()+(GetIntersectCollisionNX()*8.0)
			 tffdcy=GetIntersectCollisionY()+(GetIntersectCollisionNY()*8.0)
			 tffdcz=GetIntersectCollisionZ()+(GetIntersectCollisionNZ()*8.0)
			 tthitdist=1
			end
			if ( IntersectStatic(tffdcx,tffdcy,tffdcz,tffdcx,tffdcy,tffdcz+8,ttpersonobj) > 0 ) then 
			 tffdcx=GetIntersectCollisionX()+(GetIntersectCollisionNX()*8.0)
			 tffdcy=GetIntersectCollisionY()+(GetIntersectCollisionNY()*8.0)
			 tffdcz=GetIntersectCollisionZ()+(GetIntersectCollisionNZ()*8.0)
			 tthitdist=1
			end
		end

		-- work out if new position closer or further
		if ( tthitdist ~= 0 ) then
			-- instantly switch to closer position
			ttdiffx1=GetPlrObjectPositionX()-tffidealx
			ttdiffy1=GetPlrObjectPositionY()-tffidealy
			ttdiffz1=GetPlrObjectPositionZ()-tffidealz
			ttdiffd1=math.sqrt(math.abs(ttdiffx1*ttdiffx1)+math.abs(ttdiffy1*ttdiffy1)+math.abs(ttdiffz1*ttdiffz1))
			ttdiffx2=GetPlrObjectPositionX()-tffdcx
			ttdiffy2=GetPlrObjectPositionY()-tffdcy
			ttdiffz2=GetPlrObjectPositionZ()-tffdcz
			ttdiffd2=math.sqrt(math.abs(ttdiffx2*ttdiffx2)+math.abs(ttdiffy2*ttdiffy2)+math.abs(ttdiffz2*ttdiffz2))
			if ( ttdiffd2 < ttdiffd1 ) then
				-- smooth value controls interpolation of camera position
				SetGamePlayerControlCamCollisionSmooth(200)
				SetGamePlayerControlLastGoodcx(tffdcx-tttargetx)
				SetGamePlayerControlLastGoodcy(tffdcy-tttargety)
				SetGamePlayerControlLastGoodcz(tffdcz-tttargetz)
			end
		else
			ttadjx=(math.sin(math.rad(tdaa))*ttusecamdist)
			ttadjy=ttusecamheight
			ttadjz=(math.cos(math.rad(tdaa))*ttusecamdist)
			ttsmooth=GetGamePlayerControlCamCollisionSmooth()
			if ( ttsmooth > 0 ) then
				ttadjx2=GetGamePlayerControlLastGoodcx()
				ttadjy2=GetGamePlayerControlLastGoodcy()
				ttadjz2=GetGamePlayerControlLastGoodcz()
				ttdiffd2=math.sqrt(math.abs(ttadjx2*ttadjx2)+math.abs(ttadjz2*ttadjz2))
				ttadjx2=(math.sin(math.rad(tdaa))*ttdiffd2)
				ttadjz2=(math.cos(math.rad(tdaa))*ttdiffd2)
				if ( ttsmooth > 100 ) then
					ttadjx=ttadjx2
					ttadjy=ttadjy2
					ttadjz=ttadjz2
				else
					ttadjx=((ttadjx/100.0)*(100-ttsmooth))+((ttadjx2/100.0)*ttsmooth)
					ttadjy=((ttadjy/100.0)*(100-ttsmooth))+((ttadjy2/100.0)*ttsmooth)
					ttadjz=((ttadjz/100.0)*(100-ttsmooth))+((ttadjz2/100.0)*ttsmooth)
				end
				SetGamePlayerControlCamCollisionSmooth(ttsmooth-10)
			end
			tffdcx=tttargetx+ttadjx
			tffdcy=tttargety+ttadjy
			tffdcz=tttargetz+ttadjz
		end
		
		-- future idea is detect 'canopy' collision and lower camera height to 'duck under' (would look nice)
		
		-- set camera position
		tdcx=tffdcx  
		tdcy=tffdcy  
		tdcz=tffdcz
		ttcx=tdcx
		ttcy=tdcy
		ttcz=tdcz
		if ( GetCameraOverride() ~= 1 and GetCameraOverride() ~= 3 ) then
			PositionCamera ( 0, ttcx, ttcy, ttcz )
		end

		-- adjust horizon angle
		ttcapverticalangle=ttcapverticalangle-GetGamePlayerControlThirdpersonCameraFocus()

		-- final camera rotation
		if ( GetCameraOverride() ~= 2 and GetCameraOverride() ~= 3 ) then
			SetCameraAngle ( 0,ttcapverticalangle,GetGamePlayerControlFinalCameraAngley(),GetGamePlayerControlFinalCameraAnglez() )
		end
		
	else
		-- update camera position
		if ( GetCameraOverride() ~= 1 and GetCameraOverride() ~= 3 ) then
			PositionCamera (  0,GetGamePlayerStateFinalCamX()+GetGamePlayerStateShakeX(),GetGamePlayerStateFinalCamY()+GetGamePlayerStateShakeY(),GetGamePlayerStateFinalCamZ()+GetGamePlayerStateShakeZ() )
		end
		-- Apply flinch (for when hit hit) then
		if ( GetCameraOverride() ~= 2 and GetCameraOverride() ~= 3 ) then
			SetGamePlayerControlFlinchx(CurveValue(0,GetGamePlayerControlFlinchx(),20))
			SetGamePlayerControlFlinchy(CurveValue(0,GetGamePlayerControlFlinchy(),20))
			SetGamePlayerControlFlinchz(CurveValue(0,GetGamePlayerControlFlinchz(),20))
			SetGamePlayerControlFlinchCurrentx(CurveValue(GetGamePlayerControlFlinchx(),GetGamePlayerControlFlinchCurrentx(),10.0))
			SetGamePlayerControlFlinchCurrenty(CurveValue(GetGamePlayerControlFlinchy(),GetGamePlayerControlFlinchCurrenty(),10.0))
			SetGamePlayerControlFlinchCurrentz(CurveValue(GetGamePlayerControlFlinchz(),GetGamePlayerControlFlinchCurrentz(),10.0))
			SetCameraAngle ( 0,GetCameraAngleX(0)+GetGamePlayerControlFlinchCurrentx(),GetCameraAngleY(0)+GetGamePlayerControlFlinchCurrenty(),GetCameraAngleZ(0)+GetGamePlayerControlFlinchCurrentz()+(GetGamePlayerControlLeanover()/-10.0) )
		end
	end

	-- player footfalls
	SetGamePlayerControlFootfallType(-1)
	if ( GetGamePlayerControlGravityActive() == 1 ) then 
		if ( GetGamePlayerStatePlayerDucking() == 0 ) then 
			SetGamePlayerStatePlayerY(GetPlrObjectPositionY())
		else
			SetGamePlayerStatePlayerY(GetPlrObjectPositionY()+9)
		end
		if ( GetGamePlayerStatePlayerY()>GetGamePlayerStateWaterlineY()+36 or GetGamePlayerStateNoWater() ~= 0 ) then 
			-- takes physcs hit material as Floor (  material ) then
			if ( GetGamePlayerControlPlrHitFloorMaterial()>0 ) then 
				SetGamePlayerControlFootfallType(GetGamePlayerControlPlrHitFloorMaterial()-1)
			else
				SetGamePlayerControlFootfallType(0)
			end
		else
			if ( GetGamePlayerStatePlayerY()>GetGamePlayerStateWaterlineY()-33 ) then 
				-- Footfall water wading sound
				SetGamePlayerControlFootfallType(17)
				-- And also trigger ripples in water
				SetGamePlayerControlRippleCount(GetGamePlayerControlRippleCount()+GetTimeElapsed())
				if ( GetGamePlayerControlMovement() ~= 0  ) then SetGamePlayerControlRippleCount(GetGamePlayerControlRippleCount()+(GetTimeElapsed()*3)) end
				if ( GetGamePlayerControlRippleCount()>20.0 ) then 
					SetGamePlayerControlRippleCount(0.0)
					TriggerWaterRipple(GetPlrObjectPositionX(),GetGamePlayerStateWaterlineY()+1.0,GetPlrObjectPositionZ())
				end
			else
				SetGamePlayerControlFootfallType(18)
			end
		end
	end

	-- Manage trigger of footfall sound effects
	if ( GetGamePlayerControlFootfallType() ~= -1 ) then 
		if ( GetGamePlayerControlMovement() == 0 or GetGamePlayerControlJumpMode() == 1 ) then 
			-- no footfalls here
		else
			tttriggerfootsound=0
			if ( GetGamePlayerControlThirdpersonEnabled() == 1 ) then 
				-- third person uses protagonist animation to trigger footfalling
				tttobjframe=0.0
				tttobj=GetEntityElementObj(GetGamePlayerControlThirdpersonCharactere())
				if ( tttobj>0 ) then 
					if ( GetObjectExist(tttobj) == 1 ) then 
						tttobjframe=GetObjectFrame(tttobj)
					end
				end
				-- anim based footfall/groundsound triggers (hard coded) then
				for twalktypes = 1, 13, 1 do
					if ( twalktypes == 1 ) then ttleftframe = 306 ttrightframe = 323 end
					if ( twalktypes == 2 ) then ttleftframe = 349 ttrightframe = 369 end
					if ( twalktypes == 3 ) then ttleftframe = 386 ttrightframe = 401 end
					if ( twalktypes == 4 ) then ttleftframe = 429 ttrightframe = 442 end
					if ( twalktypes == 5 ) then ttleftframe = 470 ttrightframe = 479 end
					if ( twalktypes == 6 ) then ttleftframe = 528 ttrightframe = 538 end
					if ( twalktypes == 7 ) then ttleftframe = 555 ttrightframe = 569 end
					if ( twalktypes == 8 ) then ttleftframe = 601 ttrightframe = 615 end
					if ( twalktypes == 9 ) then ttleftframe = 641 ttrightframe = 654 end
					if ( twalktypes == 10 ) then ttleftframe = 674 ttrightframe = 684 end
					if ( twalktypes == 11 ) then ttleftframe = 705 ttrightframe = 716 end
					if ( twalktypes == 12 ) then ttleftframe = 756 ttrightframe = 759 end
					if ( twalktypes == 13 ) then ttleftframe = 817 ttrightframe = 819 end
					if ( tttobjframe >= ttleftframe and tttobjframe <= ttrightframe+1 ) then 
						if ( GetGamePlayerControlFootfallCount() == 0 and tttobjframe>ttleftframe and tttobjframe<ttrightframe ) then 
							-- left foot
							tttriggerfootsound=1
							SetGamePlayerControlFootfallCount(1)
						end
						if ( GetGamePlayerControlFootfallCount() == 1 and tttobjframe>ttrightframe ) then 
							-- right foot
							tttriggerfootsound=1
							SetGamePlayerControlFootfallCount(0)
						end
					end
				end
			else
				-- FPS triggers footfall based on wobble progress
				if ( GetGamePlayerControlPlrHitFloorMaterial() ~= 0 ) then 
					if GetGamePlayerControlWobble() > 0.0 then 
						ttFootfallPaceMultiplier = 1.0/(GetGamePlayerControlFootfallPace()/3.0)
						ttWeaponMoveSpeedMod = GetFireModeSettingsPlrMoveSpeedMod()
						if ttWeaponMoveSpeedMod == 0.0 then ttWeaponMoveSpeedMod = 1.0 end
						if ttWeaponMoveSpeedMod < 0.4 then ttWeaponMoveSpeedMod = 0.4 end
						ttAddWobbleStep = GetGamePlayerControlWobbleSpeed()*GetElapsedTime()*GetGamePlayerControlBasespeed()*GetGamePlayerControlSpeedRatio()*ttFootfallPaceMultiplier*ttWeaponMoveSpeedMod
						g_FootFallTimer = g_FootFallTimer + ttAddWobbleStep
						if g_FootFallTimer > 315 and GetGamePlayerControlFootfallCount() == 0 then
							SetGamePlayerControlFootfallCount(1)
							tttriggerfootsound=1
						end
						if g_FootFallTimer < 315 and GetGamePlayerControlFootfallCount() == 1 then 
							SetGamePlayerControlFootfallCount(0)
						end
						if g_FootFallTimer >= 360 then
							g_FootFallTimer = g_FootFallTimer - 360
						end
					end
				end
			end
			if ( tttriggerfootsound == 1 ) then 
				-- play footfall sound effect at specified position
				SetGamePlayerControlLastFootfallSound( PlayFootfallSound ( GetGamePlayerControlFootfallType(), GetCameraPositionX(0), GetCameraPositionY(0), GetCameraPositionZ(0), GetGamePlayerControlLastFootfallSound() ) )
			end
		end
	end

	-- drowning
	if ( g_PlayerHealth == 0 or GetGamePlayerStateEditModeActive() ~= 0 or GetGamePlayerStateNoWater() == 1 ) then 
		ResetUnderwaterState()
		---return
	end

	-- Water states 0 out of water, 1 in water, on surface, 2 under water, 3 under water, drowning, 4 dead underwater
	if ( GetGamePlayerStateNoWater() == 0 ) then 
		-- check for in water
		if ( GetCameraPositionY(0) <= GetGamePlayerStateWaterlineY() + 40 ) then 
			-- just entered water
			if ( GetGamePlayerControlInWaterState() == 0 ) then 
				ttsnd = GetGamePlayerControlSoundStartIndex()+13
				if ( RawSoundExist ( ttsnd ) == 1 ) then
					if ( g_PlayerHealth > 0 ) then
						PlayRawSound ( ttsnd )
					end
				end
				SetGamePlayerControlInWaterState(1)
			end

			-- PE: Simple system to get us up, when below water, this is where the real swimming below water should be made.
			-- allow swimming with head above water.
			if ( GetCameraPositionY(0) <= GetGamePlayerStateWaterlineY()+8.0 ) then 
				-- check for space to move player slowly up when underwater.
				if ( g_PlayerUnderwaterMode == 1 and g_PlrKeySPACE == 1 ) then
					if( GetCameraPositionY(0) >= GetGamePlayerStateWaterlineY()+7.0 ) then
						SetFreezePosition(GetPlrObjectPositionX(),GetPlrObjectPositionY()+0.01,GetPlrObjectPositionZ()) -- slow down
					elseif( GetCameraPositionY(0) >= GetGamePlayerStateWaterlineY()+6.0 ) then
						SetFreezePosition(GetPlrObjectPositionX(),GetPlrObjectPositionY()+0.075,GetPlrObjectPositionZ()) -- slow down
					else
						SetFreezePosition(GetPlrObjectPositionX(),GetPlrObjectPositionY()+0.25,GetPlrObjectPositionZ()) -- full speed up
					end
					SetFreezeAngle(GetCameraAngleX(0),GetCameraAngleY(0),GetCameraAngleZ(0))
					TransportToFreezePosition()
				end
			end
			-- going under water
			if ( GetCameraPositionY(0) <= GetGamePlayerStateWaterlineY() ) then 
				-- head goes under water
				if ( GetGamePlayerControlInWaterState() < 2 ) then 
					SetGamePlayerControlInWaterState(2)
					SetUnderwaterOn()
					if( g_PlayerUnderwaterMode == 1 ) then
						-- added delay before drowning damage starts
						SetGamePlayerControlDrownTimestamp(Timer()+15000)
					end
				end
			
				-- check for drowning
				if ( GetGamePlayerControlDrownTimestamp() == 0 ) then 
					if( g_PlayerUnderwaterMode == 1 ) then
						SetGamePlayerControlDrownTimestamp(Timer()+5000)
					else
						SetGamePlayerControlDrownTimestamp(Timer()+500)
					end
				else
					-- if g_PlayerUnderwaterMode == 1 lose 1 health per second until dead , else 50 health 
					if ( Timer() > GetGamePlayerControlDrownTimestamp() ) then 
						-- if there was no start marker, reset player (cannot kill, as no start marker) then. Indicated by crazy health and no lives
						if ( g_PlayerLives == 0 and g_PlayerHealth == 99999 ) then 
							-- start death sequence for player
							SetGamePlayerControlDeadTime(Timer()+2000)
							g_PlayerHealth=0
						else
							-- Gulp in water for plr damage
							if( g_PlayerUnderwaterMode == 1 ) then
								SetGamePlayerControlDrownTimestamp(Timer()+5000)
								DrownPlayer(-1,5)
							else
								SetGamePlayerControlDrownTimestamp(Timer()+500)
								DrownPlayer(-1,200)
							end
						end
						-- if player died
						if ( g_PlayerHealth == 0 ) then 
							SetGamePlayerControlInWaterState(4)
						else
							SetGamePlayerControlInWaterState(3)
						end
						ttsnd = GetGamePlayerControlSoundStartIndex()+11
						if ( RawSoundExist ( ttsnd ) == 1 ) then
							PlayRawSound ( ttsnd )
						end
					end
				end
				-- underwater swimming sound
				if ( Timer() > GetGamePlayerControlSwimTimestamp() + 2000 ) then 
					SetRawSoundSpeed(GetGamePlayerControlSoundStartIndex()+15,36000+math.random(0,4000))
					ttsnd = GetGamePlayerControlSoundStartIndex()+15
					if ( RawSoundExist ( ttsnd ) == 1 ) then
						PlayRawSound ( ttsnd )
					end
					SetGamePlayerControlSwimTimestamp(Timer()+2000)
				end
			else
				-- head comes out of water
				if ( GetGamePlayerControlInWaterState() > 1 ) then 
					-- if we were drowning, gasp for air
					if ( GetGamePlayerControlInWaterState() == 3 ) then 
						ttsnd = GetGamePlayerControlSoundStartIndex()+12
						if ( RawSoundExist ( ttsnd ) == 1 ) then
							PlayRawSound ( ttsnd )
						end
					end
					SetGamePlayerControlInWaterState(1)
					SetGamePlayerControlDrownTimestamp(0)
					SetUnderwaterOff()
				end
				-- swimming sound
				if ( Timer() > GetGamePlayerControlSwimTimestamp() + 2000 ) then 
					SetRawSoundSpeed(GetGamePlayerControlSoundStartIndex()+15,42000+math.random(0,4000))
					ttsnd = GetGamePlayerControlSoundStartIndex()+15
					if ( RawSoundExist ( ttsnd ) == 1 ) then
						PlayRawSound ( ttsnd )
					end
					SetGamePlayerControlSwimTimestamp(Timer()+2000)
				end
			end
		else
			-- climb out of water
			if ( GetGamePlayerControlInWaterState() > 0 ) then 
				ttsnd = GetGamePlayerControlSoundStartIndex()+14
				if ( RawSoundExist ( ttsnd ) == 1 ) then
					PlayRawSound ( ttsnd )
				end
				SetGamePlayerControlInWaterState(0)
				SetUnderwaterOff()
			end
		end
	else
		SetGamePlayerControlInWaterState(0)
		SetUnderwaterOff()
	end

	-- Screen REDNESS effect, and heartbeat, player is healthy, so fade away from redness
	if ( g_PlayerHealth >= 100 ) then 
		if ( GetGamePlayerControlRedDeathFog() > 0 ) then 
			SetGamePlayerControlRedDeathFog(GetGamePlayerControlRedDeathFog() - GetElapsedTime())
			if ( GetGamePlayerControlRedDeathFog() < 0 ) then SetGamePlayerControlRedDeathFog(0) end
		end
	else
		-- only for first person perspectives
		if ( GetGamePlayerControlThirdpersonEnabled() == 0 ) then 
			-- player is dead, so fade to full redness - only in single player
			if ( g_PlayerHealth <= 0 ) then 
				if ( GetGamePlayerStateGameRunAsMultiplayer() == 0 ) then 
					if ( GetGamePlayerControlRedDeathFog() < 1 ) then 
						SetGamePlayerControlRedDeathFog(GetGamePlayerControlRedDeathFog() + GetElapsedTime())
						if ( GetGamePlayerControlRedDeathFog() > 1 ) then GetGamePlayerControlRedDeathFog(1) end
					end
				end
			else
				-- player is in injured state, so play low health heart beat and fade screen proportional to health
				if ( GetGamePlayerControlHeartbeatTimeStamp() < Timer() ) then 
					ttsnd = GetGamePlayerControlSoundStartIndex()+17
					if ( RawSoundExist ( ttsnd ) == 1 ) then
						PlayRawSound ( ttsnd )
					end
					SetGamePlayerControlHeartbeatTimeStamp(Timer()+1000)
				end
				ttTargetRed = 0.5 - (g_PlayerHealth/200.0)
				if ( GetGamePlayerControlRedDeathFog() <= ttTargetRed ) then 
					SetGamePlayerControlRedDeathFog(GetGamePlayerControlRedDeathFog() + GetElapsedTime())
					if ( GetGamePlayerControlRedDeathFog() > ttTargetRed ) then SetGamePlayerControlRedDeathFog(ttTargetRed) end
				else
					SetGamePlayerControlRedDeathFog(GetGamePlayerControlRedDeathFog() - GetElapsedTime())
					if ( GetGamePlayerControlRedDeathFog() < ttTargetRed ) then SetGamePlayerControlRedDeathFog(ttTargetRed) end
				end
			end
		end
	end
	
end

function gameplayercontrol.debug()

 -- use to view PBR texture layers
 PBRDebugView = 0
 if g_InKey == "1" then PBRDebugView = 1 end
 if g_InKey == "2" then PBRDebugView = 2 end
 if g_InKey == "3" then PBRDebugView = 3 end
 if g_InKey == "4" then PBRDebugView = 4 end
 if g_InKey == "5" then PBRDebugView = 5 end
 if g_InKey == "6" then PBRDebugView = 6 end
 if g_InKey == "7" then PBRDebugView = 7 end 
 if g_InKey == "8" then PBRDebugView = 8 end 
 if g_InKey == "9" then PBRDebugView = 9 end 
 SetShaderVariable(0,"ShaderVariables",PBRDebugView,0,0,0)
 if PBRDebugView > 0 then
  if PBRDebugView == 1 then Prompt ( "Albedo" ) end
  if PBRDebugView == 2 then Prompt ( "Normal" ) end
  if PBRDebugView == 3 then Prompt ( "Metalness" ) end
  if PBRDebugView == 4 then Prompt ( "Gloss" ) end
  if PBRDebugView == 5 then Prompt ( "AO" ) end
  if PBRDebugView == 6 then Prompt ( "Ambience Contrib" ) end
  if PBRDebugView == 7 then Prompt ( "Light Contrib" ) end
  if PBRDebugView == 8 then Prompt ( "Env Contrib" ) end
  if PBRDebugView == 9 then Prompt ( "Shadows" ) end
 end
 
end

function GetPlayerCamDistance( x,y,z )
    local PDX, PDY, PDZ = x - g_PlayerPosX, 
                          y - g_PlayerPosY,
						  z - g_PlayerPosZ;
  
    if math.abs( PDY ) > 100 then PDY = PDY * 4 end
  
    return math.sqrt( PDX*PDX + PDY*PDY + PDZ*PDZ )
end

return gameplayercontrol
