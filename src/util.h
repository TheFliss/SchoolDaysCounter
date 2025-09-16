#ifndef UTIL_H
#define UTIL_H

#include "pch.h"

#define WideFromUtf8s(x) (LPCWSTR)ConvertUtf8ToWide(x).c_str()
#define WideFromUtf8(x) (LPCWSTR)ConvertUtf8ToWide(string(xorstr_(x))).c_str()

namespace util {
  extern fs::path getexepath();

  extern const char * declination_word(int counter, const char* d0, const char* d1, const char* d2);

  extern string ConvertWideToANSI(const wstring& wstr);

  extern wstring ConvertAnsiToWide(const string& str);

  extern string ConvertWideToUtf8(const wstring& wstr);

  extern wstring ConvertUtf8ToWide(const string& str);
} 

#endif /* UTIL_H */