//----------------------------------------------------
//--- GAMEGURU - M-Debug
//----------------------------------------------------

#include "gameguru.h"

// 
//  Debug Routines
// 

void debug_fulldebugview ( void )
{
	SetCurrentCamera (  0 );
	SetCameraRange (  1,70000 );
	SetCursor (  0,10 );
	Print (  "FULL DEBUG VIEW" );
	Print (  "---------------" );
	sprintf ( t.szwork ,  "KEYMAP HARDWARE KEY VALUE = %i" , ScanCode() );
	Print (  t.szwork );
	sprintf ( t.szwork ,  "inputsys.kscancode=%i" , t.inputsys.kscancode );
	Print (  t.szwork );
	sprintf ( t.szwork ,  "GetDisplayWidth()=%i    GetDisplayHeight()=%i" , GetDisplayWidth() , GetDisplayHeight() );
	Print ( t.szwork );
	sprintf ( t.szwork , "Get Child GetWindowWidth()=%i    Height()=%i" , GetChildWindowWidth() , GetChildWindowHeight() );
	Print (  t.szwork );
	//  work out visible part of full backbuffer (i.e. 1212 of 1360)
	t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
	t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
	sprintf ( t.szwork , "(scwidth()+0.0)/(GeWidth()+0.0)=%f" , t.tadjustedtoareax_f );
	Print (  t.szwork );
	sprintf ( t.szwork ,  "(scrh()+0.0)/(Geth()+0.0)=%f" , t.tadjustedtoareay_f );
	Print ( t.szwork );
	//  scale full mouse to fit in visible area
	t.tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/t.tadjustedtoareax_f;
	t.tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/t.tadjustedtoareay_f;
	sprintf ( t.szwork , "((inxmouse+0.0)/800.0)/tatoareax_f=%f" , t.tadjustedtoareax_f );
	Print ( t.szwork );
	sprintf ( t.szwork , "((iymouse+0.0)/600.0)/tadoareay_f=%f" , t.tadjustedtoareay_f );
	Print ( t.szwork );
	//  stretch scaled-mouse to projected -1 to +1
	t.tadjustedtoareax_f=(t.tadjustedtoareax_f*2)-1;
	t.tadjustedtoareay_f=((t.tadjustedtoareay_f*2)-1);
	sprintf ( t.szwork , "(t.tadjustedtoareax_f*2)-1=%f" , t.tadjustedtoareax_f );
	Print ( t.szwork );
	sprintf ( t.szwork , "(t.tadjustedtoareay_f*2)-1=%f" , t.tadjustedtoareay_f );
	Print ( t.szwork );
	sprintf ( t.szwork ,  "inputsys.localx_f=%f" , t.inputsys.localx_f ) ;
	Print ( t.szwork );
	sprintf ( t.szwork , "inputsys.localy_f=%f", t.inputsys.localy_f );
	Print (  t.szwork );
	//  restore camera range
	editor_refreshcamerarange ( );

return;

}

void backuptimestampactivity(void)
{
	// 091215 - to ensure old LOG files are preserved, make a copy of any existing LOG before write a new one
	cstr file_s = "";
	if ( g.gproducelogfiles == 1 )
	{
		cstr logfile_s;
		if ( g.gproducelogfilesdir_s.Len() > 0 )
			logfile_s = g.gproducelogfilesdir_s+"\\"+g.trueappname_s+".log";
		else
			logfile_s = g.fpscrootdir_s+"\\"+g.trueappname_s+".log";
		file_s = logfile_s; 
		if (FileExist(file_s.Get()) == 1)
		{
			cstr filedest_s = g.fpscrootdir_s + "\\" + g.trueappname_s + "-last.log";
			if (FileExist(filedest_s.Get()) == 1)  DeleteAFile(filedest_s.Get());
			CopyFile(file_s.Get(), filedest_s.Get(), TRUE);
		}
	}
}

#define USEAPPENDLOG
#ifdef USEAPPENDLOG
	//PE: Way faster , in a large standalone debug can take upto 2 minutes longer to load the old way.
	#include "stdio.h"
	int debugfilenetries = 0;

