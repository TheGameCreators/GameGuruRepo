#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "PreviewRect.h"
#include "ProgressSpinner.h"
#include "UploadThread2.h"
#include "afxwin.h"

// Paul, you don't need to keep adding this, just add the path in the project config include section
//#include "c:\program files\bcgsoft\bcg (for fpsc editor)\bcg (for fpsc editor)\vs8 atlmfc\atlmfc\include\afxwin.h"

// CUploadGame dialog

struct sClub
{
	int iClubID;
	char szName[50];
	sClub *pNextClub;

	sClub() { pNextClub = NULL; iClubID = 0; szName[0] = '\0'; }
	~sClub() {}
};

class CUploadGame : public CDialog, public Thread
{
	DECLARE_DYNAMIC(CUploadGame)

public:
	CUploadGame(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUploadGame();

// Dialog Data
	enum { IDD = IDD_UPLOADGAME };

	BOOL OnInitDialog( );
	void SetMessage( LPCSTR szMsg );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	UploadThread2 m_UploadThread;
	bool m_bRunning;
	bool m_bUpdateMessage;
	CString m_sNewMessage;
	sClub *pClubList;
	int iChosenClub;

	void AddClub( int id, const char *name );
	void ClearClubList();
	int CountClubs();
	int FindClubID( const char *name );

	unsigned Run( );

public:
	bool IsRunning( ) { return m_bRunning; }
	void UpdateSpaceUsed();
	void UpdateClubsList();

	afx_msg void OnBrowseThumbnail();
	
	CPreviewRect m_ImageThumbnail;
	CEdit m_editDescription;
	CEdit m_editGameFolder;
	CEdit m_editStatusMessages;
	CString m_sThumbnailFile;
	
	CProgressCtrl m_ProgressBar;
	CProgressSpinner2 m_ProgressSpinner;

	afx_msg void OnBrowseGameFolder();
	afx_msg void OnUploadClicked();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	CButton m_btnUpload;
	CPreviewRect m_ImageUpload;
	afx_msg void OnChangeLoginClicked();
	afx_msg void OnBnClickedCancel();
	CStatic m_textSpaceUsed;
	CComboBox m_comboUploadTo;
	CStatic m_textUploadTo;
	afx_msg void OnChooseClub();
};
