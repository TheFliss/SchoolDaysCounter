
#include "extwinapi.h"
#include "timer.h"
#include "util.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' "\
                   "name='Microsoft.Windows.Common-Controls' "\
                   "version='6.0.0.0' "\
                   "processorArchitecture='*' "\
                   "publicKeyToken='6595b64144ccf1df' "\
                   "language='*' "\
                   "\"")

using namespace util;

const wchar_t g_szClassName[] = L"SDCWindowClass";
static bool g_exitFromThread = false;
NOTIFYICONDATA g_tnd = {};

struct sdc_thread_vars_t {
  HWND hwnd;
  BOOL Minimize = 0;
};

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

void resetWallpaperWindow(){
  LPTSTR szWallpaperPath[MAX_PATH];
  FunctionHandlerL(!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");
  FunctionHandlerL(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szWallpaperPath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE), "SDC", "Cannot set wallpaper path");
}

HWND FindWallpaperWindow() {
  HWND hProgman = FindWindow(TEXT("ProgMan"), nullptr);
  if (!hProgman) return nullptr;

  SendMessage(hProgman, 0x052C, 0, 0);

  HWND hWallpaperWnd = nullptr;
  EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
    if (FindWindowEx(hwnd, nullptr, TEXT("SHELLDLL_DefView"), nullptr)) {
      HWND* result = reinterpret_cast<HWND*>(lParam);
      *result = FindWindowEx(nullptr, hwnd, TEXT("WorkerW"), nullptr);
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


#pragma region Registry

BOOL CreateRegistryKey(HKEY hKeyParent, LPCWSTR subkey) {
  HKEY hKey;
  DWORD dwDisposition; // Indicates if the key was created or opened
  LONG lResult = RegCreateKeyExW(
    hKeyParent,
    subkey,
    0,
    NULL,
    REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS,
    NULL,
    &hKey,
    &dwDisposition
  );

  FunctionHandlerR(lResult, "Error creating or opening registry key", {
    return FALSE;
  });
  RegCloseKey(hKey); // Close the handle
  return TRUE;
}

// Function to check the value in the registry
BOOL CheckRegistryValue(HKEY hKeyParent, LPCWSTR subkey, LPCWSTR valueName, DWORD qType) {
  HKEY hKey;
  LONG lResult = RegOpenKeyExW(
    hKeyParent,
    subkey,
    0,
    KEY_QUERY_VALUE,
    &hKey
  );

  if(lResult != ERROR_SUCCESS) {
    return FALSE;
  }

  DWORD dwType;
  DWORD dwSize = 0;
  lResult = RegQueryValueExW(
    hKey,
    valueName,
    NULL,
    &dwType,
    NULL,
    &dwSize
  );

  if(lResult != ERROR_SUCCESS || dwType != qType){
    RegCloseKey(hKey);
    return FALSE;
  };

  RegCloseKey(hKey);
  return TRUE;
}

// Function to set a dword value in the registry
BOOL SetRegistryDWValue(HKEY hKeyParent, LPCWSTR subkey, LPCWSTR valueName, DWORD data) {
  HKEY hKey;
  LONG lResult = RegOpenKeyExW(
    hKeyParent,
    subkey,
    0,
    KEY_SET_VALUE,
    &hKey
  );

  FunctionHandlerR(lResult, "Error opening registry key for setting value", {
    return FALSE;
  });

  lResult = RegSetValueExW(
    hKey,
    valueName,
    0,
    REG_DWORD,
    (BYTE *) &data, sizeof(data)
  );

  FunctionHandlerR(lResult, "Error setting registry value", {
    RegCloseKey(hKey);
    return FALSE;
  });

  RegCloseKey(hKey);
  return TRUE;
}

// Function to read a string value from the registry
DWORD GetRegistryDWValue(HKEY hKeyParent, LPCWSTR subkey, LPCWSTR valueName) {
  HKEY hKey;
  LONG lResult = RegOpenKeyExW(
    hKeyParent,
    subkey,
    0,
    KEY_QUERY_VALUE,
    &hKey
  );

  FunctionHandlerR(lResult, "Error opening registry key for querying value", {
    return 0;
  });

  DWORD dwType;
  DWORD dwSize = 0;
  lResult = RegQueryValueExW(
    hKey,
    valueName,
    NULL,
    &dwType,
    NULL,
    &dwSize
  );

  FunctionHandlerR(lResult != ERROR_SUCCESS || dwType != REG_DWORD, "Error querying registry value size or type mismatch", {
    RegCloseKey(hKey);
    return 0;
  });

  DWORD dwValueData;
  lResult = RegQueryValueExW(
    hKey,
    valueName,
    NULL,
    &dwType,
    (LPBYTE)&dwValueData,
    &dwSize
  );

  FunctionHandlerR(lResult, "Error Error reading registry value", {
    RegCloseKey(hKey);
    return 0;
  });

  RegCloseKey(hKey);
  return dwValueData;
}

// Function to set a string value in the registry
BOOL SetRegistrySZValue(HKEY hKeyParent, LPCWSTR subkey, LPCWSTR valueName, LPCWSTR data) {
  HKEY hKey;
  LONG lResult = RegOpenKeyExW(
    hKeyParent,
    subkey,
    0,
    KEY_SET_VALUE,
    &hKey
  );

  FunctionHandlerR(lResult, "Error opening registry key for setting value", {
    return FALSE;
  });

  lResult = RegSetValueExW(
    hKey,
    valueName,
    0,
    REG_SZ,
    (LPBYTE)data,
    (wcslen(data) + 1) * sizeof(wchar_t)
  );

  FunctionHandlerR(lResult, "Error setting registry value", {
    RegCloseKey(hKey);
    return FALSE;
  });

  RegCloseKey(hKey);
  return TRUE;
}

// Function to read a string value from the registry
LPTSTR GetRegistrySZValue(HKEY hKeyParent, LPCWSTR subkey, LPCWSTR valueName) {
  HKEY hKey;
  LONG lResult = RegOpenKeyExW(
    hKeyParent,
    subkey,
    0,
    KEY_QUERY_VALUE,
    &hKey
  );

  FunctionHandlerR(lResult, "Error opening registry key for querying value", {
    return NULL;
  });

  DWORD dwType;
  DWORD dwSize = 0;
  lResult = RegQueryValueExW(
    hKey,
    valueName,
    NULL,
    &dwType,
    NULL,
    &dwSize
  );

  FunctionHandlerR(lResult != ERROR_SUCCESS || dwType != REG_SZ, "Error querying registry value size or type mismatch", {
    RegCloseKey(hKey);
    return NULL;
  });

  HLOCAL lpDataBuf = LocalAlloc(LMEM_ZEROINIT, (dwSize * sizeof(TCHAR)));
  lResult = RegQueryValueExW(
    hKey,
    valueName,
    NULL,
    &dwType,
    (LPBYTE)lpDataBuf,
    &dwSize
  );

  FunctionHandlerR(lResult, "Error Error reading registry value", {
    RegCloseKey(hKey);
    return NULL;
  });

  RegCloseKey(hKey);
  return (LPTSTR)lpDataBuf;
}

#pragma endregion

#pragma region Window GUI

void RestoreFromTray(HWND hwnd, HANDLE hThread)
{
  if(hThread != NULL)
    g_exitFromThread = true;//TerminateThread(hThread, 0);
  resetWallpaperWindow();
  ShowWindow(hwnd, SW_SHOWNORMAL);
  SetForegroundWindow(hwnd);
  SetActiveWindow(hwnd);
  Shell_NotifyIcon(NIM_DELETE, &g_tnd);
}

void AddTrayIcon(HWND hwnd)
{
  g_tnd.cbSize = sizeof(NOTIFYICONDATA);
  g_tnd.hWnd = hwnd;
  g_tnd.uID = ID_TRAY_ICON;
  g_tnd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  g_tnd.uCallbackMessage = WM_TRAYICON;
  g_tnd.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
  wcscpy_s(g_tnd.szTip, WideFromUtf8("School Days Counter"));

  Shell_NotifyIcon(NIM_ADD, &g_tnd);
}

void MinimizeToTray(HWND hwnd)
{
  ShowWindow(hwnd, SW_HIDE);
  AddTrayIcon(hwnd);
}

void ShowTrayMenu(HWND hwnd)
{
  POINT pt;
  GetCursorPos(&pt);

  HMENU hMenu = CreatePopupMenu();
  if (hMenu)
  {
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_RESTORE, WideFromUtf8("Вернуться в меню"));
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, WideFromUtf8("Выход"));
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hMenu);
  }
}

