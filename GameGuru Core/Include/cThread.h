#pragma once

class cThread
{
	public:

		cThread( );
		virtual ~cThread( );

		bool IsRunning();
		virtual void Start( );
		virtual void Stop();
		void SleepSafe( unsigned int milliseconds );
		void Join( );

		inline static unsigned int __stdcall EntryPoint( void *pParams );

		void Finally();

	protected:

		volatile bool m_bTerminate;
		void* m_pStop; 

		virtual unsigned int Run( ) = 0;

	//private:
public:
		void* pThread;
		unsigned int iThreadID;
		volatile bool m_bRunning;

		void PlatformInit();
		void PlatformStart();
		void PlatformStop();
		void PlatformSleepSafe( unsigned int milliseconds );
		void PlatformJoin();
		void PlatformTerminate();
		void PlatformCleanUp();
};

