#ifndef TIMER_H
#define TIMER_H

#include "pch.h"
#include "config.h"

class Timer
{
private:
  tm tm_struct{};
  SDL_Color textColor = {255, 255, 255, 255};
  SDL_Color bgColor = {0, 0, 0, 240};
  config_vec2_t offset;
  config_vec2_t margin;
  bool detailed_time;
  string text;
public:
  Timer(timer_t _config);
  void render(SDL_Surface * imageSurface, TTF_Font* font);
  ~Timer();
};


#endif /* TIMER_H */