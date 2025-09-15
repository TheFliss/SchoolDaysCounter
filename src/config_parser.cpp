
#include "config.h"
#include "json/json.hpp"
#include "util.h"

using namespace util;

using json = nlohmann::json;

void check_unknown_keys(const json& j, const set<string>& known_keys) {
  for (auto it = j.begin(); it != j.end(); ++it) {
    if (known_keys.find(it.key()) == known_keys.end()) {
      throw runtime_error("Unknown key: " + it.key());
    }
  }
}

void from_json(const json& j, config_vec2_t& v) {
  if (!j.is_array() || j.size() != 2) {
    throw runtime_error("vec2 must be an array of two numbers");
  }

  if(!j[0].is_number())
    throw runtime_error(xorstr_("\n\"vec2.x\" must be a number"));
  j[0].get_to(v.x);
  if(!j[1].is_number())
    throw runtime_error(xorstr_("\n\"vec2.y\" must be a number"));
  j[1].get_to(v.y);
}

void from_json(const json& j, timer_t& t) {
  set<string> known_keys = {"text", "end_date", "anchor", "font_size", 
                            "offset", "margin", "fixed_size", "detailed_time",
                            "text_color", "bg_color"};
  check_unknown_keys(j, known_keys);

  if(!j.contains("text"))
    throw runtime_error(xorstr_("\n\"text\" key required in timer config"));
  if(!j.at("text").is_string())
    throw runtime_error(xorstr_("\n\"text\" must be a string"));

  j.at("text").get_to(t.text);

  if (j.contains("end_date")) {
    if(!j.at("end_date").is_string())
      throw runtime_error(xorstr_("\n\"end_date\" must be a string"));
    string tmp_str;
    j.at("end_date").get_to(tmp_str);
    istringstream ss(tmp_str);
    tm tm = {};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    tm.tm_isdst = 0;
    if (ss.fail())
      throw runtime_error(xorstr_("\nDate in config parsing failed!\nRequired format: \"Y-m-d H:M:S\""));
    t.end_date = mktime(&tm);
  }else{
    throw runtime_error(xorstr_("\n\"end_date\" key required in timer config"));
  }

  if (j.contains("anchor")) {
    if(!j.at("text_color").is_string())
      throw runtime_error(xorstr_("\n\"text_color\" must be a string, and contains one of them\n  (top_left, top_middle, top_right,\n   left_middle, center, right_middle,\n   bottom_left, bottom_middle, bottom_right)"));
    string tmp_str;
    j.at("anchor").get_to(tmp_str);
    if(tmp_str == "top_left"){
      t.anchor = 0;
    }else if(tmp_str == "top_middle"){
      t.anchor = 1;
    }else if(tmp_str == "top_right"){
      t.anchor = 2;
    }else if(tmp_str == "left_middle"){
      t.anchor = 3;
    }else if(tmp_str == "center"){
      t.anchor = 4;
    }else if(tmp_str == "right_middle"){
      t.anchor = 5;
    }else if(tmp_str == "bottom_left"){
      t.anchor = 6;
    }else if(tmp_str == "bottom_middle"){
      t.anchor = 7;
    }else if(tmp_str == "bottom_right"){
      t.anchor = 8;
    }else{
      throw runtime_error(xorstr_("\nAnchor must be equal to one of \n  (top_left, top_middle, top_right,\n   left_middle, center, right_middle,\n   bottom_left, bottom_middle, bottom_right)\n"));
    }
  } else {
    t.anchor = 4;
  }

  if (j.contains("font_size")) {
    if(!j.at("font_size").is_number())
      throw runtime_error(xorstr_("\n\"font_size\" must be a number"));
    j.at("font_size").get_to(t.font_size);
  } else {
    t.font_size = 6;
  }

  if (j.contains("offset")) {
    if(!j.at("offset").is_array())
      throw runtime_error(xorstr_("\n\"offset\" must be an array"));
    j.at("offset").get_to(t.offset);
  } else {
    t.offset = {0, 0};
  }

  if (j.contains("margin")) {
    if(!j.at("margin").is_array())
      throw runtime_error(xorstr_("\n\"margin\" must be an array"));
    j.at("margin").get_to(t.margin);
  } else {
    t.margin = {0, 0};
  }

  if (j.contains("fixed_size")) {
    if(!j.at("fixed_size").is_array())
      throw runtime_error(xorstr_("\n\"fixed_size\" must be an array"));
    j.at("fixed_size").get_to(t.fixed_size);
  } else {
    t.fixed_size = {-1, -1};
  }

  if (j.contains("detailed_time")) {
    if(!j.at("detailed_time").is_boolean())
      throw runtime_error(xorstr_("\n\"detailed_time\" must be a boolean"));
    j.at("detailed_time").get_to(t.detailed_time);
  } else {
    t.detailed_time = false;
  }

  if (j.contains("text_color")) {
    if(!j.at("text_color").is_string())
      throw runtime_error(xorstr_("\n\"text_color\" must be a string"));
    try {
      string tmp_str;
      j.at("text_color").get_to(tmp_str);
      t.text_color = ((uint32_t)stoul(tmp_str, nullptr, 16));
    } catch(const exception& exc) {
      throw runtime_error(xorstr_("\nException caught while parsing \"text_color\" in timer config: ") + string(exc.what()));
    }
  } else {
    t.text_color = 0xffffffff;
  }

  if (j.contains("bg_color")) {
    if(!j.at("bg_color").is_string())
      throw runtime_error(xorstr_("\n\"bg_color\" must be a string"));
    try {
      string tmp_str;
      j.at("bg_color").get_to(tmp_str);
      t.bg_color = ((uint32_t)stoul(tmp_str, nullptr, 16));
    } catch(const exception& exc) {
      throw runtime_error(xorstr_("\nException caught while parsing \"bg_color\" in timer config: ") + string(exc.what()));
    }
  } else {
    t.bg_color = 0x000000b0;
  }
}

void from_json(const json& j, sdc_config_t& c) {
  set<string> known_keys = {"update_delay", "timers"};
  check_unknown_keys(j, known_keys);

  if(j.contains("update_delay")){
    if(!j.at("update_delay").is_number_integer())
      throw runtime_error(xorstr_("\n\"update_delay\" must be a integer"));
    j.at("update_delay").get_to(c.update_delay);
  }else{
    c.update_delay = 900;
  }
  {
    if(!j.contains("timers"))
      throw runtime_error(xorstr_("\n\"timers\" key required in config file"));
    if(!j.at("timers").is_array())
      throw runtime_error(xorstr_("\n\"timers\" must be an array"));
    j.at("timers").get_to(c.timers);
    if(c.timers.size() == 0){
      throw runtime_error(xorstr_("\n\"timers\" content must be not equal to []"));
    }
  }
}

sdc_config_t parse_config(fs::path fp){
  sdc_config_t sdc_config;
  try {
    auto sdccstream = ifstream(fp);

    json j = json::parse(sdccstream, nullptr, true, true);

    sdc_config = j.get<sdc_config_t>();

    sdccstream.close();
  } catch(const exception& e) {
    string errmsg = string("Exception caught while reading config file:\n") + e.what();
    MessageBoxW(NULL, ConvertUtf8ToWide(errmsg).c_str(), ConvertUtf8ToWide(string("Ошибка")).c_str(), MB_ICONERROR);
    exit(1);
  }
  return sdc_config;
};