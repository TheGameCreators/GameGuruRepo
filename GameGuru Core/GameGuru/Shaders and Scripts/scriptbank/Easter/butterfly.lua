function butterfly_init(e)
end

function butterfly_main(e)

PlayerDist = GetPlayerDistance(e)
if g_Entity[e]['animating'] == 0 then
SetAnimation(1)
PlayAnimation(e)
g_Entity[e]['animating'] = 1
end
 if PlayerDist < 1000
 then
 Show(e)
 else
 Hide(e)
 end
 end

