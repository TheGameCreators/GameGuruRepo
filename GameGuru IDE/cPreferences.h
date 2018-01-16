#if !defined(AFX_CPREFERENCES_H__E6143A84_2CDF_4F9E_8451_10E702A3F70A__INCLUDED_)
#define AFX_CPREFERENCES_H__E6143A84_2CDF_4F9E_8451_10E702A3F70A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cPreferences.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// cPreferences dialog

#include "cMessages.h"
#include "cControls.h"

class cPreferences : public CDialog, cBaseMessage
{
// Construction
public:
	cPreferences(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cPreferences)
	enum { IDD = IDD_PREFERENCES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cPreferences)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	cMessages			m_Message;
	cIPC*				m_pIPC;
	cControls			m_Controls;

	bool				m_bOK;
	bool				m_bCancel;

	bool				m_bStart;
	DWORD				m_dwHackKeepPref;

	void HandleMessages ( void );

	// Options in preferences page
	CButton				m_VSYNC;

	// Generated message map functions
	//{{AFX_MSG(cPreferences)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnVSYNC();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// OK & Cancel
	CButton			m_Buttons [ 2 ];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPREFERENCES_H__E6143A84_2CDF_4F9E_8451_10E702A3F70A__INCLUDED_)
