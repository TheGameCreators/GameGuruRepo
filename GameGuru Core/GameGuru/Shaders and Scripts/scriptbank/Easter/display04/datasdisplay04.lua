-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs to collect

EggsCollected = 0

function datasdisplay04_init(e)
 HideHuds()
 SetFlashLightKeyEnabled(0)
end

function datasdisplay04_main(e)

if g_KeyPressE == 0 and g_FirstRun == 0 then
     	SetImagePosition(50,50)
     	ShowImage(8)
		end
	if EggsCollected == 0 then
	 TextCenterOnX(91,3,4," " .. g_Egg1Count .. "/20")
	 TextCenterOnX(91,12,4," " .. g_Egg2Count .. "/20")
	 TextCenterOnX(91,20,4," " .. g_Egg3Count .. "/20")
	 TextCenterOnX(91,28,4," " .. g_Egg4Count .. "/20")
	 end
if g_KeyPressE == 0 and g_FirstRun == 1 then
HideImage(8)
SetImagePosition(50,50)
ShowImage(15)
if g_KeyPressE == 0 and g_EKey2 == 1 then
HideImage(15)
SetImagePosition(50,50)
ShowImage(12)
if g_KeyPressE == 0 and g_EKey4 == 1 then
HideImage(12)
end	 
end	 
end	
     if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 and g_Egg4Count==20 then
	 EggsCollected = 1
	  FreezePlayer()
      SetImagePosition(50,50)
      ShowImage(10)
       if g_KeyPressE == 1 then
        UnFreezePlayer()
        HideImage(10)
        JumpToLevel("level5")
        Destroy(e)
       end
       Prompt("Press E to Continue.")
      end
      end

