-- LUA Script - precede every function and global member with lowercase name of script + '_main'
g_myimage = {}
g_mysprite = {}
g_myangle = {}

function imageinzone_init(e)
 g_myimage[e] = LoadImage(GetEntityString(e,0))
 g_mysprite[e] = 0
 g_myangle[e] = 0
end

function imageinzone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  if g_mysprite[e] == 0 then
   g_mysprite[e] = CreateSprite ( g_myimage[e] )
  end
  SetSpriteDepth ( g_mysprite[e], 100 )
  aspectratio = GetImageWidth(g_myimage[e]) / GetImageHeight(g_myimage[e])
  SetSpriteSize ( g_mysprite[e], 50*aspectratio, 50 )
  SetSpriteOffset ( g_mysprite[e], 25*aspectratio, 25 )
  SetSpritePosition ( g_mysprite[e], 50, 50 )
  --g_myangle[e] = g_myangle[e] + 0.1
  --SetSpriteAngle( g_mysprite[e], g_myangle[e] )
  --SetSpriteColor ( g_mysprite[e], math.random(0,255), math.random(0,255), math.random(0,255), 255 )
 else
  if g_mysprite[e] > 0 then
   DeleteSprite ( g_mysprite[e] )
   g_mysprite[e] = 0
  end
 end
end
