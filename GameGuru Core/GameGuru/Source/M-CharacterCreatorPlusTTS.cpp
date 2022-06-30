//----------------------------------------------------
//--- GAMEGURU - M-CharacterCreatorPlusTTS
//----------------------------------------------------

// Includes
#include "stdafx.h"
#include "gameguru.h"
#include "M-CharacterCreatorPlusTTS.h"
#include <vector>
#include "shellapi.h"

wchar_t *CharToWchar(const char *str);
long FileSize(const char *file_name);
HRESULT EnumerateVoices();
void ReleaseVoices();
HRESULT FileToWav(CComPtr<ISpVoice> spVoice, ISpObjectToken *voice, int iSpeakRate, const char *textFile, const char *wavFile);
typedef std::map<std::string, ISpObjectToken *> VoiceMap_t;
VoiceMap_t VoiceMap;
std::vector <cstr> g_voiceList_s;
std::vector <ISpObjectToken *> g_voicetoken; // for quick access.

int CreateListOfVoices ( void )
{
	// initialise COM
	HRESULT hr;
    CCoInitialize init;
	if (FAILED(init)) {

		if (init == RPC_E_CHANGED_MODE) {
			//We get this when using Photon , set in MPAudio.cpp, StartVoiceChat() - > CoInitializeEx( NULL, COINIT_MULTITHREADED );
			//No need to log it we will now switch back to multi threaded.
			//timestampactivity(0, "CoInitialize changed from multi-threaded to single-threaded mode.");
		}
		else if (init == E_UNEXPECTED) {
			timestampactivity(0, "CoInitialize failed E_UNEXPECTED.");
		}
		else {
			timestampactivity(0, "CoInitialize failed - re-initializing.");
		}
		//Try re-initializes the COM library
		CoUninitialize();
		hr = CoInitialize(NULL);
		if( !(hr == S_OK || hr == S_FALSE) ) {
			return 0;
		}
	}

	// create SAPI voice
    CComPtr<ISpVoice> spVoice;
    hr = spVoice.CoCreateInstance(CLSID_SpVoice);
    if (FAILED(hr)) 
        return 0;

	if(g_voiceList_s.size() > 0 )
		ReleaseVoices();

	// enumerate all voices available
    hr = EnumerateVoices();
    if (FAILED(hr)) 
        return 0;

	// list all voices available
    VoiceMap_t::const_iterator begin = VoiceMap.begin();
    VoiceMap_t::const_iterator end = VoiceMap.end();
    VoiceMap_t::const_iterator it;
	g_voiceList_s.clear();
	g_voicetoken.clear();
	for (it = begin; it != end; it++)
	{
		cstr pVoiceName = (LPSTR)it->first.c_str();
		g_voiceList_s.push_back ( pVoiceName );
		g_voicetoken.push_back(it->second);
	}
    //Dont release it here , we need the token.
	//ReleaseVoices();

	// success
	CoUninitialize();
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
    return g_voiceList_s.size();
}

int ConvertTXTtoWAVMeatyPart(CComPtr<ISpVoice> spVoice, ISpObjectToken* voiceItsecond, int iSpeakRate, char* textFile, char* wavFile)
{
	// ensure we have text and wav filename
	if ( strlen(textFile) == 0 || strlen(wavFile) == 0 )
        return EXIT_FAILURE;
	
	// find the required voice (if specified)
	CSpDynamicString spDesc;
	HRESULT hr = SpGetDescription(voiceItsecond, &spDesc);
    hr = FileToWav(spVoice, voiceItsecond, iSpeakRate, textFile, wavFile);

	return 0;
}

