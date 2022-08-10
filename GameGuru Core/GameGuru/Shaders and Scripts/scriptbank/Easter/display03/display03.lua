-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays Intro

g_FirstRun = 0

function display03_init(e)
 LoadImages("Easter",0)
end

function display03_main(e)

     if g_FirstRun == 1 then 
	else
     	FreezePlayer()
		TextCenterOnX(50,18,4,"You have 15 minutes to collect all eggs! Hurry up!")
     	Prompt ("Press E to Continue" )

      	 if g_KeyPressE == 1 then
	     g_FirstRun = 1
      	  UnFreezePlayer()
        end
      end
end

