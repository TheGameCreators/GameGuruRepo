-- simulated_sky_demo, lua command example

--SetStarDensity(v) 0 to 100
--SetSolarLatitude(v) -90 to 90
--SetSolarDay(v) 1 to 31
--SetSolarMonth(v) 1 to 12
--SetSolarTime(v) 0 to 24
--SetSunTime(time_hours,day,month,latitude)
--SetCloudCoverage(v) 0 to 100
--SetCloudSpeed(v) 0 to 100
--SetCloudDensity(v) 0 to 100
--SetCloudTint(v) 0 to 100 -- lower darker
--SetSkyEffects(v) 0 to 100
--
--GetStarDensity()
--GetSkyEffects()
--GetCloudCoverage()
--GetCloudSpeed()
--GetCloudDensity()
--GetCloudTint()
--GetSolarLatitude()
--GetSolarDay()
--GetSolarMonth()
--GetSolarTime()
	
local show_values = 1


status = 0
local time_hours = 5
local factor = 0

function simulated_sky_demo_init(e)
end

function simulated_sky_demo_main(e)
 
	if g_KeyPressALT == 1 then
		SetCloudCoverage(0) -- turn off procedural clouds
		SetCloudDensity(0) -- turn off procedural clouds
		SetStarDensity(50)
		SetSkyEffects(50) -- turn off
	end
	if g_KeyPressE == 1 then
		status = 1
	end
	if status == 0 then
		Prompt("Press E to Start Demo")
		return
	end
	
	--display values
	if show_values ==1 then
		
		TextColor(2,10,3,"Star Density:   " .. GetStarDensity(),255,0,0) --GetStarDensity
		TextColor(2,13,3,"Sky  Effects:   " .. GetSkyEffects(),255,0,0) --GetSkyEffects
		TextColor(2,16,3,"Cloud Coverage: " .. GetCloudCoverage(),255,0,0) --GetCloudCoverage
		TextColor(2,19,3,"Cloud Speed:    " .. GetCloudSpeed(),255,0,0) --GetCloudSpeed
		TextColor(2,22,3,"Cloud Density:  " .. GetCloudDensity(),255,0,0) --GetCloudDensity
		TextColor(2,25,3,"Cloud Tint:     " .. GetCloudTint(),255,0,0) --GetCloudTint
		TextColor(2,28,3,"Solar Latitude: " .. GetSolarLatitude(),255,0,0) --GetSolarLatitude
		TextColor(2,31,3,"Solar Day:      " .. GetSolarDay(),255,0,0) --GetSolarDay
		TextColor(2,34,3,"Solar Month:    " .. GetSolarMonth(),255,0,0) --GetSolarMonth
		TextColor(2,37,3,"Solar Time HR:  " .. GetSolarTime(),255,0,0) --GetSolarTime
		
		TextColor(2,40,3,"LUT:            " .. GetLut(),255,0,0)--GetLut
	end
	
	if status == 1 then
	
		SetSkyTo("a simulated sky")
	
		SetCloudCoverage(0) -- turn off procedural clouds
		SetCloudDensity(0) -- turn off procedural clouds
		SetStarDensity(50)
		SetSkyEffects(50) -- turn off
		
		SetSolarLatitude(55) -- -90 to 90
		SetSolarDay(21) --1 to 31
		SetSolarMonth(6) --1 to 12
		SetSolarTime(time_hours)
		
		Prompt(math.floor(time_hours) .. " DAY/NIGHT CYCLE (SOLAR TIME)")
		time_hours = time_hours + 0.00833333 
		if time_hours >= 24 then 
			--time_hours = 12 
			--SetSolarTime(time_hours)
			status = 2
		end
		return
	end
	
	if status == 2 then
		Prompt(math.floor(factor) .. " STAR DENSITY")
		SetSunTime(1,1,1,55.3) -- 1am, 1st Jan, 55.3 degrees North
		SetStarDensity(factor)
		factor = factor + 0.1 
		if factor >= 100 then 
			factor = 0.1
			SetStarDensity(50)
			status = 3
		end
		return
	end
	
	if status == 3 then
		--Prompt("AURORA")
		SetSunTime(1,1,1,55.3) -- 1am, 1st Jan, 55.3 degrees North
		if factor < 50 then 
			SetSkyEffects(factor)
			if factor <= 10 then
				SetSkyEffects(5)
				Prompt("AURORA - 5")
			end
			if factor > 10 and factor <= 20 then
				SetSkyEffects(15)
				Prompt("AURORA - 15")
			end
			if factor > 20 and factor <= 30 then
				SetSkyEffects(25)
				Prompt("AURORA - 25")
			end
			if factor > 30 and factor <= 40 then
				SetSkyEffects(35)
				Prompt("AURORA - 35")
			end
			if factor > 40 and factor < 50 then
				SetSkyEffects(45)
				Prompt("AURORA - 45")
			end
			
		end
		factor = factor + 0.1 
		if factor >= 50 then 
			factor = 50.1 
			status = 4
		end
		return
	end
	
	if status == 4 then
		Prompt("LIGHTNING")
		SetSunTime(1,1,1,55.3) -- 1am, 1st Jan, 55.3 degrees North
		SetSkyEffects(factor)
		factor = factor + 0.1 
		if factor >= 100 then 
			factor = 0 
			SetSkyEffects(50)
			status = 5
		end
		return
	end
	
	if status == 5 then
		Prompt("CLOUDS - COVERAGE & DENSITY ( " .. math.floor(factor) .. " )")
		SetSunTime(17,21,6,30) -- 5pm, 21st June, near equator 30 degrees North
		SetSkyEffects(50)
		SetCloudCoverage(50) 
		SetCloudDensity(factor)
		factor = factor + 0.1 
		if factor >= 100 then 
			factor = 0 
			SetCloudCoverage(90) 
			SetCloudDensity(30)
			status = 6
		end
		return
	end
	
	if status == 6 then
		Prompt("CLOUDS - COVERAGE ( " .. math.floor(factor) .. " ) & DENSITY")
		--SetSunTime(12,21,6,30) -- 12pm, 21st June, nerae equator 30 degrees North
		SetCloudCoverage(factor) 
		SetCloudDensity(50)
		factor = factor + 0.1 
		if factor >= 100 then 
			factor = 0 
			SetCloudCoverage(90) 
			SetCloudDensity(30)
			status = 7
		end
		return
	end
	
	if status == 7 then
		--Prompt("CLOUDS - WIND SPEED " .. math.floor(factor))
		factor = factor + 0.05 
		if factor <= 10 then
			SetCloudSpeed(1)
			Prompt("CLOUDS - WIND SPEED 1")
		end
		if factor > 10 and factor <= 20 then
			SetCloudSpeed(10)
			Prompt("CLOUDS - WIND SPEED 10")
		end
		if factor > 20 and factor <= 30 then
			SetCloudSpeed(20)
			Prompt("CLOUDS - WIND SPEED 20")
		end
		if factor > 30 and factor <= 40 then
			SetCloudSpeed(30)
			Prompt("CLOUDS - WIND SPEED 30")
		end
		if factor > 40 and factor <= 50 then
			SetCloudSpeed(40)
			Prompt("CLOUDS - WIND SPEED 40")
		end
		if factor > 50 and factor <= 60 then
			SetCloudSpeed(50)
			Prompt("CLOUDS - WIND SPEED 50")
		end
		if factor > 60 and factor <= 70 then
			SetCloudSpeed(60)
			Prompt("CLOUDS - WIND SPEED 60")
		end
		if factor > 70 and factor <= 80 then
			SetCloudSpeed(70)
			Prompt("CLOUDS - WIND SPEED 70")
		end
		if factor > 80 and factor <= 90 then
			SetCloudSpeed(80)
			Prompt("CLOUDS - WIND SPEED 80")
		end
		if factor > 90 and factor <= 100 then
			SetCloudSpeed(100)
			Prompt("CLOUDS - WIND SPEED 100")
		end
		if factor >= 100 then 
			factor = 0 
			SetCloudSpeed(40)
			status = 8
		end
		return
	end
	
	if status == 8 then
		Prompt("CLOUDS - TINT " .. math.floor(factor))
		factor = factor + 0.1 
		SetCloudTint(factor)
		if factor >= 100 then 
			factor = 0 
			SetCloudTint(50)
			status = 9
		end
		return
	end
	
	if status == 9 then
		--Prompt("MIX")
		factor = factor + 0.1 
		
		if factor >= 100 then 
			factor = 0 
			status = 0
			time_hours = 5
			SetSkyTo("clear")
		end
		return
	end

end