int ConvertTXTtoWAV ( char* voice, int iSpeakRate, char* textFile, char* wavFile )
{
	// initialise COM
    CCoInitialize init;
	if (FAILED(init)) {
		//Try re-initializes the COM library
		CoUninitialize();
		HRESULT hr = CoInitialize(NULL);
		if (!(hr == S_OK || hr == S_FALSE)) {
			return EXIT_FAILURE;
		}
	}

	// create SAPI voice
    CComPtr<ISpVoice> spVoice;
    HRESULT hr = spVoice.CoCreateInstance(CLSID_SpVoice);
    if (FAILED(hr)) 
        return EXIT_FAILURE;

	// enumerate all voices available
    hr = EnumerateVoices();
    if (FAILED(hr)) 
        return EXIT_FAILURE;

	// get voice object from map
    VoiceMap_t::const_iterator voiceIt = VoiceMap.begin();
	if ( strlen(voice) > 0 ) voiceIt = VoiceMap.find(voice);
	if (voiceIt != VoiceMap.end())
	{
		// meaty part of the conversion (can be used separately if already have voices set up)
		if (ConvertTXTtoWAVMeatyPart(spVoice, voiceIt->second, iSpeakRate, textFile, wavFile) == EXIT_FAILURE)
			return EXIT_FAILURE;
	}

	// free voices
    ReleaseVoices();

	// and finally generate LIP sync data
	cstr oldDir_s = GetDir();
	SetDir ( "charactercreatorplus\\voicearea\\" );
	char pCommandLine[1024];
	sprintf ( pCommandLine, "-o %s %s", "myrecording.lip", "myrecording.wav" );
	ShellExecuteA( NULL, "open", "rhubarb.exe" , pCommandLine, NULL, SW_HIDE );
	SetDir ( oldDir_s.Get() );

	CoUninitialize();
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// success or failure
    if (SUCCEEDED(hr))
        return EXIT_SUCCESS;
	else
	    return EXIT_FAILURE;
}

HRESULT FileToWav(CComPtr<ISpVoice> spVoice, ISpObjectToken *voice, int iSpeakRate, const char *textFile, const char *wavFile)
{
    CComPtr <ISpStream> spStream;
    CSpStreamFormat spAudioFmt;

    HRESULT hr = spVoice->SetVoice(voice);
    if (FAILED(hr)) 
	{
        //printf("Failed setting voice.\n");
        return hr;
    }

    hr = spAudioFmt.AssignFormat(SPSF_48kHz16BitStereo);
    if (FAILED(hr)) 
	{
        //printf("Failed setting audio format.\n");
        return hr;
    }
   
    hr = SPBindToFile(wavFile, SPFM_CREATE_ALWAYS, &spStream, &spAudioFmt.FormatId(), spAudioFmt.WaveFormatExPtr());
    if (FAILED(hr)) 
	{
        //printf("Failed binding to wav file.\n");
        return hr;
    }
	
    hr = spVoice->SetOutput(spStream, TRUE);
    if (FAILED(hr)) 
	{
        //printf("Failed setting output stream.\n");
        return hr;
    }
	
	/* not a text file, text is passed in directly
    long fileSize = FileSize(textFile);
    if (fileSize < 0) 
	{
        //printf("Failed getting file size for file %s.\n", textFile);
        return -1;
    }

    wchar_t *fileContents = (wchar_t *)calloc(sizeof(wchar_t), fileSize + 1);
    if (!fileContents) 
	{
        //printf("Failed allocating memory for file %s.\n", textFile);
        return -1;
    }

    wchar_t *textFileW = CharToWchar(textFile);
    FILE *in = _wfopen(textFileW, L"rt, ccs=UTF-8");
    if (!in) 
	{
        //printf("Failed opening %s for input.\n", textFile);
        SysFreeString(textFileW);
        free(fileContents);
        return -1;
    }
    SysFreeString(textFileW);
    size_t bytes = fread(fileContents, sizeof(wchar_t), fileSize, in);
    fclose(in);
	*/
	//long fileSize = strlen(textFile);
	//wchar_t *fileContents = (wchar_t *)calloc(sizeof(wchar_t), fileSize + 1);
    //free(fileContents);

	// a range of -10 to 10
	spVoice->SetRate ( iSpeakRate );

	// speak audio to the file stream
	wchar_t* fileContents = CharToWchar(textFile);
    hr = spVoice->Speak(fileContents, SPF_IS_XML, NULL);

    if (FAILED(hr)) 
	{
        //printf("Failed speaking the contents of the file.\n");
        return hr;
    }
    spStream->Close();

    return 0;
}

wchar_t * CharToWchar(const char *str) 
{
    int lenW, lenA = lstrlenA(str);
    wchar_t *unicodestr;
    lenW = MultiByteToWideChar(CP_ACP, 0, str, lenA, NULL, 0);
    if (lenW > 0) 
	{
        unicodestr = SysAllocStringLen(0, lenW);
        MultiByteToWideChar(CP_ACP, 0, str, lenA, unicodestr, lenW);
        return unicodestr;
    }
    return NULL;
}

