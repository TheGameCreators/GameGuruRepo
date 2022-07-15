-- Egg Radar using sprites

radarObjectivesEnemy = {}
radarObjectiveSprites = {}
radarSprite = 0
radarImageBack = 0
radarImageYellow = 0
radarImageRed = 0

function eggradar_init(e)
	radarImageBack		=	LoadImage ( "scriptbank\\Easter\\eggradar\\radar back.png" )
	radarImageYellow 	= 	LoadImage ( "scriptbank\\Easter\\eggradar\\radar yellow.png" )
	radarImageRed 	= 	LoadImage ( "scriptbank\\Easter\\eggradar\\radar red.png" )
	radarSprite = CreateSprite ( radarImageBack )
	SetSpriteDepth 	( radarSprite , 100 )
	g_objectiveCount = 0
end

function eggradar_main(e)
	SetSpriteSize 		( radarSprite , 15 , -1 )	
	SetSpriteOffset 	( radarSprite , 7.5 , -1 )
	SetSpritePosition 	( radarSprite , 10 , 85 )	
	SetSpriteAngle		( radarSprite , -g_PlayerAngY )
	for c = 1 , g_objectiveCount do
	 if radarObjectiveSprites[c] ~= nil then
 	  if radarObjectiveSprites[c] > 0 then
	   SetSpritePosition ( radarObjectiveSprites[c] , -99999, -99999 )		 
 	  end
	 end
	 if radarObjectives[c] ~= nil then
      if radarObjectives[c] > 0 then
		if radarObjectiveSprites[c] == nil then
	 	 if radarObjectivesEnemy[c] == 0 then
 		  radarObjectiveSprites[c] = CreateSprite( radarImageYellow )	
		 else
 		  radarObjectiveSprites[c] = CreateSprite( radarImageRed )	
		 end
		 SetSpriteSize ( radarObjectiveSprites[c] , 0.5 , -1 )
		 SetSpriteOffset ( radarObjectiveSprites[c] , 0.25 , -1 )
		 SetSpritePosition ( radarObjectiveSprites[c] , 200 , 200 )
		end
		if g_Entity[radarObjectives[c]]['health'] > 0 then
		 angle = eggradar_pointAtPlayer(radarObjectives[c]) + math.rad(g_PlayerAngY)
		 dist = GetPlayerDistance(radarObjectives[c])
		 if dist > 2576 then
		  dist = 2576
		 end
		 x = 10 + (( math.sin(angle) * ( dist / 200.0 ) * (GetDeviceHeight()/GetDeviceWidth()) ))
		 y = 85 + ( math.cos(angle) * ( dist / 200.0 ) )
		 SetSpritePosition ( radarObjectiveSprites[c] , x , y )
		else
		 radarObjectives[c] = 0
		end
	  end
	 end
	end	
end

function eggradar_pointAtPlayer(e)
	x = g_PlayerPosX - g_Entity[e]['x'];
	z = g_PlayerPosZ - g_Entity[e]['z'];
	angle = math.atan2( z , x );
	angle = angle + (-90.0*0.0174533);
	return angle;
end

function eggradar_addObjective(i)
 -- only if not already in list
 isunique = 1
 for c = 1 , g_objectiveCount do
  if radarObjectives[c] ~= nil then
   if radarObjectives[c] == i then
    isunique = 0
   end   
  end
 end
 if isunique == 1 then
  g_objectiveCount = g_objectiveCount + 1
  radarObjectives[g_objectiveCount] = i
  radarObjectivesEnemy[g_objectiveCount] = 0
 end
end

function eggradar_addEnemy(i)
 -- only if not already in list
 isunique = 1
 for c = 1 , g_objectiveCount do
  if radarObjectives[c] ~= nil then
   if radarObjectives[c] == i then
    isunique = 0
   end   
  end
 end
 if isunique == 1 then
  g_objectiveCount = g_objectiveCount + 1
  radarObjectives[g_objectiveCount] = i
  radarObjectivesEnemy[g_objectiveCount] = 1
 end
end