std::ostream&
operator<<(std::ostream& os, const LOGFONTW& x)
{
  os << x.lfHeight << " lfHeight\n"
      << x.lfWidth << " lfWidth\n"
      << x.lfEscapement << " lfEscapement\n"
      << x.lfOrientation << " lfOrientation\n"
      << x.lfWeight << " lfWeight\n"
      << (int)x.lfItalic << " lfItalic\n"
      << (int)x.lfUnderline << " lfUnderline\n"
      << (int)x.lfStrikeOut << " lfStrikeOut\n"
      << (int)x.lfCharSet << " lfCharSet\n"
      << (int)x.lfOutPrecision << " lfOutPrecision\n"
      << (int)x.lfClipPrecision << " lfClipPrecision\n"
      << (int)x.lfQuality << " lfQuality\n"
      << (int)x.lfPitchAndFamily << " lfPitchAndFamily\n";
  wcout << x.lfFaceName << " lfFaceName\n";
  return os;
}
std::ostream&
operator<<(std::ostream& os, const RECT& x);

HWND DoCreateStatusBar(HWND hwnd)
{
  HWND hwndStatus = CreateWindow(STATUSCLASSNAME, (PCTSTR)NULL,
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, hwnd, (HMENU)NULL, NULL, NULL);

  INT paParts[3] = {1, 78, -1};
  SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)3, (LPARAM)
              paParts);

  return hwndStatus;
}

