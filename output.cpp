#include "header.h"
#define MAX_DATE_TIME_LEN (25)

BOOL WriteBufferToConsoleAndFilesA(ARGS &args, LPCSTR lpBuf, DWORD dwCharsRead, BOOL AddDate, BOOL AddTime)
{
  if(AddDate | AddTime)
  {
    CHAR szDT[MAX_DATE_TIME_LEN];
    auto pHead = lpBuf;
    auto pTail = lpBuf;
    while(pTail < (lpBuf + dwCharsRead))
    {
      static BOOL bNewLine = TRUE;
      if(bNewLine)
      {
        DWORD dwBR = GetFormattedDateTimeA(szDT, AddDate, AddTime);
        if(!WriteBufferToConsoleAndFilesA(args, szDT, dwBR, FALSE, FALSE)) return FALSE;
        bNewLine = FALSE;
      }
      if(*pTail++ == 0x0A)
      {
        bNewLine = TRUE;
        if(!WriteBufferToConsoleAndFilesA(args, pHead, pTail - pHead, FALSE, FALSE)) return FALSE;
        pHead = pTail;
      }
    }
    if(!WriteBufferToConsoleAndFilesA(args, pHead, pTail - pHead, FALSE, FALSE)) return FALSE;
  }
  else
  {
    auto fi = &args.fi;
    while(fi)
    {
      if(fi->hFile != INVALID_HANDLE_VALUE)
      {
        DWORD dwBytesWritten;
        auto ok = fi->bIsConsole ?
          WriteConsoleA(fi->hFile, lpBuf, dwCharsRead, &dwBytesWritten, NULL) :
          WriteFile(fi->hFile, lpBuf, dwCharsRead, &dwBytesWritten, NULL);
        if (!ok && !args.bContinue) return FALSE;
      }
      fi = fi->fiNext;
    }
  }
  return TRUE;
}

BOOL WriteBufferToConsoleAndFilesW(ARGS &args, LPCWSTR lpBuf, DWORD dwCharsRead, BOOL AddDate, BOOL AddTime)
{
  if(AddDate | AddTime)
  {
    WCHAR szDT[MAX_DATE_TIME_LEN];
    auto pHead = lpBuf;
    auto pTail = lpBuf;
    while(pTail < (lpBuf + dwCharsRead))
    {
      static BOOL bNewLine = TRUE;
      if(bNewLine)
      {
        DWORD dwBR = GetFormattedDateTimeW(szDT, AddDate, AddTime);
        if(!WriteBufferToConsoleAndFilesW(args, szDT, dwBR, FALSE, FALSE)) return FALSE;
        bNewLine = FALSE;
      }
      if(*pTail++ == L'\n')
      {
        bNewLine = TRUE;
        if(!WriteBufferToConsoleAndFilesW(args, pHead, pTail - pHead, FALSE, FALSE)) return FALSE;
        pHead = pTail;
      }
    }
    if(!WriteBufferToConsoleAndFilesW(args, pHead, pTail - pHead, FALSE, FALSE)) return FALSE;
  }
  else
  {
    auto fi = &args.fi;
    while(fi)
    {
      if(fi->hFile != INVALID_HANDLE_VALUE)
      {
        DWORD dwBytesWritten;
        auto ok = fi->bIsConsole ?
          // The ANSI version perhaps works identically to WriteFile, however in UNICODE version 
          // WriteConsoleW is definitely needed so that it understands that we are outputting UNICODE characters
          WriteConsoleW(fi->hFile, lpBuf, dwCharsRead, &dwBytesWritten, NULL) :
          WriteFile(fi->hFile, lpBuf, dwCharsRead * sizeof(WCHAR), &dwBytesWritten, NULL);
        if (!ok && !args.bContinue) return FALSE;
      }
      fi = fi->fiNext;
    }
  }
  return TRUE;
}


BOOL OemToUnicode(LPWSTR &lpDest, LPCSTR lpSrc, DWORD &lpSize)
{
  int iWideCharLen;
  if(lpDest != nullptr)
  {
    if(!HeapFree(GetProcessHeap(), 0, lpDest)) return FALSE;
  }
  
  iWideCharLen = MultiByteToWideChar(CP_OEMCP, 0, lpSrc, lpSize, NULL, 0);

  lpDest = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, iWideCharLen * sizeof(WCHAR));
  if(lpDest == NULL) return FALSE;
  //
  // set lpsize to number of chars
  //
  
  lpSize = MultiByteToWideChar(GetConsoleCP(), 0, lpSrc, lpSize, lpDest, iWideCharLen);
 
  return TRUE;
}

BOOL UnicodeToOem(LPSTR &lpDest, LPCWSTR lpSrc, DWORD& lpSize)
{
  if(lpDest != nullptr)
  {
    if(!HeapFree(GetProcessHeap(), 0, lpDest)) return FALSE;
  }
  auto iOemCharLen = WideCharToMultiByte(CP_OEMCP, 0, lpSrc, lpSize, NULL, 0, NULL, NULL);
  lpDest = (LPSTR) HeapAlloc(GetProcessHeap(), 0, iOemCharLen * sizeof(WCHAR));
  if(lpDest == NULL) return FALSE;
  
  lpSize = WideCharToMultiByte(CP_OEMCP, 0, lpSrc, lpSize, lpDest, iOemCharLen, NULL, NULL);
  return TRUE;
}

BOOL WriteBom(PFILEINFO fi, BOOL bContinue)
{
  DWORD dwBytesWritten;
  WCHAR wcBom = 0xFEFF;
  while(fi)
  {
    DWORD dwFileSizeHigh = 0;
    DWORD siz = GetFileSize(fi->hFile, &dwFileSizeHigh);
    if((fi->hFile != INVALID_HANDLE_VALUE) && (siz == 0 && dwFileSizeHigh == 0))
    {
      if(!WriteFile(fi->hFile, &wcBom, sizeof(WCHAR), &dwBytesWritten, NULL))
      {
        if(!bContinue) return FALSE;
      }
    }
    fi = fi->fiNext;
  }

  return TRUE;
}