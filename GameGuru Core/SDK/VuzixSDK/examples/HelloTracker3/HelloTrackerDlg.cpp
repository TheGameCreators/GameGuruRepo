// HelloTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include <dbt.h>
#include "HelloTracker3.h"
#include "HelloTrackerDlg.h"
#define IWEARDRV_EXPLICIT
#define IWEAR_ONETIME_DEFINITIONS
#include <iweardrv.h>
#include <iwrstdrv.h>
#include ".\hellotrackerdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define POLL_TIMER 10


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)

END_MESSAGE_MAP()


// CHelloTrackerDlg dialog



CHelloTrackerDlg::CHelloTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHelloTrackerDlg::IDD, pParent)
	, m_csYaw(_T(""))
	, m_csPitch(_T(""))
	, m_csRoll(_T(""))
	, m_fTrackerPresent(FALSE)
    , m_csMagX(_T(""))
    , m_csMagY(_T(""))
    , m_csMagZ(_T(""))
    , m_csAccX(_T(""))
    , m_csAccY(_T(""))
    , m_csAccZ(_T(""))
    , m_csGX(_T(""))
    , m_csGY(_T(""))
    , m_csGZ(_T(""))
    , m_csLBGX(_T(""))
    , m_csLBGY(_T(""))
    , m_csLBGZ(_T(""))
    , m_csXTRN(_T(""))
    , m_csYTRN(_T(""))
    , m_csZTRN(_T(""))
    , m_csFLBGX(_T(""))
    , m_csFLBGY(_T(""))
    , m_csFLBGZ(_T(""))
    , m_csFGX(_T(""))
    , m_csFGY(_T(""))
    , m_csFGZ(_T(""))
    , m_csFAX(_T(""))
    , m_csFAY(_T(""))
    , m_csFAZ(_T(""))
    , m_csFMX(_T(""))
    , m_csFMY(_T(""))
    , m_csFMZ(_T(""))
    , m_csStatus(_T(""))
    , m_bAutoCorrect(FALSE)
    , m_csMagYaw(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHelloTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_STATIC_YAW, m_csYaw);
    DDX_Text(pDX, IDC_STATIC_PITCH, m_csPitch);
    DDX_Text(pDX, IDC_STATIC_ROLL, m_csRoll);
    DDX_Text(pDX, IDC_TXT_MAGX, m_csMagX);
    DDX_Text(pDX, IDC_TXT_MAGY, m_csMagY);
    DDX_Text(pDX, IDC_TXT_MAGZ, m_csMagZ);
    DDX_Text(pDX, IDC_TXT_ACCX, m_csAccX);
    DDX_Text(pDX, IDC_TXT_ACCY, m_csAccY);
    DDX_Text(pDX, IDC_TXT_ACCZ, m_csAccZ);
    DDX_Text(pDX, IDC_TXT_GX, m_csGX);
    DDX_Text(pDX, IDC_TXT_GY, m_csGY);
    DDX_Text(pDX, IDC_TXT_GZ, m_csGZ);
    DDX_Text(pDX, IDC_TXT_LBGX, m_csLBGX);
    DDX_Text(pDX, IDC_TXT_LBGY, m_csLBGY);
    DDX_Text(pDX, IDC_TXT_LBGZ, m_csLBGZ);
    DDX_Text(pDX, IDC_STATIC_XTRN, m_csXTRN);
    DDX_Text(pDX, IDC_STATIC_YTRN, m_csYTRN);
    DDX_Text(pDX, IDC_STATIC_ZTRN, m_csZTRN);
    DDX_Text(pDX, IDC_TXT_LBGX2, m_csFLBGX);
    DDX_Text(pDX, IDC_TXT_LBGY2, m_csFLBGY);
    DDX_Text(pDX, IDC_TXT_LBGZ2, m_csFLBGZ);
    DDX_Text(pDX, IDC_TXT_GX2, m_csFGX);
    DDX_Text(pDX, IDC_TXT_GY2, m_csFGY);
    DDX_Text(pDX, IDC_TXT_GZ2, m_csFGZ);
    DDX_Text(pDX, IDC_TXT_ACCX2, m_csFAX);
    DDX_Text(pDX, IDC_TXT_ACCY2, m_csFAY);
    DDX_Text(pDX, IDC_TXT_ACCZ2, m_csFAZ);
    DDX_Text(pDX, IDC_TXT_MAGX2, m_csFMX);
    DDX_Text(pDX, IDC_TXT_MAGY2, m_csFMY);
    DDX_Text(pDX, IDC_TXT_MAGZ2, m_csFMZ);
    DDX_Text(pDX, IDC_TXT_STATUS, m_csStatus);
    DDX_Control(pDX, IDC_BTN_GETVERSION, m_btnGetVersion);
    DDX_Check(pDX, IDC_CHK_AUTOCORRECT, m_bAutoCorrect);
    DDX_Text(pDX, IDC_TXT_MAGYAW, m_csMagYaw);
}

