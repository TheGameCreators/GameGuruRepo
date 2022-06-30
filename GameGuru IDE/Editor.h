// Editor.h : main header file for the EDITOR application
//

#define _CRT_SECURE_NO_DEPRECATE

#if !defined(AFX_EDITOR_H__9200D31C_755C_4645_B4AB_2AB08F00AB12__INCLUDED_)
#define AFX_EDITOR_H__9200D31C_755C_4645_B4AB_2AB08F00AB12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "MainFrm.h"
#include "ChildFrm.h"
#include "EditorDoc.h"
#include "EditorView.h"

#include "Splash.h"
#include "resource.h"       // main symbols
#include "about.h"
#include <wininet.h>
#include "HashTree.h"

//#include "cPopup.h"

// External to global to control interface use
extern bool		g_bStopInterfaceOperations;
extern bool		g_bDisablePrefabMode;
extern bool		g_bDisableStoreMode;


/////////////////////////////////////////////////////////////////////////////
// CEditorApp:
// See Editor.cpp for the implementation of this class
//

//#define WM_USER_THREAD_FINISHED_TEST WM_USER+0x100

//#define WM_MIKE WM_USER+0x101

#define WM_MYMESSAGE (WM_USER + 1)
#define WM_ENTITY_WINDOW (WM_USER + 2)
#define WM_CLOSE_LEVEL_WINDOW (WM_USER + 3)

#include "cDebug.h"

#include "cMessages.h"

#include "IPC.h"

class CHelpWizard;	//HELPW - for pointer def
class UpdateCheckThread;	//AutoUpdate

typedef void	( *CreateFileMapPFN		) ( char*, DWORD );
typedef void	( *DestroyFileMapPFN	) ( char* );
typedef BYTE*	( *GetFileMapDataPFN	) ( char* );

class CEditorApp : public CWinApp, public CBCGWorkspace, cBaseMessage
{
	public:
		char				m_CommandLineString[_MAX_PATH];

		CHelpWizard*		m_pHelpWizardDlg;	//HELPW - the help wizard dialog
		bool				m_bHelpWizardVisible;

		HashTree			m_TreeGroups;

		UpdateCheckThread*	m_pUpdateThread; //AutoUpdate

		HANDLE m_hFileMap;
		LPVOID m_lpVoid;
		BYTE*  m_pData;
		HANDLE m_hOnlyOneEditorMutex;

		CEditorDoc* m_pDocumentA;
		bool		m_bFocus;

		bool		m_bEntityFocus;

		bool		m_bRestart;

		bool m_bAppActive;

		CEditorView* m_pEditorView;

		CString	m_szProjectName;

		HINSTANCE m_hInstance;

		cDebug		m_Debug;

		POINT m_lastPoint;

		bool			m_bPopupDisplay;
		bool			m_bBuildGameDisplay;
		bool			m_bBrowserDisplay;

		bool				m_bSplashScreen;

		int					m_iRecover;
		DWORD				m_dwRecoverTimer;
		//bool				m_bGameOrBuildInProgress;
		DWORD				m_dwCheckProcessesTimer;
		bool				m_bAppHasStartedRunning;
		bool				m_bDoNotNeedToCheckForCrashesAnymore;

		bool				m_DBProApplicationLaunched;
		SHELLEXECUTEINFO	m_DBProApplication;

		// 020205 - add for process
		PROCESS_INFORMATION	m_DBProProcess;
		
		bool				m_bTestGameCancel;

		// Common Upload/Download Vars
		HINTERNET m_hInet;
		bool m_bSavePassword;
		bool m_bSaveUsername;
		CString m_sUsername;
		CString m_sPassword;

		// Upload Game Variables
		char *m_szSESID;
		char *m_szUploadKey;
		char *m_szCredits;
		char m_szOrderBy [ 32 ];
		char m_szAuthorName [ 30 ];
		bool m_bAdmin;
		bool m_bUploader;
		bool m_bAgreedTerms;
		bool m_bGotoAddons;
		CUploadGame m_UploadForm;
		void OpenUploadGame( );

		// Root Folder of FPSCR
		char m_pRootDir [ _MAX_PATH ];

		// Download My Store Items Variables
		CDownloadStoreItems m_DSIForm;
		void OpenDownloadStoreItems();

		// Auto update and critical section vars
		char m_szVersion [ 64 ];
		CRITICAL_SECTION m_csDirectoryChanges;

		// Global to hold window style before go full screen with Test Level
		DWORD dwRestoreGUIWindowMode;

		// Moved to global so can close it externally
		//cPopup pop;

		void SetRegistryValue( const char *szLocation, const char *szKey, const char *szValue );
		void SetRegistryValue( const char *szLocation, const char *szKey, DWORD dwValue );
		bool GetRegistryValue( const char *szLocation, const char *szKey, char *szValue, DWORD dwSize, HKEY hRoot = HKEY_CURRENT_USER );
		bool GetRegistryValue( const char *szLocation, const char *szKey, DWORD *dwValue );
		bool DeleteRegistryValue( const char *szLocation, const char *szKey );

		char* Encrypt( const char *szString );
		char* Decrypt( const char *szString );

		bool GetDefaultBrowserPath( char *szValue, DWORD dwSize );

		void OpenStore( bool bGotoAddons = false );

