-- Resolutions module

local resolutions = {}

function resolutions.findclosest()
 g_strBestResolution = "1366x768"
 iSmallestDifference = 99999
 for i = 1, 8, 1
 do  
  if i == 1 then iAvailableWidth = 1280; iAvailableHeight = 720; 
  elseif i == 2 then iAvailableWidth = 1280; iAvailableHeight = 800; 
  elseif i == 3 then iAvailableWidth = 1366; iAvailableHeight = 768; 
  elseif i == 4 then iAvailableWidth = 1440; iAvailableHeight = 900; 
  elseif i == 5 then iAvailableWidth = 1600; iAvailableHeight = 900; 
  elseif i == 6 then iAvailableWidth = 1680; iAvailableHeight = 1050; 
  elseif i == 7 then iAvailableWidth = 1920; iAvailableHeight = 1080; 
  elseif i == 8 then iAvailableWidth = 1920; iAvailableHeight = 1200; 
  else iAvailableWidth = 0
  end
  iDifference = math.abs(GetDeviceWidth()-iAvailableWidth) + math.abs(GetDeviceHeight()-iAvailableHeight)
  if iDifference < iSmallestDifference then
   iSmallestDifference = iDifference
   g_strBestResolution = iAvailableWidth .. "x" .. iAvailableHeight
  end
 end 
 return g_strBestResolution
end

return resolutions
