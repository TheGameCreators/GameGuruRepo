-- LUA Script - precede every function and global member with lowercase name of script + '_main'

local light_on = {}

function light2_init(e)
  light_on[e] = 0
  HideLight(e)
  -- if you want to see the lightfrom the start, comment the two lines above and uncomment the line down here
  --light_on[e] = 1
end

function light2_main(e)
  if g_Entity[e]['activated'] == 1 then
    if light_on[e] == 0 then
      ShowLight(e)
      light_on[e] = 1
    elseif light_on[e] == 1 then
      HideLight(e)
      light_on[e] = 0
    end
    SetActivated(e,0)
  end
end