BEGIN_MESSAGE_MAP(CHelloTrackerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_ZERO, OnBnClickedButtonZero)
    ON_BN_CLICKED(IDC_BTN_GETVERSION, &CHelloTrackerDlg::OnBnClickedBtnGetversion)
    ON_BN_CLICKED(IDC_CHK_AUTOCORRECT, &CHelloTrackerDlg::OnBnClickedChkAutocorrect)
END_MESSAGE_MAP()


// CHelloTrackerDlg message handlers

BOOL CHelloTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    m_bAutoCorrect = TRUE;
    UpdateData(FALSE);
	m_hIwear = LoadLibrary("IWEARDRV.DLL");
	if (m_hIwear) {
		IWROpenTracker = (PIWROPENTRACKER)GetProcAddress(m_hIwear, _T("IWROpenTracker"));
		IWRCloseTracker = (PIWRCLOSETRACKER)GetProcAddress(m_hIwear, _T("IWRCloseTracker"));
		IWRZeroSet = (PIWRZEROSET)GetProcAddress(m_hIwear, _T("IWRZeroSet"));
		IWRGetTracking = (PIWRGETTRACKING)GetProcAddress(m_hIwear, _T("IWRGetTracking"));
		IWRGet6DTracking = (PIWRGET6DTRACKING)GetProcAddress(m_hIwear, _T("IWRGet6DTracking"));
		IWRGetFilteredSensorData = (PIWRGETFILTEREDSENSORDATA)GetProcAddress(m_hIwear, _T("IWRGetFilteredSensorData"));
		IWRGetSensorData = (PIWRGETSENSORDATA)GetProcAddress(m_hIwear, _T("IWRGetSensorData"));
		IWRSetFilterState = (PIWRSETFILTERSTATE)GetProcAddress(m_hIwear, _T("IWRSetFilterState"));
		IWRGetProductID = (PIWRGETPRODUCTID)GetProcAddress(m_hIwear, _T("IWRGetProductID"));
		IWRGetVersion = (PIWRGETVERSION)GetProcAddress(m_hIwear, _T("IWRGetVersion"));
        IWRGetMagYaw = (PIWRGETMAGYAW)GetProcAddress(m_hIwear, _T("IWRGetMagYaw"));
        IWRSetMagAutoCorrect = (PIWRSETMAGAUTOCORRECT)GetProcAddress(m_hIwear, _T("IWRSetMagAutoCorrect"));
		RestartPoll();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHelloTrackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHelloTrackerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHelloTrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHelloTrackerDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_fTrackerPresent && IWRCloseTracker) {
		m_fTrackerPresent = FALSE;
		IWRCloseTracker();
	}
	CDialog::OnCancel();
}

void CHelloTrackerDlg::RestartPoll(void)
{
    IWRVERSION ver;
	if (IWROpenTracker() == ERROR_SUCCESS) {
        if (IWRGetVersion(&ver) == ERROR_SUCCESS) {
        }
		if (IWRSetFilterState) IWRSetFilterState(TRUE);
		m_fTrackerPresent = TRUE;
		SetTimer(POLL_TIMER,POLL_TIMER,NULL);
        m_btnGetVersion.EnableWindow(TRUE);
        m_csStatus.Format(_T("Version: %d.%d.%d.%d.%d"),ver.USBFirmwareMajor,ver.USBFirmwareMinor,ver.TrackerFirmwareMajor,ver.TrackerFirmwareMinor,ver.VideoFirmware);
	}
	else {
		m_fTrackerPresent = FALSE;
		SetTimer(POLL_TIMER,1000,NULL);
        m_csStatus.Format(_T("Not Attached"));
        m_btnGetVersion.EnableWindow(FALSE);
	}
    UpdateData(FALSE);
}

void CHelloTrackerDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == POLL_TIMER) {
        IWRSENSDATA sensdata;
		LONG yaw = 0,pitch = 0,roll = 0,myaw = 0;
		FLOAT xtrn,ytrn,ztrn;
        LONG fmx,fmy,fmz,fax,fay,faz,fgx,fgy,fgz,flgx,flgy,flgz;
		if (IWRGet6DTracking(&yaw, &pitch, &roll, &xtrn, &ytrn, &ztrn) != ERROR_SUCCESS) {
			RestartPoll();
		}
        IWRGetSensorData(&sensdata);
        IWRGetFilteredSensorData(&fax,&fay,&faz,&flgx,&flgy,&flgz,&fgx,&fgy,&fgz,&fmx,&fmy,&fmz);
        IWRGetMagYaw(&myaw);
		UpdateData(TRUE);
		m_csYaw.Format(_T("%d"), yaw);
		m_csPitch.Format(_T("%d"), pitch);
		m_csRoll.Format(_T("%d"), roll);
        m_csXTRN.Format(_T("%5.1f"), xtrn);
        m_csYTRN.Format(_T("%5.1f"), ytrn);
        m_csZTRN.Format(_T("%5.1f"), ztrn);
        m_csMagYaw.Format(_T("%d"), myaw);
        m_csMagX.Format(_T("MAGX = %d"),(signed short)(((unsigned short)sensdata.mag_sensor.magx_msb << 8) | (unsigned short)sensdata.mag_sensor.magx_lsb));
        m_csMagY.Format(_T("MAGY = %d"),(signed short)(((unsigned short)sensdata.mag_sensor.magy_msb << 8) | (unsigned short)sensdata.mag_sensor.magy_lsb));
        m_csMagZ.Format(_T("MAGZ = %d"),(signed short)(((unsigned short)sensdata.mag_sensor.magz_msb << 8) | (unsigned short)sensdata.mag_sensor.magz_lsb));
        m_csAccX.Format(_T("ACCX = %d"),(signed short)(((unsigned short)sensdata.acc_sensor.accx_msb << 8) | (unsigned short)sensdata.acc_sensor.accx_lsb));
        m_csAccY.Format(_T("ACCY = %d"),(signed short)(((unsigned short)sensdata.acc_sensor.accy_msb << 8) | (unsigned short)sensdata.acc_sensor.accy_lsb));
        m_csAccZ.Format(_T("ACCZ = %d"),(signed short)(((unsigned short)sensdata.acc_sensor.accz_msb << 8) | (unsigned short)sensdata.acc_sensor.accz_lsb));
        m_csGX.Format(_T("GX = %d"),(signed short)(((unsigned short)sensdata.gyro_sensor.gyx_msb << 8) | (unsigned short)sensdata.gyro_sensor.gyx_lsb));
        m_csGY.Format(_T("GY = %d"),(signed short)(((unsigned short)sensdata.gyro_sensor.gyy_msb << 8) | (unsigned short)sensdata.gyro_sensor.gyy_lsb));
        m_csGZ.Format(_T("GZ = %d"),(signed short)(((unsigned short)sensdata.gyro_sensor.gyz_msb << 8) | (unsigned short)sensdata.gyro_sensor.gyz_lsb));
        m_csLBGX.Format(_T("LBGX = %d"),(signed short)(((unsigned short)sensdata.lbgyro_sensor.gyx_msb << 8) | (unsigned short)sensdata.lbgyro_sensor.gyx_lsb));
        m_csLBGY.Format(_T("LBGY = %d"),(signed short)(((unsigned short)sensdata.lbgyro_sensor.gyy_msb << 8) | (unsigned short)sensdata.lbgyro_sensor.gyy_lsb));
        m_csLBGZ.Format(_T("LBGZ = %d"),(signed short)(((unsigned short)sensdata.lbgyro_sensor.gyz_msb << 8) | (unsigned short)sensdata.lbgyro_sensor.gyz_lsb));
        m_csFAX.Format(_T("FAX = %d"), fax);
        m_csFAY.Format(_T("FAY = %d"), fay);
        m_csFAZ.Format(_T("FAZ = %d"), faz);
        m_csFMX.Format(_T("FMX = %d"), fmx);
        m_csFMY.Format(_T("FMY = %d"), fmy);
        m_csFMZ.Format(_T("FMZ = %d"), fmz);
        m_csFGX.Format(_T("FGX = %d"), fgx);
        m_csFGY.Format(_T("FGY = %d"), fgy);
        m_csFGZ.Format(_T("FGZ = %d"), fgz);
        m_csFLBGX.Format(_T("FLGX = %d"), flgx);
        m_csFLBGY.Format(_T("FLGY = %d"), flgy);
        m_csFLBGZ.Format(_T("FLGZ = %d"), flgz);


		UpdateData(FALSE);
	}
	CDialog::OnTimer(nIDEvent);
}

void CHelloTrackerDlg::OnBnClickedButtonZero()
{
	// TODO: Add your control notification handler code here
	if (m_fTrackerPresent) {
		IWRZeroSet();
	}
}
void CHelloTrackerDlg::OnBnClickedBtnGetversion()
{
    IWRVERSION ver;
    // TODO: Add your control notification handler code here
    IWRGetVersion(&ver);
    m_csStatus.Format(_T("Version: %d.%d.%d.%d.%d"),ver.USBFirmwareMajor,ver.USBFirmwareMinor,ver.TrackerFirmwareMajor,ver.TrackerFirmwareMinor,ver.VideoFirmware);
}

void CHelloTrackerDlg::OnBnClickedChkAutocorrect()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    IWRSetMagAutoCorrect(m_bAutoCorrect);
}
