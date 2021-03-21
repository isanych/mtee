#define _WIN32_WINNT 0x0502
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include "cpuload.h"

#define OP_IN_OUT_SHIFT (16)      // out codes are 16 bits from in codes
#define OP_ANSI_IN      (0x00000001)
#define OP_UNICODE_IN   (0x00000002)
#define OP_ANSI_OUT     (0x00010000)
#define OP_UNICODE_OUT  (0x00020000)
#define OP_HEX_OUT      (0x00040000)

// MTEE Operations
#define OP_ANSI_IN_ANSI_OUT       (OP_ANSI_IN | OP_ANSI_OUT)
#define OP_ANSI_IN_UNICODE_OUT    (OP_ANSI_IN | OP_UNICODE_OUT)
#define OP_UNICODE_IN_ANSI_OUT    (OP_UNICODE_IN | OP_ANSI_OUT)
#define OP_UNICODE_IN_UNICODE_OUT (OP_UNICODE_IN | OP_UNICODE_OUT)
#define OP_ANSI_IN_HEX_OUT        (OP_ANSI_IN | OP_HEX_OUT)
#define OP_UNICODE_IN_HEX_OUT     (OP_UNICODE_IN | OP_HEX_OUT)

#define _MERGE_RDATA_


#if !defined (INVALID_SET_FILE_POINTER)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

//#include <stdio.h>

#define CTRL_CLEAR_EVENT (0xFFFF)

#define MAX_CONSOLE_PID_LIST    (128L)

//
// Stores details of the files to be written to
//
typedef struct _FILEINFO
{
  HANDLE    hFile;      // handle of open file
  LPWSTR    lpFileName; // pointer to dynamically assigned wide buffer holding filename
  BOOL      bIsConsole; // true if it is a console device
  _FILEINFO* fiNext;    // pointer to next FILEINFO structure
} FILEINFO, * PFILEINFO;

//
// Stores the commandline arguments/options
//
typedef struct _ARGS
{
  BOOL     bAddDate;         // if true, prefix each newline with local date
  BOOL     bAddTime;         // if true, prefix each newline with local time
  BOOL     bContinue;        // if true, continue on I/O errors
  BOOL     bHelp;            // if true, show helpscreen
  BOOL     bUnicode;         // if true, output will be unicode (converted if required)
  BOOL     bFwdExitCode;     // if true, exit with exit code of piped process
  BOOL     bOem;             // if true, output will be oem (converted if required)
  BOOL     bIntermediate;    // if true, create intermediate directories if required
  BOOL     bElapsedTime;     // if true, calculate and display elapsed time
  BOOL     bMeasureCPUUsage; // if true, measure CPU usage during runtime
  BOOL     bAppend;          // if true, append I/O to file instead of overwrite
  FILEINFO fi;               // FILEINFO structure
  DWORD    dwBufSize;        // max size of buffer for read operations
  DWORD    dwPeekTimeout;    // max ms to peek for input
} ARGS, * LPARGS;

// args.cpp
BOOL CheckFileName(LPCWSTR lpToken);
BOOL GetCommandLineTokenW(LPWSTR& tokenPtr);  // process command line args
BOOL ParseCommandlineW(ARGS& args);
LPWSTR StringAllocW(LPWSTR& dest, LPCWSTR src);
VOID FreeFileInfoStructs(PFILEINFO fi);
PFILEINFO CreateFileInfoStruct(PFILEINFO& fi, LPCWSTR fileName);


BOOL WINAPI HandlerRoutine(DWORD);
HWND GetConsoleHandle(VOID);
VOID ShowFileType(HANDLE);
void ShowHelp();
PWSTR CreateFullPathW(PWSTR szPath);

DWORD GetFormattedDateTimeA(PCHAR lpBuf, BOOL bDate, BOOL bTime);
DWORD GetFormattedDateTimeW(PWCHAR lpBuf, BOOL bDate, BOOL bTime);
BOOL IsAnOutputConsoleDevice(HANDLE h);
DWORD GetParentProcessId(VOID);
HANDLE GetPipedProcessHandle(VOID);

DWORD FormatElapsedTimeA(LARGE_INTEGER* elapsedTime, LPSTR outBuf, const size_t outBufSize);
DWORD FormatElapsedTimeW(LARGE_INTEGER* elapsedTime, LPWSTR outBuf, const size_t outBufSize);

// perr.cpp
DWORD Perror(DWORD dwErrNum);
VOID Verbose(LPCSTR szMsg);
VOID Verbose(LPCWSTR szMsg);

// output.cpp
BOOL WriteBufferToConsoleAndFilesA(ARGS& args, LPCSTR lpBuf, DWORD dwCharsRead, BOOL AddDate, BOOL AddTime);
BOOL WriteBufferToConsoleAndFilesW(ARGS& args, LPCWSTR lpBuf, DWORD dwCharsRead, BOOL AddDate, BOOL AddTime);
BOOL OemToUnicode(LPWSTR& lpDest, LPCSTR lpSrc, DWORD& lpSize);
BOOL UnicodeToOem(LPSTR& lpDest, LPCWSTR lpSrc, DWORD& lpSize);
BOOL WriteBom(PFILEINFO fi, BOOL bContinue);
