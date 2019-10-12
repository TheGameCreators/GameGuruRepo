-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays coins to collect

function gemdisplay40_init(e)
LoadImages("cellshaded",0)
end

function gemdisplay40_main(e)

     SetImagePosition(10,90)
     ShowImage(0)

     PromptTextSize(3)
     Prompt (" " .. 40 - GemCount .. " more to collect!" )

     if GemCount==40 then
      HideImage(1)
      FreezePlayer()
      SetImagePosition(50,50)
      ShowImage(1)
       if g_KeyPressE == 1 then
        UnFreezePlayer()
        HideImage(0)
        FinishLevel()
        Destroy(e)
       end
       Prompt("Press E to close or Esc to end level.")
      end
      
     end

