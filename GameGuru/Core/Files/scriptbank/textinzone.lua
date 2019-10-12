-- LUA Script - precede every function and global member with lowercase name of script + '_main'

textinzone_name = {}

function textinzone_init_name(e,name)
 textinzone_name[e] = name
end

function textinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  Prompt(textinzone_name[e])
 end
end
