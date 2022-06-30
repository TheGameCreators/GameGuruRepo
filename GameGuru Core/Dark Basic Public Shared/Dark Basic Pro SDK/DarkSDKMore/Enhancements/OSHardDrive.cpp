#define _CRT_SECURE_NO_WARNINGS

#define DARKSDK	

#include "stdafx.h"
#include "winioctl.h"
#include <stdio.h>
#include "core.h"

#include <wincon.h>
#include <stdlib.h>
#include <time.h>
#include <Nb30.h>
#include "Enchancements.h"

typedef struct _ASTAT_
{
      ADAPTER_STATUS adapt;
      NAME_BUFFER    NameBuff [30];

}ASTAT, * PASTAT;
ASTAT Adapter;

void findMACaddress ( char* pMACAddress )
{
	char pLocalString [ 1024 ];
	NCB Ncb;
	UCHAR uRetCode;
	//char NetName[50];
	LANA_ENUM   lenum;
	int      i;

	memset( &Ncb, 0, sizeof(Ncb) );
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR *)&lenum;
	Ncb.ncb_length = sizeof(lenum);
	uRetCode = Netbios( &Ncb );

	for(i=0; i < lenum.length ;i++)
	{
		memset( &Ncb, 0, sizeof(Ncb) );
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios( &Ncb );

		memset( &Ncb, 0, sizeof (Ncb) );
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		strcpy( (char*)Ncb.ncb_callname,  "*               " );
		Ncb.ncb_buffer = (PUCHAR) &Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios( &Ncb );

		if ( uRetCode == 0 )
		{
			sprintf( pLocalString, "%02X%02X%02X%02X%02X%02X",
					//lenum.lana[i],
					Adapter.adapt.adapter_address[0],
					Adapter.adapt.adapter_address[1],
					Adapter.adapt.adapter_address[2],
					Adapter.adapt.adapter_address[3],
					Adapter.adapt.adapter_address[4],
					Adapter.adapt.adapter_address[5] );
		}
	}

	// copy MAC address back
	strcpy ( pMACAddress, pLocalString );
}

struct sHardDrive
{
	LARGE_INTEGER	liCylinderCount;
	DWORD			dwTracksPerCylinder; 
	DWORD			dwSectorsPerTrack; 
	DWORD			dwBytesPerSector; 

	ULONGLONG		ulTotalBytes;
	ULONGLONG		ulTotalMB;
	ULONGLONG		ulTotalGB;
};

#define MAX_HARD_DRIVE 24

sHardDrive	g_HardDrives      [ MAX_HARD_DRIVE ];
char		g_HardDiskLetters [ MAX_HARD_DRIVE ] [ 4 ] = 
														{
															"c:\\",	"d:\\",	"e:\\",	"f:\\",	"g:\\",	"h:\\",
															"i:\\",	"j:\\",	"k:\\",	"l:\\",	"m:\\",	"n:\\",
															"o:\\",	"p:\\",	"q:\\",	"r:\\",	"s:\\",	"t:\\",
															"u:\\",	"v:\\",	"w:\\",	"x:\\",	"y:\\",	"z:\\"
														};
int			g_iHardDriveCount = 0;

bool CheckHardDriveID ( int iID )
{
	if ( iID < 0 || iID > 24 )
	{
		Error ( 7 );
		return false;
	}

	return true;
}

