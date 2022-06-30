//----------------------------------------------------
//--- GAMEGURU - Common-File
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Common File Functions
// 

void file_createmydocsfolder ( void )
{
	//  create a GameGuru folder inside MyDocs
	t.told_s=GetDir();
	SetDir (  g.mydocumentsdir_s.Get() );
	if (  PathExist(g.myfpscfiles_s.Get()) == 0  )  MakeDirectory (  g.myfpscfiles_s.Get() );
	SetDir (  t.told_s.Get() );
	if (  PathExist(g.myownrootdir_s.Get()) == 0 ) 
	{
		sprintf ( t.szwork , "Cannot create :%s" , g.myownrootdir_s.Get() );
		ExitPrompt ( t.szwork , "Path Error" );
		ExitProcess ( 0 );
	}
}

int getdatevaluefromdatestring ( char* tdayofyear_s )
{
	cstr dayofyear_s = tdayofyear_s;
	int filedatevalue = 0;
	cstr month_s =  "";
	cstr date_s =  "";
	cstr time_s =  "";
	cstr year_s = "";
	int month = 0;
	int c;
	//  get year
	year_s=Right(dayofyear_s.Get(),4);
	dayofyear_s=Left(dayofyear_s.Get(),Len(dayofyear_s.Get())-5);
	//  eat day
	for ( c = 1 ; c<= Len(dayofyear_s.Get()); c++ )
	{
		if (  cstr(Mid(dayofyear_s.Get(),c)) == " " ) 
		{
			dayofyear_s=Right(dayofyear_s.Get(),Len(dayofyear_s.Get())-c);
			break;
		}
	}
	//  find month
	for ( c = 1 ; c<= Len(dayofyear_s.Get()); c++ )
	{
		if (  cstr(Mid(dayofyear_s.Get(),c)) == " " ) 
		{
			month_s=Lower(Left(dayofyear_s.Get(),c-1));
			dayofyear_s=Right(dayofyear_s.Get(),Len(dayofyear_s.Get())-c);
			break;
		}
	}
	// 041215 - eat any spaces
	for (c = 1; c <= Len(dayofyear_s.Get()); c++)
	{
		if (cstr(Mid(dayofyear_s.Get(), c)) != " ")
		{
			break;
		}
	}
	dayofyear_s = Right(dayofyear_s.Get() + (c-1), strlen(dayofyear_s.Get()) - (c-1));
	//  find date
	for ( c = 1 ; c<= Len(dayofyear_s.Get()); c++ )
	{
		if ( cstr( Mid(dayofyear_s.Get(),c) ) == " " ) 
		{
			date_s=Lower(Left(dayofyear_s.Get(),c-1));
			dayofyear_s=Right(dayofyear_s.Get(),Len(dayofyear_s.Get())-c);
			break;
		}
	}
	//  get time
	time_s="";
	for ( c = 1 ; c<= Len(dayofyear_s.Get()); c++ )
	{
		if (  cstr(Mid(dayofyear_s.Get(),c)) != ":" )
		{
			time_s=time_s+Mid(dayofyear_s.Get(),c);
		}
	}
	time_s=Left(time_s.Get(),Len(time_s.Get())-2);
	if (  month_s == "jan"  )  month = 1;
	if (  month_s == "feb"  )  month = 2;
	if (  month_s == "mar"  )  month = 3;
	if (  month_s == "apr"  )  month = 4;
	if (  month_s == "may"  )  month = 5;
	if (  month_s == "jun"  )  month = 6;
	if (  month_s == "jul"  )  month = 7;
	if (  month_s == "aug"  )  month = 8;
	if (  month_s == "sep"  )  month = 9;
	if (  month_s == "oct"  )  month = 10;
	if (  month_s == "nov"  )  month = 11;
	if (  month_s == "dec"  )  month = 12;
	int iAddTime = ValF(time_s.Get());
	filedatevalue=((ValF(year_s.Get())-2004)*12*32*2500)+(month*32*2500)+(ValF(date_s.Get())*2500) + iAddTime;
	return filedatevalue;
}

