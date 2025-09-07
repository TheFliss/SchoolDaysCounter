#ifndef UTIL_H
#define UTIL_H

#include "pch.h"

namespace util {
  extern fs::path getexepath();

  extern bool isLeapYear(int year);

  extern int get_month_length(int month, int year);

  extern const char * declination_word(int counter, const char* d0, const char* d1, const char* d2);

  extern string ConvertWideToANSI(const wstring& wstr);

  extern wstring ConvertAnsiToWide(const string& str);

  extern string ConvertWideToUtf8(const wstring& wstr);

  extern wstring ConvertUtf8ToWide(const string& str);
} 

#endif /* UTIL_H */