void HardDriveSetup ( void )
{
	// retrieve information about the hard drive

	// local variables
	int				iIndex          = 0;		// drive index
	HANDLE			hDevice         = NULL;		// handle to device
	DWORD			dwBytesReturned = 0;		// bytes returned from io control

	DISK_GEOMETRY	drive;						// drive structure
	char			szDrive [ 256 ];			// to store drive name

	// get a handle to the device
	hDevice = CreateFile ( "\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

	// check handle is valid
	if ( hDevice == INVALID_HANDLE_VALUE )
	{
		return;
	}

	// loop round for all drives
	while ( hDevice != INVALID_HANDLE_VALUE )
	{
		// get information
		if ( !DeviceIoControl (
									hDevice,						// handle to device
									IOCTL_DISK_GET_DRIVE_GEOMETRY,	// control type
									NULL,							// no input data
									0,								// use 0 because we have no input data
									&drive,							// pointer to drive structure
									sizeof ( DISK_GEOMETRY ),		// size of data
									&dwBytesReturned,				// number of bytes returned
									( LPOVERLAPPED ) NULL			// ignored
							  ) )
									return;

		// calculate the disk size
		ULONGLONG TotalBytes = drive.Cylinders.QuadPart * ( ULONG ) drive.TracksPerCylinder * ( ULONG ) drive.SectorsPerTrack * ( ULONG ) drive.BytesPerSector;
		
		g_HardDrives [ g_iHardDriveCount ].liCylinderCount     = drive.Cylinders;
		g_HardDrives [ g_iHardDriveCount ].dwTracksPerCylinder = drive.TracksPerCylinder;
		g_HardDrives [ g_iHardDriveCount ].dwSectorsPerTrack   = drive.SectorsPerTrack;
		g_HardDrives [ g_iHardDriveCount ].dwBytesPerSector    = drive.BytesPerSector;
		g_HardDrives [ g_iHardDriveCount ].ulTotalBytes        = TotalBytes;
		g_HardDrives [ g_iHardDriveCount ].ulTotalMB           = TotalBytes / 1024 / 1024;
		g_HardDrives [ g_iHardDriveCount ].ulTotalGB           = TotalBytes / 1024 / 1024 / 1024;

		g_iHardDriveCount++;

		// close this handle
		CloseHandle ( hDevice );

		// set up string for next drive
		sprintf ( szDrive, "\\\\.\\PhysicalDrive%d", ++iIndex );

		// access the next drive
		hDevice = CreateFile ( szDrive, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
	}
}

int GetDriveCount ( void )
{
	return g_iHardDriveCount;
}

int GetDriveCylinderCount ( int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return ( int ) g_HardDrives [ iID ].liCylinderCount.LowPart;
}

int GetDriveTrackCount ( int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return ( int ) g_HardDrives [ iID ].dwTracksPerCylinder;
}

int GetDriveSectorsPerTrack	( int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return ( int ) g_HardDrives [ iID ].dwSectorsPerTrack;
}

int GetDriveBytesPerSector ( int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return ( int ) g_HardDrives [ iID ].dwBytesPerSector;
}

int GetDriveTotalSize ( int iID, int iReturn )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	if ( iReturn == 0 )
	{
		return ( int ) g_HardDrives [ iID ].ulTotalBytes;
	}
	else if ( iReturn == 1 )
	{
		return ( int ) g_HardDrives [ iID ].ulTotalMB;
	}
	else
		return ( int ) g_HardDrives [ iID ].ulTotalGB;
}

int GetDriveSpace ( int iID, int iMode, int iReturn )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	// when iMode is 0 we're looking for used space, when it's 1
	// we're after free space
	
	ULARGE_INTEGER	ulAvailableToCaller,
					ulDisk,
					ulFree;

	// make sure we're dealing with a fixed drive
	if ( GetDriveType ( g_HardDiskLetters [ iID ] ) == DRIVE_FIXED )
	{
		// get the information we need
		if ( GetDiskFreeSpaceEx ( g_HardDiskLetters [ iID ], &ulAvailableToCaller, &ulDisk, &ulFree ) )
		{
			// set up used values and the final value we need
			ULONGLONG ulUsed = ulDisk.QuadPart - ulFree.QuadPart;
			ULONGLONG ulValue;

			// see if we're going to return free or used data
			if ( iMode == 0 )
				ulValue = ulFree.QuadPart;
			else
				ulValue = ulUsed;

			// convert to mb
			if ( iReturn == 1 )
				ulValue = ulValue / 1024 / 1024;
			
			// convert to gb
			if ( iReturn == 2 )
				ulValue = ulValue / 1024 / 1024 / 1024;

			// return and cast the value to integer
			return ( int ) ulValue;
		}
	}

	// invalid
	return -1;
}

