-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays sprites

function sprites04_init(e)
 MyImage1 = LoadImage("scriptbank\\images\\Easter\\000.png")
 MySprite1 = CreateSprite (MyImage1)
 SetSpriteDepth (MySprite1, 100)
 SetSpriteSize (MySprite1 , -1 , -1 )	
 SetSpritePosition(MySprite1,95,2)
 MyImage2 = LoadImage("scriptbank\\images\\Easter\\001.png")
 MySprite2 = CreateSprite (MyImage2)
 SetSpriteDepth (MySprite2, 100)
 SetSpriteSize (MySprite2 , -1 , -1 )	
 SetSpritePosition(MySprite2,95,10.5)
 MyImage3 = LoadImage("scriptbank\\images\\Easter\\002.png")
 MySprite3 = CreateSprite (MyImage3)
 SetSpriteDepth (MySprite3, 100)
 SetSpriteSize (MySprite3 , -1 , -1 )	
 SetSpritePosition(MySprite3,95,19)
 MyImage4 = LoadImage("scriptbank\\images\\Easter\\003.png")
 MySprite4 = CreateSprite (MyImage4)
 SetSpriteDepth (MySprite4, 100)
 SetSpriteSize (MySprite4 , -1 , -1 )	
 SetSpritePosition(MySprite4,95,27)
end

function sprites04_main(e)
 if EggsCollected == 1 then
  DeleteSprite(MySprite1)
  DeleteSprite(MySprite2)
  DeleteSprite(MySprite3)
  DeleteSprite(MySprite4)
 end
 if g_PlayerHealth == 0 then
  TextCenterOnX(50,50,5,"Failure!")
 end
end

