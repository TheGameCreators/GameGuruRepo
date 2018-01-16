#include "CLightManagerC.h"

bool CLightManager::Add ( tagLightData* pData, int iID )
{
	///////////////////////////////////////
    // if an object already exists with  //
    // the same id, delete it            //
	///////////////////////////////////////
    Delete( iID, NULL );

	///////////////////////////////////////
	// create a new object and insert in //
	// the list                          //
	///////////////////////////////////////
	tagLightData* NewLight = new tagLightData;
    memcpy(NewLight, pData, sizeof(tagLightData));

    m_List.insert( std::make_pair(iID, NewLight) );

    CurrentPtr = NewLight;
    CurrentID  = iID;

	return false;
}

bool CLightManager::Delete ( int iID, LPGGDEVICE pD3D )
{
    // If deleting the cached  light, clear the cached data.
    if (CurrentID == iID)
    {
        CurrentPtr = NULL;
        CurrentID = -1;
    }

    // Locate the light, disable it, delete the light details,
    // then remove it from the list.
    LightPtr p = m_List.find( iID );
    if (p != m_List.end())
    {
		#ifdef DX11
		#else
        if (pD3D)
        	pD3D->LightEnable ( p->first, FALSE );
		#endif

        delete p->second;
        
        m_List.erase( p );
        
        return true;
    }
    return false;
}

void CLightManager::DeleteAll ( LPGGDEVICE pD3D )
{
    // Disable all lights and free their memory
    for (LightPtr p = m_List.begin(); p != m_List.end(); ++p)
    {
		#ifdef DX11
		#else
        if (pD3D)
        	pD3D->LightEnable ( p->first, FALSE );
		#endif

        delete p->second;
    }

    // Now clear the list
    m_List.clear();

    // Clear the cached pointer.
    CurrentID = -1;
    CurrentPtr = NULL;
}

tagLightData* CLightManager::GetData ( int iID )
{
    // If the light requested isn't the cached light,
    // refresh the cached pointer.
    if (CurrentPtr == NULL || CurrentID != iID)
    {
        LightPtr p = m_List.find( iID );
        if (p == m_List.end())
            CurrentPtr = NULL;
        else
            CurrentPtr = p->second;
        CurrentID = iID;
    }

    // Return the cached pointer.
    return CurrentPtr;
}
