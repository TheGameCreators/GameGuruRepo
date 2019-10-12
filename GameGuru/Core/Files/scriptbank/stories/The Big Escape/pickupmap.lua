-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects the map 

LastPlay = 0
function pickupmap_init(e)
LastPlay = 0
end

function pickupmap_main(e)
 PlayerDist = GetPlayerDistance(e)
  if LastPlay == 1 then
   Prompt ("Press E for Evac!" )
	 if g_KeyPressE == 1 then
	   UnFreezePlayer()
	   PlaySound(e,0)
	   HideImage(1)
           FinishLevel()
	   Destroy(e)
	 end
  else
   if PlayerDist < 50 then
	LastPlay = 1
	SetImagePosition(50,50)
	ShowImage(1)
	Collected(e)
    	FreezePlayer()
    end
  end
 end

