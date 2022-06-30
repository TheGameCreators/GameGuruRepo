//----------------------------------------------------
//--- GAMEGURU - M-CharacterCreatorPlusTTS
//----------------------------------------------------

// Includes
#include <stdio.h>
#include <string.h>
#include <atlbase.h>
#include <sapi.h>
#include <comip.h>
#include <spuihelp.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cstdlib>

// Externs
extern std::vector <cstr> g_voiceList_s;
extern std::vector <ISpObjectToken *> g_voicetoken;

// COM Class
class CCoInitialize
{
	public:
		CCoInitialize() : hr(CoInitialize(NULL)) { }
		~CCoInitialize() 
		{
			if (SUCCEEDED(hr))
				CoUninitialize();
		}
		operator HRESULT () const {
			return hr;
		}
	private:
		HRESULT hr;
};

// Functions Exposed

int CreateListOfVoices ( void );
int ConvertTXTtoWAV ( char* voice, int iSpeakRate, char* textFile, char* wavFile );
int ConvertTXTtoWAVMeatyPart ( CComPtr<ISpVoice> spVoice, ISpObjectToken* voiceItsecond, int iSpeakRate, char* textFile, char* wavFile );
int ConvertWAVtoLIP ( char* pAbsWAVFile );
float GetWAVtoLIPProgress(void);
int RecordWAV(char* pAbsWAVFile);
float RecordWAVProgress(void);