long FileSize(const char *file_name)
{
    BOOL ok;
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;

    if (!file_name)
        return -1;

    ok = GetFileAttributesExA(file_name, GetFileExInfoStandard, (void*)&fileInfo);
    if (!ok)
        return -1;

    return (long)fileInfo.nFileSizeLow;
}

HRESULT EnumerateVoices()
{
    ISpObjectToken *spToken;
    CComPtr<IEnumSpObjectTokens> spEnum;
	
	//PE: If called multiply times , we have old released tokens and it crash , so clear list everytime.
	VoiceMap.clear();

    HRESULT hrTokens = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &spEnum);
    if (SUCCEEDED(hrTokens)) 
	{
        while (spEnum->Next(1, &spToken, NULL) == S_OK) 
		{
            CSpDynamicString spDesc;
            HRESULT hr = SpGetDescription(spToken, &spDesc);
            if (SUCCEEDED(hr)) 
			{
                char *name = spDesc.CopyToChar();
                VoiceMap.insert(std::make_pair(name, spToken));
                CoTaskMemFree(name);
            }
            else 
			{
                spToken->Release();
            }
        }
    }
    return hrTokens;
}

void ReleaseVoices()
{
    VoiceMap_t::const_iterator begin = VoiceMap.begin();
    VoiceMap_t::const_iterator end = VoiceMap.end();
    VoiceMap_t::const_iterator it;
    for (it = begin; it != end; it++)
        it->second->Release();
}

// RHUBARB Converter

BOOL DB_ExecuteFile(HANDLE* phExecuteFileProcess, char* Operation, char* Filename, char* String, char* Path, bool bWaitForTermination);
HANDLE g_hConvertWAVtoLIPProcess = NULL;
bool g_bConvertWAVtoLIPInProgress = false;
char g_pConvertWAVtoLIPDest[MAX_PATH];

int ConvertWAVtoLIP ( char* pAbsWAVFile )
{
	// real abd of pAbsWAVFile
	char pRealAbsWAVFile[MAX_PATH];
	strcpy(pRealAbsWAVFile, pAbsWAVFile);
	GG_GetRealPath(pRealAbsWAVFile, 1);

	// get LIP file from WAV filename
	char pLIPFile[2048];
	strcpy(pLIPFile, pAbsWAVFile);
	pLIPFile[strlen(pLIPFile) - 4] = 0;
	strcat(pLIPFile, ".lip");

	// if LIP file not exist
	GG_GetRealPath(pLIPFile, 1);
	if (FileExist(pLIPFile) == 0)
	{
		// copy the WAV to the work area
		char pAbsWorkWAVFile[2048];
		strcpy(pAbsWorkWAVFile, g.fpscrootdir_s.Get());
		strcat(pAbsWorkWAVFile, "\\Files\\charactercreatorplus\\voicearea\\myrecording.wav");
		GG_GetRealPath(pAbsWorkWAVFile, 1);
		CopyFileA(pRealAbsWAVFile, pAbsWorkWAVFile, FALSE);

		// delete the old LIP work file
		char pAbsWorkLIPFile[2048];
		strcpy(pAbsWorkLIPFile, g.fpscrootdir_s.Get());
		strcat(pAbsWorkLIPFile, "\\Files\\charactercreatorplus\\voicearea\\myrecording.lip");
		GG_GetRealPath(pAbsWorkWAVFile, 1);
		DeleteFileA(pAbsWorkLIPFile);

		// do the generation 
		cstr oldDir_s = GetDir();
		char pRealVoiceArea[MAX_PATH];
		strcpy(pRealVoiceArea, g.fpscrootdir_s.Get());
		strcat(pRealVoiceArea, "\\Files\\charactercreatorplus\\voicearea\\");
		GG_GetRealPath(pRealVoiceArea, 1);
		SetDir(pRealVoiceArea);

		char pAbsRootForRhubarb[MAX_PATH];
		strcpy(pAbsRootForRhubarb, g.fpscrootdir_s.Get());
		strcat(pAbsRootForRhubarb, "\\Files\\charactercreatorplus\\voicearea\\rhubarb.exe");
		char pCommandLine[1024];
		sprintf(pCommandLine, "-o %s %s", "myrecording.lip", "myrecording.wav");
		if (!DB_ExecuteFile(&g_hConvertWAVtoLIPProcess, "hide", pAbsRootForRhubarb, pCommandLine, "", true))
		{
			SetDir(oldDir_s.Get());
			return -1;
		}
		SetDir(oldDir_s.Get());

		// proceeding with conversion
		strcpy(g_pConvertWAVtoLIPDest, pLIPFile);
		g_bConvertWAVtoLIPInProgress = true;
		return 1;
	}

	// no need to convert
	return 0;
}

