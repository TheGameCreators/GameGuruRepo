#ifndef _CSOUNDDATA_H_
#define _CSOUNDDATA_H_

#include "directx-macros.h"
#include "dsutil.h"

struct sSoundData
{
	CSound*				        pSound;
	IDirectSound3DBuffer8*		pDSBuffer3D;

	bool						b3D;
	bool						bPlaying;
	bool						bLoop;
	bool						bPause;
	int							iVolume;
	int							iPan;
	bool						bRecorded;

	int							iLoopStartPos;
	int							iLoopEndPos;

	GGVECTOR3					vecPosition;
	GGVECTOR3					vecLast;

	sSoundData ( )
	{
		pSound	     = NULL;
		pDSBuffer3D  = NULL;

		b3D     = false;
		bLoop   = false;
		bPause  = false;
		iVolume = 0;
		iPan    = 0;
		bRecorded = false;

		vecPosition         = GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
		vecLast				= GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
	}

	~sSoundData ( )
	{
		if(pSound) { delete pSound; pSound=NULL; }
		if(pDSBuffer3D) { pDSBuffer3D->Release(); pDSBuffer3D=NULL; }
	}
};

#endif _CSOUNDDATA_H_