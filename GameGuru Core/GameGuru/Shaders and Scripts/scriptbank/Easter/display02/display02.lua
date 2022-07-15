-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays Intro

g_FirstRun = 0
g_EKey1 = 0
g_EKey2 = 0

function display02_init(e)
LoadImages("Easter",0)
end

function display02_main(e)
if g_FirstRun == 1 then 
	else
     	FreezePlayer()
		TextCenterOnX(50,18,4,"Press [F] to switch on/off Flashlight")
     	Prompt ("Press E to Continue" )
        if g_KeyPressE == 1 then
	    g_FirstRun = 1
      end
      end
	  if g_EKey1 == 1 then 
	else
     	Prompt ("Press E to Continue" )
         if g_KeyPressE == 0 and  g_FirstRun == 1 then
	     g_EKey1 = 1
       end
      end
	  	  if g_EKey2 == 1 then 
	else
     	Prompt ("Press E to Continue" )
         if g_KeyPressE == 1 and  g_EKey1 == 1 then
		 g_EKey2 = 1
		 UnFreezePlayer()
       end
      end

end


