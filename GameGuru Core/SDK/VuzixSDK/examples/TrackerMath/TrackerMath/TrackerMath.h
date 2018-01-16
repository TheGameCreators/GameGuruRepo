//=====================================================================================================
// TrackerMath.h
//=====================================================================================================
//
// Collected implementation of Mahony's and Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=====================================================================================================
// 
// This file is part of TrackerMath.
//
//    TrackerMath is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    TrackerMath is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with TrackerMath.  If not, see <http://www.gnu.org/licenses/>.
//
//======================================================================================================
#ifndef TRACKERMATH_H
#define TRACKERMATH_H

//----------------------------------------------------------------------------------------------------
// Variable declaration

// Madgwick
extern volatile float beta;				// algorithm gain
extern volatile float sampleFreqMadgwick;

// Mahony
extern volatile float twoKp;			// 2 * proportional gain (Kp)
extern volatile float twoKi;			// 2 * integral gain (Ki)
extern volatile float sampleFreqMahoney;

//---------------------------------------------------------------------------------------------------
// Function declarations
#ifndef TRACKMATH_EXPLICIT
#ifdef TRACKMATH_EXPORTS
__declspec( dllexport ) void __cdecl MadgwickGetEulerAngles(float *Roll, float *Pitch, float *Yaw);
__declspec( dllexport ) void __cdecl MadgwickSetFromAxes(float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllexport ) void __cdecl MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllexport ) void __cdecl MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

__declspec( dllexport ) void __cdecl MahonyGetEulerAngles(float *Roll, float *Pitch, float *Yaw);
__declspec( dllexport ) void __cdecl MahonyGetEulerAnglesOffset(float *Roll, float *Pitch, float *Yaw);
__declspec( dllexport ) void __cdecl MahonySetFromAxes(float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllexport ) void __cdecl MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllexport ) void __cdecl MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

__declspec( dllexport ) void __cdecl MahonySetCorrectionFactor(float Kp, float Ki);
__declspec( dllexport ) void __cdecl MahonySetSampleFreq(float freq);
__declspec( dllexport ) void __cdecl MadgwickSetBeta(float b);
__declspec( dllexport ) void __cdecl MadgwickSetSampleFreq(float freq);

#else

__declspec( dllimport ) extern void	MadgwickGetEulerAngles(float *Roll, float *Pitch, float *Yaw);
__declspec( dllimport ) extern void	MadgwickSetFromAxes(float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllimport ) extern void	MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllimport ) extern void	MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

__declspec( dllimport ) extern void	MahonyGetEulerAngles(float *Roll, float *Pitch, float *Yaw);
__declspec( dllimport ) extern void	MahonyGetEulerAnglesOffset(float *Roll, float *Pitch, float *Yaw);
__declspec( dllimport ) extern void	MahonySetFromAxes(float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllimport ) extern void	MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
__declspec( dllimport ) extern void	MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

__declspec( dllexport ) extern void MahonySetCorrectionFactor(float Kp, float Ki);
__declspec( dllexport ) extern void MahonySetSampleFreq(float Kp);
__declspec( dllexport ) extern void MadgwickSetBeta(float b);
__declspec( dllexport ) extern void MadgwickSetSampleFreq(float freq);
#endif
#else

typedef DWORD	(__cdecl *PMADGWICKGETEULERANGLES)( float *, float *, float * );
typedef DWORD	(__cdecl *PMADGWICKSETFROMAXES)(float, float, float, float, float, float);
typedef DWORD	(__cdecl *PMADGWICKAHRSUPDATE)(float, float, float, float, float, float, float, float, float);
typedef DWORD	(__cdecl *PMADGWICKAHRSUPDATEIMU)(float, float, float, float, float, float);

typedef DWORD	(__cdecl *PMAHONYGETEULERANGLES)(float *, float *, float *);
typedef DWORD	(__cdecl *PMAHONYGETEULERANGLESOFFSET)(float *, float *, float *);
typedef DWORD	(__cdecl *PMAHONYSETFROMAXES)(float, float, float, float, float, float);
typedef DWORD	(__cdecl *PMAHONYAHRSUPDATE)(float, float, float, float, float, float, float, float, float);
typedef DWORD	(__cdecl *PMAHONYAHRSUPDATEIMU)(float, float, float, float, float, float);

typedef DWORD   (__cdecl *PMAHONYSETCORRECTIONFACTOR)(float, float);
typedef DWORD   (__cdecl *PMAHONYSETSAMPLEFREQ)(float);
typedef DWORD   (__cdecl *PMADGWICKSETBETA)(float);
typedef DWORD   (__cdecl *PMADGWICKSETSAMPLEFREQ)(float);

#ifdef TRACKMATH_ONETIME_DEFINITIONS
	HMODULE						TrackerMathInstance=NULL;
	PMADGWICKGETEULERANGLES		MadgwickGetEulerAngles=NULL;
	PMADGWICKSETFROMAXES		MadgwickSetFromAxes=NULL;
	PMADGWICKAHRSUPDATE			MadgwickAHRSupdate=NULL;
	PMADGWICKAHRSUPDATEIMU		MadgwickAHRSupdateIMU=NULL;

	PMAHONYGETEULERANGLES		MahonyGetEulerAngles=NULL;
	PMAHONYGETEULERANGLESOFFSET	MahonyGetEulerAnglesOffset=NULL;
	PMAHONYSETFROMAXES			MahonySetFromAxes=NULL;
	PMAHONYAHRSUPDATE			MahonyAHRSupdate=NULL;
	PMAHONYAHRSUPDATEIMU		MahonyAHRSupdateIMU=NULL;

	PMAHONYSETCORRECTIONFACTOR	MahonySetCorrectionFactor=NULL;
	PMAHONYSETSAMPLEFREQ		MahonySetSampleFreq=NULL;
	PMADGWICKSETBETA			MadgwickSetBeta=NULL;
	PMADGWICKSETSAMPLEFREQ		MadgwickSetSampleFreq=NULL;
#else
	extern HMODULE						TrackerMathInstance;
	extern PMADGWICKGETEULERANGLES		MadgwickGetEulerAngles;
	extern PMADGWICKSETFROMAXES			MadgwickSetFromAxes;
	extern PMADGWICKAHRSUPDATE			MadgwickAHRSupdate;
	extern PMADGWICKAHRSUPDATEIMU		MadgwickAHRSupdateIMU;

	extern PMAHONYGETEULERANGLES		MahonyGetEulerAngles;
	extern PMAHONYGETEULERANGLESOFFSET	MahonyGetEulerAnglesOffset;
	extern PMAHONYSETFROMAXES			MahonySetFromAxes;
	extern PMAHONYAHRSUPDATE			MahonyAHRSupdate;
	extern PMAHONYAHRSUPDATEIMU			MahonyAHRSupdateIMU;
	
	extern PMAHONYSETCORRECTIONFACTOR	MahonySetCorrectionFactor;
	extern PMAHONYSETSAMPLEFREQ			MahonySetSampleFreq;
	extern PMADGWICKSETBETA				MadgwickSetBeta;
	extern PMADGWICKSETSAMPLEFREQ		MadgwickSetSampleFreq;
#endif
#endif

#endif