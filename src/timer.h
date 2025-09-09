#ifndef TIMER_H
#define TIMER_H

#include "pch.h"
#include "config.h"

class Timer
{
private:
  timer_t cfg;
public:
  Timer(timer_t &_config);
  void render(HDC hdc, RECT *cr);
  ~Timer();
};


#endif /* TIMER_H */