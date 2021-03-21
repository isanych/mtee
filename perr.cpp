#include "header.h"

#define MAX_MSG_BUF_SIZE (1024)

DWORD Perror(DWORD dwErrNum)
{
  LPSTR pErrBuf;
  DWORD dwLastErr  = dwErrNum ? dwErrNum : GetLastError();

  //
  // get the text description for that error number from the system
  //
  auto cMsgLen = FormatMessageA
  (
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    dwLastErr,
    MAKELANGID(0, SUBLANG_ENGLISH_US),
    (LPSTR)&pErrBuf,
    MAX_MSG_BUF_SIZE,
    NULL
  );

  if(cMsgLen)
  {
    Verbose(pErrBuf);
    LocalFree(pErrBuf);
  }
  return dwLastErr;
}

VOID Verbose(LPCSTR szMsg)
{
  DWORD cBytes;

  WriteFile
  (
    GetStdHandle(STD_ERROR_HANDLE),
    szMsg,
    lstrlenA(szMsg),
    &cBytes,
    NULL
  );
}

VOID Verbose(LPCWSTR szMsg)
{
  LPSTR s = nullptr;
  DWORD size = lstrlenW(szMsg) + 1;
  UnicodeToOem(s, szMsg, size);
  if(s)
  {
    Verbose(s);
    HeapFree(GetProcessHeap(), 0, s);
  }
}
