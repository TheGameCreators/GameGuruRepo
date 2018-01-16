/*-----------------------------------------------------------------------------
 * iweardrv.h
 *
 * Copyright (c) 2007-2011 Vuzix Corporation.  All rights reserved.
 *
 * Header file for the Vuzix iWear SDK 
 *
 * Revision History:
 * $Log: /227-VR920/IWEARSDK/inc/iweardrv.h $
 * 
 * 31    2/19/13 12:12p Craig
 * Version 3.6.0.24: Altered duplicate return code for NOTRACKERINSTANCE
 * and added return code for unsupported calls
 * 
 * 30    2/19/13 11:18a Bent
 * Moved return codes from iWearSDK,h to iweardrv.h
 * 
 * 29    11/30/12 2:38p Bent
 * 
 * 28    11/16/12 10:51a Bent
 * Added define for STAR XL D and Wrap 1200 VR D
 * 
 * 27    11/12/12 5:19p Bent
 * Added IWR_TRACKER_FEATURE_2 for 386 tracker that can report in
 * different ways
 * 
 * 26    11/07/12 4:26p Bent
 * Added product ID for Wrap 1200 D even though it isn't currently used
 * 
 * 25    11/07/12 3:54p Bent
 * Release check-in for verison 3.4.0.6
 * 
 * 24    9/12/12 5:33p Bent
 * 
 * 23    9/11/12 5:16p Bent
 * Added functions for setting/getting tracker mode
 * 
 * 22    8/30/12 5:05p Bent
 * Update with setter and getter for gyro math status
 * 
 * 21    7/23/12 2:41p Bent
 * Added IWRZeroGyros back into the exports list
 * 
 * 20    2/16/12 5:16p Bent
 * Working in 32 and 64 bit with 355
 * 
 * 19    2/03/12 5:25p Bent
 * Updated to add version and product id support for 355/359
 * 
 * 18    11-08-17 3:45p Robertr
 * 
 * 17    11-08-15 4:18p Robertr
 * added a couple of defines to make it easier to work with Product
 * details.
 * 
 * 16    11-08-12 10:46a Robertr
 * added 1200AR details.   Unconfirmed that 1200 AR is sub-id 7, so this
 * may still have to change.
 * 
 * 15    11-08-10 3:54p Robertr
 * 
 * 11    6/17/11 11:06a Craigt / Robr / Bent
 *
 * 10    11/08/10 2:27p Steves
 * 
 * 9     6/18/10 9:58a Steves
 * 
 * 8     3/22/10 9:38a Steves
 * 
 * 7     3/01/10 8:36a Steves
 * 
 * 6     1/13/10 2:59p Steves
 * 
 * 5     12/16/09 11:11a Steves
 * 
 * 4     9/25/08 10:44a Steves
 * 
 * 3     6/19/07 4:14p Steves
 * 
 * 2     5/24/07 2:31p Steves
 * 
 * 1     4/10/07 9:52a Steves
 * 
 * 1     4/10/07 9:33a Steves
 * 
 */
