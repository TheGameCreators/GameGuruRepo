//----------------------------------------------------
//--- GAMEGURU - Common-Strings
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Common Code - String and FileList Functions
// 


//Strings

// we will use this to work with these functions
cstr returntext_s =  "";

char* removeedgespaces ( char* text_s )
{
	returntext_s =  "";
	int en = 0;
	int st;
	int c;
	returntext_s="";
	if (  Len(text_s)>0 ) 
	{
		st=0;
		for ( c = 0 ; c< Len(text_s); c++ )
		{
			if (  text_s[c] >32 ) { st = c+1; break; }
		}
		if (  st == 0 ) 
		{
			//  all sub-spaces
		}
		else
		{
			for ( c = Len(text_s)-1 ; c>= 0 ; c+= -1 )
			{
				if ( text_s[c] >32 ) { en = c+1; break; }
			}
			cstr text = text_s;
			for ( c = st ; c<=  en; c++ )
			{
				//returntext_s=returntext_s+Mid(text_s,c);
				returntext_s=returntext_s+text.Mid(c-1);
			}
		}
	}
//endfunction returntext$
	strcpy ( t.szwork , returntext_s.Get() );
	return t.szwork;
}

char* minstring ( char* text_s, int minsize )
{
	returntext_s = "";
	int sizegap = 0;
	sizegap=minsize-Len(text_s);
	returntext_s = text_s;
	returntext_s = returntext_s + Spaces(sizegap);
//endfunction text$
	strcpy ( t.szwork , returntext_s.Get() );
	return t.szwork;}


//FUNCTIONS TO CUT-UP STRINGS


char* getfirstdir ( char* file_s )
{
	int c = 0;
	cstr firstdir_s =  "";
	firstdir_s="";
	for ( c = 1 ; c<=  Len(file_s); c++ )
	{
		if (  cstr(Mid(file_s,c)) == "\\" || cstr(Mid(file_s,c)) == "/" ) 
		{
			firstdir_s=Left(file_s,c) ; break;
		}
	}
//endfunction firstdir$
	strcpy ( t.szwork , firstdir_s.Get() );
	return t.szwork;
}

char* getseconddir ( char* file_s )
{
	int c = 0;
	cstr firstdir_s =  "";
	int count = 0;
	firstdir_s="" ; count=2;
	for ( c = 1 ; c<= Len(file_s); c++ )
	{
		if (  cstr(Mid(file_s,c)) == "\\" || cstr(Mid(file_s,c)) == "/" ) 
		{
			--count;
			if ( count == 0 ) 
			{
				firstdir_s=Left(file_s,c) ; break;
			}
		}
	}
//endfunction firstdir$
	strcpy ( t.szwork , firstdir_s.Get() );
	return t.szwork;
}

char* getpath ( char* file_s )
{
	int c;
	cstr path_s =  "";
	path_s="";
	for ( c = Len(file_s) ; c>= 1 ; c+= -1 )
	{
		if (  cstr(Mid(file_s,c)) == "\\" || cstr(Mid(file_s,c)) == "/" ) 
		{
			path_s=Left(file_s,c) ; break;
		}
	}
//endfunction path$
	strcpy ( t.szwork , path_s.Get() );
	return t.szwork;
}

char* getfile ( char* file_s )
{
	int c = 0;
	for ( c = Len(file_s) ; c>= 1 ; c+= -1 )
	{
		if (  cstr(Mid(file_s,c)) == "\\" || cstr(Mid(file_s,c)) == "/" ) 
		{
			returntext_s=Right(file_s,Len(file_s)-c) ; break;
		}
	}
//endfunction file$
	strcpy ( t.szwork , returntext_s.Get() );
	return t.szwork;
}


//File List Building


void buildfilelist ( char* folder_s, char* rel_s )
{
	int tt = 0;
	cstr file_s =  "";
	int fin = 0;
	cstr tempcstr;
	if (  PathExist(folder_s) == 1 ) 
	{
		SetDir (  folder_s );
		FindFirst ( ); fin=0;
		while (  GetFileType()>-1 ) 
		{
			file_s=GetFileName();
			if (  file_s == "." || file_s == ".." ) 
			{
				//  ignore . and ..
			}
			else
			{
				if (  GetFileType() == 1 ) 
				{
					//  folder
					//sprintf ( t.szwork , "%s%s\\" , rel_s ,file_s.Get() );
					//buildfilelist(file_s.Get(),t.szwork);
					buildfilelist( file_s.Get() , cstr(cstr(rel_s)+file_s+"\\").Get() );
					FindFirst (  );
					if ( fin > 0 ) 
					{
						for ( tt = 1 ; tt<= fin ; tt++ )
						{
							if (  GetFileType()>-1 ) 
							{
								FindNext (  );
							}
						}
					}
				}
				else
				{
					//  file
					//array insert at bottom t.filelist_s[];
					//sprintf ( t.szwork , "%s%s" , rel_s , file_s.Get() );
					//tempcstr = t.szwork;
					int size = ArrayCount( t.filelist_s ); if ( size < 1 ) size = 1; else size++;
					Redim ( t.filelist_s , size );					
					t.filelist_s[size-1] = cstr(rel_s) + file_s;
				}
			}
			FindNext (  );
			fin=fin+1;
		}
		SetDir (  ".." );
	}
//endfunction

}