		void UpdateMouse           ( void );
		void UpdateToolBar         ( void );
		void UpdateIDEForegroundFocus ( int iForeFocusFlag );
		void UpdateStatusBar       ( void );
		void UpdateLibraryWindow   ( void );
		void UpdateBrowserWindow   ( void );
		void UpdateFileOpenAndSave ( void );
		void UpdateMessageBoxes    ( void );
		void UpdatePopup           ( void );
		void UpdateTermination     ( void );
		void UpdateRestart         ( void );
		void UpdateEntityWindow    ( void );
		void UpdateBuildGame       ( void );
		void UpdatePreferences     ( void );
		void RestoreIDEEditorView  ( void );
		void UpdateTestGame        ( void );
		void UpdateFPIWizard       ( void );
		void UpdateProject         ( void );
		void UpdateQuit			   ( void );

		void CheckForDBProApplications ( void );

		bool IsProcessRunning ( char* szProcess );

		void LoadLanguage ( void );
		void LoadSettings ( void );

		bool CheckForMultipleInstances ( void );

		void	HandleMessages ( void );
		cMessages			m_Message;

		//bool	IsSerialCodeValid ( LPSTR pTryCodeOrNULL );
		//void	CheckSerialCodeValidity ( void );
		//DWORD	X9GetTodaysDays ( void );
		//bool	NetBookAuthentication ( LPSTR pSerialCodeStringToCheck );
		//bool	FPGCSerialCodeCheck ( LPSTR pSerialCodeStringToCheck );
		//bool	X9SerialCodeCheck ( LPSTR pSerialCodeStringToCheck );
		//bool	X10SerialCodeCheck ( LPSTR pSerialCodeStringToCheck );

		bool IsParentalControlActive ( void );

	public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		CEditorApp();
		~CEditorApp();

		// Override from CBCGWorkspace
		virtual void PreLoadState ();

		void OnFileOpen();
		void OnFileNewType( int );
		void OnFileNew();
		void OnFileNewFlat();

		CMainFrame* pMainFrame;
		
		bool	m_bAppRunning;
		bool	m_bSetupFrame;
		bool	m_bInvalidate;

		bool	m_bDebugTrace;
		bool	m_bDebugProcess;
		bool	m_bDebugActivation;

		BOOL		m_bEnd;
		BOOL	m_bDisplayMouse;
		BOOL	m_bDisplayMouseInQuickPreview;

		bool	m_bDisable;

		BOOL			m_bTest [ 10 ];

		TCHAR m_szLanguage [ MAX_PATH ];
		TCHAR m_szLanguageVariant [ MAX_PATH ];
		TCHAR m_szLanguageName [ 40 ];
		TCHAR m_szLanguagePath [ MAX_PATH ];
		TCHAR m_szDirectory [ MAX_PATH ];
		TCHAR m_szErrorString [ 64 ];
		TCHAR m_szUserPath [ MAX_PATH ];

		HANDLE m_hThread;

		bool m_bFileNewValid;
		void ClearFileMap ( void );
		void SetFileMapData ( int iOffset, DWORD dwData );
		void SetFileMapDataString ( int iOffset, LPTSTR pString );
		DWORD GetFileMapData ( int iOffset );
		void GetFileMapDataString ( int iOffset , LPSTR string );

		static UINT CheckMemoryStatus ( LPVOID pParameter );
		void CheckMemoryStatus ( void );

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CEditorApp)
	public:
		virtual BOOL InitInstance();
		virtual int ExitInstance();
		virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual int Run();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

	// Implementation
		//{{AFX_MSG(CEditorApp)

	afx_msg void OnAppEditorKeyboardShortcuts();
	afx_msg void OnAppGameKeyboardControlsShortcuts();
	afx_msg void OnAppGameVRControlsShortcuts();
	afx_msg void OnAppDesktopPlay();
	afx_msg void OnAppSoloVRPlay();
	afx_msg void OnAppSocialVRPlay();
	/*
	afx_msg void OnAppObjectsAddNewEntity();
	afx_msg void OnAppObjectsAddNewSite();
	afx_msg void OnAppObjectsPlayerStarter();
	afx_msg void OnAppObjectsMultiplayerStart();
	afx_msg void OnAppObjectsCreateNewWayPoint();
	*/
	afx_msg void OnInteractiveTutorial();
	afx_msg void OnAppAbout();
	afx_msg void OnAppAboutThisSomething();
	afx_msg void OnAppExit1();
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnEditCopy1();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditCut1();
	afx_msg void OnEditPaste();
	afx_msg void OnEditPaste1();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnEditClear();
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBuildGame(CCmdUI* pCmdUI);
	afx_msg void OnHelpWebsite();
	afx_msg void OnTutorials(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileNewFlat(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAsEx(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePreferences(CCmdUI* pCmdUI);
	afx_msg void OnDownloadStoreItems(CCmdUI* pCmdUI);
	afx_msg void OnImportModel(CCmdUI* pCmdUI);
	afx_msg void OnCharacterCreator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile4(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile5(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile6(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile7(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile8(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile9(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile10(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile11(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile12(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile13(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile14(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMruFile15(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewToolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewOutput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPrefab(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDraw(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewWaypoint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewTestGame(CCmdUI* pCmdUI);

	afx_msg void OnTimer( UINT_PTR nIDEvent );	//AutoUpdate

	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

			
};


extern CEditorApp theApp;

void ConvertWideToANSI ( BYTE* pData, CString* pUNI, char* pANSI );
void ConvertWideToANSI ( BYTE* pData, CString* pUNI );

void ConvertANSIToWide ( BYTE* pData, char* pANSI, CString* pUNI );

void ConvertANSIToWide ( char* pANSI, CString* pUNI );
CString GetLanguageData ( CString group, CString key );

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITOR_H__9200D31C_755C_4645_B4AB_2AB08F00AB12__INCLUDED_)
