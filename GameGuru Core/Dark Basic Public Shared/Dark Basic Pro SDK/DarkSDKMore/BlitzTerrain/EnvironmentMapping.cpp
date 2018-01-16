#include "EnvironmentMapping.h"
#include "main.h"
#include <stdlib.h>
#include <memory.h>

unsigned short BT_Intern_AddEnvironment(BT_EnvironmentMap* EnvironmentMap,unsigned long Colour)
{
//Check that this environment map is not made
	if(EnvironmentMap->Made==false)
	{
	//Increase buffer size
		EnvironmentMap->EnvironmentCount++;
		EnvironmentMap->EnvironmentBuffer=(unsigned long*)realloc(EnvironmentMap->EnvironmentBuffer,(EnvironmentMap->EnvironmentCount+1)*sizeof(unsigned long));
		if(EnvironmentMap->EnvironmentBuffer==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		//EnvironmentMap->EnvironmentDetailMap=(unsigned long*)realloc(EnvironmentMap->EnvironmentBuffer,(EnvironmentMap->EnvironmentCount+1)*sizeof(unsigned long));

	//Set last value
		EnvironmentMap->EnvironmentBuffer[EnvironmentMap->EnvironmentCount]=Colour;
		//EnvironmentMap->EnvironmentDetailMap[EnvironmentMap->EnvironmentCount]=0;

	//Return count
		return EnvironmentMap->EnvironmentCount;
	}

//Return nothing
	return 0;
}

void BT_Intern_CreateEnvironmentMap(BT_EnvironmentMap* EnvironmentMap,unsigned long Width,unsigned long Height,unsigned long* Buffer)
{
//Allocate buffer
	EnvironmentMap->Buffer=(unsigned short*)malloc(Width*Height*sizeof(unsigned short));
	if(EnvironmentMap->Buffer==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	memset(EnvironmentMap->Buffer,0,Width*Height*sizeof(unsigned short));

//Set width and height
	EnvironmentMap->Width=Width;
	EnvironmentMap->Height=Height;

//Loop through points and add to the environment map
	for(unsigned long x=0;x<Width;x++)
	{
		for(unsigned long y=0;y<Height;y++)
		{
			for(unsigned short CurrentEnvironment=1;CurrentEnvironment<EnvironmentMap->EnvironmentCount+1;CurrentEnvironment++)
			{
				if(EnvironmentMap->EnvironmentBuffer[CurrentEnvironment]==Buffer[x+y*Width])
					EnvironmentMap->Buffer[x+y*Width]=CurrentEnvironment;
			}
		}
	}

//Cleanup
	free(EnvironmentMap->EnvironmentBuffer);
	EnvironmentMap->EnvironmentBuffer=0;

//Say that its made
	EnvironmentMap->Made=true;
}

void BT_Intern_CleanupEnvironmentMap(BT_EnvironmentMap* EnvironmentMap)
{
	free(EnvironmentMap->EnvironmentBuffer);
	EnvironmentMap->EnvironmentBuffer=0;
}

void BT_Intern_DeleteEnvironmentMap(BT_EnvironmentMap* EnvironmentMap)
{
//Delete Environment info
	if(EnvironmentMap->EnvironmentBuffer!=0)
		free(EnvironmentMap->EnvironmentBuffer);

//Delete buffer
	if(EnvironmentMap->Buffer!=0)
		free(EnvironmentMap->Buffer);

//Clean up
	memset(EnvironmentMap,0,sizeof(BT_EnvironmentMap));
}

unsigned short BT_Intern_GetPointEnvironment(BT_EnvironmentMap* EnvironmentMap,unsigned long x,unsigned long y)
{
	return EnvironmentMap->Buffer[x+y*EnvironmentMap->Width];
}

void BT_Intern_SetEnvironmentDetailmap(BT_EnvironmentMap* EnvironmentMap,unsigned short EnvironmentMapID,unsigned long DetailMapID)
{
	EnvironmentMap->EnvironmentDetailMap[EnvironmentMapID]=DetailMapID;
}

unsigned long BT_Intern_GetEnvironmentDetailmap(BT_EnvironmentMap* EnvironmentMap,unsigned short EnvironmentMapID)
{
	return EnvironmentMap->EnvironmentDetailMap[EnvironmentMapID];
}