#include "cThread.h"
#include "SoftwareCulling.h"

extern bool g_bEarlyExcludeMode;
extern void DoEarlyExclude ( void );

class cOccluderThread : public cThread
{
	public:
		cOccluderThread ( )
		{
		}

		~cOccluderThread ( )
		{
		}

		unsigned int Run ( )
		{
			while ( 1 )
			{
				//Dave Performance - wait until we are told to begin
				if ( g_hOccluderBegin ) WaitForSingleObject ( g_hOccluderBegin, INFINITE );
				//Do some occluding!
				CPU3DDoOcclude ();
				//Set the event to let everyone know we have finished
				//if ( g_hOccluderEnd ) SetEvent ( g_hOccluderEnd );
			}

			return 1;
		}
};