DWORD WINAPI MainSDCThreadProc(CONST LPVOID lpParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static LPCWSTR wndSubKey = L"SOFTWARE\\SchoolDaysCounter";
  static LPCWSTR configPathValueName = L"ConfigPath";
  static LPCWSTR runInTrayValueName = L"RunInTrayFirst";
  static const char* myValueData = "Hello, Registry, привет!";
  static HWND hEdit = NULL;
  static HANDLE hThread = NULL;
  static HANDLE hMutex = NULL;
  switch (msg)
  {
  case WM_CREATE:
  {
    hMutex = CreateMutexW(NULL, TRUE, xorstr_(L"SchoolDayCounterMutex1"));
    FunctionHandlerR(hMutex == INVALID_HANDLE_VALUE, "Could not create mutex", {
      PostQuitMessage(0);
      break;
    });
    LPTSTR rConfigPath = NULL;
    DWORD rRunInTrayFlag;
    if (CreateRegistryKey(HKEY_CURRENT_USER, wndSubKey)) {
      if(!CheckRegistryValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName, REG_SZ)){
        SetRegistrySZValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName, L"sdc_config.json");
      }
      if(!CheckRegistryValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName, REG_DWORD)){
        SetRegistryDWValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName, 0);
      }

      // RegDeleteTreeW(HKEY_CURRENT_USER, wndSubKey);
    }else{
      PostQuitMessage(0);
      break;
    }
    rConfigPath = GetRegistrySZValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName);
    if (rConfigPath != NULL) {
      wcout << "Read from registry: " << rConfigPath << endl;
    }
    rRunInTrayFlag = GetRegistryDWValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName);
      cout << "Read from registry: " << rRunInTrayFlag << endl;
    RECT dWnd;
    GetClientRect(hwnd, &dWnd);
    //cout << dWnd << endl;
    HDC hdc = GetDC(hwnd);
    NONCLIENTMETRICS metrics = {};
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

    HFONT guiFont = CreateFontIndirect(&metrics.lfMessageFont);

    HFONT hOldFont = (HFONT)SelectObject(hdc, guiFont);
    HWND hStatusBar = DoCreateStatusBar(hwnd);
    RECT dStatusBar;
    GetClientRect(hStatusBar, &dStatusBar);
    //cout << dStatusBar << endl;
    int dWhndHalf = (dWnd.right-15)/2;
    CreateWindowW(WC_BUTTON, WideFromUtf8("Обновить конфиг"),
                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  5, dWnd.bottom-dStatusBar.bottom-50, dWhndHalf, 20, hwnd, (HMENU)ID_BUTTON1, NULL, NULL);

    CreateWindowW(WC_BUTTON, WideFromUtf8("Восстановить обои"),
                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  dWhndHalf+10, 5, dWhndHalf+1, 40, hwnd, (HMENU)ID_BUTTON2, NULL, NULL);

    CreateWindowW(WC_BUTTON, WideFromUtf8("Запуск"),
                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  5, 5, dWhndHalf, 40, hwnd, (HMENU)ID_BUTTON3, NULL, NULL);

    HWND hSelectFile = CreateWindowW(WC_BUTTON, WideFromUtf8("Выбрать"),
                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                  5, dWnd.bottom-dStatusBar.bottom-25, 60, 20, hwnd, (HMENU)ID_BUTTON4, NULL, NULL);

    HWND hRunInTray = CreateWindowW(WC_BUTTON, WideFromUtf8("Запускать в трее"),
                  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                  dWhndHalf+10, dWnd.bottom-dStatusBar.bottom-50, dWhndHalf+1, 20, hwnd, (HMENU)ID_BUTTON5, NULL, NULL);
    Button_SetCheck(hRunInTray, rRunInTrayFlag > 0);

    RECT dSelectFile;
    GetClientRect(hSelectFile, &dSelectFile);

    hEdit = CreateWindowW(WC_EDIT, rConfigPath,
                  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                  70, dWnd.bottom-dStatusBar.bottom-25, dWnd.right-15-dSelectFile.right, 20, hwnd, (HMENU)ID_EDIT1, NULL, NULL);

    SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1, (LPARAM)xorstr_(L"Version 3.1.0"));
    SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)2, (LPARAM)xorstr_(L"Made by github.com/TheFliss"));
    EnumChildWindows(hwnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
      HFONT hfDefault = *(HFONT *) lParam;
      SendMessage(hwnd, WM_SETFONT, (WPARAM) hfDefault, TRUE);
      return TRUE;
    }, (LPARAM)&guiFont);
    SelectObject(hdc, hOldFont);

    ReleaseDC(hwnd, hdc);
    if(rRunInTrayFlag){
      sdc_thread_vars_t* threadParams = new sdc_thread_vars_t(
        hwnd,
        FALSE
      );
      g_exitFromThread = false;
      hThread = CreateThread(NULL, 0, &MainSDCThreadProc, threadParams, 0, NULL);
      delete[] threadParams;
    }
    break;
  }
  case WM_COMMAND:
  {
    switch (LOWORD(wParam))
    {
    case ID_BUTTON1:
    {
      int textLength = GetWindowTextLength(hEdit) + 1;

      LPTSTR pszBuf = NULL;
      pszBuf = (LPTSTR)LocalAlloc(LPTR, textLength * sizeof(TCHAR));

      FunctionHandler(pszBuf == NULL, "Could not allocate pointer");

      GetWindowText(hEdit, pszBuf, textLength);

      SetRegistrySZValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName, pszBuf);

      LocalFree(pszBuf);
      break;
    }
    case ID_BUTTON2:
      resetWallpaperWindow();
      break;
    case ID_BUTTON3:
    {
      DWORD dwExitCode;
      if(hThread != NULL
         && GetExitCodeThread(hThread, &dwExitCode)){
        if(dwExitCode == STILL_ACTIVE){
          MinimizeToTray(hwnd);
          break;
        }
      }
      sdc_thread_vars_t* threadParams = new sdc_thread_vars_t(
        hwnd,
        TRUE
      );
      g_exitFromThread = false;
      hThread = CreateThread(NULL, 0, &MainSDCThreadProc, threadParams, 0, NULL);
      delete[] threadParams;
      break;
    }
    case ID_BUTTON4:
    {
      IFileOpenDialog *pFileOpen;

      HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

      if (SUCCEEDED(hr))
      {
          hr = pFileOpen->Show(NULL);

          if (SUCCEEDED(hr)) {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
              PWSTR pszFilePath;
              hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

              if (SUCCEEDED(hr)) {
                Edit_SetText(hEdit, pszFilePath);
                SetRegistrySZValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName, pszFilePath);
                CoTaskMemFree(pszFilePath);
              }

              pItem->Release();
            }
          }
          pFileOpen->Release();
      }
      break;
    }
    case ID_BUTTON5:
    {
      if (HIWORD(wParam) == BN_CLICKED)
      {
        SetRegistryDWValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName, IsDlgButtonChecked(hwnd, ID_BUTTON5) == BST_CHECKED);
      }
      break;
    }
    case ID_TRAY_RESTORE:
      RestoreFromTray(hwnd, hThread);
      break;
    case ID_TRAY_EXIT:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    }
    break;
  }
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
    {
      MinimizeToTray(hwnd);
      return 0;
    }else
    break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, &g_tnd);
    PostQuitMessage(0);
    break;
  case WM_TRAYICON:
    switch (LOWORD(lParam))
    {
    case WM_LBUTTONUP:
      RestoreFromTray(hwnd, hThread);
      break;
    case WM_RBUTTONUP:
      ShowTrayMenu(hwnd);
      break;
    }
    break;
  default:
    return DefWindowProcW(hwnd, msg, wParam, lParam);
  }
  return 0;
}

