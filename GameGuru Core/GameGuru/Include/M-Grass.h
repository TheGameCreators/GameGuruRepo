//----------------------------------------------------
//--- GAMEGURU - M-Grass
//----------------------------------------------------

#include "cstr.h"

void grass_init ( void );
void grass_initstyles(void);
void grass_initstyles_reset(void);
void grass_assignnewshader ( void );
void grass_applyshader ( void );
void grass_setgrassimage ( void );
void grass_setgrassgridandfade ( void );
void grass_loop ( void );
void grass_clearregion ( void );
void grass_updatedirtyregionfast ( void );
void grass_clamptomemblockres ( void );
void grass_updategrassfrombitmap ( void );
void grass_loadgrass ( void );
void grass_savegrass ( void );
void grass_buildblankgrass ( void );
void grass_buildblankgrass_fornew ( void );
void grass_free ( void );
void grass_changevegstyle(void);

void grass_editcontrol(void);
void grass_paint(void);
void grass_resetchoices(void);
