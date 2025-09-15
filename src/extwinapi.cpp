#include "extwinapi.h"
#include <tchar.h>
#include <strsafe.h>
#include <locale>

void DisplayError(LPCTSTR desc, UINT uType) {
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
              xorstr_(TEXT("%s\n\nFunction failed with error code %d as follows:\n%s")),
              (LPTSTR)desc,
              dw, lpMsgBuf))) {
    printf(xorstr_("FATAL: Unable to output error code.\n"));
  }

  MessageBox(nullptr, (LPTSTR)lpDisplayBuf, TEXT("Error"), uType);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}