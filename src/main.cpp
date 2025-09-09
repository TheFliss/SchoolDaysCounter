
#include "extwinapi.h"
#include "timer.h"
#include "util.h"
#include "json/json.hpp"

using namespace util;
using json = nlohmann::json;

static void usage(const char *prog){

  vector<string> message{
    xorstr_("Optional arguments:"),
    xorstr_("\t-config-path                  ()"),
    xorstr_("\t-restore -r                   (sets the original wallpaper)"),
    "",
  };

  printf_s(xorstr_("\nUsage: %s [options]\n"), prog);
  for(auto x : message)
    printf_s("%s\n", x.c_str());
}

static void test_arg(int i, int argc, const char *argv){
  if(i+1 >= argc){
    printf_s(xorstr_("\nNeed more arguments after %s. Use \"sdc -help\" for usage.\n\n"), argv);
    exit(1);
  }
}

HWND FindWallpaperWindow() {
  HWND hProgman = FindWindow(TEXT("ProgMan"), nullptr);
  if (!hProgman) return nullptr;

  SendMessage(hProgman, 0x052C, 0, 0);

  HWND hWallpaperWnd = nullptr;
  EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
    if (FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr)) {
      HWND* result = reinterpret_cast<HWND*>(lParam);
      *result = FindWindowEx(nullptr, hwnd, L"WorkerW", nullptr);
      return FALSE;
    }
    return TRUE;
  }, reinterpret_cast<LPARAM>(&hWallpaperWnd));

  return hWallpaperWnd;
}

//https://habr.com/ru/articles/185252/

