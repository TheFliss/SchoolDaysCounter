#ifndef CONFIG_H
#define CONFIG_H

#include "pch.h"
#include "json/json.hpp"

using json = nlohmann::json;

struct config_vec2_t {
  float x, y;
};

struct timer_t {
  string text;
  time_t end_date;
  uint8_t anchor;
  float font_size;
  config_vec2_t offset;
  config_vec2_t margin;
  config_vec2_t fixed_size;
  bool detailed_time;
  uint32_t text_color;
  uint32_t bg_color;
};

struct sdc_config_t {
  int update_delay;
  vector<timer_t> timers;
};

void from_json(const json& j, sdc_config_t& c);
sdc_config_t parse_config(fs::path fp);

#endif /* CONFIG_H */