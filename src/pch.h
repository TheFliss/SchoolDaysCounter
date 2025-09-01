#ifndef PCH_H
#define PCH_H

// Put commonly included files here to speed up the build process

//#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE

#include <Windows.h>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <string_view>
#include <memory>
#include <future>
#include <execution>
#include <optional>
#include <chrono>
#include <format>
#include <array>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <regex>
#include <map>
#include <set>
#include <variant>
#include <algorithm>
#include <xorstr.hpp>

using namespace std;
namespace fs = std::filesystem;

class FilePath {
public:
  fs::path fp;
  string fp_s;
  FilePath(){}
  FilePath(fs::path _fp){
    fp = _fp;
    fp_s = _fp.string();
  }
  void updatePath(fs::path _fp){
    fp = _fp;
    fp_s = _fp.string();
  }
  void updatePath(){
    fp_s = fp.string();
  }
};


enum ConsoleForeground
{
  BLACK             = 0,
  DARKBLUE          = FOREGROUND_BLUE,
  DARKGREEN         = FOREGROUND_GREEN,
  DARKCYAN          = FOREGROUND_GREEN | FOREGROUND_BLUE,
  DARKRED           = FOREGROUND_RED,
  DARKMAGENTA       = FOREGROUND_RED | FOREGROUND_BLUE,
  DARKYELLOW        = FOREGROUND_RED | FOREGROUND_GREEN,
  DARKGRAY          = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
  GRAY              = FOREGROUND_INTENSITY,
  BLUE              = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
  GREEN             = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
  CYAN              = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
  RED               = FOREGROUND_INTENSITY | FOREGROUND_RED,
  MAGENTA           = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
  YELLOW            = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
  WHITE             = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};
//extern PCONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
//extern HANDLE hConsoleOutput;
 
#endif