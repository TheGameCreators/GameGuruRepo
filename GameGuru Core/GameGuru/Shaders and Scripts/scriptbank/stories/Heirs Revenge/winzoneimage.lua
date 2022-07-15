-- LUA Script - precede every function and global member with lowercase name of script

function winzoneimage_init(e)
end

function winzoneimage_main(e)
 if g_Entity[e]['plrinzone']==1 then
   SetImagePosition(50,50)
     ShowImage(1)
     FreezePlayer()
     Prompt ("Press E to Continue" )

       if g_KeyPressE == 1 then
        UnFreezePlayer()
        HideImage(1)
        Destroy(e)
	FinishLevel()
       end

 end
end

