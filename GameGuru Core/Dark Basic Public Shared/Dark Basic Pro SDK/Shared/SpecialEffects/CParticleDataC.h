#ifndef _CPARTICLEDATA_H_
#define _CPARTICLEDATA_H_

#include "cpositionc.h"

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>   
#include <windowsx.h>

// vertex structure for particles
struct POINTVERTEX
{
	GGVECTOR3		v;			// vector
	GGCOLOR			color;		// colour
};

// particle data
struct PARTICLE
{
	BOOL			m_bSpark;     // use sparks?
	
	GGVECTOR3		m_vPos;       // current position
	GGVECTOR3		m_vVel;       // current velocity

	GGVECTOR3		m_vPos0;      // initial position
	GGVECTOR3		m_vVel0;      // initial velocity
	FLOAT			m_fTime0;     // time of creation

	GGCOLOR			m_clrDiffuse; // initial diffuse color
	GGCOLOR			m_clrFade;    // faded diffuse color
	FLOAT			m_fFade;      // fade progression

	PARTICLE*		m_pNext;      // next particle in list
};

// object data
struct tagParticleData : public tagObjectPos
{
	LPGGDEVICE		pD3D;				// pointer to d3d device

	FLOAT					m_fRadius;			// radius

	DWORD					m_dwBase;			// base
	DWORD					m_dwFlush;			// flush
	DWORD					m_dwDiscard;		// discard

	DWORD					m_dwParticles;		// num of particles
	DWORD					m_dwParticlesLim;	// max num of particles
	PARTICLE*				m_pParticles;		// pointer to particle data
	PARTICLE*				m_pParticlesFree;	// particles

	LPGGVERTEXBUFFER		m_pVB;				// vertex buffer

	LPGGTEXTURE				m_pTexture;

	float					fSecsPerFrame;
	DWORD					dwNumParticlesToEmit;
	float					fEmitVel;
	GGVECTOR3				vPosition;
	GGCOLOR					EmitColor;

	float					fTime;

	bool					m_bEffect;

	bool					m_bVisible;
	float					m_fGravity;
	int						m_iLife;
	int						m_iFloorActive;
	float					m_fChaos;

	bool					m_bGhost;
	int						m_iGhostMode;
	float					m_fGhostPercentage;
};


#endif _CPARTICLEDATA_H_