int GetDriveUsedSpace ( int iID, int iReturn )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return GetDriveSpace ( iID, 1, iReturn );
}

int GetDriveFreeSpace ( int iID, int iReturn )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	return GetDriveSpace ( iID, 0, iReturn );
}

int GetDriveFileLengthSupport ( int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	DWORD dwMaxFileLen = 0;
			
	if ( GetVolumeInformation ( g_HardDiskLetters [ iID ], NULL, 0, NULL, &dwMaxFileLen, NULL, NULL, 0 ) )
		return ( int ) dwMaxFileLen;

	return -1;
}

bool ReadFromRegistry ( LPSTR PerfmonNamesKey, LPSTR key, LPSTR* pReturnString )
{
	HKEY hKeyNames = 0;
	DWORD Status;
	char ObjectType[256];

	// Name of key and optiontype
	strcpy(ObjectType,"Num");

	// Try to create it first
	Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PerfmonNamesKey, 0L, KEY_READ, &hKeyNames);
		
	// We got the handle, now use it
	LPSTR pData = NULL;
    if(Status==ERROR_SUCCESS)
	{
		DWORD Size = 256;
		DWORD Type = REG_SZ;
		pData = new char [ Size ];
		memset ( pData, 0, Size );
		Status = RegQueryValueEx(hKeyNames, key, NULL, &Type, (LPBYTE)pData, &Size);
		RegCloseKey(hKeyNames);
	}

	// Return data poiner
	*pReturnString = pData;

	// complete
	return true;
}

