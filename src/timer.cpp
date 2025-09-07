#include "timer.h"
#include "util.h"

using namespace util;

Timer::Timer(timer_t config) {
  istringstream ss(config.end_date);
  ss >> get_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
  tm_struct.tm_year += 1900;
  if (ss.fail()) {
    cerr << "Date in config parsing failed!" << endl;
    exit(1);
  }
  //else {
  //  char buf[32];
  //  strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &tm_struct);
  //  cout << "Parsed date: " << buf << endl;
  //}

  try {
    uint32_t hex_color = byteswap((uint32_t)stoul(config.text_color, nullptr, 16));
    textColor = *(SDL_Color *)&hex_color;
    hex_color = byteswap((uint32_t)stoul(config.bg_color, nullptr, 16));
    bgColor = *(SDL_Color *)&hex_color;
  }
  catch(const exception& e)
  {
    cerr << "\nException caught while parsing \"bgColor\" or \"textColor\" in timer config: " << e.what() << endl;
    exit(1);
  }
  font_size = config.font_size;

  if(config.margin.size() != 2)
  {
    printf_s(xorstr_("Margin must be equal to 2\n"));
    exit(1);
  }else
    margin = {config.margin[0], config.margin[1]};

  if(config.offset.size() != 2)
  {
    printf_s(xorstr_("Offset must be equal to 2\n"));
    exit(1);
  }else
    offset = {config.offset[0], config.offset[1]};
  
  if(config.anchor == "top_left"){
    anchor = 0;
  }else if(config.anchor == "top_middle"){
    anchor = 1;
  }else if(config.anchor == "top_right"){
    anchor = 2;
  }else if(config.anchor == "left_middle"){
    anchor = 3;
  }else if(config.anchor == "center"){
    anchor = 4;
  }else if(config.anchor == "right_middle"){
    anchor = 5;
  }else if(config.anchor == "bottom_left"){
    anchor = 6;
  }else if(config.anchor == "bottom_middle"){
    anchor = 7;
  }else if(config.anchor == "bottom_right"){
    anchor = 8;
  }else{
    printf_s(xorstr_("anchor must be equal to one of (top_left, top_middle, top_right, left_middle, center, right_middle, bottom_left, bottom_middle, bottom_right)\n"));
    exit(1);
  }

  text = config.text;
}

Timer::~Timer() {
  if(!font)
    TTF_CloseFont(font);
}

