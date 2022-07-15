-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays sprites

function sprites01_init(e)
 MyImage1 = LoadImage("scriptbank\\images\\Easter\\000.png")
 MySprite1 = CreateSprite (MyImage1)
 SetSpriteDepth (MySprite1, 100)
 SetSpriteSize (MySprite1 , -1 , -1 )	
 SetSpritePosition(MySprite1,95,2)
end

function sprites01_main(e)
 if EggsCollected == 1 then
  DeleteSprite(MySprite1)
 end
 if g_PlayerHealth == 0 then
  TextCenterOnX(50,50,5,"Failure!")
 end
end
