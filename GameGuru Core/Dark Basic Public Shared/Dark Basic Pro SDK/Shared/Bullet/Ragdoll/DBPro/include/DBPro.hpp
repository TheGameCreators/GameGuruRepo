#ifndef INCLUDE_GUARD_DBPro_hpp
#define INCLUDE_GUARD_DBPro_hpp

// Version of DBPro that this interface is based upon
#define DBPRO__INTERFACE_VERSION 1074

#ifdef DBPRO__COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

// Load the DBPro Globstruct and Object structures
#include "Globstruct.h"
#include ".\..\..\..\..\Camera\ccameradatac.h"
#include ".\..\..\..\..\DBOFormat\DBOData.h"
#include ".\..\..\..\..\DBOFormat\DBOFile.h"
#include ".\..\..\..\..\DBOFormat\DBOBlock.h"
#include "CObjectsC.h"

namespace DBPro
{
	int FindFreeObject ( void );
	int FindFreeMesh ( void );
}

#endif

