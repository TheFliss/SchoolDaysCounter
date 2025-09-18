
#ifndef EXTENDED_WINAPI_H
#define EXTENDED_WINAPI_H

#include <pch.h>

void DisplayError(LPCTSTR desc, UINT uType);

#ifdef DEBUG
#define PROTECT_ERROR_LOG(x) printf(xorstr_("\n%s(%d): "##x), xorstr_(__FILE__), __LINE__)
#else
#define PROTECT_ERROR_LOG(x) printf(xorstr_(x))
#endif

#define FunctionHandler(cond, desc) if (cond) {\
  DisplayError(WideFromUtf8(desc), MB_ICONERROR);\
  exit(1);\
}

#define FunctionHandlerR(cond, desc, cont) if (cond) {\
  DisplayError(xorstr_(TEXT(desc)), MB_ICONERROR);\
  cont;\
}

#define FunctionHandlerD(cond, desc, file) if (cond) {\
  TCHAR buffer[128]{};\
  _snwprintf_s(buffer, 128, xorstr_(L"%s: %s"), file, xorstr_(TEXT(desc)));\
  DisplayError(buffer, MB_ICONERROR);\
  exit(1);\
}

#define FunctionHandlerL(cond, from, desc) if (cond) {\
  TCHAR buffer[128]{};\
  _snwprintf_s(buffer, 128, xorstr_(L"%s: %s"), xorstr_(TEXT(from)), xorstr_(TEXT(desc)));\
  DisplayError(buffer, MB_ICONERROR);\
  exit(1);\
}

#define FunctionHandlerWarnL(cond, from, desc) if (cond) {\
  TCHAR buffer[128]{};\
  _snwprintf_s(buffer, 128, xorstr_(L"%s: %s"), xorstr_(TEXT(from)), xorstr_(TEXT(desc)));\
  DisplayError(buffer, MB_ICONERROR);\
}

#define hndFunc(n, x) Handle##n##S(n(x))

#define HandleSetFilePointerS(hnd) FunctionHandler(hnd == INVALID_SET_FILE_POINTER, "Could not set file pointer")
#define HandleGetFilePointerS(hnd) FunctionHandler(hnd == INVALID_SET_FILE_POINTER, "Could not get file pointer")
#define HandleWriteFileS(hnd) FunctionHandler(!hnd, "Could not write file")
#define HandleReadFileS(hnd) FunctionHandler(!hnd, "Could not read file")
#define HandleRegOpenKeyExAS(hnd) FunctionHandler(hnd, "Could not open registry key")
#define HandleRegOpenKeyExWS(hnd) FunctionHandler(hnd, "Could not open registry key")
#define HandleCreateFileS(hnd) FunctionHandler(hnd == INVALID_HANDLE_VALUE, "Could not open file")
#define HandleCreateFileSD(hnd, file) FunctionHandlerD(hnd == INVALID_HANDLE_VALUE, "Could not open file", file)
#define HandleSetConsoleTextAttributeS(hnd) FunctionHandler(!hnd, "Could not set console attributes")

#define SetFilePointerS(...) hndFunc(SetFilePointer, __VA_ARGS__)
#define GetFilePointerS(...) hndFunc(GetFilePointer, __VA_ARGS__)
#define ReadFileS(...) hndFunc(ReadFile, __VA_ARGS__)
#define WriteFileS(...) hndFunc(WriteFile, __VA_ARGS__)
#define SetConsoleTextAttributeS(...) hndFunc(SetConsoleTextAttribute, __VA_ARGS__)
#define RegOpenKeyExS(...) hndFunc(RegOpenKeyEx, __VA_ARGS__)

#endif