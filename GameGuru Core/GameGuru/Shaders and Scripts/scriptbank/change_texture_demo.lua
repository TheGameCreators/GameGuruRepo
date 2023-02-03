local timer = {}
local alt_delay = {}
local using_alt_texture = {}


function change_texture_demo_init(e)

	alt_delay[e] = 2.5 --number of seconds before texture change

	timer[e] = GetTimer(e)
	using_alt_texture[e] = 0
	
end
 
function change_texture_demo_main(e)

 if GetTimer(e) > (timer[e] + (alt_delay[e]*1000)) then 
	if using_alt_texture[e]  == 0 then
		
		ChangeTexture(e,"entitybank\\Industrial\\Compressor Hub_D.dds") 		-- use for single mesh/material objects
		--ChangeTexture(e,"entitybank\\Industrial\\Compressor Hub_D.dds",2) 	-- add mesh/material number if multi texture object
		
		if GetPlayerDistance(e) < 300 then
			PromptDuration("original texture re-applied!",2000)
		end
		
		using_alt_texture[e] = 1
		--PlaySoundIfSilent(e,0)
	else
		
		ChangeTexture(e,"entitybank\\Industrial\\Gas Tank_D.dds")
		--ChangeTexture(e,"entitybank\\Industrial\\Gas Tank_D.dds",2) 	-- add mesh/material number if multi texture object
		
		if GetPlayerDistance(e) < 300 then
			PromptDuration("alternative texture applied!",2000)
		end
		
		using_alt_texture[e] = 0
	end
	timer[e] = GetTimer(e)
 end

end