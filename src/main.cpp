
#include "extwinapi.h"
#include "timer.h"
#include "struct_mapping/struct_mapping.h"
#include "util.h"

using namespace util;

static void usage(const char *prog){

  vector<string> message{
    xorstr_("The following is required:"),
    "",
    xorstr_("Optional arguments:"),
    xorstr_("\t-enable-auto-change           ()"),
    xorstr_("\t-update-time                  ( in miliseconds, default 60000)"),
    xorstr_("\t-set-save-quality             (sets output image quality (JPG format) [0; 33] is Lowest quality, [34; 66] is Middle quality, [67; 100] is Highest quality.)"),
    xorstr_("\t-set-font-size                (sets counter font size in image height percnetage, dafault 6)"),
    xorstr_("\t-set-offset-x                 (sets X offset of counter in image width percnetage, dafault 50)"),
    xorstr_("\t-set-offset-y                 (sets Y offset of counter in image height percnetage, dafault 50)"),
    xorstr_("\t-set-margin-x                 (sets X margin of background in image width percnetage, dafault 1)"),
    xorstr_("\t-set-margin-y                 (sets Y margin of background in image height percnetage, dafault 1)"),
    xorstr_("\t-set-text-color               (sets text color in hex format RGBA, default #FFFFFFFF)"),
    xorstr_("\t-set-bg-color                 (sets background color in hex format RGBA, default #000000C0)"),
    xorstr_("\t-high-precision               ()"),
    xorstr_("\t-get-wallpaper -gw            (gets desktop wallpaper)"),
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

int main(int argc, char const *argv[]) {
  setlocale(LC_ALL, "Russian");
  SetConsoleOutputCP(866);

  //cout << (getexepath()) << endl;


  bool get_wallpaper = false;
  bool restore = false;
  bool highp = false;
  bool auto_change = false;
  int update_time = 60000;
  int font_size = 6;
  int offset_x = 50;
  int offset_y = 50;
  int margin_x = 1;
  int margin_y = 1;
  int save_quality = 100;
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Color bgColor = {0, 0, 0, 240};

  for (int i = 1; i < argc; i++) {
    try
    {
      if(strcmp(argv[i], xorstr_("-update-time")) == 0){
        test_arg(i, argc, argv[i]);
        update_time = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-save-quality")) == 0){
        test_arg(i, argc, argv[i]);
        save_quality = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-font-size")) == 0){
        test_arg(i, argc, argv[i]);
        font_size = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-margin-x")) == 0){
        test_arg(i, argc, argv[i]);
        margin_x = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-margin-y")) == 0){
        test_arg(i, argc, argv[i]);
        margin_y = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-offset-x")) == 0){
        test_arg(i, argc, argv[i]);
        offset_x = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-offset-y")) == 0){
        test_arg(i, argc, argv[i]);
        offset_y = stoi(argv[i+1]);
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-text-color")) == 0){
        test_arg(i, argc, argv[i]);
        uint32_t hex_color = byteswap((uint32_t)stoul(argv[i+1], nullptr, 16));
        textColor = *(SDL_Color *)&hex_color;
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-set-bg-color")) == 0){
        test_arg(i, argc, argv[i]);
        uint32_t hex_color = byteswap((uint32_t)stoul(argv[i+1], nullptr, 16));
        bgColor = *(SDL_Color *)&hex_color;
        //cout << hex << (int)bgColor.r << " " << (int)bgColor.g << " " << (int)bgColor.b << " " << (int)bgColor.a << " " << endl;
        i++;
      }else 
      if(strcmp(argv[i], xorstr_("-high-precision")) == 0){
        highp = true;
      }else 
      if(strcmp(argv[i], xorstr_("-enable-auto-change")) == 0){
        auto_change = true;
      }else 
      if(strcmp(argv[i], xorstr_("-get-wallpaper")) == 0 || strcmp(argv[i], xorstr_("-gw")) == 0){
        get_wallpaper = true;
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

  //string dateString = "2025-09-06 18:09:00";
  //tm tm_struct = {}; // Initialize to all zeros
  //istringstream ss(dateString);
  //ss >> get_time(&tm_struct, "%Y-%m-%d %H:%M:%S"); // Specify the format
  //if (ss.fail()) {
  //    cerr << "Date parsing failed!" << endl;
  //} else {
  //    time_t time_value = mktime(&tm_struct); // Convert tm to time_t
  //    // Use time_value as needed
  //    cout << "Parsed date: " << asctime(localtime(&time_value));
  //}

  //  // 1. Find Progman
  //  HWND progman = FindWindowW(L"Progman", NULL);
  //  if (progman == NULL) {
  //      // Handle error
  //      return 1;
  //  }

  //  // 2. Send message to Progman to reveal WorkerW
  //  SendMessageW(progman, 0x052C, 0, 0);

  //  // 3. Find WorkerW (this might require iterating through child windows of Progman)
  //  // This is a simplified placeholder; actual WorkerW finding is more complex.
  //  HWND workerW = FindWindowW(L"WorkerW", NULL); 
  //  if (workerW == NULL) {
  //      // Handle error
  //      return 1;
  //  }

  //  // 4. Get device context for WorkerW
  //  HDC hdcWorkerW = GetDC(workerW);
  //  if (hdcWorkerW == NULL) {
  //      // Handle error
  //      return 1;
  //  }

  //  // 5. Animation loop (example: drawing a simple rectangle)
  //  RECT rect;
  //  GetClientRect(workerW, &rect); // Get client area of WorkerW

  //  for (int i = 0; i < 100; ++i) { // Simple loop for demonstration
  //      // Clear previous drawing (optional, depending on animation)
  //      FillRect(hdcWorkerW, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH)); 

  //      // Draw something new
  //      RECT currentRect = {i, i, i + 50, i + 50}; // Example: moving rectangle
  //      FillRect(hdcWorkerW, &currentRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

  //      Sleep(50); // Small delay for animation
  //  }

  //  // Clean up
  //  ReleaseDC(workerW, hdcWorkerW);


  //return 0;

  FilePath origWallpaper = FilePath(getexepath() / "original.jpg");
  FilePath restoreFile = FilePath(getexepath() / "original_path.txt");
  FilePath SDCConfigFile = FilePath(getexepath() / "sdc_config.json");

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
  //ostringstream out_json_data;
  //struct_mapping::map_struct_to_json(timers, out_json_data, "  ");

  //cout << out_json_data.str() << endl;

  if(update_time < 1){
    printf_s(xorstr_("\nERROR: Update time must be greater than 0\n"));
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Initialize SDL stuff
  if (!SDL_Init(0)) {
      printf_s(xorstr_("SDL could not initialize! SDL_Error: %s\n"), SDL_GetError());
      SDL_Quit();
      return EXIT_FAILURE;
  }
  if (!TTF_Init()) {
      printf_s(xorstr_("SDL_ttf could not initialize! SDL_Error: %s\n"), SDL_GetError());
      SDL_Quit();
      return EXIT_FAILURE;
  }

  if(!PathFileExistsA(origWallpaper.fp_s.c_str()) || !PathFileExistsA(restoreFile.fp_s.c_str())){
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

    HANDLE in_file_handle = CreateFile((LPTSTR)szWallpaperPath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    HandleCreateFileWSD(in_file_handle, (LPTSTR)szWallpaperPath);

    DWORD size = GetFileSize(in_file_handle, NULL);
    PVOID virtualpointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

    ReadFileS(in_file_handle, virtualpointer, size, NULL, NULL);

    CloseHandle(in_file_handle);

    HANDLE out_file_handle = CreateFileA(origWallpaper.fp_s.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    HandleCreateFileSD(out_file_handle, origWallpaper.fp_s.c_str());

    //Copying to output
    WriteFileS(out_file_handle, virtualpointer, size, NULL, NULL);

    CloseHandle(out_file_handle);
  }

  fs::path output_fn = origWallpaper.fp.stem();
  FilePath output_file = FilePath(getexepath() / (output_fn.string()+xorstr_("_updated")+origWallpaper.fp.extension().string()));

  SDL_Surface* imageSurface = IMG_Load(origWallpaper.fp_s.c_str());
  if (!imageSurface){
    printf_s(xorstr_("Wallpaper image could not initialize! SDL_Error: %s\n"), SDL_GetError());
    TTF_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  offset_x *= imageSurface->w/100;
  offset_y *= imageSurface->h/100;

  TTF_Font* font = TTF_OpenFont(xorstr_("C:/Windows/Fonts/calibri.ttf"), imageSurface->h/100*font_size);
  if (!font){
    printf_s(xorstr_("Font could not initialize! SDL_Error: %s\n"), SDL_GetError());
    TTF_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
  }

  vector<unique_ptr<Timer>> timers;

  struct_mapping::reg(&sdc_config_t::background_run, "background_run");
  struct_mapping::reg(&sdc_config_t::timers, "timers");
  struct_mapping::reg(&timer_t::text, "text");
  struct_mapping::reg(&timer_t::end_date, "end_date");
  struct_mapping::reg(&timer_t::anchor, "anchor");
  struct_mapping::reg(&timer_t::offset, "offset");
  struct_mapping::reg(&timer_t::margin, "margin");
  struct_mapping::reg(&timer_t::detailed_time, "detailed_time");
  struct_mapping::reg(&timer_t::text_color, "text_color");
  struct_mapping::reg(&timer_t::bg_color, "bg_color");

  sdc_config_t sdc_config;
  auto sdccstream = ifstream(SDCConfigFile.fp);
  struct_mapping::map_json_to_struct(sdc_config, sdccstream);

  {
    for(auto &x : sdc_config.timers){
      Timer current_timer(x);
      timers.push_back(make_unique<Timer>(current_timer));
    }
  }

  //cout << ConvertWideToANSI(ConvertUtf8ToWide(sdc_config.timers[0].text)) << endl;

  //int prev_secs = -1;
  //int prev_mins = -1;
  //int prev_days = -1;
  //int prev_hours = -1;
  while(1){
  //SYSTEMTIME st = {2025, 9, 1, 1, 0, 0, 0, 0};
    //time_t st1;
    
    //string dateString = "2025-09-06 18:09:00";
    //tm tm_struct = {}; // Initialize to all zeros
    //istringstream ss(dateString);
    //ss >> get_time(&tm_struct, "%Y-%m-%d %H:%M:%S"); // Specify the format
    //if (ss.fail()) {
    //    cerr << "Date parsing failed!" << endl;
    //} else {
    //    char buf[32];
    //    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &tm_struct);
    //    cout << "Parsed date: " << buf << endl;
    //}
    //SYSTEMTIME st;
    //GetLocalTime(&st);

    //bool is_summer = st.wMonth > 5 && st.wMonth < 9;

    //DWORD SchoolDays = (!(st.wYear&0b11)+28)+31*5+30*3;

    //int day_acc = 0;
    //for (int i = 0; i < 9; i++) {
    //  int mi = i+(i > 3 ? -3 : 9);
    //  if(mi == st.wMonth){
    //    day_acc += st.wDay;
    //    break;
    //  }
    //  day_acc += get_month_length(mi, st.wYear);
    //}

    //int curr_seconds = 59-st.wSecond;
    //int curr_mins = 59-st.wMinute;
    //int curr_hours = 23-st.wHour;
    //int curr_days = SchoolDays-day_acc-1;

    //if(prev_secs == curr_seconds && prev_mins == curr_mins && prev_hours == curr_hours && prev_days == curr_days){
    //  //Sleep(update_time*100);
    //  continue;
    //};

    //prev_secs = curr_seconds;
    //prev_mins = curr_mins;
    //prev_hours = curr_hours;
    //prev_days = curr_days;

    //char display_text[250];
    //if(highp){
    //  sprintf_s(display_text, xorstr_("До лета осталось: %d %s, %d %s, %d %s и %d %s!"),
    //    prev_days,
    //    declination_word(prev_days, xorstr_("день"), xorstr_("дня"), xorstr_("дней")),
    //    prev_hours,
    //    declination_word(prev_hours, xorstr_("час"), xorstr_("часа"), xorstr_("часов")),
    //    prev_mins,
    //    declination_word(prev_mins, xorstr_("минута"), xorstr_("минуты"), xorstr_("минут")),
    //    prev_secs,
    //    declination_word(prev_secs, xorstr_("секунда"), xorstr_("секунды"), xorstr_("секунд"))
    //  );
    //}else
    //{
    //  sprintf_s(display_text, xorstr_("До лета осталось %d %s и %d %s!"),
    //    prev_days,
    //    declination_word(prev_days, xorstr_("день"), xorstr_("дня"), xorstr_("дней")),
    //    prev_hours,
    //    declination_word(prev_hours, xorstr_("час"), xorstr_("часа"), xorstr_("часов"))
    //  );
    //}

    //#ifdef DEBUG

    //DWORD cMonthDays = get_month_length(st.wMonth, st.wYear);
    //cout << "Time (UTC): "
    //          << "\nisLeapYear " << (!(st.wYear&0b11))
    //          << "\nwYear " << st.wYear
    //          << "\nwMonth " << st.wMonth
    //          << "\nwDay " << st.wDay
    //          << "\nwHour " << st.wHour
    //          << "\nwMinute " << st.wMinute
    //          << "\nSchoolDays " << SchoolDays
    //          << "\ncMonthDays " << cMonthDays
    //          << "\nis_summer " << is_summer
    //          << "\nday_acc " << day_acc
    //          << endl;
    //#endif

    //// Render text to a surface
    //SDL_Surface* textSurface = TTF_RenderText_Blended(font, display_text, strlen(display_text), textColor);
    //if (!textSurface){
    //  printf_s(xorstr_("Text surface could not initialize! SDL_Error: %s\n"), SDL_GetError());
    //  TTF_Quit();
    //  SDL_Quit();
    //  return EXIT_FAILURE;
    //}
    //int lmargin_x = margin_x*imageSurface->w/100;
    //int lmargin_y = margin_y*imageSurface->h/100;

    //SDL_Rect textRect = {lmargin_x/2, lmargin_y/2, textSurface->w, textSurface->h};

    //lmargin_x += textSurface->w;
    //lmargin_y += textSurface->h;

    //SDL_Rect bgRect = {0, 0, lmargin_x, lmargin_y};
    //SDL_Surface* bgSurface = SDL_CreateSurface(bgRect.w, bgRect.h, textSurface->format);

    //SDL_FillSurfaceRect(bgSurface, &bgRect, SDL_MapRGBA(SDL_GetPixelFormatDetails(bgSurface->format), NULL, bgColor.r, bgColor.g, bgColor.b, bgColor.a));
    //SDL_BlitSurface(textSurface, NULL, bgSurface, &textRect);

    //SDL_Rect destRect = {offset_x-lmargin_x/2, offset_y-lmargin_y/2, lmargin_x, lmargin_y};
    SDL_Surface* convertedImageSurface = SDL_ConvertSurface(imageSurface, SDL_PIXELFORMAT_RGBA8888);
    if (!convertedImageSurface){
      printf_s(xorstr_("Wallpaper image could not be converted to RGBA format! SDL_Error: %s\n"), SDL_GetError());
      TTF_Quit();
      SDL_Quit();
      return EXIT_FAILURE;
    }
    for(auto &x : timers)
      x->render(convertedImageSurface, font);

    //SDL_BlitSurface(bgSurface, NULL, convertedImageSurface, &destRect);

    IMG_SaveJPG(convertedImageSurface, output_file.fp_s.c_str(), save_quality);
    SDL_DestroySurface(convertedImageSurface);

    FunctionHandlerL(!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (LPVOID)output_file.fp_s.c_str(), SPIF_UPDATEINIFILE), "SDC", "Cannot set wallpaper path");

    if(!auto_change) break;
    else{
      SDL_DestroySurface(imageSurface);
      imageSurface = IMG_Load(origWallpaper.fp_s.c_str());
      if (!imageSurface){
        printf_s(xorstr_("Wallpaper image could not initialize! SDL_Error: %s\n"), SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
      }
      ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
      Sleep(update_time);
    }
  }
  SDL_DestroySurface(imageSurface);
  TTF_CloseFont(font);

  TTF_Quit();
  SDL_Quit();

  return 0;
}