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
  float font_size;
  vector<float> offset;
  vector<float> margin;
  bool detailed_time;
  string text_color;
  string bg_color;
};

struct sdc_config_t {
  float update_delay;
  float save_quality;
  bool background_run;
  vector<timer_t> timers;
};

#endif /* CONFIG_H */