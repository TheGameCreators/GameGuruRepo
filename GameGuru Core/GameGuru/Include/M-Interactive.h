//----------------------------------------------------
//--- GAMEGURU - M-Interactive
//----------------------------------------------------

#include "cstr.h"

void interactive_init ( void );
void interactive_loop ( void );
void interactive_restartalpha ( void );
void interactive_welcome ( void );
void interactive_picktemplate ( void );
void interactive_entitywork ( void );
void interactive_summary ( void );
void interactivedrawonly ( int x, int y, int img, int noalpha );
void interactivedrawimgalpha ( int x, int y, int img, int itemindex, int highlightable, int noalpha );
void interactivedrawimg ( int x, int y, int img, int itemindex, int highlightable );
