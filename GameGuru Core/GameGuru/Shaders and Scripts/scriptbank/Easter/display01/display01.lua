-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays Intro

g_FirstRun = 0

function display01_init(e)
 LoadImages("Easter",0)
end

function display01_main(e)
 if g_FirstRun == 1 then 
 else
  FreezePlayer()
  Prompt ("Press E to Continue" )
  if g_KeyPressE == 1 then
   g_FirstRun = 1
   UnFreezePlayer()
  end
 end
end

