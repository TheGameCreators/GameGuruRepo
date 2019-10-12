-- LUA Script - precede every function and global member with lowercase name of script

ai_viewanimations_frm = 0
ai_viewanimations_pressed = 0

function ai_viewanimationstatic_init(e)
 CharacterControlLimbo(e)
 SetAnimationFrames(e,0,10000)
 LoopAnimation(e)
end

function ai_viewanimationstatic_main(e)
 CharacterControlLimbo(e)
 if g_MouseClick==0 then
  ai_viewanimations_pressed = 0
 end
 if g_KeyPressSHIFT==1 or ai_viewanimations_pressed==0 then
  if g_MouseClick==1 then
   StopAnimation(e)
   ai_viewanimations_frm = ai_viewanimations_frm + 1
   ai_viewanimations_pressed = 1
  end
  if g_MouseClick==2 and ai_viewanimations_frm > 0 then
   StopAnimation(e)
   ai_viewanimations_frm = ai_viewanimations_frm - 1
   ai_viewanimations_pressed = 1
  end
 end
 SetAnimationFrame(e,ai_viewanimations_frm)
 Prompt ( "ai_viewanimations_frm = " .. ai_viewanimations_frm )
end