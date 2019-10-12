-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Checkpoint Zone

--if firstrun == 1 then
 --else
-- firstrun = 1
 KillCount = 0
--end

function supplydump_init(e)
end

function supplydump_main(e)

 if g_Entity[e]['plrinzone']==1 then
   if KillCount < 5 then
    Prompt ( "Destroy the supply dump on your right! " .. 5 - KillCount .. " left" )
   end
 end
end
