//
// 3DCol.h: interface for the C3DCollision class.
//

#include <windows.h>
#include "directx-macros.h"

#define EPSILON 1.0e-5f
#define FABS _fabs

// floating absolute value
// the fabs C runtime uses doubles
inline float _fabs(float n) {
	return (n < 0.0f ? -n : n);
}

class C3DCollision
{
	public:
		C3DCollision();
		~C3DCollision();

		// Main Checks
		int TestBoxInBox(GGVECTOR3 box1p, GGVECTOR3 box1s, GGMATRIX box1r, GGVECTOR3 box2p, GGVECTOR3 box2s, GGMATRIX box2r);

		// Core Tests
		int intersect_boxbox(GGVECTOR3& vBox1Position, GGVECTOR3& vBox1Size, GGVECTOR3* pvBox1Basis, GGVECTOR3& vBox2Position, GGVECTOR3& vBox2Size, GGVECTOR3* pvBox2Basis);
};
