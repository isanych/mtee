#include "header.h"

extern DWORD dwCtrlEvent;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
  dwCtrlEvent = dwCtrlType;
  return dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT;
}