DWORD GetDriveSerial ( DWORD dwReturn, int iID, int iUniqueCode )
{
	// temp
	LPSTR pString = new char [ 102400 ];
	char szSerialA [ 256 ] = "";
	char szSerialB [ 256 ] = "";

	// unique or typical
	if ( iUniqueCode==1 )
	{
		// scan registry for processor information
		LPSTR pProcessorIdentifier, pProcessorNameString, pProcessorVendorIdentifier;
		LPSTR pPRKey = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
		ReadFromRegistry ( pPRKey, "Identifier", &pProcessorIdentifier );
		ReadFromRegistry ( pPRKey, "ProcessorNameString", &pProcessorNameString );
		ReadFromRegistry ( pPRKey, "VendorIdentifier", &pProcessorVendorIdentifier );

		// scan registry for harddrive information
		char pHDKey [ 1024 ];
		LPSTR pHardDriveIdentifier [ 5 ][ 8 ][ 2 ];
		memset ( pHardDriveIdentifier, 0, sizeof(pHardDriveIdentifier) );
		for ( int iPort=0; iPort<5; iPort++ )
		{
			for ( int iBus=0; iBus<8; iBus++ )
			{
				for ( int iTarget=0; iTarget<2; iTarget++ )
				{
					// get identifier of hardware device
					wsprintf ( pHDKey, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port %d\\Scsi Bus %d\\Target ID %d\\Logical Unit Id 0", iPort, iBus, iTarget );
					ReadFromRegistry ( pHDKey, "Identifier", &pHardDriveIdentifier[iPort][iBus][iTarget] );
				}
			}
		}

		// build single string with all hardware profile data in
		strcpy ( pString, "" );
		if ( pProcessorIdentifier ) strcat ( pString, pProcessorIdentifier );
		if ( pProcessorNameString ) strcat ( pString, pProcessorNameString );
		if ( pProcessorVendorIdentifier ) strcat ( pString, pProcessorVendorIdentifier );
		for ( int iPort=0; iPort<5; iPort++ )
			for ( int iBus=0; iBus<8; iBus++ )
				for ( int iTarget=0; iTarget<2; iTarget++ )
					if ( pHardDriveIdentifier[iPort][iBus][iTarget] ) strcat ( pString, pHardDriveIdentifier[iPort][iBus][iTarget] );

		// now find MAC address too
		findMACaddress( szSerialA );

		// turn string into unique serial code
		DWORD dwHugeNumber = 0x10000;
		for ( int n=0; n<(int)strlen(pString); n++ )
		{
			dwHugeNumber = dwHugeNumber + pString[n];
		}
		sprintf ( szSerialB, "%X", dwHugeNumber );
		szSerialB[5]=0;
		strcat ( szSerialB, szSerialA );
	}
	else
	{
		if ( !CheckHardDriveID ( iID ) )
			return 0;

		DWORD dwSerialNumber    = 0;
		if ( GetVolumeInformation ( g_HardDiskLetters [ iID ], NULL, 0, &dwSerialNumber, NULL, NULL, NULL, 0 ) )
		{
			// copy the serial number into the buffer
			sprintf ( szSerialA, "%X", dwSerialNumber );

			// now insert the "-" into the middle
			strcpy ( szSerialB, "" );
			memcpy ( szSerialB, szSerialA,               sizeof ( char ) * 4 );
			memcpy ( &szSerialB [ 4 ], "-",              sizeof ( char ) * 1 );
			memcpy ( &szSerialB [ 5 ], &szSerialA [ 4 ], sizeof ( char ) * 4 );

			// end the string
			szSerialB [ 9 ] = 0;
		}
	}

	// free usages
	if ( pString )
	{
		delete pString;
		pString = NULL;
	}

	// return the string
	return ( DWORD ) SetupString ( szSerialB );
}

DWORD GetDriveSerial ( DWORD dwReturn, int iID )
{
	return GetDriveSerial ( dwReturn, iID, 0 );
}

DWORD GetDriveFileSystem ( DWORD dwReturn, int iID )
{
	if ( !CheckHardDriveID ( iID ) )
		return 0;

	char szFS         [ 256 ] = "";
	char szFileSystem [ 256 ] = "";
	
	if ( GetVolumeInformation ( g_HardDiskLetters [ iID ], NULL, 0, NULL, NULL, NULL, szFS, 256 ) )
		sprintf ( szFileSystem, "%s", szFS );
	
	return ( DWORD ) SetupString ( szFileSystem );
}

int GetCDCount ( void )
{
	int iCount = 0;
	
	for ( int iCounter = 0; iCounter < MAX_HARD_DRIVE; iCounter++ )
	{
		if ( GetDriveType ( g_HardDiskLetters [ iCounter] ) == DRIVE_CDROM )
			iCount++;
	}

	return iCount;
}

DWORD GetCDLetter ( DWORD dwReturn, int iNTHCDIndex )
{
	if ( !CheckHardDriveID ( iNTHCDIndex ) )
		return 0;

	/* mikes old code for this
	char szLetter [ 4 ] = "";
	for ( int iCounter = 0; iCounter < MAX_HARD_DRIVE; iCounter++ )
		if ( GetDriveType ( g_HardDiskLetters [ iCounter ] ) == DRIVE_CDROM )
			strcpy ( szLetter, g_HardDiskLetters [ iCounter ] );
	*/

	// default is no letter
	char szLetter [ 4 ] = "";

	// lee - 070406 - u6rc7 - return CD letter corresponding to nth one found
	if ( iNTHCDIndex > 0 )
	{
		int iCountDown = iNTHCDIndex - 1;
		for ( int iCounter = 0; iCounter < MAX_HARD_DRIVE; iCounter++ )
		{
			if ( GetDriveType ( g_HardDiskLetters [ iCounter ] ) == DRIVE_CDROM )
			{
				if ( iCountDown==0 )
				{
					strcpy ( szLetter, g_HardDiskLetters [ iCounter ] );
					break;
				}
				else
					iCountDown--;
			}
		}
	}

	// return string
	return ( DWORD ) SetupString ( szLetter );
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
