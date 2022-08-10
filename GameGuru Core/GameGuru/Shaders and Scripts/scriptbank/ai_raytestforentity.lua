-- Cast a ray from camera 500 units in front and report on what is detected

function ai_raytestforentity_init(e)
end

function ai_raytestforentity_main(e)
 selected_entity = 0
 interX = 0
 x1=GetCameraPositionX(0)
 y1=GetCameraPositionY(0)
 z1=GetCameraPositionZ(0)
 MoveCamera(0,500)
 x2=GetCameraPositionX(0)
 y2=GetCameraPositionY(0)
 z2=GetCameraPositionZ(0)
 MoveCamera(0,-500)
 interX=IntersectAll(x1,y1,z1,x2,y2,z2,0) 
 if interX > 0 then
  d=GetIntersectCollisionX()
  e=GetIntersectCollisionY()
  f=GetIntersectCollisionZ()
  for h=1,200 do
   if g_Entity[h] ~= nil and g_Entity[h]['obj'] == interX then 
    selected_entity = h
   end
  end
  TextCenterOnXColor(50,5,1,"Object="..interX.." Dynamic Entity="..selected_entity.. " ("..math.floor(d)..","..math.floor(e).."," ..math.floor(f)..")",100,255,255) 
 end
end
