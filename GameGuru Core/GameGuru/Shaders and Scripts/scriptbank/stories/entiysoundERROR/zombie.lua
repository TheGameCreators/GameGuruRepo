-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Sound Zone


function zombie_init(e)

end

function zombie_main(e)

         if g_Entity[e]['activated'] == 1 then
			SetActivated(e,2)			   
            PlaySound(e,0)
            PlaySound(e,1)
            PlaySound(e,2)
            PlaySound(e,3)
            PlaySound(e,4)
         end
		 
end
