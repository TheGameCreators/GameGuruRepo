-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Zone
local test=0
function plrinzonechangesky_init(e)
end

function plrinzonechangesky_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if test == 0 then
    SetSkyTo("dark")
    test=1;
  end
 end
end
