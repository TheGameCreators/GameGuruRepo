
#ifndef H_STATESET
#define H_STATESET

#include "..\Entity.h"
#include "..\Hero.h"
#include "..\Path.h"
#include "..\PathFinderAdvanced.h"
#include "..\World.h"
#include "..\Container.h"
#include "..\TeamController.h"

#include <math.h>
#include <stdlib.h>

#define PI 3.14159265f
#define RADTODEG 57.295779513f
#define DEGTORAD 0.01745329252f

class StateSet;

//template
class State
{
public:
	
	//all states derived from this state will have the same world and state machine pointers.
	static World* pWorld;
	static StateSet* pStateSet;

	State ( ) { }
	virtual ~State ( ) { }

	//what an entity should do when it enters this state
	virtual void Enter   ( Entity *pEntity ) = 0;

	//what an entity should do when it leaves this state
	virtual void Exit    ( Entity *pEntity ) = 0;

	//what an entity should do when it is in this state
	virtual void Execute ( Entity *pEntity ) = 0;

	virtual char* GetName ( ) { return "None"; }
};



//contains all available states for this set
class StateSet
{
	class StateManual : public State
	{
	public:
		static int iNumUsers;

		StateManual ( );
		~StateManual ( );
		
		void Enter	 ( Entity *pEntity );
		void Exit	 ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Manual"; }
	};
	
	class StateIdle : public State
	{
	public:
		static int iNumUsers;
		
		StateIdle ( );
		~StateIdle ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Idle"; }
	};

	class StatePatrol : public State
	{
	public:
		static int iNumUsers;
		
		StatePatrol ( );
		~StatePatrol ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Patrol"; }
	};

	class StateInvestigate : public State
	{
	public:
		static int iNumUsers;
		
		StateInvestigate ( );
		~StateInvestigate ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Investigate"; }
	};

	class StateGoToDest : public State
	{
	public:
		static int iNumUsers;

		StateGoToDest ( );
		~StateGoToDest ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Go To Destination"; }
	};

	class StateSearchArea : public State
	{
	public:
		static int iNumUsers;

		StateSearchArea ( );
		~StateSearchArea ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Search Area"; }
	};

	class StateAttack : public State
	{
	public:
		static int iNumUsers;

		StateAttack ( );
		~StateAttack ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Attack"; }
	};

	class StateChaseAttack : public State
	{
	public:
		static int iNumUsers;

		StateChaseAttack ( );
		~StateChaseAttack ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Run and Attack"; }
	};

	class StateStrafeAttack : public State
	{
	public:
		static int iNumUsers;

		StateStrafeAttack ( );
		~StateStrafeAttack ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Strafe and Attack"; }
	};

	class StateDefend : public State
	{
	public:
		static int iNumUsers;

		StateDefend ( );
		~StateDefend ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Defend"; }
	};

	class StateFallBack : public State
	{
	public:
		static int iNumUsers;

		StateFallBack ( );
		~StateFallBack ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Fall Back"; }
	};

	class StateWaitInCover : public State
	{
	public:
		static int iNumUsers;

		StateWaitInCover ( );
		~StateWaitInCover ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Wait In Cover"; }
	};

	class StateRemoveCover : public State
	{
	public:
		static int iNumUsers;

		StateRemoveCover ( );
		~StateRemoveCover ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Reveal"; }
	};

	class StatePeekFromCorner : public State
	{
	public:
		static int iNumUsers;

		StatePeekFromCorner ( );
		~StatePeekFromCorner ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Peeking From Corner"; }
	};

	class StateDiving : public State
	{
	public:
		static int iNumUsers;

		StateDiving ( );
		~StateDiving ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Diving"; }
	};

	class StateLeaping : public State
	{
	public:
		static int iNumUsers;

		StateLeaping ( );
		~StateLeaping ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Leeping"; }
	};

	class StateAttackFromCover : public State
	{
	public:
		static int iNumUsers;

		StateAttackFromCover ( );
		~StateAttackFromCover ( );

		void Enter   ( Entity *pEntity );
		void Exit    ( Entity *pEntity );
		void Execute ( Entity *pEntity );
		char* GetName ( ) { return "Attack From Cover"; }
	};

public:

	StateManual*		pStateManual;

	StateIdle*			pStateIdle;
	StatePatrol*		pStatePatrol;
	StateInvestigate*	pStateInvestigate;
	StateGoToDest*		pStateGoToDest;
	StateSearchArea*	pStateSearchArea;

	StateAttack*		pStateAttack;
	StateChaseAttack*	pStateChaseAttack;
	StateStrafeAttack*	pStateStrafeAttack;
	
	StateDefend*		pStateDefend;
	StateFallBack*		pStateFallBack;
	StateWaitInCover*	pStateWaitInCover;

	StatePeekFromCorner* pStatePeekFromCorner;
	StateDiving*		pStateDiving;
	StateLeaping*		pStateLeaping;
	StateAttackFromCover* pStateAttackFromCover;

	StateSet ( )
	{
		pStateManual		= new StateManual ( );
		
		pStateIdle			= new StateIdle ( );
		pStatePatrol		= new StatePatrol ( );
		pStateInvestigate	= new StateInvestigate ( );
		pStateGoToDest		= new StateGoToDest ( );
		pStateSearchArea	= new StateSearchArea ( );

		pStateAttack		= new StateAttack ( );
		pStateChaseAttack	= new StateChaseAttack ( );
		pStateStrafeAttack	= new StateStrafeAttack ( );
		
		pStateDefend		= new StateDefend ( );
		pStateFallBack		= new StateFallBack ( );
		pStateWaitInCover	= new StateWaitInCover ( );

		pStatePeekFromCorner = new StatePeekFromCorner ( );
		pStateDiving		= new StateDiving();
		pStateLeaping		= new StateLeaping();
		pStateAttackFromCover = new StateAttackFromCover();
	}

	~StateSet ( )
	{
		if ( pStateManual )			delete pStateManual;

		if ( pStateIdle )			delete pStateIdle;
		if ( pStatePatrol )			delete pStatePatrol;
		if ( pStateInvestigate )	delete pStateInvestigate;
		if ( pStateGoToDest )		delete pStateGoToDest;
		if ( pStateSearchArea )		delete pStateSearchArea;

		if ( pStateAttack )			delete pStateAttack;
		if ( pStateChaseAttack )	delete pStateChaseAttack;
		if ( pStateStrafeAttack )	delete pStateStrafeAttack;
		
		if ( pStateDefend )			delete pStateDefend;
		if ( pStateFallBack )		delete pStateFallBack;
		if ( pStateWaitInCover )	delete pStateWaitInCover;

		if ( pStatePeekFromCorner )	delete pStatePeekFromCorner;
		if ( pStateDiving )	delete pStateDiving;
		if ( pStateLeaping )	delete pStateLeaping;
		if ( pStateAttackFromCover )	delete pStateAttackFromCover;
	}
};





#endif