int isbinaryfileolderthantxtfile ( char* binaryfilefull_s, char* textfilefull_s )
{
	int filedatevalueB = 0;
	int filedatevalueT = 0;
	cstr binaryfile_s =  "";
	cstr dayofyearB_s =  "";
	cstr dayofyearT_s =  "";
	cstr tthispath_s =  "";
	cstr textfile_s =  "";
	cstr stdir_s =  "";
	int result = 0;
	cstr this_s =  "";
	result=0;
	if (  FileExist(binaryfilefull_s) == 1 ) 
	{
		stdir_s=GetDir();
		binaryfile_s=getfile(binaryfilefull_s);
		textfile_s=getfile(textfilefull_s);
		tthispath_s=getpath(binaryfilefull_s);
		if (  PathExist(tthispath_s.Get()) == 1 ) 
		{
			SetDir (  tthispath_s.Get() );
			filedatevalueB=0;
			filedatevalueT=0;
			FindFirst (  );
			while (  GetFileType() != -1 ) 
			{
				if (  GetFileType() == 0 ) 
				{
					if (  cstr(Lower(GetFileName())) == cstr(Lower(binaryfile_s.Get())) ) 
					{
						this_s=GetFileDate();
						dayofyearB_s=this_s;
						filedatevalueB=getdatevaluefromdatestring(dayofyearB_s.Get());

						// 041215 - pick creation date if newer
						cstr creation_date_s = GetFileCreation();
						int filedatevalueC = getdatevaluefromdatestring(creation_date_s.Get());
						if (filedatevalueC > filedatevalueB ) filedatevalueB = filedatevalueC;
					}
					else
					{
						if (  cstr(Lower(GetFileName())) == cstr(Lower(textfile_s.Get())) )  
						{
							// get number representing file date and time
							this_s = GetFileDate();
							dayofyearT_s = this_s;
							filedatevalueT = getdatevaluefromdatestring(dayofyearT_s.Get());

							// 021215 - pick creation date if newer
							cstr creation_date_s = GetFileCreation();
							int filedatevalueC = getdatevaluefromdatestring(creation_date_s.Get());
							if ( filedatevalueC > filedatevalueT ) filedatevalueT = filedatevalueC;
						}
					}
					if (  filedatevalueB != 0 && filedatevalueT != 0  )  break;
				}
				FindNext (  );
			}
			if (  filedatevalueB != 0 && filedatevalueT != 0 ) 
			{
				if (  filedatevalueB<filedatevalueT ) 
				{
					result=1;
				}
			}
		}
		SetDir (  stdir_s.Get() );
	}
//endfunction result
	return result
;
}

int isbinaryfileolderthantxtfileforseg ( char* binaryfilefull_s, char* textfilefull_s )
{
	int filedatevalueB = 0;
	int filedatevalueT = 0;
	cstr binaryfile_s =  "";
	cstr dayofyearB_s =  "";
	cstr dayofyearT_s =  "";
	cstr textfile_s =  "";
	int tversion = 0;
	cstr stdir_s =  "";
	int result = 0;
	cstr this_s =  "";
	int tval = 0;
	//  segment BIN files have version number signalled with a -1 in the first field
	result=0;
	if (  FileExist(binaryfilefull_s) == 1 ) 
	{
		stdir_s=GetDir();
		binaryfile_s=getfile(binaryfilefull_s);
		textfile_s=getfile(textfilefull_s);
		SetDir (  getpath(binaryfilefull_s) );
		filedatevalueB=0;
		filedatevalueT=0;
		FindFirst (  );
		while (  GetFileType() != -1 ) 
		{
			if (  GetFileType() == 0 ) 
			{
				if (  cstr(Lower(GetFileName())) == cstr(Lower(binaryfile_s.Get())) ) 
				{
					this_s=GetFileDate();
					dayofyearB_s=this_s;
					filedatevalueB=getdatevaluefromdatestring(dayofyearB_s.Get());
				}
				else
				{
					if (  cstr(Lower(GetFileName())) == cstr(Lower(textfile_s.Get())) )
					{
						this_s=GetFileDate();
						dayofyearT_s=this_s;
						filedatevalueT=getdatevaluefromdatestring(dayofyearT_s.Get());
					}
				}
				if (  filedatevalueB != 0 && filedatevalueT != 0  )  break;
			}
			FindNext (  );
		}
		if (  filedatevalueB != 0 && filedatevalueT != 0 ) 
		{
			if (  filedatevalueB<filedatevalueT ) 
			{
				result=1;
			}
		}
		SetDir (  stdir_s.Get() );
		if (  result == 0 ) 
		{
			//  V110 - 160508 - added extra code to hold new version number in segment BIN file
			OpenToRead (  3,binaryfilefull_s );
				tval = ReadLong (  3 );
				if (  tval == -1 ) 
				{
					tversion = ReadLong (  3 );
					//  if BIN created with older version, trigger delete BIN file
					result = versionnumberislarger(g.gversion,tversion);
				}
				else
				{
					//  if an old BIN, also delete it
					result=1;
				}
			CloseFile (  3 );
		}
	}
//endfunction result
	return result
;
}

