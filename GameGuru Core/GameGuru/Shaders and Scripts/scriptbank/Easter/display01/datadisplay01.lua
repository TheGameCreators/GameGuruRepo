-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs to collect

EggsCollected = 0

function datadisplay01_init(e)
 HideHuds()
 SetFlashLightKeyEnabled(0)
end

function datadisplay01_main(e)
 if g_KeyPressE == 0 and g_FirstRun == 0 then
  SetImagePosition(50,50)
  ShowImage(5)
 else
  HideImage(5)
 end
 if EggsCollected == 0 then
  TextCenterOnX(91,3,4," " .. g_Egg1Count .. "/20")
 end
 if g_Egg1Count==20 then
  EggsCollected = 1
  FreezePlayer()
  SetImagePosition(50,50)
  ShowImage(10)
  if g_KeyPressE == 1 then
   UnFreezePlayer()
   HideImage(10)
   JumpToLevel("level2")
   Destroy(e)
  end
  Prompt("Press E to Continue.")
 end
end