FILE* fc;
void DestroyConsole(){
  fclose(fc);
  FreeConsole();
}

void InitializeConsole(){
  AllocConsole();
  freopen_s(&fc, "CONOUT$", "w", stdout);
  freopen_s(&fc, "CONOUT$", "w", stderr);

  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD consoleMode;

  GetConsoleMode(consoleHandle, &consoleMode);
  consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(consoleHandle, consoleMode);
  SetConsoleTitleW(L"[SDC] Debug Console");
}

#pragma endregion

#pragma region Window main

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  setlocale(LC_ALL, "Russian");

  HANDLE hMutex = CreateMutexW(NULL, TRUE, xorstr_(L"SchoolDayCounterMutex0"));

  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBoxW(NULL, WideFromUtf8("Другой экземпляр этого приложения уже запущен."), WideFromUtf8("Внимание"), MB_OK | MB_ICONWARNING);
    CloseHandle(hMutex);
    return EXIT_FAILURE;
  }

  INITCOMMONCONTROLSEX icc;
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icc);

  FunctionHandlerR(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | 
      COINIT_DISABLE_OLE1DDE)), "Could not initialize the COM library", {
    return EXIT_FAILURE;
  });

#ifdef DEBUG
  InitializeConsole();
#endif

  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WndProc;
  //wc.style  = CS_HREDRAW | CS_VREDRAW;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
  wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
  wc.lpszClassName = g_szClassName;

  if (!RegisterClassExW(&wc))
  {
    MessageBoxW(NULL, WideFromUtf8("Ошибка регистрации окна!"), WideFromUtf8("Ошибка"), MB_ICONERROR);
    return EXIT_FAILURE;
  }

  HWND hwnd = CreateWindowExW(0, g_szClassName, WideFromUtf8("School Days Counter"),
                              WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 300, 162, NULL, NULL, hInstance, NULL);

  if (!hwnd)
  {
    MessageBoxW(NULL, WideFromUtf8("Ошибка создания окна!"), WideFromUtf8("Ошибка"), MB_ICONERROR);
    return EXIT_FAILURE;
  }

  {
    static LPCWSTR wndSubKey = L"SOFTWARE\\SchoolDaysCounter";
    static LPCWSTR runInTrayValueName = L"RunInTrayFirst";
    if(CheckRegistryValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName, REG_DWORD)
        && GetRegistryDWValue(HKEY_CURRENT_USER, wndSubKey, runInTrayValueName)){
      ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
    }else
      ShowWindow(hwnd, SW_SHOW);
  }

  SetWindowTheme(hwnd, L"Explorer", NULL);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  CoUninitialize();
  CloseHandle(hMutex);

  return (int)msg.wParam;
}


