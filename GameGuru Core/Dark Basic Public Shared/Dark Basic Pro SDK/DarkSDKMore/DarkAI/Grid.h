#ifndef H_GRID
#define H_GRID

#include <windows.h>

class GridElement
{

public:

	GridElement( ) { pNextElement = NULL; m_value = 0; m_x = 0; m_z = 0; m_reserved = 0; }
	~GridElement( ) {};

	void SetData( int x, int z, int value ) { m_x = x; m_z = z; m_value = value; m_timer2 = 1.0f; }
	int GetX( ) { return m_x; }
	int GetZ( ) { return m_z; }
	int GetValue( ) { return m_value; }
	void SetReserved( int value ) { m_reserved = value; m_timer = 5.0f; }
	int GetReserved( ) { return m_reserved; }
	void DecreaseTime( float fTimeDelta ) { m_timer -= fTimeDelta; m_timer2 -= fTimeDelta; }
	bool ReserveHasExpired( ) { return m_timer <= 0.0f; }
	bool PositionHasExpired( ) { return m_timer2 <= 0.0f; }
	void Increment( ) { m_value++; }
	void Decrement( ) { m_value--; }

	GridElement *pNextElement;

private:

	int m_x;
	int m_z;
	int m_value;
	int m_reserved;
	float m_timer;
	float m_timer2;
	
};

class Grid
{

public:

	Grid( );
	~Grid( );

	void SetPosition( int x, int z, int value );
	int GetPosition( int x, int z );
	void DeletePosition( int x, int z );

	void Increment( int x, int z );
	void Decrement( int x, int z );

	void Reserve( int x, int z, int value );
	int GetReserved( int x, int z );

	void Reset( );
	void Update( float fTimeDelta );

	GridElement* GetRootElement( int x, int z );

private:

	HANDLE hMutex;
	GridElement* m_pGrid [ 100 ][ 100 ];
};

#endif