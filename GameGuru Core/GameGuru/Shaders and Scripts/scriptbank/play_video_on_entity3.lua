-- plays the specified mp4 video file onto the entity texture when player in range
-- note for performance reasons, master in-game videos at an appropriate size, e.g 640x360, 170kbps (or lower)

video_to_play = {}
loop_video = {}
video_trigger_range = {}
prompt_to_start_video = {}
prompt_to_stop_video = {}
play_once = {}

function play_video_on_entity3_init(e)

	-- adjust these values ----------------------------------------------------------------
	--video_to_play[e] = "videobank\\GG_mars_helicopter.mp4"
	--video_to_play[e] = "videobank\\GG_Video.mp4"
	--video_to_play[e] = "videobank\\GG_western.mp4"
	video_to_play[e] = "videobank\\GG_tesla.mp4"
	--video_to_play[e] = "videobank\\GG_firefx.mp4"
	loop_video[e] = 1 -- 0 to play video once, 1 to loop video
	video_trigger_range[e] = 500
	prompt_to_start_video[e] = "Press E key to Start Video 3"
	prompt_to_stop_video[e] = "Press ALT key to Stop Video 3"
	---------------------------------------------------------------------------------------

play_once[e] = 0
end

function play_video_on_entity3_main(e)

 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < video_trigger_range[e] then
	if play_once[e] == 0 or IsVideoOnEntityPlaying() == 0 then
		Prompt(prompt_to_start_video[e])
		play_once[e] = 0
		if g_KeyPressE == 1 then 
			play_once[e] = 1
			PlayVideoOnEntity(e,video_to_play[e],loop_video[e])
		end
	end	
	if IsVideoOnEntityPlaying() == 1 and play_once[e] == 1 then
		Prompt(prompt_to_stop_video[e])
		if g_KeyPressALT == 1 then 
			StopVideoOnEntity(e)
			play_once[e] = 0
		end
	end
 else
	if play_once[e] == 1 then
		StopVideoOnEntity(e)
		play_once[e] = 0
	end
 end

end