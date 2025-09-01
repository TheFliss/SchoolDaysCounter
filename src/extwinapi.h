
#ifndef EXTENDED_WINAPI_H
#define EXTENDED_WINAPI_H

//#define UNICODE 0
//#define UNICODE 1
//#include <windows.h>
//#include <winnt.h>

#include <pch.h>

void DisplayError(LPCTSTR desc);

#ifdef DEBUG
#define PROTECT_ERROR_LOG(x) printf(xorstr_("\n%s(%d): "##x), xorstr_(__FILE__), __LINE__)
#else
#define PROTECT_ERROR_LOG(x) printf(xorstr_(x))
#endif

#define ResetTextAttrib SetConsoleTextAttribute(hConsoleOutput, consoleScreenInfo->wAttributes)

#define FunctionHandler(cond, desc) if (cond) {\
  SetConsoleTextAttribute(hConsoleOutput, ConsoleForeground::DARKRED); \
  PROTECT_ERROR_LOG("\n[SDC] error: ");\
  ResetTextAttrib;\
  DisplayError(xorstr_(TEXT(desc)));\
  exit(1);\
}

#define FunctionHandlerD(cond, desc, file) if (cond) {\
  SetConsoleTextAttribute(hConsoleOutput, ConsoleForeground::DARKRED); \
  PROTECT_ERROR_LOG("\n[SDC] error: ");\
  ResetTextAttrib;\
  printf_s(xorstr_("%s: "), file);\
  DisplayError(xorstr_(TEXT(desc)));\
  exit(1);\
}

#define FunctionHandlerL(cond, from, desc) if (cond) {\
  HANDLE hConsoleOutput;\
  PCONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo = new CONSOLE_SCREEN_BUFFER_INFO{0};\
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);\
  GetConsoleScreenBufferInfo(hConsoleOutput, consoleScreenInfo);\
  SetConsoleTextAttribute(hConsoleOutput, ConsoleForeground::DARKRED); \
  PROTECT_ERROR_LOG("\n["##from##"] error: ");\
  ResetTextAttrib;\
  DisplayError(xorstr_(TEXT(desc)));\
  exit(1);\
}

#define FunctionHandlerWarnL(cond, from, desc) if (cond) {\
  HANDLE hConsoleOutput;\
  PCONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo = new CONSOLE_SCREEN_BUFFER_INFO{0};\
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);\
  GetConsoleScreenBufferInfo(hConsoleOutput, consoleScreenInfo);\
  SetConsoleTextAttribute(hConsoleOutput, ConsoleForeground::DARKYELLOW); \
  PROTECT_ERROR_LOG("\n["##from##"] warning: ");\
  ResetTextAttrib;\
  DisplayError(xorstr_(TEXT(desc)));\
}

#define hndFunc(n, x) Handle##n##S(n(x))

#define HandleSetFilePointerS(hnd) FunctionHandler(hnd == INVALID_SET_FILE_POINTER, "Could not set file pointer")
#define HandleGetFilePointerS(hnd) FunctionHandler(hnd == INVALID_SET_FILE_POINTER, "Could not get file pointer")
#define HandleWriteFileS(hnd) FunctionHandler(!hnd, "Could not write file")
#define HandleReadFileS(hnd) FunctionHandler(!hnd, "Could not read file")
#define HandleCreateFileS(hnd) FunctionHandler(hnd == INVALID_HANDLE_VALUE, "Could not open file")
#define HandleCreateFileSD(hnd, file) FunctionHandlerD(hnd == INVALID_HANDLE_VALUE, "Could not open file", file)
#define HandleSetConsoleTextAttributeS(hnd) FunctionHandler(!hnd, "Could not set console attributes")

#define SetFilePointerS(...) hndFunc(SetFilePointer, __VA_ARGS__)
#define GetFilePointerS(...) hndFunc(GetFilePointer, __VA_ARGS__)
#define ReadFileS(...) hndFunc(ReadFile, __VA_ARGS__)
#define WriteFileS(...) hndFunc(WriteFile, __VA_ARGS__)
#define SetConsoleTextAttributeS(...) hndFunc(SetConsoleTextAttribute, __VA_ARGS__)

#endif