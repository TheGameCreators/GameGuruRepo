--tracks specified in music.lua

function musicplaytrack4_init()
end

function musicplaytrack4_main(e)
 if g_Entity[e]['plrinzone']==1 then
  music_play(4,1000)
  music_set_volume(100,100)
  ActivateIfUsed(e)
 end
end
