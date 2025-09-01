#include "extwinapi.h"
#include <tchar.h>
#include <strsafe.h>
#include <locale>

void DisplayError(LPCTSTR desc) {
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, 
    NULL);

  HLOCAL lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + 560) * sizeof(TCHAR));

  if (FAILED(StringCchPrintf(
              (LPTSTR)lpDisplayBuf, 
              LocalSize(lpDisplayBuf) / sizeof(TCHAR),
              xorstr_(TEXT("Function failed with error code %d as follows:\n      %s")),
              dw, lpMsgBuf))) {
    printf(xorstr_("FATAL: Unable to output error code.\n"));
  }

  wprintf_s(xorstr_(TEXT("%s\n      %s\n")), (LPWSTR)desc, (LPWSTR)lpDisplayBuf);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}