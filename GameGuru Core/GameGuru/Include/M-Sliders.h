//----------------------------------------------------
//--- GAMEGURU - M-Sliders
//----------------------------------------------------

#include "cstr.h"

void sliders_init ( void );
void sliders_free ( void );
void sliders_loop ( void );
void sliders_readall ( void );
void sliders_draw ( void );
void sliders_getchoice ( void );
void sliders_getnamefromvalue ( void );
void sliders_write( bool bOnlyVisualSettings = false );
void sliders_scope_draw ( void );
float SlidersAdjustValue ( float value_f, float minFrom_f, float maxFrom_f, float minTo_f, float maxTo_f );
float SlidersCutExtendedValues(float value);