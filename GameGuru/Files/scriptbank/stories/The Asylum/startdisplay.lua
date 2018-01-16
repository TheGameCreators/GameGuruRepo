-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays Intro

FirstRun = 0

function startdisplay_init(e)
 LoadImages("The Asylum",0)
 FirstRun = 0
end

function startdisplay_main(e)

     if FirstRun == 1 then 
	else
     	SetImagePosition(50,50)
     	ShowImage(0)
     	FreezePlayer()
     	Prompt ("Press E to Continue" )

      	 if g_KeyPressE == 1 then
	  FirstRun = 1
      	  UnFreezePlayer()
       	  HideImage(0)
        end
      end
end

