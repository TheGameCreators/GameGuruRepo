-- Player hits marker

function marker_init(e)
end

function marker_main(e)
PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
 ActivateIfUsed(e)
  Destroy(e)
 end
end
