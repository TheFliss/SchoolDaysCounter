#include "util.h"

fs::path util::getexepath()
{
  char path[MAX_PATH] = { 0 };
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return fs::path(path).parent_path();
}

const char * util::declination_word(int counter, const char* d0, const char* d1, const char* d2){
  int cTens = counter%10;
  return ((counter%100-cTens) != 10) ? ((cTens > 1 && cTens < 5) ? d1 : ((cTens == 1) ? d0 : d2)) : d2;
};

string util::ConvertWideToANSI(const wstring& wstr)
{
    int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    string str(count, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

wstring util::ConvertAnsiToWide(const string& str)
{
    int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
    wstring wstr(count, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], count);
    return wstr;
}

string util::ConvertWideToUtf8(const wstring& wstr)
{
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

wstring util::ConvertUtf8ToWide(const string& str)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], count);
    return wstr;
}