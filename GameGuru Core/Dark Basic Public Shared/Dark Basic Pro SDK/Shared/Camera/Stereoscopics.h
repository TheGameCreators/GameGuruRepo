//
// STEREOSCOPICS HEADER
//

// Includes
//#include ".\..\data\cdatac.h"
#include "CCameraDatac.h"

// Prototypes
void StereoscopicConvertLeftRightToBackFront ( int iMode, LPGGTEXTURE pLeft, LPGGTEXTURE pRight, LPGGSURFACE pBack, LPGGSURFACE pFront );
void StereoscopicConvertLeftToGreySideBySide ( int iMode, LPGGTEXTURE pLeft, LPGGSURFACE pGrey, LPGGSURFACE pSideBySide );
void FreeStereoscopicResources ( void );