//https://stackoverflow.com/a/56107709
uint64_t timeStampMil() {
  return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void check_unknown_keys(const json& j, const set<string>& known_keys) {
  for (auto it = j.begin(); it != j.end(); ++it) {
    if (known_keys.find(it.key()) == known_keys.end()) {
      throw runtime_error("Unknown key: " + it.key());
    }
  }
}

void from_json(const json& j, config_vec2_t& v) {
  if (!j.is_array() || j.size() != 2) {
    throw runtime_error("vec2 must be an array of two numbers");
  }

  if(!j[0].is_number())
    throw runtime_error(xorstr_("\n\"vec2.x\" must be a number"));
  j[0].get_to(v.x);
  if(!j[1].is_number())
    throw runtime_error(xorstr_("\n\"vec2.y\" must be a number"));
  j[1].get_to(v.y);
}

void from_json(const json& j, timer_t& t) {
  set<string> known_keys = {"text", "end_date", "anchor", "font_size", 
                            "offset", "margin", "fixed_size", "detailed_time",
                            "text_color", "bg_color"};
  check_unknown_keys(j, known_keys);

  if(!j.contains("text"))
    throw runtime_error(xorstr_("\n\"text\" key required in timer config"));
  if(!j.at("text").is_string())
    throw runtime_error(xorstr_("\n\"text\" must be a string"));

  j.at("text").get_to(t.text);

  if (j.contains("end_date")) {
    if(!j.at("end_date").is_string())
      throw runtime_error(xorstr_("\n\"end_date\" must be a string"));
    string tmp_str;
    j.at("end_date").get_to(tmp_str);
    istringstream ss(tmp_str);
    tm tm = {};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    tm.tm_isdst = 0;
    if (ss.fail())
      throw runtime_error(xorstr_("\nDate in config parsing failed!\nRequired format: \"Y-m-d H:M:S\""));
    t.end_date = mktime(&tm);
  }else{
    throw runtime_error(xorstr_("\n\"end_date\" key required in timer config"));
  }

  if (j.contains("anchor")) {
    if(!j.at("text_color").is_string())
      throw runtime_error(xorstr_("\n\"text_color\" must be a string, and contains one of them\n  (top_left, top_middle, top_right,\n   left_middle, center, right_middle,\n   bottom_left, bottom_middle, bottom_right)"));
    string tmp_str;
    j.at("anchor").get_to(tmp_str);
    if(tmp_str == "top_left"){
      t.anchor = 0;
    }else if(tmp_str == "top_middle"){
      t.anchor = 1;
    }else if(tmp_str == "top_right"){
      t.anchor = 2;
    }else if(tmp_str == "left_middle"){
      t.anchor = 3;
    }else if(tmp_str == "center"){
      t.anchor = 4;
    }else if(tmp_str == "right_middle"){
      t.anchor = 5;
    }else if(tmp_str == "bottom_left"){
      t.anchor = 6;
    }else if(tmp_str == "bottom_middle"){
      t.anchor = 7;
    }else if(tmp_str == "bottom_right"){
      t.anchor = 8;
    }else{
      throw runtime_error(xorstr_("\nAnchor must be equal to one of \n  (top_left, top_middle, top_right,\n   left_middle, center, right_middle,\n   bottom_left, bottom_middle, bottom_right)\n"));
    }
  } else {
    t.anchor = 4;
  }

  if (j.contains("font_size")) {
    if(!j.at("font_size").is_number())
      throw runtime_error(xorstr_("\n\"font_size\" must be a number"));
    j.at("font_size").get_to(t.font_size);
  } else {
    t.font_size = 6;
  }

  if (j.contains("offset")) {
    if(!j.at("offset").is_array())
      throw runtime_error(xorstr_("\n\"offset\" must be an array"));
    j.at("offset").get_to(t.offset);
  } else {
    t.offset = {0, 0};
  }

  if (j.contains("margin")) {
    if(!j.at("margin").is_array())
      throw runtime_error(xorstr_("\n\"margin\" must be an array"));
    j.at("margin").get_to(t.margin);
  } else {
    t.margin = {0, 0};
  }

  if (j.contains("fixed_size")) {
    if(!j.at("fixed_size").is_array())
      throw runtime_error(xorstr_("\n\"fixed_size\" must be an array"));
    j.at("fixed_size").get_to(t.fixed_size);
  } else {
    t.fixed_size = {-1, -1};
  }

  if (j.contains("detailed_time")) {
    if(!j.at("detailed_time").is_boolean())
      throw runtime_error(xorstr_("\n\"detailed_time\" must be a boolean"));
    j.at("detailed_time").get_to(t.detailed_time);
  } else {
    t.detailed_time = false;
  }

  if (j.contains("text_color")) {
    if(!j.at("text_color").is_string())
      throw runtime_error(xorstr_("\n\"text_color\" must be a string"));
    try {
      string tmp_str;
      j.at("text_color").get_to(tmp_str);
      t.text_color = ((uint32_t)stoul(tmp_str, nullptr, 16));
    } catch(const exception& exc) {
      throw runtime_error(xorstr_("\nException caught while parsing \"text_color\" in timer config: ") + string(exc.what()));
    }
  } else {
    t.text_color = 0xffffffff;
  }

  if (j.contains("bg_color")) {
    if(!j.at("bg_color").is_string())
      throw runtime_error(xorstr_("\n\"bg_color\" must be a string"));
    try {
      string tmp_str;
      j.at("bg_color").get_to(tmp_str);
      t.bg_color = ((uint32_t)stoul(tmp_str, nullptr, 16));
    } catch(const exception& exc) {
      throw runtime_error(xorstr_("\nException caught while parsing \"bg_color\" in timer config: ") + string(exc.what()));
    }
  } else {
    t.bg_color = 0x000000b0;
  }
}

void from_json(const json& j, sdc_config_t& c) {
  set<string> known_keys = {"update_delay", "timers"};
  check_unknown_keys(j, known_keys);

  if(j.contains("update_delay")){
    if(!j.at("update_delay").is_number_integer())
      throw runtime_error(xorstr_("\n\"update_delay\" must be a integer"));
    j.at("update_delay").get_to(c.update_delay);
  }else{
    c.update_delay = 900;
  }
  {
    if(!j.contains("timers"))
      throw runtime_error(xorstr_("\n\"timers\" key required in config file"));
    if(!j.at("timers").is_array())
      throw runtime_error(xorstr_("\n\"timers\" must be an array"));
    j.at("timers").get_to(c.timers);
    if(c.timers.size() == 0){
      throw runtime_error(xorstr_("\n\"timers\" content must be not equal to []"));
    }
  }
}

int main(int argc, char const *argv[]) {
  setlocale(LC_ALL, "Russian");
  SetConsoleOutputCP(866);

  bool restore = false;
  FilePath SDCConfigFile = FilePath(getexepath() / "sdc_config.json");

  for (int i = 1; i < argc; i++) {
    try
    {
      if(strcmp(argv[i], xorstr_("-config-path")) == 0){
        test_arg(i, argc, argv[i]);
        SDCConfigFile = FilePath(string(argv[i+1]));
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-restore")) == 0 || strcmp(argv[i], xorstr_("-r")) == 0){
        restore = true;
      }else{
        if(strcmp(argv[i], xorstr_("-help")) != 0) printf_s(xorstr_("\nERROR: Unknown argument %s\n"), argv[i]);
        usage(argv[0]);
        exit(1);
      }
    }
    catch(const exception& e)
    {
      cerr << "\nException caught while parsing argument \"" << argv[i] << "\": " << e.what() << endl;
        usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  FilePath restoreFile = FilePath(getexepath() / "original_path.txt");

  //DEPRECATED
  if(!PathFileExistsA(restoreFile.fp_s.c_str())){
    wchar_t *szWallpaperPath[MAX_PATH];
    FunctionHandlerL(!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");

    //Writing restore file
    HANDLE hRestoreFile = CreateFileA(restoreFile.fp_s.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    HandleCreateFileSD(hRestoreFile, restoreFile.fp_s.c_str());

    string cyrillic_text = ConvertWideToUtf8(wstring((LPTSTR)szWallpaperPath));

    WriteFileS(hRestoreFile, cyrillic_text.c_str(), cyrillic_text.length() * sizeof(char), NULL, NULL);

    CloseHandle(hRestoreFile);

#ifdef DEBUG
    wprintf_s(xorstr_(TEXT("%s\n")), (LPTSTR)szWallpaperPath);
#endif
  }

  if(restore){
    printf_s(xorstr_("\nRestoring orginal wallpaper\n"));
    HANDLE hRestoreFile = CreateFileA(restoreFile.fp_s.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    HandleCreateFileSD(hRestoreFile, restoreFile.fp_s.c_str());

    DWORD size = GetFileSize(hRestoreFile, NULL);
    PVOID virtualpointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

    ReadFileS(hRestoreFile, virtualpointer, size, NULL, NULL);

    wstring cyrillic_text = ConvertUtf8ToWide(string((LPCSTR)virtualpointer));

    FunctionHandlerL(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (LPVOID)cyrillic_text.c_str(), SPIF_UPDATEINIFILE), "SDC", "Cannot set wallpaper path");
#ifdef DEBUG
    wprintf_s(xorstr_(TEXT("%s\n")), cyrillic_text.c_str());
#endif

    CloseHandle(hRestoreFile);
    return 0;
  }

  LPTSTR szWallpaperPath[MAX_PATH];
  FunctionHandlerL(!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");
  FunctionHandlerL(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szWallpaperPath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE), "SDC", "Cannot set wallpaper path");

  HWND hWallpaper = FindWallpaperWindow();
  if (!hWallpaper) {
    MessageBox(nullptr, L"Не удалось найти окно обоев", L"Ошибка", MB_ICONERROR);
    return 1;
  }

  vector<unique_ptr<Timer>> timers;

  sdc_config_t sdc_config;
  try {
    auto sdccstream = ifstream(SDCConfigFile.fp);

    json j = json::parse(sdccstream, nullptr, true, true);

    sdc_config = j.get<sdc_config_t>();

    sdccstream.close();
  } catch(const exception& e) {
    cerr << "\nException caught while reading config file: \n  " << e.what() << '\n' << endl;
    return EXIT_FAILURE;
  }

  if(sdc_config.update_delay < 1){
    printf_s(xorstr_("\nERROR: Update time must be greater than 0\n"));
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  {
    for(auto &x : sdc_config.timers){
      Timer current_timer(x);
      timers.push_back(make_unique<Timer>(current_timer));
    }
  }

  ShowWindow(GetConsoleWindow(), SW_HIDE);

  HDC hdc = GetDC(hWallpaper);

  RECT cr;
  GetClientRect(hWallpaper, &cr);

  //saving original wp hdc
  HDC hdcSRC = CreateCompatibleDC(hdc);
  HBITMAP hbmSRC = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
  HANDLE hOldSRC = SelectObject(hdcSRC, hbmSRC);
  BitBlt(hdcSRC, 0, 0, cr.right, cr.bottom, hdc, 0, 0, SRCCOPY);

  HDC hdcMem = CreateCompatibleDC(hdc);
  HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
  HANDLE hOld = SelectObject(hdcMem, hbmMem);

  while(1){
    BitBlt(hdcMem, 0, 0, cr.right, cr.bottom, hdcSRC, 0, 0, SRCCOPY);
    for(auto &x : timers)
      x->render(hdcMem, &cr);

    BitBlt(hdc, 0, 0, cr.right, cr.bottom, hdcMem, 0, 0, SRCCOPY);
    Sleep(sdc_config.update_delay);
  }

  BitBlt(hdc, 0, 0, cr.right, cr.bottom, hdcSRC, 0, 0, SRCCOPY);
  SelectObject(hdcMem, hOld);
  SelectObject(hdcSRC, hOldSRC);

  DeleteObject(hbmMem);
  DeleteDC (hdcMem);
  DeleteObject(hbmSRC);
  DeleteDC (hdcSRC);

  ReleaseDC(hWallpaper, hdc);

  return 0;
}