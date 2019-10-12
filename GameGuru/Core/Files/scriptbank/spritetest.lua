myImage = 0
myImage2 = 0
mySprites = {}
bigSprite = 0
toggle = 0
counter = 0


function spritetest_init(e)
	myImage = LoadImage("scriptbank\\images\\sprite test\\lee.png")
	myImage2 = LoadImage("scriptbank\\images\\sprite test\\ravey.png")
	
	bigSprite = CreateSprite ( myImage )
	SetSpriteDepth ( bigSprite , 100 )
	SetSpriteSize ( bigSprite , 10 , 10 )
	
	flag = 0
	for i = 1, 100 , 1
	do
		flag = 1-flag
		if flag == 1 then
			mySprites[i] = { spriteID = CreateSprite ( myImage ) ; x = math.random(0,100) ; y = math.random(0,100) , speed = math.random(), angle = math.random(0,359), direction = math.random(0,1) }
			SetSpriteOffset ( mySprites[i]['spriteID'] , 32 , 22 )
		else
			mySprites[i] = { spriteID = CreateSprite ( myImage2 ) ; x = math.random(0,100) ; y = math.random(0,100) , speed = math.random(), angle = math.random(0,359), direction = math.random(0,1) }
			SetSpriteOffset ( mySprites[i]['spriteID'] , 32 , 31 )
		end
		SetSpriteSize ( mySprites[i]['spriteID'] , 5 , -1 )
	end
end

function spritetest_main(e)

	counter = counter + 1
	if counter > 60 then
		counter = 0
		toggle = 1 - toggle
		if toggle == 0 then
			SetSpriteImage ( bigSprite , myImage )
		else
			SetSpriteImage ( bigSprite , myImage2 )
		end
	end
	
	distMulti = 3.0 - GetPlayerDistance(e)/500

	for i = 1 , 100 , 1
	do
		x = mySprites[i]['x'] + (math.sin(mySprites[i]['angle'] * 0.0174533 ) * mySprites[i]['speed'])
		y = mySprites[i]['y'] + (math.cos(mySprites[i]['angle'] * 0.0174533 ) * mySprites[i]['speed'])
		
		if mySprites[i]['direction'] == 0 then 
			mySprites[i]['angle'] = mySprites[i]['angle'] + (mySprites[i]['speed'])*distMulti
			mySprites[i]['angle'] = mySprites[i]['angle'] + (mySprites[i]['speed'])*distMulti			
		else
			mySprites[i]['angle'] = mySprites[i]['angle'] - (mySprites[i]['speed'])*distMulti
			mySprites[i]['angle'] = mySprites[i]['angle'] - (mySprites[i]['speed'])*distMulti
		end
		
		SetSpritePosition ( mySprites[i]['spriteID'] , x , y )
		
		if counter == 0 then
			SetSpriteColor ( mySprites[i]['spriteID'] , math.random(0,255) , math.random(0,255) , math.random(0,255) , 255 )
		end
		SetSpriteAngle( mySprites[i]['spriteID'] , mySprites[i]['angle'] )
	end
	
	if counter == 0 then
		SetSpriteColor ( bigSprite , math.random(0,255) , math.random(0,255) , math.random(0,255) , 255 )
	end	
	
end