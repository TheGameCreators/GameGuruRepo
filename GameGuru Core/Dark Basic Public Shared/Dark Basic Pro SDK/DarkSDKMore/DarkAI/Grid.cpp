#include "Grid.h"
#include <stdio.h>

Grid::Grid( )
{
	for( int i = 0; i < 100; i++ )
		for( int j = 0; j < 100; j++ )
			m_pGrid [ i ][ j ] = NULL;

	hMutex = CreateMutex( NULL, FALSE, NULL );
}

Grid::~Grid( )
{
	Reset( );
	CloseHandle( hMutex );
}

void Grid::Reset( )
{
	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (reset) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	for( int i = 0; i < 100; i++ )
	{
		for( int j = 0; j < 100; j++ )
		{
			GridElement *pElement = m_pGrid [ i ][ j ];

			while ( pElement )
			{
				m_pGrid [ i ][ j ] = pElement->pNextElement;

				delete pElement;
				pElement = m_pGrid [ i ][ j ];
			}
		}
	}

	ReleaseMutex( hMutex );
}

void Grid::SetPosition( int x, int z, int value )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (set) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( !m_pGrid [ indexX ][ indexZ ] )
	{
		m_pGrid [ indexX ][ indexZ ] = new GridElement();
		m_pGrid [ indexX ][ indexZ ]->SetData( x,z, value );
	}
	else
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement )
		{
			if ( pElement->GetX() == x && pElement->GetZ() == z )
			{
				pElement->SetData( x,z, value );
				ReleaseMutex( hMutex );
				return;
			}

			pElement = pElement->pNextElement;
		}

		GridElement *pNewElement = new GridElement();
		pNewElement->SetData( x,z, value );

		pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement->pNextElement )
		{
			pElement = pElement->pNextElement;
		}

		pElement->pNextElement = pNewElement;
	}

	ReleaseMutex( hMutex );
}

int Grid::GetPosition( int x, int z )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (get) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( m_pGrid [ indexX ][ indexZ ] )
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement )
		{
			if ( pElement->GetX( ) == x && pElement->GetZ( ) == z ) 
			{
				int value = pElement->GetValue( );
				ReleaseMutex( hMutex );
				return value;
			}

			pElement = pElement->pNextElement;
		}
		
	}
	
	ReleaseMutex( hMutex );
	return 0;
}

void Grid::DeletePosition( int x, int z )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (delete) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( m_pGrid [ indexX ][ indexZ ] )
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];
		GridElement *pPrevElement = NULL;

		while ( pElement )
		{
			if ( pElement->GetX( ) == x && pElement->GetZ( ) == z ) 
			{
				if ( !pPrevElement ) m_pGrid [ indexX ][ indexZ ] = pElement->pNextElement;
				else pPrevElement->pNextElement = pElement->pNextElement;

				delete pElement;
			}
			else
			{
				pPrevElement = pElement;
			}

			if ( pPrevElement ) pElement = pPrevElement->pNextElement;
			else pElement = m_pGrid [ indexX ][ indexZ ];
		}
		
	}
	
	ReleaseMutex( hMutex );
}

void Grid::Increment( int x, int z )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (inc) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( !m_pGrid [ indexX ][ indexZ ] )
	{
		m_pGrid [ indexX ][ indexZ ] = new GridElement();
		m_pGrid [ indexX ][ indexZ ]->SetData( x,z, 1 );
	}
	else
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement )
		{
			if ( pElement->GetX() == x && pElement->GetZ() == z )
			{
				pElement->Increment( );
				ReleaseMutex( hMutex );
				return;
			}

			pElement = pElement->pNextElement;
		}

		GridElement *pNewElement = new GridElement();
		pNewElement->SetData( x,z, 1 );

		pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement->pNextElement )
		{
			pElement = pElement->pNextElement;
		}

		pElement->pNextElement = pNewElement;
	}

	ReleaseMutex( hMutex );
}

