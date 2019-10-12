-- LUA Script - precede every function and global member with lowercase name of script + '_main'
g_myimage = {}

function imageinzone_init(e)
 g_myimage[e] = LoadImage(GetEntityString(e,0))
end

function imageinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PromptImage ( g_myimage[e] )
 end
end