#ifndef _IWEARDRV_H
#define _IWEARDRV_H
#ifdef __cplusplus
extern "C" {
#endif

// iWear Tracker Product IDs
#define IWR_PROD_NONE				000 // indicated an unsupported product
#define IWR_PROD_VR920				227 // This is equvalent to the IRW_FEATURE_TRACKER_STYLE_0 bit set in product details.
#define IWR_PROD_WRAP920			329 // IWR_PROD_WRAP920 as a define is deprecated
#define IWR_PROD_WRAP				329 // This is equvalent to the IRW_FEATURE_TRACKER_STYLE_1 bit set in product details.
#define IWR_PROD_386				386 

// iWear Tracker Product sub-IDs
#define IWR_PROD_DETAIL_VR920		0 // No sub IDs for VR920
// Wrap sub IDS
#define IWR_PROD_DETAIL_WRAP310		1
#define IWR_PROD_DETAIL_WRAP920		2
#define IWR_PROD_DETAIL_WRAP280		3
#define IWR_PROD_DETAIL_WRAP230		4
#define IWR_PROD_DETAIL_WRAP1200	5
#define IWR_PROD_DETAIL_STAR1200	6
#define IWR_PROD_DETAIL_WRAP1200AR	7
#define IWR_PROD_DETAIL_STAR1200XL	8
#define IWR_PROD_DETAIL_WRAP1200D	9
#define IWR_PROD_DETAIL_STAR1200XLD 10
#define IWR_PROD_DETAIL_WRAPVR1200D 11
// 386 sub IDs
#define IWR_PROD_DETAIL_PCAS		1
#define IWR_PROD_DETAIL_M2000AR		2

enum IWR_RETURNS {
	IWR_FAILURE				= -1,							// -1=General failure case
	IWR_OK					= ERROR_SUCCESS,				// 0= Tracker driver interface installed and connected ok.
	IWR_NO_TRACKER			= ERROR_FILE_NOT_FOUND,			// 1= Tracker Driver is not installed.
	IWR_NO_STEREO			= ERROR_OPEN_FAILED,			// 5= Stereo driver not installed.
	IWR_OFFLINE				= ERROR_DEV_NOT_EXIST,			// 2= Tracker driver interface is installed, yet appears to be offline or not responding.
	IWR_TRACKER_CORRUPT		= ERROR_INVALID_FUNCTION,		// 3= Tracker driver installed; however, there are missing exports.
	IWR_STEREO_CORRUPT		= ERROR_INVALID_DATA,			// 6= Stereo driver installed; however, there are missing exports.
//	IWR_NOTRACKER_INSTANCE	= ERROR_INVALID_HANDLE,			// 4= Attempt to free the tracker driver; however, the tracker driver did not previously load successfully.
	IWR_NOTRACKER_INSTANCE	= ERROR_INVALID_TARGET_HANDLE,	// Altered for non-overlapping error codes.
	IWR_OFFLINE_OLD         = ERROR_INVALID_HANDLE,
    IWR_NOT_SUPPORTED		= ERROR_NOT_SUPPORTED,
    IWR_NO_RESOURCES		= ERROR_NOT_ENOUGH_MEMORY,
    IWR_RD_IOFAILURE		= ERROR_READ_FAULT,
    IWR_NO_ACCESS_HNDLE     = ERROR_INSUFFICIENT_BUFFER,
	IWR_ERR_UNSUPPORTED		= 0x78 // Return ERROR_CALL_NOT_IMPLEMENTED on VR920 stereo calls when running 64 bit
};
#define ERR_UNSUPPORTED				0x78 // Return ERROR_CALL_NOT_IMPLEMENTED on VR920 stereo calls when running 64 bit

typedef struct tag_IWRVERSION {
	unsigned short DLLMajor;
	unsigned short DLLMinor;
	unsigned short DLLSubMinor;
	unsigned short DLLBuildNumber;
	char USBFirmwareMajor;
	char USBFirmwareMinor;
	char TrackerFirmwareMajor;
	char TrackerFirmwareMinor;
	char VideoFirmware;
} IWRVERSION, *PIWRVERSION;
typedef struct tag_IWRMAGSENSOR {
    unsigned char magx_lsb;
    unsigned char magx_msb;
    unsigned char magy_lsb;
    unsigned char magy_msb;
    unsigned char magz_lsb;
    unsigned char magz_msb;
} IWRMAGSENSOR, *IWRPMAGSENSOR;
typedef struct tag_IWRACCELSENSOR {
    unsigned char accx_lsb;
    unsigned char accx_msb;
    unsigned char accy_lsb;
    unsigned char accy_msb;
    unsigned char accz_lsb;
    unsigned char accz_msb;
} IWRACCELSENSOR, *PIWRACCELSENSOR;
// High Bandwidth Gyros 2000 degrees per second
// = 1.465 dps/cnt
typedef struct tag_IWRGYROSENSOR {
    unsigned char gyx_lsb;
    unsigned char gyx_msb;
    unsigned char gyy_lsb;
    unsigned char gyy_msb;
    unsigned char gyz_lsb;
    unsigned char gyz_msb;
} IWRGYROSENSOR, *PIWRGYROSENSOR;
// High Bandwidth Gyros 500 degrees per second
// = 0,366 dps/cnt
typedef struct tag_IWRLBGYROSENSOR {
    unsigned char gyx_lsb;
    unsigned char gyx_msb;
    unsigned char gyy_lsb;
    unsigned char gyy_msb;
    unsigned char gyz_lsb;
    unsigned char gyz_msb;
} IWRLBGYROSENSOR, *PIWRLBGYROSENSOR;
typedef struct tag_IWRSENSDATA {
    IWRMAGSENSOR mag_sensor;
    IWRACCELSENSOR acc_sensor;
    IWRGYROSENSOR gyro_sensor;
    IWRLBGYROSENSOR lbgyro_sensor;
} IWRSENSDATA, *PIWRSENSDATA;

/// Parse the subid out of the product details returned by IWRGetProductDetails()
#define IWR_GET_SUBID( x )          ((x & IWR_FEATURE_SUB_ID_MASK) >> 16)

/// Parse the aspect ratio out of the product details returned by IWRGetProductDetails()
#define IWR_GET_ASPECT_RATIO( x )   (x & IWR_FEATURE_ASPECT_RATIO_MASK)

/// Parse the tracker style out of the product details returned by IWRGetProductDetails()
#define IWR_GET_TRACKER_STYLE( x )  (x & IWR_FEATURE_TRACKER_STYLE_MASK)

// Product feature flags as returned by IWRGetProductDetails.
// This contains some information about the hardware associated with this tracker.
// Supported and best resolutions of the associated iWear come from the DDC info 
// and should be handled as any other DDC info.
// Tracker style is shorthand for for the kind of information the tracker returns.
#define IWR_FEATURE_TRACKER_STYLE_MASK	0x00000003

// Style 0 is the VR920.  This tracker is built in.
// It doesn't return data from IWRGetFilteredSensorData(), 
// IWRGetMagYaw(), or IWRGetSensorData()
// It doesn't return translation data from IWRGet6DTracking()
#define IWR_FEATURE_TRACKER_STYLE_0		0x00000000

// Style 1 is the wrap line.  This tracker pluggs into a port and may not always be present.
#define IWR_FEATURE_TRACKER_STYLE_1		0x00000001

// Style 2 is the 386 PCAS/M2000AR line.  This tracker is built in and can report raw, euler 
// or quaternion data
#define IWR_FEATURE_TRACKER_STYLE_2		0x00000002

// The upper 16 flag bits are dedicated the the associated iWear hardware.
// SubID as listed above.
#define IWR_FEATURE_SUB_ID_MASK			0x000F0000

#define IWR_FEATURE_ASPECT_RATIO_MASK	0x07000000
#define IWR_FEATURE_ASPECT_RATIO_4x3	0x00000000
#define IWR_FEATURE_ASPECT_RATIO_16x9	0x01000000
#define IWR_FEATURE_ASPECT_RATIO_16x10	0x02000000

#define IWR_FEATURE_SEETHROUGH			0x08000000
#define IWR_FEATURE_WINDOWED_3D			0x10000000 // All iWear supports FullScreen 3D, only some supports windowed 3D.
#define IWR_FEATURE_USB_PSCAN_3D		0x20000000 // Supports Progressive scan USB stereoscopy
#define IWR_FEATURE_SIDE_X_SIDE			0x40000000 // Supports side by side stereoscopy

#define TRK_I2C_ADDR 0x40

#define TRK_REG_HW_REV 0x01             // 1 byte
#define TRK_REG_SW_REV 0x02              // 1 byte

#ifndef IWEARDRV_EXPLICIT
#ifdef IWEARDRV_EXPORTS
__declspec( dllexport ) DWORD __cdecl	IWROpenTracker( void );
__declspec( dllexport ) void __cdecl	IWRCloseTracker( void );
__declspec( dllexport ) void __cdecl	IWRZeroSet( void );
__declspec( dllexport ) void __cdecl	IWRZeroGyros( void );
__declspec( dllexport ) void __cdecl	IWRCalGyros( void );
__declspec( dllexport ) DWORD __cdecl	IWRGetTracking(LONG *yaw, LONG *pitch, LONG *roll);
__declspec( dllexport ) DWORD __cdecl	IWRGet6DTracking(LONG *yaw, LONG *pitch, LONG *roll, FLOAT *xtrn, FLOAT *ytrn, FLOAT *ztrn);
__declspec( dllexport ) DWORD __cdecl	IWRGetFilteredSensorData(LONG *ax, LONG *ay, LONG *az, LONG *lgx, LONG *lgy, LONG *lgz, LONG *gx, LONG *gy, LONG *gz, LONG *mx, LONG *my, LONG *mz);
__declspec( dllexport ) DWORD __cdecl	IWRGetSensorData(PIWRSENSDATA sensdataptr);
__declspec( dllexport ) DWORD __cdecl	IWRBeginCalibrate(void);
__declspec( dllexport ) void __cdecl	IWREndCalibrate(BOOL save);
__declspec( dllexport ) DWORD __cdecl	IWRGetVersion(PIWRVERSION ver);
__declspec( dllexport ) void __cdecl	IWRSetFilterState(BOOL on);
__declspec( dllexport ) BOOL __cdecl	IWRGetFilterState(void);
__declspec( dllexport ) WORD __cdecl	IWRGetProductID(void);
__declspec( dllexport ) DWORD __cdecl	IWRGetProductDetails(void);
__declspec( dllexport ) void __cdecl	IWRSetMagAutoCorrect(BOOL on);
__declspec( dllexport ) DWORD __cdecl	IWRGetMagYaw(LONG *myaw);
__declspec( dllexport ) BOOL __cdecl	IWRGetGyroMath(void);
__declspec( dllexport ) void __cdecl	IWRSetGyroMath(BOOL on);
__declspec( dllexport ) DWORD __cdecl	IWRGetBrightness(int *brightness);
__declspec( dllexport ) DWORD __cdecl	IWRSetBrightness(int brightness);
__declspec( dllexport ) DWORD __cdecl	IWRGetShutter(int *shutter);
__declspec( dllexport ) DWORD __cdecl	IWRSetShutter(int shutter);
__declspec( dllexport ) DWORD __cdecl	IWRGetLeftEyeMode(BOOL *on);
__declspec( dllexport ) DWORD __cdecl	IWRSetLeftEyeMode(BOOL on);
__declspec( dllexport ) DWORD __cdecl	IWRGetColorMode(BOOL *on);
__declspec( dllexport ) DWORD __cdecl	IWRSetColorMode(BOOL on);
__declspec( dllexport ) DWORD __cdecl	IWRGetTrackingMode(int *mode);
__declspec( dllexport ) DWORD __cdecl	IWRSetTrackingMode(int mode);

#else
__declspec( dllimport ) extern DWORD	IWROpenTracker( void );
__declspec( dllimport ) extern void		IWRCloseTracker( void );
__declspec( dllimport ) extern void		IWRZeroSet( void );
__declspec( dllexport ) extern void 	IWRZeroGyros( void );
__declspec( dllexport ) extern void 	IWRCalGyros( void );
__declspec( dllimport ) extern DWORD	IWRGetTracking(LONG *yaw, LONG *pitch, LONG *roll);
__declspec( dllimport ) extern DWORD	IWRGet6DTracking(LONG *yaw, LONG *pitch, LONG *roll, FLOAT *xtrn, FLOAT *ytrn, FLOAT *ztrn);
__declspec( dllimport ) extern DWORD	IWRGetFilteredSensorData(LONG *ax, LONG *ay, LONG *az, LONG *lgx, LONG *lgy, LONG *lgz, LONG *gx, LONG *gy, LONG *gz, LONG *mx, LONG *my, LONG *mz);
__declspec( dllimport ) extern DWORD	IWRGetSensorData(PIWRSENSDATA sensdataptr);
__declspec( dllexport ) extern DWORD	IWRBeginCalibrate(void);
__declspec( dllexport ) extern void		IWREndCalibrate(BOOL save);
__declspec( dllimport ) extern DWORD	IWRGetVersion(PIWRVERSION ver);
__declspec( dllimport ) extern void		IWRSetFilterState(BOOL on);
__declspec( dllimport ) extern BOOL		IWRGetFilterState(void);
__declspec( dllimport ) extern WORD		IWRGetProductID(void);
__declspec( dllimport ) extern DWORD	IWRGetProductDetails(void);
__declspec( dllimport ) extern void		IWRSetMagAutoCorrect(BOOL on);
__declspec( dllimport ) extern DWORD	IWRGetMagYaw(LONG *myaw);
__declspec( dllimport ) extern BOOL		IWRGetGyroMath(void);
__declspec( dllexport ) extern void		IWRSetGyroMath(BOOL enabled);
__declspec( dllexport ) extern DWORD	IWRGetBrightness(int *brightness);
__declspec( dllexport ) extern DWORD	IWRSetBrightness(int brightness);
__declspec( dllexport ) extern DWORD	IWRGetShutter(int *shutter);
__declspec( dllexport ) extern DWORD	IWRSetShutter(int shutter);
__declspec( dllexport ) extern DWORD 	IWRGetLeftEyeMode(BOOL *enabled);
__declspec( dllexport ) extern DWORD 	IWRSetLeftEyeMode(BOOL enabled);
__declspec( dllexport ) extern DWORD 	IWRGetColorMode(BOOL *enabled);
__declspec( dllexport ) extern DWORD 	IWRSetColorMode(BOOL enabled);
__declspec( dllexport ) extern DWORD	IWRGetTrackingMode(int *mode);
__declspec( dllexport ) extern DWORD	IWRSetTrackingMode(int mode);
#endif
#else
typedef DWORD	(__cdecl *PIWROPENTRACKER)( void );
typedef void	(__cdecl *PIWRCLOSETRACKER)( void );
typedef void	(__cdecl *PIWRZEROSET)( void );
typedef void	(__cdecl *PIWRZEROGYROS)( void );
typedef void	(__cdecl *PIWRCALGYROS)( void );
typedef DWORD	(__cdecl *PIWRGETTRACKING)(LONG *, LONG *, LONG *);
typedef DWORD	(__cdecl *PIWRGET6DTRACKING)(LONG *, LONG *, LONG *, FLOAT *, FLOAT *, FLOAT *);
typedef DWORD	(__cdecl *PIWRGETFILTEREDSENSORDATA)(LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *, LONG *);
typedef DWORD	(__cdecl *PIWRGETSENSORDATA)(PIWRSENSDATA);
typedef DWORD	(__cdecl *PIWRGETVERSION)(PIWRVERSION);
typedef DWORD	(__cdecl *PIWRBEGINCALIBRATE)( void );
typedef void	(__cdecl *PIWRENDCALIBRATE)( BOOL );
typedef void	(__cdecl *PIWRSETFILTERSTATE)( BOOL );
typedef BOOL	(__cdecl *PIWRGETFILTERSTATE)(void);
typedef WORD	(__cdecl *PIWRGETPRODUCTID)(void);
typedef DWORD   (__cdecl *PIWRGETPRODUCTDETAILS)(void);
typedef void	(__cdecl *PIWRSETMAGAUTOCORRECT)( BOOL );
typedef DWORD	(__cdecl *PIWRGETMAGYAW)(LONG *);
typedef BOOL	(__cdecl *PIWRGETGYROMATH)( void );
typedef void	(__cdecl *PIWRSETGYROMATH)( BOOL );
typedef DWORD	(__cdecl *PIWRGETBRIGHTNESS)( int * );
typedef DWORD	(__cdecl *PIWRSETBRIGHTNESS)( int );
typedef DWORD	(__cdecl *PIWRGETSHUTTER)( int * );
typedef DWORD	(__cdecl *PIWRSETSHUTTER)( int );
typedef DWORD	(__cdecl *PIWRGETLEFTEYEMODE)( BOOL * );
typedef DWORD	(__cdecl *PIWRSETLEFTEYEMODE)( BOOL );
typedef DWORD	(__cdecl *PIWRGETCOLORMODE)( BOOL * );
typedef DWORD	(__cdecl *PIWRSETCOLORMODE)( BOOL );
typedef DWORD	(__cdecl *PIWRGETTRACKINGMODE)( int * );
typedef DWORD	(__cdecl *PIWRSETTRACKINGMODE)( int );

#ifdef IWEAR_ONETIME_DEFINITIONS
	HMODULE						IWRTrackerInstance=NULL;
	PIWROPENTRACKER				IWROpenTracker=NULL;
	PIWRCLOSETRACKER			IWRCloseTracker=NULL;
	PIWRZEROSET					IWRZeroSet=NULL;
	PIWRZEROGYROS				IWRZeroGyros=NULL;
	PIWRZEROGYROS				IWRCalGyros=NULL;
	PIWRGETTRACKING				IWRGetTracking=NULL;
	PIWRGET6DTRACKING			IWRGet6DTracking=NULL;
	PIWRGETFILTEREDSENSORDATA	IWRGetFilteredSensorData=NULL;
	PIWRGETSENSORDATA			IWRGetSensorData=NULL;
	PIWRGETVERSION				IWRGetVersion=NULL;
	PIWRBEGINCALIBRATE			IWRBeginCalibrate=NULL;
	PIWRENDCALIBRATE			IWREndCalibrate=NULL;
	PIWRSETFILTERSTATE			IWRSetFilterState=NULL;
	PIWRGETFILTERSTATE			IWRGetFilterState=NULL;
	PIWRGETPRODUCTID			IWRGetProductID=NULL;
	PIWRGETPRODUCTDETAILS		IWRGetProductDetails=NULL;
	PIWRSETMAGAUTOCORRECT		IWRSetMagAutoCorrect=NULL;
	PIWRGETMAGYAW				IWRGetMagYaw=NULL;
	PIWRGETGYROMATH				IWRGetGyroMath=NULL;
	PIWRSETGYROMATH				IWRSetGyroMath=NULL;
	PIWRGETBRIGHTNESS			IWRGetBrightness=NULL;
	PIWRSETBRIGHTNESS			IWRSetBrightness=NULL;
	PIWRGETSHUTTER				IWRGetShutter=NULL;
	PIWRSETSHUTTER				IWRSetShutter=NULL;
	PIWRGETLEFTEYEMODE			IWRGetLeftEyeMode=NULL;
	PIWRSETLEFTEYEMODE			IWRSetLeftEyeMode=NULL;
	PIWRGETCOLORMODE			IWRGetColorMode=NULL;
	PIWRSETCOLORMODE			IWRSetColorMode=NULL;
	PIWRGETTRACKINGMODE			IWRGetTrackingMode=NULL;
	PIWRSETTRACKINGMODE			IWRSetTrackingMode=NULL;
#else
	extern HMODULE				IWRTrackerInstance;
	extern PIWROPENTRACKER		IWROpenTracker;
	extern PIWRCLOSETRACKER		IWRCloseTracker;
	extern PIWRZEROSET			IWRZeroSet;
	extern PIWRZEROGYROS		IWRZeroGyros;
	extern PIWRZEROGYROS		IWRCalGyros;
	extern PIWRGETTRACKING		IWRGetTracking;
	extern PIWRGET6DTRACKING	IWRGet6DTracking;
	extern PIWRGETFILTEREDSENSORDATA IWRGetFilteredSensorData;
	extern PIWRGETSENSORDATA	IWRGetSensorData;
	extern PIWRGETVERSION		IWRGetVersion;
	extern PIWRBEGINCALIBRATE	IWRBeginCalibrate;
	extern PIWRENDCALIBRATE		IWREndCalibrate;
	extern PIWRSETFILTERSTATE	IWRSetFilterState;
	extern PIWRGETFILTERSTATE	IWRGetFilterState;
	extern PIWRGETPRODUCTID		IWRGetProductID;
	extern PIWRGETPRODUCTDETAILS IWRGetProductDetails;
	extern PIWRSETMAGAUTOCORRECT IWRSetMagAutoCorrect;
	extern PIWRGETMAGYAW		IWRGetMagYaw;
	extern PIWRGETGYROMATH		IWRGetGyroMath;
	extern PIWRSETGYROMATH		IWRSetGyroMath;
	extern PIWRGETBRIGHTNESS	IWRGetBrightness;
	extern PIWRSETBRIGHTNESS	IWRSetBrightness;
	extern PIWRGETSHUTTER		IWRGetShutter;
	extern PIWRSETSHUTTER		IWRSetShutter;
	extern PIWRGETLEFTEYEMODE	IWRGetLeftEyeMode;
	extern PIWRSETLEFTEYEMODE	IWRSetLeftEyeMode;
	extern PIWRGETCOLORMODE		IWRGetColorMode;
	extern PIWRSETCOLORMODE		IWRSetColorMode;
	extern PIWRGETTRACKINGMODE	IWRGetTrackingMode;
	extern PIWRSETTRACKINGMODE	IWRSetTrackingMode;
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif // _IWEARDRV_H