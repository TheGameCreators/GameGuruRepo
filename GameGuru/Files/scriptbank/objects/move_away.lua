--

function move_away_init(e)
end
function move_away_main(e)
 PlayerDist = GetPlayerDistance(e)
 fDetectionRange = 300
 if PlayerDist < fDetectionRange then
  tDistX = g_PlayerPosX - g_Entity[e]['x']
  tDistZ = g_PlayerPosZ - g_Entity[e]['z']
  tDistD = math.abs(tDistX) + math.abs(tDistZ)
  tDistX = (tDistX / tDistD) * 10.0
  tDistZ = (tDistZ / tDistD) * 10.0
  g_Entity[e]['x'] = g_Entity[e]['x'] - tDistX
  g_Entity[e]['z'] = g_Entity[e]['z'] - tDistZ
  fFinalY = GetGroundHeight(g_Entity[e]['x'],g_Entity[e]['z'])
  SetPosition(e,g_Entity[e]['x'],fFinalY,g_Entity[e]['z'])
 end
end
