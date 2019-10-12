-- AI : Manual entity animation frame control
ai_dance_speed = {}
ai_dance_frame = {}
ai_dance_dir = {}

function ai_dance_init(e)
 CharacterControlLimbo(e)
 ai_dance_speed[e] = 0.1
 ai_dance_frame[e] = 1
 ai_dance_dir[e] = 0
end

function ai_dance_main(e)
 RotateToPlayer(e)
 if ai_dance_dir[e] == 0 then
  ai_dance_frame[e]=ai_dance_frame[e]+ai_dance_speed[e]
  if ai_dance_frame[e] > 637 then
   ai_dance_frame[e] = 637
   ai_dance_dir[e] = 1
  end
 end
 if ai_dance_dir[e] == 1 then
  ai_dance_frame[e]=ai_dance_frame[e]-ai_dance_speed[e]
  if ai_dance_frame[e] < 1 then
   ai_dance_frame[e] = 1
   ai_dance_dir[e] = 0
  end
 end
 SetAnimationFrame(e,ai_dance_frame[e])
end