-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Sound Zone
local evttimer= 0
local evtcounter= 0

function zombiearea_init(e)
end

function zombiearea_main(e)

         if g_Entity[e]['activated'] == 0 then
			 TextCenterOnXColor(50,70,4,"1 -Revolve around the Concrete Girder ",255,255,255)	
             if g_Entity[e]['plrinzone']==1 then SetActivated(e,1) end			 
         end
		 
         if g_Entity[e]['activated'] == 1 then
			 TextCenterOnXColor(50,70,4,"1 -Revolve around the Concrete Girder ",255,255,255)	
 			 TextCenterOnXColor(50,74,4,"2 -When you do it..",255,255,255)	
 			 TextCenterOnXColor(50,78,4,"2 -Press the key E",255,255,255)
             if g_KeyPressE == 1 then 
			    SetActivated(e,2) 
                ActivateIfUsed(e)				
			 end					 
         end		 

         if g_Entity[e]['activated'] == 2 then
            if g_Time > evttimer then	
               evttimer = g_Time + 100
	           evtcounter = evtcounter + 1
			end	
 			TextCenterOnXColor(50,70,4,"Wait a moment please..",255,255,255)
 			TextCenterOnXColor(50,74,4,"The Zombies have something to say.",255,255,255)
            if evtcounter >= 50 then
			   SetActivated(e,3) 
--               ActivateIfUsed(e)				
			end			
		 end
		 
         if g_Entity[e]['activated'] == 3 then
			 TextCenterOnXColor(50,70,4,"1 -Revolve around the Concrete Girder.. again ",255,255,255)	
			 TextCenterOnXColor(50,74,4,"1 -You feel the difference?",255,255,255)	
			 
		 end
		 
 end
