-- Press E to Advance to next Animation
-- Press Q to Reverse to previous Animation

local pressed = 0

local animation = -1

function animation_finder_init(e)

end

function animation_finder_main(e)

if (g_InKey == "e" or g_InKey == "E") and pressed == 0 then

animation = animation + 1
StopAnimation(e)
pressed = 1
end

if (g_InKey == "q" or g_InKey == "Q") and pressed == 0 and animation > 0 then

animation = animation - 1
StopAnimation(e)
pressed = 1
end


if g_Entity[e]['animating'] == 0 then
SetAnimation(animation)
PlayAnimation(e)
g_Entity[e]['animating'] = 1
end




if g_InKey == "" then
pressed = 0
end



Prompt("[Q] Previous [E] Next | Animation number: "..animation.." | Animation Frame:  "..GetAnimationFrame(e))

end

function animation_finder_exit(e)

end
