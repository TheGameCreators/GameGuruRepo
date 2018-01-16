--tracks specified in music.lua

function musicplaytrack1_init()
end

function musicplaytrack1_main(e)
 if g_Entity[e]['plrinzone']==1 then
  music_play(1,1000)
  music_set_volume(100,100)
  ActivateIfUsed(e)
 end
end
