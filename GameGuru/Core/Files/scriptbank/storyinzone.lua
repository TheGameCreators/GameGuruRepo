-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Story Zone

g_storyinzone_mode = {}

function storyinzone_init(e)
 g_storyinzone_mode[e] = 0
end

function storyinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if g_storyinzone_mode[e] ~= nil then
   if g_storyinzone_mode[e] == 0 then
    g_storyinzone_mode[e] = 1
    HideHuds()
	if radar_hideallsprites ~= nil then radar_hideallsprites() end
   else
    if g_storyinzone_mode[e] == 1 then
     PlaySound(e,0)
     PlayVideo(e,1)
     StopSound(e,0) 
  	 g_storyinzone_mode[e] = 2
    else 
     if g_storyinzone_mode[e] == 2 then
  	  if radar_showallsprites ~= nil then radar_showallsprites() end
      ShowHuds()
      Destroy(e)
      ActivateIfUsed(e)
 	  g_storyinzone_mode[e] = 3 
 	 end
    end
   end
  end
 end
end
