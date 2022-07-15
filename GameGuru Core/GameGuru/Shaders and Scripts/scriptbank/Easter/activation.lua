-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function activation_init(e)
end

function activation_main(e)
if GetPlayerDistance(e) < 250 or g_Entity[e]['plrinzone']==1 then
PlaySound(e,0)
ActivateIfUsed(e)
Destroy(e)
end
end
