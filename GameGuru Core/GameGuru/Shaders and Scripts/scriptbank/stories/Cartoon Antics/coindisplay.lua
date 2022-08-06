-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays coins to collect

function coindisplay_init(e)
end

function coindisplay_main(e)

     SetImagePosition(50,10)
     ShowImage(0)

     PromptTextSize(5)
     Prompt (" " .. 100 - CoinCount .. " more to collect!" )

     if CoinCount==100 then
      HideImage(1)
      FreezePlayer()
      SetImagePosition(50,50)
      ShowImage(3)
       if g_KeyPressE == 1 then
        UnFreezePlayer()
        HideImage(3)
        FinishLevel()
        Destroy(e)
       end
       Prompt("Press E to close or Esc to end level.")
      end
      
     end

