#pragma once

#include "..\resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "..\ProgressSpinner.h"
#include "afxwin.h"
#include <vector>
#include "Wininet.h"

// CDownloadStoreItems dialog

void DownloadStoreItemsLog( char* s, char* s2);

struct sDownloadItem
{
  char fileName [ MAX_PATH ];
};

struct sStoreDownloadItem
{
 char name [ MAX_PATH ];
 char url [ MAX_PATH ];
 char checksum [ MAX_PATH ];
 char id [ MAX_PATH ];
 std::vector < sDownloadItem > files;
};

class CDownloadStoreItems : public CDialog
{
	DECLARE_DYNAMIC(CDownloadStoreItems)

public:
	CDownloadStoreItems(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDownloadStoreItems();

	enum { IDD = IDD_DOWNLOADSTOREITEMS };

	BOOL OnInitDialog( );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
	bool UpdateList();

public:

	afx_msg void OnBrowseThumbnail();
	
	CEdit m_editStatus;
	CProgressCtrl m_ProgressBar;
	CProgressSpinner2 m_ProgressSpinner;
	CEdit m_Lines [ 10 ];

	char	m_szUserName[256];
	char	m_szPassword[256];
	char	m_szUserID[32];
	bool	m_bIsLoggedIn;
	bool	m_bFirstTimeLogin;
	int		m_NumberOfFilesToDownload;
	bool	m_busyCheckingList;
	int		m_TimerMode;
	bool	m_callDownloadFiles;

	UINT_PTR	m_pTimer;

	HINTERNET	m_hInternet;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRequest;
	float		m_ProgressAmount;	
	float		m_FileProgress;
	char		m_listURL[MAX_PATH];
	int			m_storeCount;
	FILE*		m_file;
	DWORD		m_FileLength;
	char		m_fileToExtract[ MAX_PATH ];
	int			m_fileCountLoop;
	char		m_fileName [ MAX_PATH ];

	std::vector < sStoreDownloadItem > m_StoreItems;

	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnUploadClicked();
	afx_msg void OnChangeLoginClicked();
	afx_msg void OnBnClickedCancel();

	bool CDownloadStoreItems::UserLogin();
};
