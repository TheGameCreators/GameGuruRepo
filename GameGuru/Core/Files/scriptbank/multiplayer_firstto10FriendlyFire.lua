-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- First to 10 kills multiplayer script

GAME_KILLS_TO_WIN = 10
GAME_KILLS_TO_WIN_TEAM = 30
GAME_MODE_BEGIN = 0
MAX_PLAYERS = 8
the_winner = ""
local win_delaytime = 0

multiplayer_firstto10FriendlyFire_gameMode = 0

function multiplayer_firstto10FriendlyFire_init(e)
	for i = 1, MAX_PLAYERS, 1 do
		mp_playerConnected[i] = 0
		mp_playerNames[i] = ""
		mp_playerKills[i] = 0
		mp_playerDeaths[i] = 0
	end -- i
 end
 
 function multiplayer_firstto10FriendlyFire_reset(e)
	for i = 1, MAX_PLAYERS, 1 do
		mp_playerKills[i] = 0
		mp_playerDeaths[i] = 0
	end -- i
 end

function multiplayer_firstto10FriendlyFire_main(e)
 if MP_IsServer() == 1 then
 --
	--Text(-1,5,2,"You are the server")
	--Text(-1,10,2,"Game Mode " .. multiplayer_firstto10FriendlyFire_gameMode )
  
  if multiplayer_firstto10FriendlyFire_gameMode == 0 then
	multiplayer_firstto10FriendlyFire_reset(e)
	multiplayer_firstto10FriendlyFire_gameMode = 1
	SetMultiplayerGameMode(multiplayer_firstto10FriendlyFire_gameMode)
  elseif multiplayer_firstto10FriendlyFire_gameMode == 1 then
	
	if GetMultiplayerTeamBased() == 0 then
		SetServerKillsToWin(GAME_KILLS_TO_WIN)
		
		for i = 1, MAX_PLAYERS, 1 do
			if mp_playerKills[i] >= GAME_KILLS_TO_WIN then
				if win_delaytime == 0 then
					win_delaytime = os.time()
				end
				if os.time() - win_delaytime > 1 then
					multiplayer_firstto10FriendlyFire_gameMode = 2
					SetMultiplayerGameMode(multiplayer_firstto10FriendlyFire_gameMode)
					SetServerTimer(os.time())
					ServerEndPlay()				
					win_delaytime = 0
				end
			end
		end -- i		
		
	else
		SetServerKillsToWin(GAME_KILLS_TO_WIN_TEAM)
		
		redScore = 0
		blueScore = 0
		for i = 1, MAX_PLAYERS, 1 do
			if mp_playerTeam[i] == 0 then
				blueScore = blueScore + mp_playerKills[i]
			else
				redScore = redScore + mp_playerKills[i]
			end
		end -- i
		
		weHaveWinner = 0
		blueWon = 0
		if blueScore >= GAME_KILLS_TO_WIN_TEAM then
			weHaveWinner = 1
			blueWon = 1		
		end
		if redScore >= GAME_KILLS_TO_WIN_TEAM then
			weHaveWinner = 1
			blueWon = 0
		end
		
		if weHaveWinner == 1 then
			if win_delaytime == 0 then
				win_delaytime = os.time()
			end
			if os.time() - win_delaytime > 1 then
				multiplayer_firstto10FriendlyFire_gameMode = 2
				SetMultiplayerGameMode(multiplayer_firstto10FriendlyFire_gameMode)
				SetServerTimer(os.time())
				ServerEndPlay()				
				win_delaytime = 0
			end
		end			
		
	end	
	
  elseif multiplayer_firstto10FriendlyFire_gameMode == 2 then
	if GetServerTimerPassed() >= 20 then
		multiplayer_firstto10FriendlyFire_reset(e)		
		multiplayer_firstto10FriendlyFire_gameMode = 1
		SetMultiplayerGameMode(multiplayer_firstto10FriendlyFire_gameMode)		
		ServerRespawnAll()	
	end
  end
 -- 
 else
 --
	--Text(-1,5,2,"You are a client")
	--Text(-1,10,2,"Game Mode " .. GetMultiplayerGameMode() )
 --
 end
  
 if GetMultiplayerGameMode() == 1 then
	if GetShowScores() == 1 then		
		multiplayer_firstto10FriendlyFire_results()
	end -- show scores
	if GetMultiplayerTeamBased() == 1 then
		ShowLiveTeamScore()
	end
 elseif GetMultiplayerGameMode() == 2 then
	
	if GetMultiplayerTeamBased() == 0 then
		for i = 1, MAX_PLAYERS, 1 do
			if mp_playerKills[i] >= GAME_KILLS_TO_WIN then
				the_winner = mp_playerNames[i]
			end
		end -- i
		if the_winner == "" then
			the_winner = "Somebody"
		end	
	else	
		redScore = 0
		blueScore = 0
		for i = 1, MAX_PLAYERS, 1 do
			if mp_playerTeam[i] == 0 then
				blueScore = blueScore + mp_playerKills[i]
			else
				redScore = redScore + mp_playerKills[i]
			end
		end -- i
		if blueScore > redScore then
			if mp_playerTeam[mp_me] == 0 then
				the_winner = "Your Team"
			else
				the_winner = "Enemy Team"
			end
		end
		if blueScore < redScore then
			if mp_playerTeam[mp_me] == 1 then
				the_winner = "Your Team"
			else
				the_winner = "Enemy Team"
			end
		end	
		if blueScore == redScore then
			the_winner = "No team"
		end	
		if the_winner == "" then
			the_winner = "No team"
		end		
	end
		
	Panel (25,10,75,20)
	Text (-1,15,3,string.upper(the_winner .. " is victorious!"))
	
	multiplayer_firstto10FriendlyFire_results()
		
	Panel(25,88,75,98)
	ttimeleft = 20-GetServerTimerPassed()
	if ttimeleft < 0 then
		ttimeleft = 0
	end
	Text(-1,93,3,string.upper("New round starts in " .. ttimeleft .. " seconds") )
 end
   