#endif

void timestampactivity ( int i, char* desc_s )
{
	cstr videomemsofardesc_s =  "";
	cstr videomemdesc_s =  "";
	cstr memdesc_s =  "";
	cstr tpart1_s =  "";
	cstr tpart2_s =  "";
	cstr tpart3_s =  "";
	cstr file_s =  "";
	int smem = 0;
	int mem = 0;

#ifdef USEAPPENDLOG
	if (g.gproducelogfiles == 1)
	{
		Dim(t.timestampactivity_s, 1);

		file_s = g.fpscrootdir_s + "\\" + g.trueappname_s + ".log";

		if (debugfilenetries == 0) {
			if (FileExist(file_s.Get()) == 1)  DeleteAFile(file_s.Get());
		}

		smem = SMEMAvailable(1);
		memdesc_s = Str((smem - g.timestampactivitymemthen) / 1024);
		memdesc_s = memdesc_s + "MB";
		g.timestampactivitymemthen = smem;
		if (g.gproducetruevidmemreading == 1)
		{
			Sync();
		}
		mem = DMEMAvailable();
		videomemdesc_s = Str((mem - g.timestampactivityvideomemthen)); videomemdesc_s = videomemdesc_s + "MB";
		g.timestampactivityvideomemthen = mem;
		videomemsofardesc_s = " ("; videomemsofardesc_s = videomemsofardesc_s + Str(smem / 1024) + "," + Str(DMEMAvailable()) + ")";
		tpart1_s = Str(Timer()); tpart1_s = tpart1_s + " : " + desc_s + " ";
		tpart2_s = "S:"; tpart2_s = tpart2_s + memdesc_s;
		tpart3_s = "V:"; tpart3_s = tpart3_s + videomemsofardesc_s;
		if (Len(tpart1_s.Get())<64)  tpart1_s = tpart1_s + Spaces(64 - Len(tpart1_s.Get()));
		if (Len(tpart2_s.Get())<8)  tpart2_s = tpart2_s + Spaces(8 - Len(tpart2_s.Get()));
		if (Len(tpart3_s.Get())<16)  tpart3_s = tpart3_s + Spaces(16 - Len(tpart3_s.Get()));
		t.timestampactivity_s[0] = tpart1_s;
		t.timestampactivity_s[0] += tpart2_s;
		t.timestampactivity_s[0] += tpart3_s;

		FILE * pFile;
		pFile = fopen(file_s.Get(), "a+");
		if (pFile != NULL)
		{
			fputs( t.timestampactivity_s[0].Get(), pFile);
			fputs("\n", pFile);
			fclose(pFile);
		}
		debugfilenetries++;
	}
#else
	if ( g.gproducelogfiles == 1 ) 
	{
		if ( i == 0 ) 
		{
			++g.timestampactivityindex;
			if (  g.timestampactivityindex>7999 ) //PE: Increase to 8000 lines.
			{
				g.timestampactivityindex=7999;
			}
			i=g.timestampactivityindex;
		}
		else
		{
			g.timestampactivityindex=i;
		}
		if (  i>g.timestampactivitymax ) 
		{
			g.timestampactivitymax=i;
		}
		Dim (  t.timestampactivity_s,g.timestampactivitymax  );
		smem=SMEMAvailable(1);
		memdesc_s=Str((smem-g.timestampactivitymemthen)/1024);
		memdesc_s = memdesc_s + "MB";
		g.timestampactivitymemthen=smem;
		if (  g.gproducetruevidmemreading == 1 ) 
		{
			Sync (  );
		}
		mem=DMEMAvailable();
		videomemdesc_s=Str((mem-g.timestampactivityvideomemthen)); videomemdesc_s = videomemdesc_s +"MB";
		g.timestampactivityvideomemthen=mem;
		videomemsofardesc_s=" ("; videomemsofardesc_s = videomemsofardesc_s+Str(smem/1024)+","+Str(DMEMAvailable())+")";
		tpart1_s=Str(Timer()); tpart1_s = tpart1_s + " : "+desc_s+" "; 
		tpart2_s="S:"; tpart2_s = tpart2_s+memdesc_s;
		tpart3_s="V:"; tpart3_s = tpart3_s+videomemsofardesc_s;
		if (  Len(tpart1_s.Get())<64  )  tpart1_s = tpart1_s+Spaces(64-Len(tpart1_s.Get()));
		if (  Len(tpart2_s.Get())<8  )  tpart2_s = tpart2_s+Spaces(8-Len(tpart2_s.Get()));
		if (  Len(tpart3_s.Get())<16  )  tpart3_s = tpart3_s+Spaces(16-Len(tpart3_s.Get()));
		t.timestampactivity_s[i]=tpart1_s;
		t.timestampactivity_s[i]+=tpart2_s;
		t.timestampactivity_s[i]+=tpart3_s;
		cstr logfile_s;
		if ( g.gproducelogfilesdir_s.Len() > 0 )
			logfile_s = g.gproducelogfilesdir_s+"\\"+g.trueappname_s+".log";
		else
			logfile_s = g.fpscrootdir_s+"\\"+g.trueappname_s+".log";
		file_s = logfile_s; if (  FileExist(file_s.Get()) == 1  )  DeleteAFile (  file_s.Get() );
		if (  Len(g.trueappname_s.Get())>0 ) 
		{
			OpenToWrite (  13,file_s.Get() );
			//PE: using t.t here can cause tons of problems as t.t is used in so many places.
			//PE: So adding debug to functions can break everything. just use a local.
			//PE: Sorry just spent 4 hours to find a bug , just to find out that the debug logging caused the bugs.
			for ( int localt = 1 ; localt <=  g.timestampactivitymax; localt++ )
			{
				WriteString (  13,t.timestampactivity_s[localt].Get() );
			}
			CloseFile (  13 );
		}
	}
#endif
}

