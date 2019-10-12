-- use to add objectives to your level

haveAddedObjective = {}

function objective_init(e)
 haveAddedObjective[e] = 0
end

function objective_main(e)
 if haveAddedObjective[e] == 0 then
  if addObjective ~= nil then
   addObjective(e)
  end
  haveAddedObjective[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 and g_PlayerHealth > 0 then
   PromptDuration("You have reached an objective",3000)
   PlaySound(e,0)
   SetEntityHealth(e,0)
   Destroy(e)
 end 
end
