
#include "Polygon.h"

using namespace std;

Polygon2D::Polygon2D( )
{
    sVertex_list.clear();
	bHalfHeight = false;
	id = 0;
	bDiveOver = false;
	bTerrain = false;
}

Polygon2D::~Polygon2D()
{
    sVertex_list.clear ( );
}

int Polygon2D::CountVertices ( ) { return (int) sVertex_list.size ( ); }
void Polygon2D::Clear ( ) { sVertex_list.clear ( ); bHalfHeight = false; }

void Polygon2D::AddVertex ( float fX, float fY )
{
    sVertex pVertex;

    pVertex.x = fX; pVertex.y = fY;
    
    sVertex_list.push_back ( pVertex );
}

void Polygon2D::AddVertex ( sVertex* pVertex )
{
    sVertex_list.push_back ( *pVertex );
}

sVertex Polygon2D::GetVertex ( int iIndex )
{
    if ( iIndex < 0 || iIndex >= (int) sVertex_list.size ( ) ) return sVertex();
    
    return sVertex_list [ iIndex ];
}

void Polygon2D::Draw ( )
{
    /*
	MoveToEx( hBackBuffer, sVertex_list [ 0 ].x, sVertex_list [ 0 ].y, NULL );
    
    for ( int i = 1; i<sVertex_list.size( ); i++ )
        LineTo( hBackBuffer, sVertex_list [ i ].x, sVertex_list [ i ].y );
    
    LineTo( hBackBuffer, sVertex_list [ 0 ].x, sVertex_list [ 0 ].y );
	*/
}


