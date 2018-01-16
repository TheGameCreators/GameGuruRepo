#pragma once
#include "afxwin.h"
#include "resource.h"
#include "PreviewRect.h"

// CPoster dialog

class CPoster : public CDialog
{
	DECLARE_DYNAMIC(CPoster)

public:
	CPoster(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPoster();

// Dialog Data
	enum { IDD = IDD_POSTER };

	BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CPreviewRect m_ChosenImage;
	CString m_sCurrentFile;
	
	CEdit m_editCurrentFile;
	CEdit m_editWidth;
	CEdit m_editHeight;
	CEdit m_editDepth;

	CButton m_btnCancel;
	CButton m_btnMake;
	CButton m_btnBrowse;

	CStatic m_TxtFile;
	CStatic m_txtWidth;
	CStatic m_txtHeight;
	CStatic m_txtDepth;
	CStatic m_txtSettingsBox;

	CButton m_RadioWall;
	CButton m_RadioFloor;
	CButton m_RadioCeiling;
	
	CPoint ptStart, ptEnd;
	bool m_bDragging;

	bool m_bInit;

	void MakeFPE( CString sName, int width, int height, int depth, int type );

public:
	afx_msg void OnMake();
	afx_msg void OnBrowse();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnSizing( UINT nSide, LPRECT lpRect );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
public:
	CEdit m_editName;
public:
	CStatic m_txtName;
public:
	CStatic m_txtHint;
};
