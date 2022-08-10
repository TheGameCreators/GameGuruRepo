-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs to collect

EggsCollected = 0

function datadisplay05_init(e)
 SetFlashLightKeyEnabled(0)
 HideHuds()
end

function datadisplay05_main(e)
 if g_KeyPressE == 0 and g_FirstRun == 0 then
  SetImagePosition(50,50)
  ShowImage(9)
 end
 if EggsCollected == 0 then
  TextCenterOnX(91,3,4," " .. g_Egg1Count .. "/20")
  TextCenterOnX(91,12,4," " .. g_Egg2Count .. "/20")
  TextCenterOnX(91,20,4," " .. g_Egg3Count .. "/20")
  TextCenterOnX(91,28,4," " .. g_Egg4Count .. "/20")
  TextCenterOnX(91,36,4," " .. g_Egg5Count .. "/20")
 end
 if g_KeyPressE == 0 and g_FirstRun == 1 then
  HideImage(9)
  SetImagePosition(50,50)
  ShowImage(15)
  if g_KeyPressE == 0 and g_EKey2 == 1 then
   HideImage(15)
   SetImagePosition(50,50)
   ShowImage(13)
   if g_KeyPressE == 0 and g_EKey4 == 1 then
    HideImage(13)
    SetImagePosition(50,50)
    ShowImage(14)
    if g_KeyPressE == 0 and g_EKey6 == 1 then
     HideImage(14)
    end	 
   end	 
  end	 
 end	
 if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 and g_Egg4Count==20 and g_Egg5Count==20 then
  EggsCollected = 1
  SetImagePosition(95,12)
  ShowImage(16)
 end
 if g_princess_rescued==1 then
  HideImage(16)
  FreezePlayer()
  SetImagePosition(50,50)
  ShowImage(10)
  if g_KeyPressE == 1 then
   UnFreezePlayer()
   HideImage(10)
   lastdisplay = 1
   FinishLevel()
   Destroy(e)
  end
  Prompt("Press E to Continue.")
 end
end