int isbinaryfileolderthanglobalversion ( char* binaryfilefull_s )
{
	int tversion = 0;
	int result = 0;
	cstr tval_s =  "";
	result=0;
	if (  FileExist(binaryfilefull_s) == 1 ) 
	{
		OpenToRead (  3,binaryfilefull_s );
		tval_s = ReadString ( 3 );
		if (  cstr(Lower(tval_s.Get())) == "version" ) 
		{
			tversion = ReadLong (  3 );
//    `timestampactivity(0,"tversion from BIN file="+Str(tversion))

			//  if BIN created with older version, trigger delete BIN file
//    `timestampactivity(0,"current gentityprofileversion="+Str(gentityprofileversion))

			result = versionnumberislarger(g.gentityprofileversion,tversion);
//    `timestampactivity(0,"resultislarger?="+Str(result))

			if (  result == 1 ) 
			{
				sprintf ( t.szwork , "refreshing BIN file for:%s (from %i to %i)" , binaryfilefull_s , tversion , g.gentityprofileversion );
				timestampactivity(0, t.szwork );
			}
		}
		else
		{
			//  if an old BIN, also delete it
			result=1;
		}
		CloseFile (  3 );
	}
//endfunction result
	return result
;
}

int versionnumberislarger ( int tNum1, int tNum2 )
{
	int tChar1Val = 0;
	int tChar2Val = 0;
	int tMaxChars = 0;
	cstr tNum1_s =  "";
	cstr tNum2_s =  "";
	int tLen1 = 0;
	int tLen2 = 0;
	int tPos = 0;

	//  returns 1 if the version representation of num1 is larger than num2.
	//  NOTE; This will only work for values with a first value to a maximum of 9.
	//  For example; 2.01.232 is fine. 9.01.006 is fine, but 10.02.005 will be smaller than 4.02.005
	//  and at this GetPoint (  this function will return an incorrect result. )
	tNum1_s = Str(tNum1);
	tLen1 = Len(tNum1_s.Get());
	tNum2_s = Str(tNum2);
	tLen2 = Len(tNum2_s.Get());

	//  get the num of chars in the shortest string
	if (  tLen1 > tLen2 ) 
	{
		tMaxChars = tLen2;
	}
	else
	{
		tMaxChars = tLen1;
	}

	//  step through each char and compare the value
	for ( tPos = 1 ; tPos <= tMaxChars; tPos++ )
	{
		tChar1Val = ValF(Mid(tNum1_s.Get(),tPos));
		tChar2Val = ValF(Mid(tNum2_s.Get(),tPos));
		if (  tChar1Val > tChar2Val  )  return 1;
		if (  tChar1Val < tChar2Val  )  return 0;
	}

	//  if (  we get here,  )  the value is the same in each string, up until the max character count.
	//  if (  string 1 has more characters,  )  it will be larger
	if (  tLen1 > tLen2  )  return 1;

//endfunction 0
	return 0
;
}

void deleteOutOfDateDBO ( char* tfile_s )
{
	cstr tfiledbo_s =  "";
	cstr tfilex_s =  "";

	//  takes a file name and deletes the DBO version of this filename if the .X exists and the .DBO is older
	if (  strcmp ( Lower(Right(tfile_s,2)) , ".x" ) == 0 ) 
	{
		tfilex_s=tfile_s;
		tfiledbo_s=Left(tfile_s,Len(tfile_s)-2);
		tfiledbo_s += ".dbo";
	}
	else
	{
		if (  cstr(Lower(Right(tfile_s,4))) == ".dbo" ) 
		{
			tfiledbo_s=tfile_s;
			tfilex_s=Left(tfile_s,Len(tfile_s)-4);
			tfilex_s += ".x";
		}
		else
		{
			return;
		}
	}
	if ( FileExist(tfilex_s.Get()) == 1 && FileExist(tfiledbo_s.Get()) == 1 ) 
	{
		if ( isbinaryfileolderthantxtfile(tfiledbo_s.Get(),tfilex_s.Get()) == 1 )  
		{
			DeleteAFile (  tfiledbo_s.Get() );
		}
	}
}

