g_cabinet_setup = {}
g_cabinet_drawA = {}
g_cabinet_drawAinvestigate = {}
g_cabinet_drawAox = {}
g_cabinet_drawAoy = {}
g_cabinet_drawAoz = {}
g_cabinet_drawAopenvalue = {}
g_cabinet_drawB = {}
g_cabinet_drawBinvestigate = {}
g_cabinet_drawBox = {}
g_cabinet_drawBoy = {}
g_cabinet_drawBoz = {}
g_cabinet_drawBopenvalue = {}
g_cabinet_doorA = {}
g_cabinet_doorAinvestigate = {}
g_cabinet_doorAox = {}
g_cabinet_doorAoy = {}
g_cabinet_doorAoz = {}
g_cabinet_doorAopenvalue = {}
g_cabinet_doorB = {}
g_cabinet_doorBinvestigate = {}
g_cabinet_doorBox = {}
g_cabinet_doorBoy = {}
g_cabinet_doorBoz = {}
g_cabinet_doorBopenvalue = {}
g_cabinet_investigate = {}
g_cabinetpart_name = {}

function cabinet_init(e)
 g_cabinet_setup[e] = 0
 g_cabinet_drawA[e] = 0
 g_cabinet_drawAinvestigate[e] = 0
 g_cabinet_drawB[e] = 0
 g_cabinet_drawBinvestigate[e] = 0
 g_cabinet_doorA[e] = 0
 g_cabinet_doorAinvestigate[e] = 0
 g_cabinet_doorB[e] = 0
 g_cabinet_doorBinvestigate[e] = 0
 g_cabinet_investigate[e] = 1
end

