#include "timer.h"
#include "util.h"

using namespace util;

Timer::Timer(timer_t config) {
  istringstream ss(config.end_date);
  ss >> get_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
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

  if(config.margin.size() != 2)
  {
    printf_s(xorstr_("Margin must be equal to 2\n"), SDL_GetError());
    exit(1);
  }else
    margin = {config.margin[0], config.margin[1]};

  if(config.offset.size() != 2)
  {
    printf_s(xorstr_("Offset must be equal to 2\n"), SDL_GetError());
    exit(1);
  }else
    offset = {config.offset[0], config.offset[1]};

  text = config.text;

  cout << ConvertWideToANSI(ConvertUtf8ToWide(text)) << endl;
}

Timer::~Timer() {

}

void Timer::render(SDL_Surface * imageSurface, TTF_Font* font) {
  cout << "rendered: " << text << endl;
  SYSTEMTIME st;
  GetLocalTime(&st);

  bool is_summer = st.wMonth > 5 && st.wMonth < 9;

  DWORD SchoolDays = (!(st.wYear&0b11)+28)+31*5+30*3;

  int day_acc = 0;
  for (int i = 0; i < 9; i++) {
    int mi = i+(i > 3 ? -3 : 9);
    if(mi == st.wMonth){
      day_acc += st.wDay;
      break;
    }
    day_acc += get_month_length(mi, st.wYear);
  }

  int curr_seconds = 59-st.wSecond;
  int curr_mins = 59-st.wMinute;
  int curr_hours = 23-st.wHour;
  int curr_days = SchoolDays-day_acc-1;

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

  #ifdef DEBUG

  DWORD cMonthDays = get_month_length(st.wMonth, st.wYear);
  cout << "Time (UTC): "
            << "\nisLeapYear " << (!(st.wYear&0b11))
            << "\nwYear " << st.wYear
            << "\nwMonth " << st.wMonth
            << "\nwDay " << st.wDay
            << "\nwHour " << st.wHour
            << "\nwMinute " << st.wMinute
            << "\nSchoolDays " << SchoolDays
            << "\ncMonthDays " << cMonthDays
            << "\nis_summer " << is_summer
            << "\nday_acc " << day_acc
            << endl;
  #endif

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

  SDL_Rect destRect = {(int)(offset.x*imageSurface->w/100-lmargin_x/2), (int)(offset.y*imageSurface->h/100-lmargin_y/2), lmargin_x, lmargin_y};

  SDL_BlitSurface(bgSurface, NULL, imageSurface, &destRect);
  SDL_DestroySurface(textSurface);
  SDL_DestroySurface(bgSurface);
}