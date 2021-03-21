#include "header.h"
#include <stdio.h>
#include <stdlib.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// CreateFullPathW creates the directory structure pointed to by szPath
// It creates everything upto the last backslash. For example:-
// c:\dir1\dir2\ <-- creates dir1 and dir2
// c:\dir1\file.dat <-- creates only dir1. This means you can pass this
// function the path to a file, and it will just create the required
// directories.
// Absolute or relative paths can be used, as can path length of 32,767
// characters, composed of components up to 255 characters in length. To
// specify such a path, use the "\\?\" prefix. For example, "\\?\D:\<path>".
// To specify such a UNC path, use the "\\?\UNC\" prefix. For example,
// "\\?\UNC\<server>\<share>".
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

PWSTR CreateFullPathW(PWSTR szPath)
{
  PWCHAR p = szPath;

  while (*p++)
  {
    if (*p == L'\\')
    {
      *p = L'\0';
      CreateDirectoryW(szPath, NULL);
      *p = L'\\';
    }
  }
  return szPath;
}

//----------------------------------------------------------------------------
DWORD GetFormattedDateTimeA(PCHAR lpBuf, BOOL bDate, BOOL bTime)
{
  SYSTEMTIME st;
  DWORD dwSize = 0;

  GetLocalTime(&st);

  if (bDate)
  {
    dwSize += wsprintfA(lpBuf, "%04u-%02u-%02u ", st.wYear, st.wMonth, st.wDay);
  }
  if (bTime)
  {
    dwSize += wsprintfA(lpBuf + dwSize, "%02u:%02u:%02u.%03u ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
  }
  return dwSize;
}

//----------------------------------------------------------------------------
DWORD GetFormattedDateTimeW(PWCHAR lpBuf, BOOL bDate, BOOL bTime)
{
  SYSTEMTIME st;
  DWORD dwSize = 0;

  GetLocalTime(&st);

  if (bDate)
  {
    dwSize += wsprintfW(lpBuf, L"%04u-%02u-%02u ", st.wYear, st.wMonth, st.wDay);
  }
  if (bTime)
  {
    dwSize += wsprintfW(lpBuf + dwSize, L"%02u:%02u:%02u.%03u ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
  }
  return dwSize; // return characters written, not bytes
}

// determine whether the output is a console
// this is hard. I first tried to use GetConsoleMode but it returns FALSE in case: mtee > con
BOOL IsAnOutputConsoleDevice(HANDLE h)
{
  if (GetFileType(h) == FILE_TYPE_CHAR)
  {
    // CON, NUL, ...
    DWORD dwBytesWritten;
    if (WriteConsoleA(h, "", 0, &dwBytesWritten, NULL))
      return TRUE;
  }
  return FALSE;
}

DWORD GetParentProcessId(VOID)
{
  DWORD ppid = 0, pid = GetCurrentProcessId();

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE) Perror(0);

  PROCESSENTRY32 pe = { 0 };
  pe.dwSize = sizeof(PROCESSENTRY32);

  //
  // find our PID in the process snapshot then lookup parent PID
  //
  if (Process32First(hSnapshot, &pe)) {
    do {
      if (pe.th32ProcessID == pid) {
        ppid = pe.th32ParentProcessID;
        break;
      }
    } while (Process32Next(hSnapshot, &pe));
  }
  CloseHandle(hSnapshot);
  return ppid;
}

static DWORD getActualNumberOfConsoleProcesses(VOID)
{
  DWORD dummyProcessId;
  DWORD numberOfProcesses;

  numberOfProcesses = GetConsoleProcessList(&dummyProcessId, 1);

  return numberOfProcesses;
}

HANDLE GetPipedProcessHandle(VOID)
{
  //
  // returns a handle to the process piped into mtee
  //
  DWORD dwProcCount = 0;
  DWORD* lpdwProcessList;
  HANDLE hPipedProcess = INVALID_HANDLE_VALUE;

  //
  // get an array of PIDs attached to this console
  //

  dwProcCount = getActualNumberOfConsoleProcesses();
  lpdwProcessList = (DWORD*)malloc(dwProcCount * sizeof(DWORD));
  if (NULL != lpdwProcessList && dwProcCount > 0)
  {
    dwProcCount = GetConsoleProcessList(lpdwProcessList, dwProcCount);
  }
  else
  {
    return INVALID_HANDLE_VALUE;
  }

  // in tests it __appears__ array element 0 is this PID, element 1 is process
  // piped into mtee, and last element is cmd.exe. if more than one pipe used,
  // element 2 is next process to rhe left:
  // eg A | B | C | mtee /e ==> lpdwProcessList[mtee][A][B][C][cmd]
  //
  // find the first PID that is not this PID and not parent PID.
  //
  DWORD ppid = GetParentProcessId();
  DWORD cpid = GetCurrentProcessId();
  for (DWORD dw = 0; dw < dwProcCount; dw++)
  {
    if ((cpid != lpdwProcessList[dw]) && (ppid != lpdwProcessList[dw]))
    {
      HANDLE Handle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        lpdwProcessList[dw]);
      if (INVALID_HANDLE_VALUE != Handle)
      {
        hPipedProcess = Handle;
        break;
      }
    }
  }

  free(lpdwProcessList);
  return hPipedProcess;
}

DWORD FormatElapsedTimeA(LARGE_INTEGER* elapsedTime, LPSTR outBuf, const size_t outBufSize)
{
  int h = 0;
  int m = 0;

  float s = float(elapsedTime->QuadPart / 1000000);
  m = (int)(s / 60.0);
  s = s - 60 * m;

  h = (int)((float)m / 60.0);
  m = m - 60 * h;

  return snprintf(outBuf, outBufSize, "Elapsed time: %02dh%02dm%06.3fs\n", h, m, s);
}

DWORD FormatElapsedTimeW(LARGE_INTEGER* elapsedTime, LPWSTR outBuf, const size_t outBufSize)
{
  int h = 0;
  int m = 0;
  int len = 0;

  float s = float(elapsedTime->QuadPart / 1000000);
  m = (int)(s / 60.0);
  s = s - 60 * m;

  h = (int)((float)m / 60.0);
  m = m - 60 * h;

  return _snwprintf(outBuf, outBufSize, L"Elapsed time: %02dh%02dm%06.3fs\n", h, m, s);
}
