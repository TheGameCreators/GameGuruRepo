-- LUA Script - precede every function and global member with lowercase name of script + '_main'
uselander_endsequence = 0

function decal_trigger_init(e)
end

function decal_trigger_main(e)
 RotateToCamera(e)
 if g_Entity[e]['activated'] == 0 then
  SetActivated(e,1)
  Hide(e)
 else
  if uselander_endsequence > 0 then
   Show(e)
  end
 end
end
