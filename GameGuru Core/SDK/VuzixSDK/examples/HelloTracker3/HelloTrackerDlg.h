// HelloTrackerDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CHelloTrackerDlg dialog
class CHelloTrackerDlg : public CDialog
{
// Construction
public:
	CHelloTrackerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HELLOTRACKER2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_csYaw;
	CString m_csPitch;
	CString m_csRoll;
	BOOL m_fTrackerPresent;
protected:
	virtual void OnCancel();
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButtonZero();
	void RestartPoll(void);
	HMODULE m_hIwear;
    CString m_csMagX;
    CString m_csMagY;
    CString m_csMagZ;
    CString m_csAccX;
    CString m_csAccY;
    CString m_csAccZ;
    CString m_csGX;
    CString m_csGY;
    CString m_csGZ;
    CString m_csLBGX;
    CString m_csLBGY;
    CString m_csLBGZ;
    CString m_csXTRN;
    CString m_csYTRN;
    CString m_csZTRN;
    CString m_csFLBGX;
    CString m_csFLBGY;
    CString m_csFLBGZ;
    CString m_csFGX;
    CString m_csFGY;
    CString m_csFGZ;
    CString m_csFAX;
    CString m_csFAY;
    CString m_csFAZ;
    CString m_csFMX;
    CString m_csFMY;
    CString m_csFMZ;
    CString m_csStatus;
    CButton m_btnGetVersion;
    afx_msg void OnBnClickedBtnGetversion();
    BOOL m_bAutoCorrect;
    CString m_csMagYaw;
    afx_msg void OnBnClickedChkAutocorrect();
};