void Grid::Decrement( int x, int z )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (inc) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	bool bDelete = false;

	if ( !m_pGrid [ indexX ][ indexZ ] )
	{
		ReleaseMutex( hMutex );
		return;
	}

	GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

	while ( pElement )
	{
		if ( pElement->GetX() == x && pElement->GetZ() == z )
		{
			pElement->Decrement( );
			if ( pElement->GetValue( ) <= 0 ) bDelete = true;
			break;
		}

		pElement = pElement->pNextElement;
	}
	
	ReleaseMutex( hMutex );

	if ( bDelete ) DeletePosition( x,z );
}

void Grid::Reserve( int x, int z, int value )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (reserve) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( !m_pGrid [ indexX ][ indexZ ] )
	{
		m_pGrid [ indexX ][ indexZ ] = new GridElement();
		m_pGrid [ indexX ][ indexZ ]->SetData( x,z, -1 );
		m_pGrid [ indexX ][ indexZ ]->SetReserved( value );
	}
	else
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement )
		{
			if ( pElement->GetX() == x && pElement->GetZ() == z )
			{
				//pElement->SetData( x,z, 0 );
				pElement->SetReserved( value );
				ReleaseMutex( hMutex );
				return;
			}

			pElement = pElement->pNextElement;
		}

		GridElement *pNewElement = new GridElement();
		pNewElement->SetData( x,z, -1 );
		pNewElement->SetReserved( value );

		pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement->pNextElement )
		{
			pElement = pElement->pNextElement;
		}

		pElement->pNextElement = pNewElement;
	}

	ReleaseMutex( hMutex );
}

int Grid::GetReserved( int x, int z )
{
	int indexX = abs(x) % 100;
	int indexZ = abs(z) % 100;

	if ( indexX > 99 ) indexX = 99;
	if ( indexZ > 99 ) indexZ = 99;
	if ( indexX < 0 ) indexX = 0;
	if ( indexZ < 0 ) indexZ = 0;

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Grid (getR) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( m_pGrid [ indexX ][ indexZ ] )
	{
		GridElement *pElement = m_pGrid [ indexX ][ indexZ ];

		while ( pElement )
		{
			if ( pElement->GetX( ) == x && pElement->GetZ( ) == z ) 
			{
				int value = pElement->GetReserved( );
				ReleaseMutex( hMutex );
				return value;
			}

			pElement = pElement->pNextElement;
		}
		
	}
	
	ReleaseMutex( hMutex );
	return 0;
}

void Grid::Update( float fTimeDelta )
{
	/* this does nothing as grid not used now
	for( int i = 0; i < 100; i++ )
	{
		for( int j = 0; j < 100; j++ )
		{
			GridElement *pElement = m_pGrid [ i ][ j ];
			GridElement *pPrevElement = NULL;

			while ( pElement )
			{
				pElement->DecreaseTime( fTimeDelta );

				if ( pElement->PositionHasExpired( ) )
				{
					if ( pElement->GetReserved( ) )
					{
						pElement->SetData( pElement->GetX(), pElement->GetZ(), -1 );
					}
					else
					{
						if ( pPrevElement ) pPrevElement->pNextElement = pElement->pNextElement;
						else m_pGrid [ i ][ j ] = pElement->pNextElement;

						GridElement *pDelElement = pElement;
						pElement = pElement->pNextElement;
						delete pDelElement;
						continue;
					}
				}

				if ( pElement->GetReserved( ) )
				{
					if ( pElement->ReserveHasExpired( ) )
					{
						pElement->SetReserved( 0 );
						if ( pElement->GetValue( ) <= 0 )
						{
							if ( pPrevElement ) pPrevElement->pNextElement = pElement->pNextElement;
							else m_pGrid [ i ][ j ] = pElement->pNextElement;

							GridElement *pDelElement = pElement;
							pElement = pElement->pNextElement;
							delete pDelElement;
							continue;
						}
					}
				}

				pPrevElement = pElement;
				pElement = pElement->pNextElement;
			}
		}
	}
	*/
}

GridElement* Grid::GetRootElement( int x, int z )
{
	if ( x < 0 ) x = 0;
	if ( z < 0 ) z = 0;
	if ( x > 99 ) x = 99;
	if ( z > 99 ) z = 99;

	return m_pGrid [ x ][ z ];
}