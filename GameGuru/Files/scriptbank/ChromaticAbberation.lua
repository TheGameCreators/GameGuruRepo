-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Test script to control post processing shaders from LUA.
-- To test this , enable CHROMATICABBERATION and CHROMATICABBERATIONLUA in settings.fx
-- assign ChromaticAbberation.lua to a object
-- set "Static Mode" = "No" , and "Always Active?" = "Yes".
-- When you get near the object you will see the effect.
-- Also checkout: http://store.steampowered.com/app/623270/AppGameKit__GameGuru_Loader/
-- Enjoy and happy game making.

g_ca_effect_time = Timer()

function chromaticabberation_init(e)
	math.randomseed ( Timer() )
	CA_r = math.random(0,40)
	CA_g = math.random(0,40)
	CA_b = math.random(0,40)
  	SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 0)
	g_ca_effect_time = Timer()
  	CA_trigger = 1
end

function chromaticabberation_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 120 then

	if CA_trigger >= 1 then
		if CA_trigger == 1 then
			CA_trigger = 2
			PlaySound(e,0)
			-- HurtPlayer(e,1)
		end
		if math.random(0,5) == 0 then

			if math.random(0,3) == 0 then
				CA_r = math.random(-80,180)
				CA_g = math.random(-80,180)
				CA_b = math.random(-80,180)
			else
				CA_r = math.random(-8,18)
				CA_g = math.random(-8,18)
				CA_b = math.random(-8,18)
			end
		end

		if (Timer()-g_ca_effect_time) < 1000 then -- effect time up to 1.0 sec.
			if math.random(0,6) == 0 then
				Prompt ( "11100 01 101 11001 1101010 11 101010 10101 101 11100 01 101 11001 101 00 01 101 11001 1101010 11 101010 10101 101" )
			elseif math.random(0,3) == 0 then
				Prompt ( "00010 1101 01 00010 1101 1100 11 01100 1 10110 11010 1101 01" )
			else
				Prompt ( "" )
			end
			SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 1) -- Set random CA
		else
			if (Timer()-g_ca_effect_time) < 3500 then -- effect time up to 3.5 sec.
				Prompt ( "Ohhh - Something is interfering with my vision." )
				SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 1) -- Set random CA
			else
				if (Timer()-g_ca_effect_time) < 5000 then -- effect time more then 5.0 sec. disable again
					Prompt ( "Ohhh - Something is interfering with my vision." ) -- Keep text on screen while CA is disabled for 1.5 sec.
					SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 0) -- Disable CA
				else
					CA_trigger = 0
					SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 0) -- Disable CA
				end
			end
		end
	end
 else
	if not CA_trigger == 1 then
		SetShaderVariable(0,"ChromaticAbberation", CA_r , CA_g, CA_b, 0) -- Disable CA
	end
	CA_trigger = 1
  	g_ca_effect_time = Timer()
 end
end
