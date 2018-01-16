//
// Collision Functions Header
//

// Define
#ifndef _COLLISION_H_
#define _COLLISION_H_

// Includes
#include "directx-macros.h"
#include <basetsd.h>
#include <stdio.h>
#include <math.h>
#include <vector>
using namespace std;
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include "col_global.h"
#include "..\\Universe.h"

// Class
class Collision 
{
	public:
		void	Init	( void );
		BOOL	World	( GGVECTOR3 o_pos, GGVECTOR3 n_pos, GGVECTOR3 eRadius, float fScale );
};

#endif
