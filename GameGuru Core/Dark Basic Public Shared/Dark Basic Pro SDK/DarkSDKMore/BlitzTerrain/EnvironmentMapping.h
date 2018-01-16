#ifndef _ENVIRONMENTMAPPING_H
#define _ENVIRONMENTMAPPING_H

struct BT_EnvironmentMap
{
	bool Made;
	unsigned long Width;
	unsigned long Height;
	unsigned short* Buffer;

	unsigned short EnvironmentCount;
	unsigned long* EnvironmentBuffer;
	unsigned long* EnvironmentDetailMap;
};


unsigned short BT_Intern_AddEnvironment(BT_EnvironmentMap* EnvironmentMap,unsigned long Colour);
void BT_Intern_CreateEnvironmentMap(BT_EnvironmentMap* EnvironmentMap,unsigned long Width,unsigned long Height,unsigned long* Buffer);
void BT_Intern_DeleteEnvironmentMap(BT_EnvironmentMap* EnvironmentMap);
unsigned short BT_Intern_GetPointEnvironment(BT_EnvironmentMap* EnvironmentMap,unsigned long x,unsigned long y);
void BT_Intern_SetEnvironmentDetailmap(BT_EnvironmentMap* EnvironmentMap,unsigned short EnvironmentMapID,unsigned long DetailMapID);
unsigned long BT_Intern_GetEnvironmentDetailmap(BT_EnvironmentMap* EnvironmentMap,unsigned short EnvironmentMapID);

#endif