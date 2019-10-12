-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Win Zone

function gotoprompt_init(e)
end

function gotoprompt_main(e)
 if g_Entity[e]['plrinzone']==1 then
 PromptDuration("Search the building site for information!",10000)
 
 end
end