function cabinet_main(e)
 -- setup
 if g_cabinet_setup[e] == 0 then
  g_cabinet_setup[e] = 1
  for ee = 1, 1000 do
   if g_cabinetpart_name[ee] ~= nil then
    if g_cabinetpart_name[ee] == GetEntityString(e,0) then
     g_cabinet_drawA[e] = ee
     g_cabinet_drawAox[e] = g_Entity[ee]['x'] - g_Entity[e]['x']
     g_cabinet_drawAoy[e] = g_Entity[ee]['y'] - g_Entity[e]['y']
     g_cabinet_drawAoz[e] = g_Entity[ee]['z'] - g_Entity[e]['z']
	 g_cabinet_drawAopenvalue[e] = 0
    end
    if g_cabinetpart_name[ee] == GetEntityString(e,1) then
     g_cabinet_drawB[e] = ee
     g_cabinet_drawBox[e] = g_Entity[ee]['x'] - g_Entity[e]['x']
     g_cabinet_drawBoy[e] = g_Entity[ee]['y'] - g_Entity[e]['y']
     g_cabinet_drawBoz[e] = g_Entity[ee]['z'] - g_Entity[e]['z']
	 g_cabinet_drawBopenvalue[e] = 0
    end
    if g_cabinetpart_name[ee] == GetEntityString(e,2) then
     g_cabinet_doorA[e] = ee
     g_cabinet_doorAox[e] = g_Entity[ee]['x'] - g_Entity[e]['x']
     g_cabinet_doorAoy[e] = g_Entity[ee]['y'] - g_Entity[e]['y']
     g_cabinet_doorAoz[e] = g_Entity[ee]['z'] - g_Entity[e]['z']
	 g_cabinet_doorAopenvalue[e] = 0
    end
    if g_cabinetpart_name[ee] == GetEntityString(e,3) then
     g_cabinet_doorB[e] = ee
     g_cabinet_doorBox[e] = g_Entity[ee]['x'] - g_Entity[e]['x']
     g_cabinet_doorBoy[e] = g_Entity[ee]['y'] - g_Entity[e]['y']
     g_cabinet_doorBoz[e] = g_Entity[ee]['z'] - g_Entity[e]['z']
	 g_cabinet_doorBopenvalue[e] = 0
    end
   end
  end 
 end
 -- use
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 120 and g_PlayerHealth > 0 then
  if g_PlayerController==0 then
   Prompt("Press E To investigate cabinet" )
  else
   Prompt("Press Y Button to investigate cabinet" )
  end
  if g_KeyPressE == 1 then
   g_cabinet_setup[e] = 2
   if g_cabinet_investigate[e] == 1 then g_cabinet_drawAinvestigate[e] = 1 end
   if g_cabinet_investigate[e] == 2 then g_cabinet_drawBinvestigate[e] = 1 end
   if g_cabinet_investigate[e] == 3 then g_cabinet_doorAinvestigate[e] = 1 end
   if g_cabinet_investigate[e] == 4 then g_cabinet_doorBinvestigate[e] = 1 end
  else
   if g_cabinet_setup[e] == 2 then
    if g_cabinet_investigate[e] == 1 then g_cabinet_drawAinvestigate[e] = 2 end
    if g_cabinet_investigate[e] == 2 then g_cabinet_drawBinvestigate[e] = 2 end
    if g_cabinet_investigate[e] == 3 then g_cabinet_doorAinvestigate[e] = 2 end
    if g_cabinet_investigate[e] == 4 then g_cabinet_doorBinvestigate[e] = 2 end
    g_cabinet_investigate[e]=g_cabinet_investigate[e]+1
	if g_cabinet_investigate[e] > 4 then g_cabinet_investigate[e] = 1 end
    g_cabinet_setup[e] = 1
   end
  end
 end
 -- animate investigation - drawA
 if g_cabinet_drawAinvestigate[e] > 0 then
  if g_cabinet_drawAinvestigate[e] == 1 then
   g_cabinet_drawAopenvalue[e]=g_cabinet_drawAopenvalue[e]+1
   if g_cabinet_drawAopenvalue[e] > 10 then 
    g_cabinet_drawAopenvalue[e] = 10 
   end
  end
  if g_cabinet_drawAinvestigate[e] == 2 then
   g_cabinet_drawAopenvalue[e]=g_cabinet_drawAopenvalue[e]-1
   if g_cabinet_drawAopenvalue[e] < 0 then 
    g_cabinet_drawAopenvalue[e] = 0 
	g_cabinet_drawAinvestigate[e] = 0
   end
  end
  offx=math.cos((g_Entity[e]['angley']/360.0)*6.28)*1.0
  offz=math.sin((g_Entity[e]['angley']/360.0)*6.28)*-1.0
  xx=g_Entity[e]['x']+g_cabinet_drawAox[e]+(offx*g_cabinet_drawAopenvalue[e])
  yy=g_Entity[e]['y']+g_cabinet_drawAoy[e]
  zz=g_Entity[e]['z']+g_cabinet_drawAoz[e]+(offz*g_cabinet_drawAopenvalue[e])
  ResetPosition(g_cabinet_drawA[e],xx,yy,zz)
 end 
 -- animate investigation - drawB
 if g_cabinet_drawBinvestigate[e] > 0 then
  if g_cabinet_drawBinvestigate[e] == 1 then
   g_cabinet_drawBopenvalue[e]=g_cabinet_drawBopenvalue[e]+1
   if g_cabinet_drawBopenvalue[e] > 10 then 
    g_cabinet_drawBopenvalue[e] = 10 
   end
  end
  if g_cabinet_drawBinvestigate[e] == 2 then
   g_cabinet_drawBopenvalue[e]=g_cabinet_drawBopenvalue[e]-1
   if g_cabinet_drawBopenvalue[e] < 0 then 
    g_cabinet_drawBopenvalue[e] = 0 
	g_cabinet_drawBinvestigate[e] = 0
   end
  end
  offx=math.cos((g_Entity[e]['angley']/360.0)*6.28)*1.0
  offz=math.sin((g_Entity[e]['angley']/360.0)*6.28)*-1.0
  xx=g_Entity[e]['x']+g_cabinet_drawBox[e]+(offx*g_cabinet_drawBopenvalue[e])
  yy=g_Entity[e]['y']+g_cabinet_drawBoy[e]
  zz=g_Entity[e]['z']+g_cabinet_drawBoz[e]+(offz*g_cabinet_drawBopenvalue[e])
  ResetPosition(g_cabinet_drawB[e],xx,yy,zz)
 end 
 -- animate investigation - doorA
 if g_cabinet_doorAinvestigate[e] > 0 then
  if g_cabinet_doorAinvestigate[e] == 1 then
   g_cabinet_doorAopenvalue[e]=g_cabinet_doorAopenvalue[e]+3
   if g_cabinet_doorAopenvalue[e] > 80 then 
    g_cabinet_doorAopenvalue[e] = 80 
   end
  end
  if g_cabinet_doorAinvestigate[e] == 2 then
   g_cabinet_doorAopenvalue[e]=g_cabinet_doorAopenvalue[e]-3
   if g_cabinet_doorAopenvalue[e] < 0 then 
    g_cabinet_doorAopenvalue[e] = 0 
	g_cabinet_doorAinvestigate[e] = 0
   end
  end
  SetRotation(g_cabinet_doorA[e],0,g_Entity[e]['angley']+g_cabinet_doorAopenvalue[e],0)
 end 
 -- animate investigation - doorB
 if g_cabinet_doorBinvestigate[e] > 0 then
  if g_cabinet_doorBinvestigate[e] == 1 then
   g_cabinet_doorBopenvalue[e]=g_cabinet_doorBopenvalue[e]+3
   if g_cabinet_doorBopenvalue[e] > 80 then 
    g_cabinet_doorBopenvalue[e] = 80 
   end
  end
  if g_cabinet_doorBinvestigate[e] == 2 then
   g_cabinet_doorBopenvalue[e]=g_cabinet_doorBopenvalue[e]-3
   if g_cabinet_doorBopenvalue[e] < 0 then 
    g_cabinet_doorBopenvalue[e] = 0 
	g_cabinet_doorBinvestigate[e] = 0
   end
  end
  SetRotation(g_cabinet_doorB[e],0,g_Entity[e]['angley']+(g_cabinet_doorBopenvalue[e]*-1),0)
 end 
end