void timestampactivityConsole(int i, char* desc_s)
{
	cstr videomemsofardesc_s = "";
	cstr videomemdesc_s = "";
	cstr memdesc_s = "";
	cstr tpart1_s = "";
	cstr tpart2_s = "";
	cstr tpart3_s = "";
	cstr file_s = "";
	int smem = 0;
	int mem = 0;

	smem = SMEMAvailable(1);
	memdesc_s = Str((smem - g.timestampactivitymemthen) / 1024);
	memdesc_s = memdesc_s + "MB";
	g.timestampactivitymemthen = smem;
	if (g.gproducetruevidmemreading == 1)
	{
		Sync();
	}
	mem = DMEMAvailable();
	videomemdesc_s = Str((mem - g.timestampactivityvideomemthen)); videomemdesc_s = videomemdesc_s + "MB";
	g.timestampactivityvideomemthen = mem;
	videomemsofardesc_s = " ("; videomemsofardesc_s = videomemsofardesc_s + Str(smem / 1024) + "," + Str(DMEMAvailable()) + ")";
	tpart1_s = Str(Timer()); tpart1_s = tpart1_s + " : " + desc_s + " ";
	tpart2_s = "S:"; tpart2_s = tpart2_s + memdesc_s;
	tpart3_s = "V:"; tpart3_s = tpart3_s + videomemsofardesc_s;
	if (Len(tpart1_s.Get())<64)  tpart1_s = tpart1_s + Spaces(64 - Len(tpart1_s.Get()));
	if (Len(tpart2_s.Get())<8)  tpart2_s = tpart2_s + Spaces(8 - Len(tpart2_s.Get()));
	if (Len(tpart3_s.Get())<16)  tpart3_s = tpart3_s + Spaces(16 - Len(tpart3_s.Get()));
	t.timestampactivity_s[0] = tpart1_s;
	t.timestampactivity_s[0] += tpart2_s;
	t.timestampactivity_s[0] += tpart3_s;
	OutputDebugString(t.timestampactivity_s[0].Get());
	OutputDebugString("\n");
}


