#ifndef _CVECTORMANAGER_H_
#define _CVECTORMANAGER_H_

#include "cVectorDataC.h"

#include "directx-macros.h"
#include <basetsd.h>
#include <stdio.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>   
#include <windowsx.h> 

#include <map>

class BaseVector
{
public:
    enum eVectorType
    {
        E_TYPE_VECTOR2 = 1,
        E_TYPE_VECTOR3 = 2,
        E_TYPE_VECTOR4 = 3,
        E_TYPE_MATRIX  = 4,
    };

    BaseVector() {}
    virtual eVectorType GetType() const = 0;
};

class Vector2 : public BaseVector
{
private:
    GGVECTOR2 V;
public:
    Vector2() : V(0.0f, 0.0f) { }
    eVectorType GetType() const { return E_TYPE_VECTOR2; }

    GGVECTOR2& Get() { return V; }
};

class Vector3 : public BaseVector
{
private:
    GGVECTOR3 V;
public:
    Vector3() : V(0.0f, 0.0f, 0.0f) { }
    eVectorType GetType() const { return E_TYPE_VECTOR3; }

    GGVECTOR3& Get() { return V; }
};

class Vector4Ex : public BaseVector
{
private:
    GGVECTOR4 V;
public:
    Vector4Ex() : V(0.0f, 0.0f, 0.0f, 0.0f) { }
    eVectorType GetType() const { return E_TYPE_VECTOR4; }

    GGVECTOR4& Get() { return V; }
};

class Matrix : public BaseVector
{
private:
    GGMATRIX V;
public:
    Matrix()
    {
        memset( &V, 0, sizeof(V) );
    }
    eVectorType GetType() const { return E_TYPE_MATRIX; }

    GGMATRIX& Get() { return V; }
};

class cDataManager
{
	private:
        typedef std::map<int, BaseVector*>	List_t;
        typedef List_t::iterator			ListPtr;
        typedef List_t::const_iterator		ConstListPtr;
        typedef std::pair<ListPtr, bool>    ListInsertStatus;

        List_t	m_List;

	public:
		cDataManager  		( );
		~cDataManager 		( );
		void ClearAll		( );

		bool Add    		( BaseVector* pData, int iID );
		bool Delete 		( int iID );

        bool Exist ( int iID ) const { return m_List.count( iID ) > 0; }

        template <typename T>
		T* GetData ( int iID ) const 
        {
            ConstListPtr p = m_List.find( iID );
            return (p == m_List.end()) ? NULL : dynamic_cast<T*>(p->second);
        };
};

#endif _CVECTORMANAGER_H_