//=====================================================================================================
// MahonyAHRS.c
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author			Notes
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

//---------------------------------------------------------------------------------------------------
// Header files

//#include "MahonyAHRS.h"
#include "TrackerMath.h"
#include <math.h>

//---------------------------------------------------------------------------------------------------
// Definitions

  // This is better as a define as it saves math on each pass.   
  // During development I use a variable to change back and 
  // forth between trackers
#define sampleFreqDef	100.0f			// sample frequency in Hz
#define twoKpDef	(2.0f * 0.5f)	// 2 * proportional gain
#define twoKiDef	(2.0f * 0.0f)	// 2 * integral gain

//---------------------------------------------------------------------------------------------------
// Variable definitions

volatile float twoKp = twoKpDef;											// 2 * proportional gain (Kp)
volatile float twoKi = twoKiDef;											// 2 * integral gain (Ki)
volatile float sampleFreqMahoney = sampleFreqDef;
static volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;					// quaternion of sensor frame relative to auxiliary frame
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;	// integral error terms scaled by Ki

//---------------------------------------------------------------------------------------------------
// Function declarations

float invSqrt(float x);

void MahonySetCorrectionFactor( float Kp, float Ki ){
	twoKp = 2*Kp;
	twoKi = 2*Ki;
}

void MahonySetSampleFreq( float freq ){
	sampleFreqMahoney = freq;
}

// RWR:
static void getAngles( float Q0, float Q1, float Q2, float Q3, float *Roll, float *Pitch, float *Yaw) {
	*Yaw = atan2(2 * (Q0 * Q3 + Q1 * Q2), 1 - (2 * (Q2 * Q2 + Q3 * Q3)));
	*Pitch = asin(2 * (Q0 * Q2 - Q3 * Q1));
	*Roll = atan2(2 * (Q0 * Q1 + Q2 * Q3), 1 - 2 * (Q1 * Q1 + Q2 * Q2));

}
void MahonyGetEulerAngles(float *Roll, float *Pitch, float *Yaw) {
	getAngles (q0, q1, q2, q3, Roll, Pitch, Yaw);
}

quatProduct(float a0, float a1, float a2, float a3, 
			float b0, float b1, float b2, float b3, 
			float *o0, float *o1, float *o2, float *o3) {
	float recipNorm;
	float t0, t1, t2, t3;

	t0 = a0 * b0 - a1 * b1 - a2 * b2 - a3 * b3;
	t1 = a0 * b1 + a1 * b0 + a2 * b3 - a3 * b2;
	t2 = a0 * b2 - a1 * b3 + a2 * b0 + a3 * b1;
	t3 = a0 * b3 + a1 * b2 - a2 * b1 + a3 * b0;

	// renormalise quaternion
	recipNorm = invSqrt(t0 * t0 + t1 * t1 + t2 * t2 + t3 * t3);
	t0 *= recipNorm;
	t1 *= recipNorm;
	t2 *= recipNorm;
	t3 *= recipNorm;
	
	*o0 = t0;
	*o1 = t1;
	*o2 = t2;
	*o3 = t3;	
}

void MahonyGetEulerAnglesOffset(float *Roll, float *Pitch, float *Yaw) {
	float halfangle;
	float recipNorm;
	float o0, o1, o2, o3;	// offset angle.

	// The offset of the wrap tracker from the actual front is 20°.  
	halfangle = -20.0f * 3.14159 / 180 / 2.0f; 

	o0 = cos(halfangle);
	o1 = 0.0f;
	o2 = 0.0f;
	o3 = -1 * sin(halfangle);

	recipNorm = invSqrt(o0 * o0 + o1 * o1 + o2 * o2 + o3 * o3);
	o0 *= recipNorm;
	o1 *= recipNorm;
	o2 *= recipNorm;
	o3 *= recipNorm;

//	quatProduct(o0, o1, o2, o3, q0, q1, q2, q3, &o0, &o1, &o2, &o3);
	quatProduct(q0, q1, q2, q3, o0, o1, o2, o3, &o0, &o1, &o2, &o3);

	getAngles (o0, o1, o2, o3, Roll, Pitch, Yaw);
}


// RWR:
/// Current version assumes device is roughly level on surface and only uses magnetic axes.
/// This only bothers getting close.
void MahonySetFromAxes(float ax, float ay, float az, float mx, float my, float mz) {
	float recipNorm;
	float halfAngle;

	// The assumption that the sensor is roughly level makes this easier.
	// The only rotation we care about is around Z Assumption includes Theta = 0 so q1 and q2 all zero out.

	halfAngle = atan2(my, mx)/2; 
	q0 = cos(halfAngle);
	q1 = 0.0f;
	q2 = 0.0f;
	q3 = -1 * sin(halfAngle);

	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

//====================================================================================================
// Functions

//---------------------------------------------------------------------------------------------------
// AHRS algorithm update

void MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float recipNorm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;  
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MahonyAHRSupdateIMU(gx, gy, gz, ax, ay, az);
		return;
	}

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;     

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;   

        // Auxiliary variables to avoid repeated arithmetic
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;   

        // Reference direction of Earth's magnetic field
        hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
        hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
        bx = sqrt(hx * hx + hy * hy);
        bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
        halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
        halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
        halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);  
	
		// Error is sum of cross product between estimated direction and measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			integralFBx += twoKi * halfex * (1.0f / sampleFreqMahoney);	// integral error scaled by Ki
			integralFBy += twoKi * halfey * (1.0f / sampleFreqMahoney);
			integralFBz += twoKi * halfez * (1.0f / sampleFreqMahoney);
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		}
		else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}
	
	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / sampleFreqMahoney));		// pre-multiply common factors
	gy *= (0.5f * (1.0f / sampleFreqMahoney));
	gz *= (0.5f * (1.0f / sampleFreqMahoney));
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx); 
	
	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

//---------------------------------------------------------------------------------------------------
// IMU algorithm update

void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
	float recipNorm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;        

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = q1 * q3 - q0 * q2;
		halfvy = q0 * q1 + q2 * q3;
		halfvz = q0 * q0 - 0.5f + q3 * q3;
	
		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			integralFBx += twoKi * halfex * (1.0f / sampleFreqMahoney);	// integral error scaled by Ki
			integralFBy += twoKi * halfey * (1.0f / sampleFreqMahoney);
			integralFBz += twoKi * halfez * (1.0f / sampleFreqMahoney);
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		}
		else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}
	
	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / sampleFreqMahoney));		// pre-multiply common factors
	gy *= (0.5f * (1.0f / sampleFreqMahoney));
	gz *= (0.5f * (1.0f / sampleFreqMahoney));
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx); 
	
	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

//---------------------------------------------------------------------------------------------------
// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root

static float invSqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

//====================================================================================================
// END OF CODE
//====================================================================================================
