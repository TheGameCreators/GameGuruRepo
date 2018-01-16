#pragma once


// CProgressSpinner

class CProgressSpinner2 : public CStatic
{
	DECLARE_DYNAMIC(CProgressSpinner2)

public:
	CProgressSpinner2();
	virtual ~CProgressSpinner2();

	void StartSpinning( );
	void StopSpinning( );

protected:

	bool m_bSpinning;
	int m_iAngle;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
};


