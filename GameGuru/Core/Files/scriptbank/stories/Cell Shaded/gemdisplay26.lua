-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays coins to collect

function gemdisplay26_init(e)
LoadImages("cellshaded",0)
end

function gemdisplay26_main(e)

     PromptTextSize(3)
     Prompt (" " .. 26 - GemCount .. " more to collect!" )

     if GemCount==26 then
      SetImagePosition(20,20)
      ShowImage(5)
	  ActivateIfUsed(e)
		
       end
       
      end

