-- LUA Script - precede every function and global member with lowercase name of script

g_floating_platform_activated = 0
g_floating_platform_associateplayer = 0

function floating_platform_init(e)
 ai_soldier_state[e] = "patrol";
 ai_soldier_pathindex[e] = -1;
end

function floating_platform_main(e)
 if ai_soldier_state[e] == "patrol" then
  if ai_soldier_pathindex[e] == -1 then
   ai_soldier_pathindex[e] = -2
   PathIndex = -1;
   pClosest = 99999;
   for pa = 1, AIGetTotalPaths(), 1 do
    for po = 1 , AIGetPathCountPoints(pa), 1 do
     pDX = g_Entity[e]['x'] - AIPathGetPointX(pa,po);
     pDZ = g_Entity[e]['z'] - AIPathGetPointZ(pa,po);
     pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDZ*pDZ));
     if pDist < pClosest and pDist < 200 then
      pClosest = pDist;
      PathIndex = pa;
     end
    end
   end
   if PathIndex > -1 then
    ai_soldier_pathindex[e] = PathIndex;
	ai_path_point_index[e] = 2
	ai_path_point_direction[e] = 1
	ai_path_point_max[e] = AIGetPathCountPoints(ai_soldier_pathindex[e])
   end   
  end
  PlayerDist = GetPlayerDistance(e)
  if ai_path_point_direction[e] == 1 and PlayerDist < 80 and g_PlayerHealth > 0 then
   g_floating_platform_activated = 1
   g_floating_platform_associateplayer = 1
  end
  if g_floating_platform_activated == 1 and ai_soldier_pathindex[e] > -1 then 
   -- move to current node
   ai_patrol_x[e] = AIPathGetPointX(ai_soldier_pathindex[e],ai_path_point_index[e])
   ai_patrol_z[e] = AIPathGetPointZ(ai_soldier_pathindex[e],ai_path_point_index[e])
   EntObjNo = g_Entity[e]['obj'];
   AIEntityGoToPosition(EntObjNo,ai_patrol_x[e],ai_patrol_z[e])
   tDistX = g_Entity[e]['x'] - ai_patrol_x[e]
   tDistZ = g_Entity[e]['z'] - ai_patrol_z[e]
   DistFromPath = math.sqrt(math.abs(tDistX*tDistX)+math.abs(tDistZ*tDistZ))	
   tIncX = tDistX / DistFromPath
   tIncZ = tDistZ / DistFromPath
   tNewX = g_Entity[e]['x'] - (tIncX*0.25)
   tNewY = g_Entity[e]['y']
   tNewZ = g_Entity[e]['z'] - (tIncZ*0.25)
   -- Associate Player with this moving entity platform
   ResetPosition ( e, tNewX, tNewY, tNewZ )
   if g_floating_platform_associateplayer == 1 then
    SetFreezePosition ( tNewX, tNewY+40, tNewZ )
    TransportToFreezePositionOnly()
   end
   -- When reach node, decide where to go next   
   if DistFromPath < 50 then
	if ai_path_point_direction[e] == 1 then
	 ai_path_point_index[e] = ai_path_point_index[e] + 1
	 if ( ai_path_point_index[e] > ai_path_point_max[e] ) then
	  ai_path_point_index[e] = ai_path_point_max[e] -1
	  ai_path_point_direction[e] = 0
	  g_floating_platform_associateplayer = 0
	 end
	else
	 ai_path_point_index[e] = ai_path_point_index[e] - 1
	 if ( ai_path_point_index[e] < 1 ) then
	  ai_path_point_index[e] = 2
	  ai_path_point_direction[e] = 1
	 end		
	end
   end
  end
 end
end

