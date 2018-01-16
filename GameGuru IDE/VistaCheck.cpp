#include "Userenv.h"

BOOL Is_Win_Vista_Or_Later () 
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;
   osvi.dwMinorVersion = 0;
   osvi.wServicePackMajor = 0;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi, 
      VER_MAJORVERSION | VER_MINORVERSION | 
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

typedef HRESULT (CALLBACK * P_DwmIsCompositionEnabled)(BOOL *pfEnabled);
typedef HRESULT (CALLBACK * P_DwmEnableComposition)   (BOOL   fEnable);

struct DWMAPILib 
{

HMODULE lib;

P_DwmIsCompositionEnabled fIsEnabled;

P_DwmEnableComposition fEnable;

DWMAPILib() : lib((HMODULE)0xFFFFFFFF), fIsEnabled(NULL), fEnable(NULL) {}

~DWMAPILib()

{

if (lib != NULL && lib != (HMODULE)0xFFFFFFFF)

::FreeLibrary(lib);

lib = (HMODULE)0xFFFFFFFF;

}

BOOL Load()

{

if (lib == NULL) return FALSE;

lib = ::LoadLibrary("dwmapi.dll");

if (lib == NULL) return FALSE;

fIsEnabled = (P_DwmIsCompositionEnabled)::GetProcAddress(lib, "DwmIsCompositionEnabled");

fEnable    = (P_DwmEnableComposition)   ::GetProcAddress(lib, "DwmEnableComposition");

return TRUE;

}

bool IsCompositionEnabled()

{

BOOL enabled = FALSE;

return (fIsEnabled != NULL && SUCCEEDED(fIsEnabled(&enabled)) && enabled);

}

HRESULT EnableComposition(BOOL enable)

{

if (!fIsEnabled) return 0x80070000 + ERROR_PROC_NOT_FOUND;  // @@@efh really should get from GetLastError / ERROR_MOD_NOT_FOUND if lib == NULL

return fEnable(enable);

}

};

static DWMAPILib dwmapiLib;

static BOOL CompositionWasEnabled = false;

void DisableComposition()

{

if (!dwmapiLib.Load()) return;

if ((CompositionWasEnabled = dwmapiLib.IsCompositionEnabled()) == true)

dwmapiLib.EnableComposition(FALSE); // DWM_EC_DISABLECOMPOSITION

}

void RestoreComposition()

{

if (CompositionWasEnabled && dwmapiLib.Load())

dwmapiLib.EnableComposition(TRUE); // DWM_EC_ENABLECOMPOSITION

}