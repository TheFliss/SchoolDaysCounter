#include "timer.h"
#include "util.h"

using namespace util;
using namespace chrono;
struct date_time_t
{
  int d;
  int h;
  int m;
  int s;
};

#ifdef DEBUG
std::ostream&
operator<<(std::ostream& os, const RECT& x)
{
  os << x.left << " left "
      << x.top << " top "
      << x.right << " right "
      << x.bottom << " bottom";
  return os;
}
std::ostream&
operator<<(std::ostream& os, const date_time_t& x)
{
  os << x.d << " days "
      << x.h << " hours "
      << x.m << " minutes "
      << x.s << " seconds";
  return os;
}
#endif

using second_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;

template <class Rep, class Period>
inline hh_mm_ss<std::chrono::duration<Rep, Period>> make_time(const std::chrono::duration<Rep, Period>& d) {
  return hh_mm_ss<std::chrono::duration<Rep, Period>>(d);
}

date_time_t time_diff(second_point t1, second_point t0) {
  auto dp0 = floor<days>(t0);
  auto dp1 = floor<days>(t1);
  year_month_day ymd0 = dp0;
  year_month_day ymd1 = dp1;
  auto time0 = t0 - dp0;
  auto time1 = t1 - dp1;
  auto dd = dp1 - dp0;
  dp0 += dd;
  if (dp0 + time0 > t1) {
    --dd;
    dp0 -= days{1};
  }
  auto delta_time = time1 - time0;
  if (time0 > time1)
    delta_time += days{1};
  auto dt = make_time(delta_time);
  return {dd.count(), dt.hours().count(), dt.minutes().count(), (int)dt.seconds().count()};
}

Timer::Timer(timer_t &_config) {
  cfg = _config;
}

Timer::~Timer() {
}

void Timer::render(HDC hdc, RECT *cr) {

  SetBkMode(hdc, TRANSPARENT);

  HFONT hFont = CreateFont(
    (int)(cfg.font_size*(cr->bottom-cr->top)/100.0f), 0, 0, 0, FW_NORMAL, 
    FALSE, FALSE, FALSE, DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH, L"Calibri"
  );

  HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

  SetTextColor(hdc, RGB(255, 255, 255));

  time_t timestamp_now = time(&timestamp_now);
  tm st = *localtime(&timestamp_now);

  string display_str = cfg.text + ' ';
  if(timestamp_now <= cfg.end_date){
    date_time_t tdiff = time_diff(second_point{seconds(cfg.end_date)}, second_point{seconds(timestamp_now)});
    if(tdiff.d > 0)
      display_str += to_string(tdiff.d) + ' ' + declination_word(tdiff.d, xorstr_("день"), xorstr_("дня"), xorstr_("дней")) + (cfg.detailed_time ? ", " : " и ");

    if(tdiff.h > 0 || tdiff.d > 0)
      display_str += to_string(tdiff.h) + ' ' + declination_word(tdiff.h, xorstr_("час"), xorstr_("часа"), xorstr_("часов"));

    if(cfg.detailed_time){
      if(tdiff.h > 0 || tdiff.d > 0)
        display_str += ", ";

      if(tdiff.m > 0 || tdiff.h > 0 || tdiff.d > 0)
        display_str += to_string(tdiff.m) + ' ' + declination_word(tdiff.m, xorstr_("минута"), xorstr_("минуты"), xorstr_("минут")) + " и ";

      display_str += to_string(tdiff.s) + ' ' + declination_word(tdiff.s, xorstr_("секунда"), xorstr_("секунды"), xorstr_("секунд"));
    }
  }else
    display_str += "0 секунд";
  RECT textRect{};
  DrawText(hdc, ConvertUtf8ToWide(display_str).c_str(), -1, &textRect, DT_CALCRECT);

  int textWidth = textRect.right - textRect.left;
  int textHeight = textRect.bottom - textRect.top;

  int xPos = cr->left+(int)(cfg.offset.x*(cr->right-cr->left)/100.0f);

  int lmargin_x;
  if(cfg.fixed_size.x < 0){
    lmargin_x = (int)(cfg.margin.x*(cr->right-cr->left)/100.0f)+textWidth;
  }else{
    lmargin_x = (int)(cfg.fixed_size.x);
  }

  switch (cfg.anchor)
  {
    case 1:
    case 4:
    case 7:
      xPos += (int)((cr->right-cr->left)/2.0f);
      break;
    case 2:
    case 5:
    case 8:
      xPos += (cr->right-cr->left)-lmargin_x/2;
      break;
    default:
      xPos += lmargin_x;
      break;
  }

  int yPos = cr->top+(int)(cfg.offset.y*(cr->bottom-cr->top)/100.0f);
  int lmargin_y;
  if(cfg.fixed_size.y < 0){
    lmargin_y = (int)(cfg.margin.y*(cr->bottom-cr->top)/100.0f)+textHeight;
  }else{
    lmargin_y = (int)(cfg.fixed_size.y);
  }

  switch (cfg.anchor)
  {
    case 3:
    case 4:
    case 5:
      yPos += (int)((cr->bottom-cr->top)/2.0f);
      break;
    case 6:
    case 7:
    case 8:
      yPos += (cr->bottom-cr->top)-lmargin_y/2;
      break;
    default:
      yPos += lmargin_y/2;
      break;
  }

  RECT bgRect = {
    xPos - lmargin_x/2,
    yPos - lmargin_y/2,
    xPos + lmargin_x/2,
    yPos + lmargin_y/2
  };

  HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));

  HDC hdcAlpha = CreateCompatibleDC(hdc);
  HBITMAP hBmpAlpha = CreateCompatibleBitmap(hdc, bgRect.right - bgRect.left, bgRect.bottom - bgRect.top);
  HBITMAP hOldBmpAlpha = (HBITMAP)SelectObject(hdcAlpha, hBmpAlpha);

  RECT alphaRect = {0, 0, bgRect.right - bgRect.left, bgRect.bottom - bgRect.top};
  FillRect(hdcAlpha, &alphaRect, hBrush);
  DeleteObject(hBrush);

  BLENDFUNCTION bf = {0};
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 128;
  bf.AlphaFormat = 0;

#ifdef DEBUG
  cout << bgRect << endl;
#endif

  AlphaBlend(
    hdc, 
    bgRect.left, bgRect.top, 
    min(bgRect.right - bgRect.left, cr->right-cr->left), min(bgRect.bottom - bgRect.top, cr->bottom-cr->top),
    hdcAlpha, 
    0, 0, 
    bgRect.right - bgRect.left, bgRect.bottom - bgRect.top,
    bf
  );

  textRect.left = xPos - textWidth/2;
  textRect.top = yPos - textHeight/2;
  textRect.right = xPos + textWidth/2;
  textRect.bottom = yPos + textHeight/2;
  DrawText(hdc, ConvertUtf8ToWide(display_str).c_str(), -1, &textRect, DT_LEFT);

  SelectObject(hdcAlpha, hOldBmpAlpha);
  SelectObject(hdc, hOldFont);

  DeleteObject(hBmpAlpha);
  DeleteObject(hFont);
  DeleteDC(hdcAlpha);
}