/*-----------------------------------------------------**
 * iwrstreo.h
 * 
 * Copyright (c) 2007 Icuiti Corporation. All rights reserved.
 *
 * This header file is for user mode apps to communicate
 * with the kernel mode driver.
 *
 *
 * $Log: /227-VR920/IWEARSDK/inc/iwrstreo.h $
 * 
 * 2     11/01/07 5:07p Steves
 * 
 * 1     6/05/07 5:18p Steves
 * 
 * 2     5/29/07 4:54p Steves
 * 
 * 1     4/10/07 12:47p Steves
 * 
**-----------------------------------------------------*/

#ifndef _IOCTL_H
#define _IOCTL_H

#ifndef DRIVER
#include <basetyps.h>
#include <winioctl.h>
#include <usbioctl.h>
#endif

#include <initguid.h>

/*- IOCTL info -------------*/
#define IOCTL_IWRS_BASE 0x800


#define IOCTL_IWRS_VENDOR_REQUEST CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_GET_VERSION CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_RESET CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_GET_STRING CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_SET_STRING CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_GET_CONFIG_DESCRIPTOR CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_WAIT_ON_INTERRUPT CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IWRS_ACQUIRE CTL_CODE(FILE_DEVICE_USB, IOCTL_IWRS_BASE+8, METHOD_BUFFERED, FILE_ANY_ACCESS)
/*- end of IOCTL info ------*/

#define MANUFACTURER_STRING	0x1
#define PRODUCT_STRING		0x2
#define SERIAL_STRING		0x3

/*- User defined types -----*/

typedef struct _IWRS_STRING_REQUEST {
	UCHAR StringID;
	USHORT LangID;
	UCHAR Buffer[512];
	ULONG BufferLength;
} IWRS_STRING_REQUEST, *PIWRS_STRING_REQUEST;

typedef struct _IWRS_REQUEST_HDR {
	UCHAR bRequest;
	USHORT wValue;
	USHORT wIndex;
	USHORT wLength;
} IWRS_REQUEST_HDR, *PIWRS_REQUEST_HDR;

typedef struct _IWRS_VENDOR_REQUEST {
	IWRS_REQUEST_HDR Hdr;
	UCHAR Data[1];
} IWRS_VENDOR_REQUEST, *PIWRS_VENDOR_REQUEST;

typedef struct _IWRS_VERSION {
	USHORT wDriverVersion;
	USHORT wFirmwareVersion;
} IWRS_VERSION, *PIWRS_VERSION;

// VR920 Vendor Request Command Constants
#define IWRSTREO_BREQUEST_CODE 0x09;
#define IWRSTREO_WVALUE_CODE   0x0302;
#define IWRSTREO_WINDEX_CODE   0x0003;
#define IWRSTREO_REPORT_ID     2

#ifndef SET_STEREO_OFF
// SET_STEREO_OFF - turns all stereo processing off
#define SET_STEREO_OFF			0x06
// SET_STEREO_LEFT - current frame data should go to left eye
#define SET_STEREO_LEFT			0x07
// SET_STEREO_RIGHT - current frame data should go to right eye
#define SET_STEREO_RIGHT		0x08
// SET_STEREO_DDC - VR920 uses the DDC line to direct frame data (nVidia)
#define SET_STEREO_DDC			0x09


// Exported Types *****************
typedef struct _generalReport {
	unsigned short repMode;
	unsigned short reserved[6];
} DEVICEREPORT, *PDEVICEREPORT;
#endif

/*- end of User defined types -*/


#endif
