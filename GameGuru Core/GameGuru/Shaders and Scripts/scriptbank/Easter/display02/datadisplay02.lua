-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs to collect

EggsCollected = 0

function datadisplay02_init(e)
 HideHuds()
end

function datadisplay02_main(e)

if g_KeyPressE == 0 and g_FirstRun == 0 then
     	SetImagePosition(50,50)
     	ShowImage(6)
	end
	if EggsCollected == 0 then
	 TextCenterOnX(91,3,4," " .. g_Egg1Count .. "/20")
	 TextCenterOnX(91,12,4," " .. g_Egg2Count .. "/20")
	 end
	 if g_KeyPressE == 0 and g_FirstRun == 1 then
HideImage(6)
SetImagePosition(50,50)
ShowImage(11)
if g_KeyPressE == 0 and g_EKey2 == 1 then
HideImage(11)
end	 
end	
	 if g_Egg1Count==20 and g_Egg2Count==20 then
	 EggsCollected = 1
	  FreezePlayer()
      SetImagePosition(50,50)
      ShowImage(10)
       if g_KeyPressE == 1 then
       UnFreezePlayer()
        HideImage(10)
        JumpToLevel("level3")
        Destroy(e)
       end
       Prompt("Press E to Continue.")
      end
      
     end

