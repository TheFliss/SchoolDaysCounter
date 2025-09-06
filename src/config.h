#ifndef CONFIG_H
#define CONFIG_H

#include "pch.h"

struct config_vec2_t {
  float x, y;
};

struct timer_t {
  string text;
  string end_date;
  string anchor;
  vector<float> offset;
  vector<float> margin;
  bool detailed_time;
  string text_color;
  string bg_color;
};

struct sdc_config_t {
  bool background_run;
  vector<timer_t> timers;
};

#endif /* CONFIG_H */