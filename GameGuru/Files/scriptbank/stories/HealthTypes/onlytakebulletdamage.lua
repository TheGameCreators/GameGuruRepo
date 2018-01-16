-- LUA Script - precede every function and global member with lowercase name of script + '_main'

-- Create your own entity health types using global values in your LUA scripts
myEntityLastRealHealth = {}
myEntityHealth = {}

function onlytakebulletdamage_init(e)
 myEntityLastRealHealth[e] = 0
 myEntityHealth[e] = 100
end
function onlytakebulletdamage_main(e)
 if g_Entity[e]['health'] > 0 then
  if g_Entity[e]['health'] < myEntityLastRealHealth[e] then
   thisDifference = myEntityLastRealHealth[e] - g_Entity[e]['health']
   if myEntityHealth[e] > 0 then
    -- only weapons that can inflict damage would be a weapon that uses bullets (from modern weapons folder)
	isImmune = 1
    if string.find(string.lower(g_PlayerGunName), "modern") ~= nil then
	 isImmune = 0
	end
	if isImmune==0 then 
     myEntityHealth[e] = myEntityHealth[e] - thisDifference
     if myEntityHealth[e] < 0 then 
      myEntityHealth[e] = 0
     end
	end
   end
   if myEntityHealth[e]==0 then
    SetEntityHealth(e,-1)
	myEntityLastRealHealth[e] = 0
   else
    SetEntityHealth(e,10000)
	myEntityLastRealHealth[e] = 10000
   end
  else
   myEntityLastRealHealth[e] = g_Entity[e]['health']
  end
 end
 PromptLocal ( e, "My Entity Health = " .. myEntityHealth[e] )
end