float GetWAVtoLIPProgress ( void )
{
	if ( g_bConvertWAVtoLIPInProgress == true )
	{
		// now we wait until the conversion process is done (the exe is finished)
		if(g_hConvertWAVtoLIPProcess)
		{
			DWORD dwStatus;
			if(GetExitCodeProcess(g_hConvertWAVtoLIPProcess, &dwStatus)==TRUE)
			{
				if(dwStatus!=STILL_ACTIVE)
				{
					// Closes process after it deactivates
					CloseHandle(g_hConvertWAVtoLIPProcess);
					g_hConvertWAVtoLIPProcess=NULL;
				}
			}
			if(g_pGlob->ProcessMessageFunction()==1)
			{
				// Closes process if main app terminates
				CloseHandle(g_hConvertWAVtoLIPProcess);
				g_hConvertWAVtoLIPProcess=NULL;
			}
			if (g_hConvertWAVtoLIPProcess == NULL)
			{
				// we know the process has finished
				// copy the resulting LIP file to its final location alongside WAV file
				char pAbsWorkLIPFile[2048];
				strcpy(pAbsWorkLIPFile, g.fpscrootdir_s.Get());
				strcat(pAbsWorkLIPFile, "\\Files\\charactercreatorplus\\voicearea\\myrecording.lip");
				GG_GetRealPath(pAbsWorkLIPFile, 1);
				if (FileExist(pAbsWorkLIPFile) == 1)
					CopyFileA(pAbsWorkLIPFile, g_pConvertWAVtoLIPDest, FALSE);

				// successfully converted
				g_bConvertWAVtoLIPInProgress = false;
				strcpy(g_pConvertWAVtoLIPDest, "");
				return 1.0f;
			}
		}
		// still running
		// any way to get the TEXT output from a process in progress 
		// (i.e. get what the command line window shows as I can get progress from there)
		return 0.5f;
	}
	return 0;
}

// VOICE Recorder

HANDLE g_hRecordWAVProcess = NULL;
bool g_bRecordWAVInProgress = false;
char g_pRecordWAVDest[MAX_PATH];

int RecordWAV ( char* pAbsWAVFile )
{
	// do the recording
	cstr oldDir_s = GetDir();
	SetDir(g.fpscrootdir_s.Get());
	SetDir("Files\\charactercreatorplus\\voicearea\\");
	char pCommandLine[1024];
	sprintf(pCommandLine, "\"%s\"", pAbsWAVFile);
	if (!DB_ExecuteFile(&g_hRecordWAVProcess, "", "voicerecorder.exe", pCommandLine, "", true))
	{
		SetDir(oldDir_s.Get());
		return -1;
	}
	SetDir(oldDir_s.Get());

	// proceeding with recording
	g_bRecordWAVInProgress = true;
	return 1;
}

float RecordWAVProgress ( void )
{
	if ( g_bRecordWAVInProgress == true )
	{
		// now we wait until the recording process is done
		if(g_hRecordWAVProcess)
		{
			DWORD dwStatus;
			if(GetExitCodeProcess(g_hRecordWAVProcess, &dwStatus)==TRUE)
			{
				if(dwStatus!=STILL_ACTIVE)
				{
					// Closes process after it deactivates
					CloseHandle(g_hRecordWAVProcess);
					g_hRecordWAVProcess=NULL;
				}
			}
			if(g_pGlob->ProcessMessageFunction()==1)
			{
				// Closes process if main app terminates
				CloseHandle(g_hRecordWAVProcess);
				g_hRecordWAVProcess=NULL;
			}
			if (g_hRecordWAVProcess == NULL)
			{
				// successfully recorded and saved direct to abs WAV path destination
				g_bRecordWAVInProgress = false;
				return 1.0f;
			}
		}
		// still running
		return 0.5f;
	}
	return 0;
}
