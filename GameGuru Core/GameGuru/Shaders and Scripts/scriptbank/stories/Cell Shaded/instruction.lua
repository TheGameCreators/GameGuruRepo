-- Player hits text

function instruction_init(e)
end

function instruction_main(e)
PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 then
  Destroy(e)
 end
end