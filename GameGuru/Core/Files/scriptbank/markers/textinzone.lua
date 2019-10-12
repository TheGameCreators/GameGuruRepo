-- LUA Script - precede every function and global member with lowercase name of script + '_main'

g_textinzone = {}

function textinzone_init(e)
 g_textinzone[e] = GetEntityString(e,0)
end

function textinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if g_textinzone[e] ~= nil then
   Prompt ( g_textinzone[e] )
  end
 end
end