end

function multiplayer_firstto10FriendlyFire_results()

	howMany = 0
	for i = 1, MAX_PLAYERS, 1 do
		if mp_playerConnected[i] == 1 then
			howMany = howMany + 1
		end
	end
	Panel(25,35,55,40+(howMany*5))	
	Panel(55,35,65,40+(howMany*5))	
	Panel(65,35,75,40+(howMany*5))	
		
	results = {}
	counter = 1
	for i = 1, MAX_PLAYERS, 1 do
	 if mp_playerConnected[i] == 1 then
		results[counter] = i
		counter = counter + 1
	 end
	end -- i
	counter = counter - 1
		
	for i = 2,counter,1 do
		j = i
		while j > 1 and ((mp_playerKills[results[j-1]] < mp_playerKills[results[j]]) or ((mp_playerKills[results[j-1]] == mp_playerKills[results[j]]) and (mp_playerDeaths[results[j-1]] > mp_playerDeaths[results[j]]))) do
			tresult = results[j-1]
			results[j-1] = results[j]
			results[j] = tresult
			j = j - 1
		end
	end
	
	Panel(25,25,55,35)
	Panel(55,25,65,35)
	Panel(65,25,75,35)
	TextCenterOnX(40,30,1,"PLAYER")
	TextCenterOnX(60,30,1,"KILLS")
	TextCenterOnX(70,30,1,"DEATHS")
	
	y = 40
	if GetMultiplayerTeamBased() == 0 then
		for i = 1, counter, 1 do
			Text(30,y,1, i .. ".   " .. string.upper(mp_playerNames[results[i]]))
			TextCenterOnX(60,y,1, mp_playerKills[results[i]])
			TextCenterOnX(70,y,1, mp_playerDeaths[results[i]])	
			y = y + 5;
		end -- i
	else
		for i = 1, counter, 1 do
			if mp_playerTeam[results[i]] == 0 then
				r = 100
				g = 100
				b = 255
				if results[i] == mp_me then
					r = 180
					g = 180
					b = 255				
				end
				TextColor(30,y,1, i .. ".   " .. string.upper(mp_playerNames[results[i]]),r,g,b)
				TextCenterOnXColor(60,y,1, mp_playerKills[results[i]],r,g,b)
				TextCenterOnXColor(70,y,1, mp_playerDeaths[results[i]],r,g,b)	
				y = y + 5;
			end
		end -- i
		for i = 1, counter, 1 do
			if mp_playerTeam[results[i]] == 1 then
				r = 255
				g = 100
				b = 100		
				if results[i] == mp_me then
					r = 255
					g = 180
					b = 180				
				end				
				TextColor(30,y,1, i .. ".   " .. string.upper(mp_playerNames[results[i]]),r,g,b)
				TextCenterOnXColor(60,y,1, mp_playerKills[results[i]],r,g,b)
				TextCenterOnXColor(70,y,1, mp_playerDeaths[results[i]],r,g,b)	
				y = y + 5;
			end
		end -- i		
	end
end

function ShowLiveTeamScore()

	redScore = 0
	blueScore = 0
	for i = 1, MAX_PLAYERS, 1 do
		if mp_playerTeam[i] == 0 then
			blueScore = blueScore + mp_playerKills[i]
		else
			redScore = redScore + mp_playerKills[i]
		end
	end -- i
	
	Panel(30,85,40,95)
	Panel(60,85,70,95)
	
	if mp_playerTeam[mp_me] == 0 then
		TextCenterOnXColor(35,90,1, "Allies " .. blueScore,100,100,255)
		TextCenterOnXColor(65,90,1, "Enemies " .. redScore,255,100,100)
	else
		TextCenterOnXColor(35,90,1, "Allies " .. redScore,255,100,100)
		TextCenterOnXColor(65,90,1, "Enemies " .. blueScore,100,100,255)
	end
	
end
