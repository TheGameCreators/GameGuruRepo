-- LUA Script - precede every function and global member with lowercase name of script

g_atester = 0

function atester_init(e)
end

function atester_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if g_atester == 0 then
   Prompt("new sky")
   LoadGlobalSound ( "audiobank\\misc\\ping.wav", 1 ) 
   LoopGlobalSound ( 1 )
   SetSkyTo("dark")
   g_atester = 1
  end
 end
end

