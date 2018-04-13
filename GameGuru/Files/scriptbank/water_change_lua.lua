-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- I-key to increase value, K-key to lower value
-- J and L-keys to switch through the different settings
local switch=0
local keypressed_now = 0
local multiply = 1
local water_state_array = {"Height","Red","Green","Blue","WaveIntensity","Transparancy","WaterReflection","ReflectionSparkleIntensity","WaterFlowDirection X","WaterFlowDirection Y","WaterFlowDirection Speed","WaterDistortionWaves","RippleWaterSpeed"}

function water_change_lua_init(e)
end

function water_change_lua_main(e)
  Prompt("Water change state: " .. water_state_array[switch+1])

  if g_Scancode == 23 or g_Scancode == 37 then
    if switch == 0 then
      --Height
      SetWaterHeight(GetWaterHeight()+(1*multiply)) 
    elseif switch == 1 then
      --Color Red
      SetWaterColor(GetWaterShaderColorRed()+(1*multiply), GetWaterShaderColorGreen(),GetWaterShaderColorBlue())
    elseif switch == 2 then
      --Color Green
      SetWaterColor(GetWaterShaderColorRed(), GetWaterShaderColorGreen()+(1*multiply),GetWaterShaderColorBlue())
    elseif switch == 3 then
      --Color Blue
      SetWaterColor(GetWaterShaderColorRed(),GetWaterShaderColorGreen(),GetWaterShaderColorBlue()+(1*multiply))
    elseif switch == 4 then
      --WaveIntensity
      SetWaterWaveIntensity(GetWaterWaveIntensity()+(1*multiply)) 
    elseif switch == 5 then
      --Transparancy
      SetWaterTransparancy(GetWaterTransparancy()+(0.01*multiply)) 
    elseif switch == 6 then
      --WaterReflection
      SetWaterReflection(GetWaterReflection()+(0.01*multiply)) 
    elseif switch == 7 then
      --ReflectionSparkleIntensity
      SetWaterReflectionSparkleIntensity(GetWaterReflectionSparkleIntensity()+(0.1*multiply))
    elseif switch == 8 then
      --WaterFlowDirection X
      SetWaterFlowDirection(GetWaterFlowDirectionX()+(0.01*multiply),GetWaterFlowDirectionY(),GetWaterFlowSpeed())  
    elseif switch == 9 then
      --WaterFlowDirection Y
      SetWaterFlowDirection(GetWaterFlowDirectionX(),GetWaterFlowDirectionY()+(0.01*multiply),GetWaterFlowSpeed())  
    elseif switch == 10 then
      --WaterFlowDirection Speed
      SetWaterFlowDirection(GetWaterFlowDirectionX(),GetWaterFlowDirectionY(),GetWaterFlowSpeed()+(1*multiply))
    elseif switch == 11 then
      --WaterDistortionWaves
      SetWaterDistortionWaves(GetWaterDistortionWaves()+(0.001*multiply)) 
    elseif switch == 12 then
      --RippleWaterSpeed
      SetRippleWaterSpeed(GetRippleWaterSpeed()+(1*multiply)) 
    end
  end

  if g_Scancode == 23 then multiply=1
  elseif g_Scancode == 37 then multiply=-1
  end

  if g_Scancode == 36 and keypressed_now == 0 then
    if switch > 0 then 
      switch = switch-1 
      keypressed_now = 1
    end
  elseif g_Scancode == 38 and keypressed_now == 0 then
    if switch < 11 then
      switch = switch+1
      keypressed_now = 1
    end
  end

  if g_Scancode==0 then
    keypressed_now = 0
  end
end
