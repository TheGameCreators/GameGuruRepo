#pragma once
#include "afxwin.h"
#include "resource.h"
//#include "HyperlinkLabel.h"

// CLoginDialog dialog

class CMarketWindow;

class CLoginDialog2 : public CDialog
{
	DECLARE_DYNAMIC(CLoginDialog2)

public:
	CLoginDialog2(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDialog2();

	void SetUsername( const char* name );
	void SetPassword( const char* name );
	const char* GetUsername( ) { return szUsername; }
	const char* GetPassword( ) { return szPassword; }
	bool Completed() { return m_bOK; }

	virtual BOOL OnInitDialog( );

// Dialog Data
	enum { IDD = IDD_LOGINDIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	char *szUsername;
	char *szPassword;
	CEdit m_edtUsername;
	CEdit m_edtPassword;
	bool m_bOK;
	
	//CHyperlinkLabel m_txtRegister;
	CStatic m_txtUsername;
	CStatic m_txtPassword;
	CStatic m_txtStatement;

	CEdit m_Lines [ 10 ];
	CStatic m_Button [ 2 ];

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
};
