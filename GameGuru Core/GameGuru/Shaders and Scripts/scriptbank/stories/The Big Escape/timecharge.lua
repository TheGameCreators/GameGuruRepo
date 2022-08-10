-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Detonate a supply dump and count

destroy_started = {}

function timecharge_init(e)
 destroy_started[e]=0
end

function timecharge_main(e)
 
PlayerDist = GetPlayerDistance(e)
 
   if PlayerDist<90 and destroy_started[e] == 0 then 
    Prompt ("Press E To pick up the plant charge")	 
     if g_KeyPressE == 1 then
      destroy_started[e] = GetTimer(e)+2500
      PlaySound(e,0)
     end
   end
 
 if destroy_started[e]>0 then
  if GetTimer(e) > destroy_started[e] then
   SetEntityHealth(e,0)
   KillCount = KillCount + 1
   destroy_started[e]=0
  end
 end

 if g_Entity[e]['health'] <= 0 then
  StopSound(e)
 end
end
