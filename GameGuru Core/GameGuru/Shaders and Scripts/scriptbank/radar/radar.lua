-- Radar using sprites
-- added new x y positions for easy setup - Dvader aka retrogamebloke

enemyCount = 0
radarFoundEnemies = 0
radarEnemies = {}
radarEnemySprites = {}
radarObjectiveSprites = {}
radarSprite = 0
radarImageBack = 0
radarImageRed = 0
radarImageGreen = 0
radarImageYellow = 0
aspectRatio = 0
waitForAllEntities = 1
radarx=90
radary=15
radarMakeInvisible=0

function radar_init(e)
	radarImageBack		=	LoadImage ( "scriptbank\\radar\\radar-back.png" )
	radarImageRed 		= 	LoadImage ( "scriptbank\\radar\\radar-red.png" )
	radarImageGreen 	= 	LoadImage ( "scriptbank\\radar\\radar-green.png" )
	radarImageYellow 	= 	LoadImage ( "scriptbank\\radar\\radar-yellow.png" )
	radarSprite = CreateSprite ( radarImageBack )
	SetSpriteDepth 	( radarSprite , 100 )
end

function radar_main(e)

	SetSpriteSize 		( radarSprite , 15 , -1 )	
	SetSpriteOffset 	( radarSprite , 7.5 , -1 )
	if radarMakeInvisible == 0 then
 	 SetSpritePosition 	( radarSprite , radarx , radary )	
	else
 	 SetSpritePosition 	( radarSprite , 200 , 200 )	
	end
	SetSpriteAngle		( radarSprite , -g_PlayerAngY )
	
	if waitForAllEntities == 1 then
		waitForAllEntities = 0
		return
	end
	
	if radarFoundEnemies == 0 then
		enemyCount = 0
		for c = 1, 3000 do
			if ai_bot_state[c] ~= nil then
				if g_Entity[c] ~= nil then
					enemyCount = enemyCount + 1
					radarEnemies[enemyCount] = c	
					radarEnemySprites[enemyCount] = CreateSprite( radarImageRed )
					SetSpriteOffset ( radarEnemySprites[enemyCount] , 0.25 , -1 )
					SetSpriteSize ( radarEnemySprites[enemyCount] , 0.5 , -1 )
					SetSpritePosition ( radarEnemySprites[enemyCount] , 200 , 200 )
				end
			end
		end		
		radarFoundEnemies = 1		
	end
	
	newEntity = GetFirstEntitySpawn()
	if newEntity ~= 0 then
		if ai_bot_state[newEntity] ~= nil then
			if g_Entity[newEntity] ~= nil then
				enemyCount = enemyCount + 1
				radarEnemies[enemyCount] = newEntity
				radarEnemySprites[enemyCount] = CreateSprite( radarImageRed )
				SetSpriteOffset ( radarEnemySprites[enemyCount] , 0.25 , -1 )
				SetSpriteSize ( radarEnemySprites[enemyCount] , 0.5 , -1 )
				SetSpritePosition ( radarEnemySprites[enemyCount] , 200 , 200 )
			end
		end	
	end
	
	for c = 1 , enemyCount do
	 e = radarEnemies[c]
	 if g_Entity[e] ~= nil then
		if g_Entity[e]['health'] > 0 and GetEntityVisibility(e) == 1 and radarMakeInvisible == 0 then
			angle = pointAtPlayer(e) + math.rad(g_PlayerAngY)
			dist = GetPlayerDistance(e)
			if dist > 2576 then
				dist = 2576
			end
			x = radarx + (( math.sin(angle) * ( dist / 200.0 ) * (GetDeviceHeight()/GetDeviceWidth()) ))
			y = radary + ( math.cos(angle) * ( dist / 200.0 ) )
			SetSpritePosition ( radarEnemySprites[c] , x , y )
		else
			SetSpritePosition ( radarEnemySprites[c] , 200 , 200 )
		end
	 end
	end
	
	for c = 1 , g_objectiveCount do
	 e=radarObjectives[c]
	 if g_Entity[e] ~= nil then
		 if g_Entity[e]['health'] > 0 and radarMakeInvisible == 0 then
			angle = pointAtPlayer(radarObjectives[c]) + math.rad(g_PlayerAngY)
			dist = GetPlayerDistance(radarObjectives[c])
			if dist > 2576 then
				dist = 2576
			end
			x = radarx + (( math.sin(angle) * ( dist / 200.0 ) * (GetDeviceHeight()/GetDeviceWidth()) ))
			y = radary + ( math.cos(angle) * ( dist / 200.0 ) )
			SetSpritePosition ( radarObjectiveSprites[c] , x , y )
		 else
			SetSpritePosition ( radarObjectiveSprites[c] , 200 , 200 )
		 end
	 end
	end	

end

function radar_hideallsprites()
 radarMakeInvisible = 1
end

function radar_showallsprites()
 radarMakeInvisible = 0
end

function pointAtPlayer (i)
 if g_Entity[i] ~= nil then
	x = g_PlayerPosX - g_Entity[i]['x'];
	z = g_PlayerPosZ - g_Entity[i]['z'];
	angle = math.atan2( z , x );
	angle = angle + (-90.0*0.0174533);
	return angle;
 end
end

function addObjective (i)
	g_objectiveCount = g_objectiveCount + 1
	radarObjectives[g_objectiveCount] = i
	radarObjectiveSprites[g_objectiveCount] = CreateSprite( radarImageYellow )
	SetSpriteSize ( radarObjectiveSprites[g_objectiveCount] , 0.5 , -1 )
	SetSpriteOffset ( radarObjectiveSprites[g_objectiveCount] , 0.25 , -1 )
	SetSpritePosition ( radarObjectiveSprites[g_objectiveCount] , 200 , 200 )
end