DWORD WINAPI MainSDCThreadProc(CONST LPVOID lpParam) {
  sdc_thread_vars_t *threadParams = (sdc_thread_vars_t*)lpParam;
  LPTSTR lpConfig = NULL;
  {
    static LPCWSTR wndSubKey = L"SOFTWARE\\SchoolDaysCounter";
    static LPCWSTR configPathValueName = L"ConfigPath";
    if(CheckRegistryValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName, REG_SZ)){
      lpConfig = GetRegistrySZValue(HKEY_CURRENT_USER, wndSubKey, configPathValueName);
    }
  }
  //cout << (LPVOID)lpConfig << endl;
  //MessageBox(NULL, lpConfig, L"Lol", MB_OK);
  DWORD i;
  HWND hWallpaper = FindWallpaperWindow();
  if (!hWallpaper) {
    MessageBox(nullptr, L"Не удалось найти окно обоев", L"Ошибка", MB_ICONERROR);
    //RestoreFromTray(threadParams->hwnd, NULL);
    ExitThread(1);
  }

  resetWallpaperWindow();

  vector<unique_ptr<Timer>> timers;

  sdc_config_t sdc_config;
  try {
    auto sdccstream = ifstream(ConvertWideToUtf8(wstring(lpConfig)));

    json j = json::parse(sdccstream, nullptr, true, true);

    sdc_config = j.get<sdc_config_t>();

    sdccstream.close();
  } catch(const exception& e) {
    string errmsg = string("Exception caught while reading config file:\n") + e.what();
    MessageBoxW(NULL, WideFromUtf8s(errmsg), WideFromUtf8("Ошибка"), MB_ICONERROR);
    //RestoreFromTray(threadParams->hwnd, NULL);
    ExitThread(1);
  }

  {
    for(auto &x : sdc_config.timers){
      Timer current_timer(x);
      timers.push_back(make_unique<Timer>(current_timer));
    }
  }

  HDC hdc = GetDC(hWallpaper);

  RECT cr;
  GetClientRect(hWallpaper, &cr);

  float scale = (float)GetDpiForWindow(hWallpaper)/96.0f;

  cr.right = (LONG)round((float)cr.right*scale);
  cr.bottom = (LONG)round((float)cr.bottom*scale);

  //saving original wp hdc
  HDC hdcSRC = CreateCompatibleDC(hdc);
  HBITMAP hbmSRC = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
  HANDLE hOldSRC = SelectObject(hdcSRC, hbmSRC);
  BitBlt(hdcSRC, 0, 0, cr.right, cr.bottom, hdc, 0, 0, SRCCOPY);

  HWND hwndDesktop = GetDesktopWindow(); 

  HDC hdcMem = CreateCompatibleDC(hdc);
  HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
  HANDLE hOld = SelectObject(hdcMem, hbmMem);

  if(threadParams->Minimize)
    MinimizeToTray(threadParams->hwnd);
  while(1){
    if(g_exitFromThread){
      break;
    }
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
  //RestoreFromTray(threadParams->hwnd, NULL);
  //LocalFree(lpConfig);
  ExitThread(0);
}

