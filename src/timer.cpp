#include "timer.h"
#include "util.h"

using namespace util;

Timer::Timer(timer_t config) {
  istringstream ss(config.end_date);
  ss >> get_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
  tm_struct.tm_isdst = 0;
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
  detailed_time = config.detailed_time;

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
  #ifdef DEBUG
  cout << "rendered: " << ConvertWideToANSI(ConvertUtf8ToWide(text)) << endl;
  #endif
  time_t timestamp = time(&timestamp);
  tm st = *localtime(&timestamp);
  //cout << "timestamp: " << timestamp  << " : " << mktime(&tm_struct) << endl;

  string display_str = text + ' ';
  if(timestamp <= mktime(&tm_struct)){
    int wait_days = 0;
    for (int i = st.tm_mon; i < 12*(tm_struct.tm_year-st.tm_year)+tm_struct.tm_mon; i++)
    {
      int mi = i%12+1;
      int yi = i/12+st.tm_year+1900;
      wait_days += get_month_length(mi, yi);
    }
    wait_days -= st.tm_mday;
    wait_days += tm_struct.tm_mday;

    #ifdef DEBUG
    cout << "Time (UTC): "
              << "\nwYear " << tm_struct.tm_year
              << "\nwMonth " << tm_struct.tm_mon
              << "\nwDay " << tm_struct.tm_mday
              << "\nwHour " << tm_struct.tm_hour
              << "\nwMinute " << tm_struct.tm_min
              << "\nwait_days " << wait_days
              << endl;
    #endif

    int curr_seconds = tm_struct.tm_sec-st.tm_sec;
    int curr_mins = tm_struct.tm_min-st.tm_min;
    int curr_hours = tm_struct.tm_hour-st.tm_hour;
    int curr_days = wait_days-1*(curr_hours < 0);
    curr_hours += 24*(curr_hours < 0)-1*(curr_mins < 0);
    curr_mins += 60*(curr_mins < 0)-1*(curr_seconds < 0);
    curr_seconds += 60*(curr_seconds < 0);
    if(curr_days > 0)
      display_str += to_string(curr_days) + ' ' + declination_word(curr_days, xorstr_("день"), xorstr_("дня"), xorstr_("дней")) + (detailed_time ? ", " : " и ");
    if(curr_hours > 0 || curr_days > 0)
      display_str += to_string(curr_hours) + ' ' + declination_word(curr_hours, xorstr_("час"), xorstr_("часа"), xorstr_("часов"));
    if(detailed_time){
      if(curr_hours > 0 || curr_days > 0)
        display_str += ", ";
      if(curr_mins > 0 || curr_days > 0 || curr_days > 0)
        display_str += to_string(curr_mins) + ' ' + declination_word(curr_mins, xorstr_("минута"), xorstr_("минуты"), xorstr_("минут")) + " и ";
      display_str += to_string(curr_seconds) + ' ' + declination_word(curr_seconds, xorstr_("секунда"), xorstr_("секунды"), xorstr_("секунд"));
    }
  }else
    display_str += "0 секунд";

  #ifdef DEBUG

  DWORD cMonthDays = get_month_length(st.tm_mon, st.tm_year);
  cout << "Time (UTC): "
            << "\nisLeapYear " << isLeapYear(st.tm_year)
            << "\nwYear " << st.tm_year
            << "\nwMonth " << st.tm_mon
            << "\nwDay " << st.tm_mday
            << "\nwHour " << st.tm_hour
            << "\nwMinute " << st.tm_min
            << "\ncMonthDays " << cMonthDays
            << "\nday_acc " << wait_days
            << endl;
  #endif

  // Render text to a surface
  SDL_Surface* textSurface = TTF_RenderText_Blended(font, display_str.c_str(), display_str.length(), textColor);
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