void Timer::render(SDL_Surface * imageSurface) {
  if(!font){
    font = TTF_OpenFont(xorstr_("C:/Windows/Fonts/calibri.ttf"), (int)(font_size*imageSurface->h/100.0f));
    if (!font){
      printf_s(xorstr_("Font could not initialize! SDL_Error: %s\n"), SDL_GetError());
      TTF_Quit();
      SDL_Quit();
      exit(1);
    }
  }
  cout << "rendered: " << ConvertWideToANSI(ConvertUtf8ToWide(text)) << endl;
  //SYSTEMTIME st = {2025, 9, 1, 7, 0, 0, 0, 0};
  SYSTEMTIME st;
  GetLocalTime(&st);

  bool is_summer = st.wMonth > 5 && st.wMonth < 9;

  DWORD SchoolDays = (!(st.wYear&0b11)+28)+31*5+30*3;

  int wait_days = 0;
  //int d1_year = st.wYear;
  //int d2_year = tm_struct.tm_year;
  //int d1_mon = st.wMonth;
  //int d2_mon = tm_struct.tm_mon+1;
  //int d1_day = st.wDay;
  //int d2_day = tm_struct.tm_mday;
  //  while ((d1_year*366 + d1_mon*31 + d1_day) < (d2_year*366 + d2_mon*31 + d2_day))
  //  {
  //      wait_days++;
 
  //      if (++d1_day > get_month_length(d1_mon, d1_year)) {
  //          d1_day = 1;
  //          if (++d1_mon > 12) {
  //              d1_mon = 1;
  //              ++d1_year;
  //          }
  //      }
  //  }
  for (int i = st.wMonth-1; i < 12*(tm_struct.tm_year-st.wYear)+tm_struct.tm_mon; i++)
  {
    int mi = i%12+1;
    int yi = i/12+st.wYear;
    wait_days += get_month_length(mi, yi);
    //if((mi == st.wMonth && yi == st.wYear) || (mi == tm_struct.tm_mon && yi == tm_struct.tm_year))
    //  wait_days--;
    cout << i << " " << mi << " " << get_month_length(mi, yi) << " " << yi << endl;
    /* code */
  }
  wait_days -= st.wDay;
  wait_days += tm_struct.tm_mday;
  
  //for (int i = 0; i < 12; i++) {
  //  cout<< i<< endl;
  //  if(i == tm_struct.tm_mon){
  //    wait_days += tm_struct.tm_mday;
  //    break;
  //  }
  //  wait_days += get_month_length(i, tm_struct.tm_year);
  //}
  //for (int i = st.wMonth; i <= 12; i++) {
  //  cout<< i<< endl;
  //  if(i == st.wMonth){
  //    wait_days -= st.wDay;
  //  }
  //  wait_days += get_month_length(i, st.wYear);
  //}
  //for (int i = st.wYear+1; i < tm_struct.tm_year; i++)
  //  wait_days += 365+!(i&0b11);

  cout << "Time (UTC): "
            << "\nisLeapYear " << (!(tm_struct.tm_year&0b11))
            << "\nwYear " << tm_struct.tm_year
            << "\nwMonth " << tm_struct.tm_mon
            << "\nwDay " << tm_struct.tm_mday
            << "\nwHour " << tm_struct.tm_hour
            << "\nwMinute " << tm_struct.tm_min
            << "\nSchoolDays " << SchoolDays
            << "\nwait_days " << wait_days
            << endl;

  //int day_acc = 0;
  //for (int i = 0; i < 9; i++) {
  //  int mi = i+(i > 3 ? -3 : 9);
  //  if(mi == st.wMonth){
  //    day_acc += st.wDay;
  //    break;
  //  }
  //  day_acc += get_month_length(mi, st.wYear);
  //}

  int curr_seconds = tm_struct.tm_sec-st.wSecond;
  curr_seconds += 60*(curr_seconds < 0);
  int curr_mins = tm_struct.tm_min-st.wMinute;
  curr_mins += 60*(curr_mins < 0);
  int curr_hours = tm_struct.tm_hour-st.wHour;
  curr_hours += 24*(curr_hours < 0);
  int curr_days = wait_days;//SchoolDays-day_acc-1;

  char display_text[250];
  if(detailed_time){
    sprintf_s(display_text, xorstr_("%s %d %s, %d %s, %d %s и %d %s!"),
      text.c_str(),
      curr_days,
      declination_word(curr_days, xorstr_("день"), xorstr_("дня"), xorstr_("дней")),
      curr_hours,
      declination_word(curr_hours, xorstr_("час"), xorstr_("часа"), xorstr_("часов")),
      curr_mins,
      declination_word(curr_mins, xorstr_("минута"), xorstr_("минуты"), xorstr_("минут")),
      curr_seconds,
      declination_word(curr_seconds, xorstr_("секунда"), xorstr_("секунды"), xorstr_("секунд"))
    );
  }else
  {
    sprintf_s(display_text, xorstr_("%s %d %s и %d %s!"),
      text.c_str(),
      curr_days,
      declination_word(curr_days, xorstr_("день"), xorstr_("дня"), xorstr_("дней")),
      curr_hours,
      declination_word(curr_hours, xorstr_("час"), xorstr_("часа"), xorstr_("часов"))
    );
  }

  //#ifdef DEBUG

  DWORD cMonthDays = get_month_length(st.wMonth, st.wYear);
  cout << "Time (UTC): "
            << "\nisLeapYear " << isLeapYear(st.wYear)
            << "\nwYear " << st.wYear
            << "\nwMonth " << st.wMonth
            << "\nwDay " << st.wDay
            << "\nwHour " << st.wHour
            << "\nwMinute " << st.wMinute
            << "\nSchoolDays " << SchoolDays
            << "\ncMonthDays " << cMonthDays
            << "\nis_summer " << is_summer
            << "\nday_acc " << wait_days
            << endl;
  //#endif

  // Render text to a surface
  SDL_Surface* textSurface = TTF_RenderText_Blended(font, display_text, strlen(display_text), textColor);
  if (!textSurface){
    printf_s(xorstr_("Text surface could not initialize! SDL_Error: %s\n"), SDL_GetError());
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }
  int lmargin_x = (int)(margin.x*imageSurface->w/100);
  int lmargin_y = (int)(margin.y*imageSurface->h/100);

  SDL_Rect textRect = {lmargin_x/2, lmargin_y/2, textSurface->w, textSurface->h};

  lmargin_x += textSurface->w;
  lmargin_y += textSurface->h;

  SDL_Rect bgRect = {0, 0, lmargin_x, lmargin_y};
  SDL_Surface* bgSurface = SDL_CreateSurface(bgRect.w, bgRect.h, textSurface->format);

  SDL_FillSurfaceRect(bgSurface, &bgRect, SDL_MapRGBA(SDL_GetPixelFormatDetails(bgSurface->format), NULL, bgColor.r, bgColor.g, bgColor.b, bgColor.a));
  SDL_BlitSurface(textSurface, NULL, bgSurface, &textRect);

  SDL_Rect destRect = {0, 0, lmargin_x, lmargin_y};

  switch (anchor)
  {
    case 0:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f);
      break;
    }
    case 1:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x/2.0f+imageSurface->w/2.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f);
      break;
    }
    case 2:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x+imageSurface->w);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f);
      break;
    }
    case 3:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y/2.0f+imageSurface->h/2.0f);
      break;
    }
    case 4:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x/2.0f+imageSurface->w/2.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y/2.0f+imageSurface->h/2.0f);
      break;
    }
    case 5:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x+imageSurface->w);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y/2.0f+imageSurface->h/2.0f);
      break;
    }
    case 6:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y+imageSurface->h);
      break;
    }
    case 7:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x/2.0f+imageSurface->w/2.0f);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y+imageSurface->h);
      break;
    }
    case 8:{
      destRect.x = (int)(offset.x*imageSurface->w/100.0f-lmargin_x+imageSurface->w);
      destRect.y = (int)(offset.y*imageSurface->h/100.0f-lmargin_y+imageSurface->h);
      break;
    }
    default:
      break;
  }

  SDL_BlitSurface(bgSurface, NULL, imageSurface, &destRect);
  SDL_DestroySurface(textSurface);
  SDL_DestroySurface(bgSurface);
}