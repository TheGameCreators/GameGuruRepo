#ifndef H_UPLOAD_THREAD2
#define H_UPLOAD_THREAD2

#include "stdafx.h"
#include "HTTPComm.h"
#include "Thread.h"

class CProgressCtrl;
class CUploadGame;

class UploadThread2 : public Thread
{

public:

	UploadThread2( );
	~UploadThread2( );

	void SetFields( CUploadGame *parent, CProgressCtrl *progress, CString model, CString thumbnail, CString description, int iClub=0 );

	bool IsUploading( ) { return m_bUploading; }
	bool IsSuccessful( ) { return m_bUploadSuccessful; }
	bool Cancelled( ) { return m_bCancelled; }

	void Cancel();
	LPCSTR GetError() { return sError.GetString(); }

protected:

	virtual unsigned Run( );

private:

	CProgressCtrl *m_pProgressBar;
	CUploadGame *m_pParent;

	HTTPComm m_Server;
	bool m_bUploading;
	bool m_bUploadSuccessful;
	bool m_bIsPacking;
	bool m_bCancelled;
	
	CString m_sModelFilename;
	CString m_sThumbnail;
	CString m_sDescription;
	int iClubID;

	CString sError;
};

#endif