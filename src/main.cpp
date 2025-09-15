
#include "extwinapi.h"
#include "timer.h"
#include "util.h"

using namespace util;

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

    WriteFileS(hRestoreFile, cyrillic_text.c_str(), (DWORD)cyrillic_text.length() * sizeof(char), NULL, NULL);

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

  sdc_config_t sdc_config = parse_config(SDCConfigFile.fp);

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

  //ShowWindow(GetConsoleWindow(), SW_HIDE);

  HDC hdc = GetDC(hWallpaper);

  RECT cr;
  GetClientRect(hWallpaper, &cr);

  float scale = (float)GetDpiForWindow(hWallpaper)/96.0f;
  cout << "scale: " << scale << endl;

  cr.right = (LONG)round((float)cr.right*scale);
  cr.bottom = (LONG)round((float)cr.bottom*scale);

  //saving original wp hdc
  HDC hdcSRC = CreateCompatibleDC(hdc);
  HBITMAP hbmSRC = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
  HANDLE hOldSRC = SelectObject(hdcSRC, hbmSRC);
  BitBlt(hdcSRC, 0, 0, cr.right, cr.bottom, hdc, 0, 0, SRCCOPY);

  HWND hwndDesktop = GetDesktopWindow(); 
  //cout << "scale: " << ((float)dpi/96.0f) << endl;
  cout << hWallpaper << endl;
  cout << hwndDesktop << endl;
  cout << cr.left << " left "
      << cr.top << " top "
      << cr.right << " right "
      << cr.bottom << " bottom" << endl;
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