-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function hidelightwhenclose_init(e)
end
function hidelightwhenclose_main(e)
 local Ent = g_Entity[ e ]
 local PDX, PDY, PDZ = Ent.x - g_PlayerPosX, Ent.y - g_PlayerPosY, Ent.z - g_PlayerPosZ;
 tPlrDist = math.sqrt( PDX*PDX + PDY*PDY + PDZ*PDZ )
 Prompt("tPlrDist = " .. tPlrDist)
 if tPlrDist < 200 then
  HideLight(e)
 end
end
