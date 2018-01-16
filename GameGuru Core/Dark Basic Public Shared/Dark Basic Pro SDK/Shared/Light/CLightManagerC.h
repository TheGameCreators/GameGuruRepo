#ifndef _CLIGHTMANAGER_H_
#define _CLIGHTMANAGER_H_

#include "directx-macros.h"
#include <basetsd.h>
#include <stdio.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>   
#include <windowsx.h>

#include "CLightDataC.h"
#include <map>

class CLightManager
{
    public:
        typedef std::map<int, tagLightData*>    LightList_t;
        typedef LightList_t::iterator           LightPtr;

	private:
        LightList_t     m_List;
        tagLightData*   CurrentPtr;
        int             CurrentID;

    public:
        CLightManager	( ) : CurrentPtr(NULL), CurrentID(-1) { }

		bool Add	( tagLightData* pData, int iID );
		bool Delete ( int iID, LPGGDEVICE pD3D = NULL );

        void DeleteAll ( LPGGDEVICE pD3D = NULL );

		tagLightData* GetData ( int iID );
};

#endif _CLIGHTMANAGER_H_