//int main(int argc, char const *argv[]) {

//  bool restore = false;
//  FilePath SDCConfigFile = FilePath(getexepath() / "sdc_config.json");

//  for (int i = 1; i < argc; i++) {
//    try
//    {
//      if(strcmp(argv[i], xorstr_("-config-path")) == 0){
//        test_arg(i, argc, argv[i]);
//        SDCConfigFile = FilePath(string(argv[i+1]));
//        i++;
//      }else 
//      if(strcmp(argv[i], xorstr_("-restore")) == 0 || strcmp(argv[i], xorstr_("-r")) == 0){
//        restore = true;
//      }else{
//        if(strcmp(argv[i], xorstr_("-help")) != 0) printf_s(xorstr_("\nERROR: Unknown argument %s\n"), argv[i]);
//        usage(argv[0]);
//        exit(1);
//      }
//    }
//    catch(const exception& e)
//    {
//      cerr << "\nException caught while parsing argument \"" << argv[i] << "\": " << e.what() << endl;
//        usage(argv[0]);
//      return EXIT_FAILURE;
//    }
//  }

//  FilePath restoreFile = FilePath(getexepath() / "original_path.txt");

//  //DEPRECATED
//  if(!PathFileExistsA(restoreFile.fp_s.c_str())){
//    wchar_t *szWallpaperPath[MAX_PATH];
//    FunctionHandlerL(!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");

//    //Writing restore file
//    HANDLE hRestoreFile = CreateFileA(restoreFile.fp_s.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

//    HandleCreateFileSD(hRestoreFile, restoreFile.fp_s.c_str());

//    string cyrillic_text = ConvertWideToUtf8(wstring((LPTSTR)szWallpaperPath));

//    WriteFileS(hRestoreFile, cyrillic_text.c_str(), (DWORD)cyrillic_text.length() * sizeof(char), NULL, NULL);

//    CloseHandle(hRestoreFile);

//#ifdef DEBUG
//    wprintf_s(xorstr_(TEXT("%s\n")), (LPTSTR)szWallpaperPath);
//#endif
//  }

