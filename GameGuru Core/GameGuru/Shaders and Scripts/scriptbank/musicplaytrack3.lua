--tracks specified in music.lua

function musicplaytrack3_init()
end

function musicplaytrack3_main(e)
 if g_Entity[e]['plrinzone']==1 then
  music_play(3,1000)
  music_set_volume(100,100)
  ActivateIfUsed(e)
 end
end
