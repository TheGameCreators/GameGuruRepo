-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- I-key to increase value, K-key to lower value
-- J and L-keys to switch through the different settings
local switch=0
local keypressed_now = 0
local multiply = 1
local water_state_array = {"Height","Red","Green","Blue","WaveIntensity","Transparancy","WaterReflection","ReflectionSparkleIntensity","WaterFlowDirection X","WaterFlowDirection Y","WaterFlowDirection Speed","WaterDistortionWaves","RippleWaterSpeed"}
local water_value = 0

function water_change_lua_init(e)
  water_value = GetCurrentValueFromWaterState(switch)
end

function water_change_lua_main(e)
  Prompt("Water change state: " .. water_state_array[switch+1] .. "/ Current value: " .. water_value)

  if g_Scancode == 23 or g_Scancode == 37 then
    if switch == 0 then
      --Height 
      SetWaterHeight(GetWaterHeight()+(1*multiply))
      water_value = GetWaterHeight()
    elseif switch == 1 then
      --Color Red
      SetWaterColor(GetWaterShaderColorRed()+(1*multiply), GetWaterShaderColorGreen(),GetWaterShaderColorBlue())
      water_value = GetWaterShaderColorRed()
    elseif switch == 2 then
      --Color Green
      SetWaterColor(GetWaterShaderColorRed(), GetWaterShaderColorGreen()+(1*multiply),GetWaterShaderColorBlue())
      water_value = GetWaterShaderColorGreen()
    elseif switch == 3 then
      --Color Blue
      SetWaterColor(GetWaterShaderColorRed(),GetWaterShaderColorGreen(),GetWaterShaderColorBlue()+(1*multiply))
      water_value = GetWaterShaderColorBlue()
    elseif switch == 4 then
      --WaveIntensity
      SetWaterWaveIntensity(GetWaterWaveIntensity()+(1*multiply)) 
      water_value = GetWaterWaveIntensity()
    elseif switch == 5 then
      --Transparancy
      SetWaterTransparancy(GetWaterTransparancy()+(0.01*multiply)) 
      water_value = GetWaterTransparancy()
    elseif switch == 6 then
      --WaterReflection
      SetWaterReflection(GetWaterReflection()+(0.01*multiply)) 
      water_value = GetWaterReflection()
    elseif switch == 7 then
      --ReflectionSparkleIntensity
      SetWaterReflectionSparkleIntensity(GetWaterReflectionSparkleIntensity()+(0.1*multiply))
      water_value = GetWaterReflectionSparkleIntensity()
    elseif switch == 8 then
      --WaterFlowDirection X
      SetWaterFlowDirection(GetWaterFlowDirectionX()+(0.01*multiply),GetWaterFlowDirectionY(),GetWaterFlowSpeed()) 
      water_value = GetWaterFlowDirectionX() 
    elseif switch == 9 then
      --WaterFlowDirection Y
      SetWaterFlowDirection(GetWaterFlowDirectionX(),GetWaterFlowDirectionY()+(0.01*multiply),GetWaterFlowSpeed())  
      water_value = GetWaterFlowDirectionY()
    elseif switch == 10 then
      --WaterFlowDirection Speed
      SetWaterFlowDirection(GetWaterFlowDirectionX(),GetWaterFlowDirectionY(),GetWaterFlowSpeed()+(1*multiply))
      water_value = GetWaterFlowSpeed()
    elseif switch == 11 then
      --WaterDistortionWaves
      SetWaterDistortionWaves(GetWaterDistortionWaves()+(0.001*multiply)) 
      water_value = GetWaterDistortionWaves()
    elseif switch == 12 then
      --RippleWaterSpeed
      SetRippleWaterSpeed(GetRippleWaterSpeed()+(1*multiply)) 
      water_value = GetRippleWaterSpeed()
    end
  end

  if g_Scancode == 23 then multiply=1
  elseif g_Scancode == 37 then multiply=-1
  end

  if g_Scancode == 36 and keypressed_now == 0 then
    if switch > 0 then 
      switch = switch-1 
      keypressed_now = 1
      water_value = GetCurrentValueFromWaterState(switch)
    end
  elseif g_Scancode == 38 and keypressed_now == 0 then
    if switch < 11 then
      switch = switch+1
      keypressed_now = 1
      water_value = GetCurrentValueFromWaterState(switch)
    end
  end

  if g_Scancode==0 then
    keypressed_now = 0
  end
end

function GetCurrentValueFromWaterState(state)
  if state == 0 then return GetWaterHeight()
  elseif state == 1 then return GetWaterShaderColorRed()
  elseif state == 2 then return GetWaterShaderColorGreen()
  elseif state == 3 then return GetWaterShaderColorBlue()
  elseif state == 4 then return GetWaterWaveIntensity()
  elseif state == 5 then return GetWaterTransparancy()
  elseif state == 6 then return GetWaterReflection()
  elseif state == 7 then return GetWaterReflectionSparkleIntensity()
  elseif state == 8 then return GetWaterFlowDirectionX() 
  elseif state == 9 then return GetWaterFlowDirectionY()
  elseif state == 10 then return GetWaterFlowSpeed()
  elseif state == 11 then return GetWaterDistortionWaves()
  elseif state == 12 then return GetRippleWaterSpeed()
  end
end