//  if(restore){
//    printf_s(xorstr_("\nRestoring orginal wallpaper\n"));
//    HANDLE hRestoreFile = CreateFileA(restoreFile.fp_s.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//    HandleCreateFileSD(hRestoreFile, restoreFile.fp_s.c_str());

//    DWORD size = GetFileSize(hRestoreFile, NULL);
//    PVOID virtualpointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

//    ReadFileS(hRestoreFile, virtualpointer, size, NULL, NULL);

//    wstring cyrillic_text = ConvertUtf8ToWide(string((LPCSTR)virtualpointer));

//    FunctionHandlerL(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (LPVOID)cyrillic_text.c_str(), SPIF_UPDATEINIFILE), "SDC", "Cannot set wallpaper path");
//#ifdef DEBUG
//    wprintf_s(xorstr_(TEXT("%s\n")), cyrillic_text.c_str());
//#endif

//    CloseHandle(hRestoreFile);
//    return 0;
//  }

//  LPTSTR szWallpaperPath[MAX_PATH];
//  FunctionHandlerL(!SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");
//  FunctionHandlerL(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szWallpaperPath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE), "SDC", "Cannot set wallpaper path");

//  HWND hWallpaper = FindWallpaperWindow();
//  if (!hWallpaper) {
//    MessageBox(nullptr, L"Не удалось найти окно обоев", L"Ошибка", MB_ICONERROR);
//    return 1;
//  }

//  vector<unique_ptr<Timer>> timers;

//  sdc_config_t sdc_config = parse_config(SDCConfigFile.fp);

//  if(sdc_config.update_delay < 1){
//    printf_s(xorstr_("\nERROR: Update time must be greater than 0\n"));
//    usage(argv[0]);
//    return EXIT_FAILURE;
//  }

//  {
//    for(auto &x : sdc_config.timers){
//      Timer current_timer(x);
//      timers.push_back(make_unique<Timer>(current_timer));
//    }
//  }

//  //ShowWindow(GetConsoleWindow(), SW_HIDE);

//  HDC hdc = GetDC(hWallpaper);

//  RECT cr;
//  GetClientRect(hWallpaper, &cr);

//  float scale = (float)GetDpiForWindow(hWallpaper)/96.0f;
//  cout << "scale: " << scale << endl;

//  cr.right = (LONG)round((float)cr.right*scale);
//  cr.bottom = (LONG)round((float)cr.bottom*scale);

//  //saving original wp hdc
//  HDC hdcSRC = CreateCompatibleDC(hdc);
//  HBITMAP hbmSRC = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
//  HANDLE hOldSRC = SelectObject(hdcSRC, hbmSRC);
//  BitBlt(hdcSRC, 0, 0, cr.right, cr.bottom, hdc, 0, 0, SRCCOPY);

//  HWND hwndDesktop = GetDesktopWindow(); 
//  //cout << "scale: " << ((float)dpi/96.0f) << endl;
//  cout << hWallpaper << endl;
//  cout << hwndDesktop << endl;
//  cout << cr.left << " left "
//      << cr.top << " top "
//      << cr.right << " right "
//      << cr.bottom << " bottom" << endl;
//  HDC hdcMem = CreateCompatibleDC(hdc);
//  HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
//  HANDLE hOld = SelectObject(hdcMem, hbmMem);

//  while(1){
//    BitBlt(hdcMem, 0, 0, cr.right, cr.bottom, hdcSRC, 0, 0, SRCCOPY);
//    for(auto &x : timers)
//      x->render(hdcMem, &cr);

//    BitBlt(hdc, 0, 0, cr.right, cr.bottom, hdcMem, 0, 0, SRCCOPY);
//    Sleep(sdc_config.update_delay);
//  }

//  BitBlt(hdc, 0, 0, cr.right, cr.bottom, hdcSRC, 0, 0, SRCCOPY);
//  SelectObject(hdcMem, hOld);
//  SelectObject(hdcSRC, hOldSRC);

//  DeleteObject(hbmMem);
//  DeleteDC (hdcMem);
//  DeleteObject(hbmSRC);
//  DeleteDC (hdcSRC);

//  ReleaseDC(hWallpaper, hdc);

//  return 0;
//}

#pragma endregion