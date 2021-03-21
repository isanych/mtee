#include "header.h"

inline WCHAR upper(WCHAR c)
{
  return c >= L'a' && c <= 'z' ? c - (L'a' - L'A') : c;
}

bool ParseToken(PFILEINFO& fi, ARGS& args, LPCWSTR lpToken)
{
  auto len = lstrlenW(lpToken);
  auto c = lpToken[0];
  if (c == L'/' || c == L'-')
  {
    if (len == 2)
    {
      switch (upper(lpToken[1]))
      {
      case L'?':
      case L'H': args.bHelp = TRUE; return true;
      case L'A': args.bAppend = TRUE; return true;
      case L'O': args.bOem = TRUE; return true;
      case L'C': args.bContinue = TRUE; return true;
      case L'D': args.bAddDate = TRUE; return true;
      case L'I': args.bIntermediate = TRUE; return true;
      case L'E': args.bFwdExitCode = TRUE; return true;
      case L'T': args.bAddTime = TRUE; return true;
      case L'U': args.bUnicode = TRUE; return true;
      }
      return false;
    }
    if(lstrcmpiW(lpToken + 1, L"ET") == 0)
    {
      args.bElapsedTime = TRUE;
      return true;
    }
    if(lstrcmpiW(lpToken + 1, L"CPU") == 0)
    {
      args.bMeasureCPUUsage = TRUE;
      return true;
    }
  }
  // unknown token so assume a filename
  if (!CheckFileName(lpToken))
  {
    Verbose(lpToken);
    Verbose(" unknown option\n");
    return false;
  }
  if(!CreateFileInfoStruct(fi, lpToken)) ExitProcess(Perror(0));
  return true;
}

BOOL ParseCommandlineW(ARGS &args)
{
  LPWSTR lpToken;
  //
  // init defaults
  //
  args.dwBufSize    = 0x4000;
  args.dwPeekTimeout = 50;
  auto fi = &args.fi;
  fi->hFile      = INVALID_HANDLE_VALUE;

  while(GetCommandLineTokenW(lpToken))
  {
    if (!ParseToken(fi, args, lpToken)) return FALSE;
  }

  //
  // validate the options
  //
  if(args.bOem && args.bUnicode)
  {
    Verbose("The /O and /U switches cannot be used together.\n");
    return FALSE;
  }

  return TRUE;
}

BOOL CheckFileName(LPCWSTR lpToken)
{
  auto p = lpToken;
  WCHAR strBadChars[] = L"<>|\"/";
  PWCHAR lpBadChars;
  if(!lstrcmpW(lpToken, L"")) return FALSE;
  while(*p)
  {
    lpBadChars = strBadChars;
    while(*lpBadChars) if(*p == *lpBadChars++) return FALSE;
    p++;
  }
  return TRUE;
}

PFILEINFO CreateFileInfoStruct(PFILEINFO &fi, LPCWSTR fileName)
{
  fi->fiNext = (PFILEINFO) HeapAlloc(GetProcessHeap(), 0, sizeof(FILEINFO));
  if(!fi->fiNext) return NULL;
  //
  // init and point to new struct
  //
  fi = fi->fiNext;
  *fi = {};
  fi->hFile = INVALID_HANDLE_VALUE;
  if(!StringAllocW(fi->lpFileName, fileName)) ExitProcess(Perror(0));
  return fi;
}

VOID FreeFileInfoStructs(PFILEINFO fi)
{
  while(fi)
  {
    HeapFree(GetProcessHeap(), 0, fi->lpFileName);
    auto old = fi;
    fi = fi->fiNext;
    HeapFree(GetProcessHeap(), 0, old);
  }
}

LPWSTR StringAllocW(LPWSTR &dest, LPCWSTR src)
{
  dest = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, (lstrlenW(src) * sizeof(WCHAR) + sizeof(WCHAR)));
  if(!dest) return NULL;
  return lstrcpyW(dest, src);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Function: GetCommandLineToken()
//
// Synopsis: Parses a commandline, returning one token at a time
//
// Arguments: [lpToken] Pointer to a pointer. Points to the found token
//
// Returns: TRUE if a token is found, otherwise FALSE (end of commandline
//      reached
//
// Notes: Each call sets lpToken to point to each token in turn until no
//      more found
//
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
BOOL GetCommandLineTokenW(LPWSTR &lpToken)
{
  static PWCHAR p = NULL;          // keeps track of where we've got to on windows cmdline
  static PWCHAR lpTokenBuf = NULL; // buffer to hold each token in turn
  PWCHAR t;                        // keeps track of current token
  DWORD cQuote = 0;                // keeps tracks of quotes
  BOOL bOutQuotes = TRUE;          // true if outside of quotes
  BOOL bInToken = FALSE;           // true if inside a token

  //
  // if p is NULL, then I'm being called for first time
  //
  if(!p)
  {
    //
    // get a pointer to windows commandline
    //
    p = GetCommandLineW();

    //
    // allocate enough memory to hold any tokens in the commandline
    //
    lpTokenBuf = (PWCHAR) HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * (lstrlenW(p) + 1));

    if(!lpTokenBuf) ExitProcess(Perror(0));

    //
    // skip over name of exe to get to start arguments
    // windows always removes leading whitespace and adds a trailing space
    // if not already present. [; ; prog/s] --> [prog /s]
    //
    if(*p == L'"')
    {
      //
      // if exe name begins with a quote(s), then skip over all beginning quotes
      //
      while(*p == L'"') p++;
      //
      // now skip to the ending quote(s)
      //
      while(*p != L'"') p++;
      //
      // finally skip any further trailing quotes
      //
      while(*p == L'"') p++;
    }
    //
    // didn't start with a quote, so skip to first whitespace
    //
    else
    {
      while((*p) && (*p != L' ') && (*p != L'\t')) p++;
    }

    //
    // skip over any whitespace to first token or end of commandline
    //
    while(*p && (*p == L' ' || *p == L'\t')) p++;

  }

  //
  // initialise token pointers and token buffer
  //
  lpToken = t = lstrcpyW(lpTokenBuf, L"");

  //
  // now start to copy characters from windows commandline to token buffer
  //
  if(*p)
  {
    while(*p)
    {
      //
      // skip over whitespace if not inside a quoted token
      //
      if((*p == L' ' || *p == L'\t') && bOutQuotes)
      {
        break;
      }
      else if(*p == L'"')
      {
        bOutQuotes = !bOutQuotes;
        p++;

        //
        // three consecutive quotes make one 'real' quote
        //
        if(++cQuote == 3)
        {
          *t++ = L'"';
          cQuote = 0;
          bOutQuotes = !bOutQuotes;
        }
        continue;
      }
      //
      // if forward slash found then reset cQuote and toggle bInToken flag
      //
      else if(*p == L'/' || *p == L'-')
      {
        cQuote = 0;
        bInToken = !bInToken;

        //
        // if not a token and not in quotes then break and l
        //
        if(!bInToken && bOutQuotes) break;
      }

      cQuote = 0;
      bInToken = TRUE;

      //
      // add the character to the token
      //
      *t++ = *p++;

    } // while(*p)


    //
    // terminate the token
    //
    *t = L'\0';

    //
    // set p to point to next argument ready for next call
    //
    while(*p && (*p == L' ' || *p == L'\t')) p++;

    return TRUE;
  }

  //
  // reached the end of windows commandline
  //
  return FALSE;
}
