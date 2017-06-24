#include "console_log.h"

void console_log::init_color(log_level level)
{
  // https://en.wikipedia.org/wiki/ANSI_escape_code#graphics
  //          foreground background
  // black        30         40
  // red          31         41
  // green        32         42
  // yellow       33         43
  // blue         34         44
  // magenta      35         45
  // cyan         36         46
  // white        37         47

  // reset             0  (everything back to normal)
  // bold/bright       1  (often a brighter shade of the same colour)
  // underline         4
  // inverse           7  (swap foreground and background colours)
  // bold/bright off  21
  // underline off    24
  // inverse off      27

  switch (level) {
  case LOG_CRY: {
    color_beg_ = "\033[0;41m";
    color_end_ = "\033[0;m";
    break;
  }
  case LOG_ERR: {
    color_beg_ = "\033[0;31m";
    color_end_ = "\033[0;m";
    break;
  }
  case LOG_WRN: {
    color_beg_ = "\033[0;33m";
    color_end_ = "\033[0;m";
    break;
  }
  case LOG_TRC: {
    color_beg_ = "\033[0;35m";
    color_end_ = "\033[0;m";
    break;
  }
  case LOG_INF: {
    color_beg_ = "";
    color_end_ = "";
    break;
  }
default:
    break;
  }
}

int console_log::format_output(const char *log, log_level level)
{
  std::string log_line;
  log_line += color_beg_;
  append_time(log_line);

  switch (level) {
  case LOG_CRY: {
    log_line += "[CRY] ";
    break;
  }
  case LOG_ERR: {
    log_line += "[ERR] ";
    break;
  }
  case LOG_WRN: {
    log_line += "[WRN] ";
    break;
  }
  case LOG_TRC: {
    log_line += "[TRC] ";
    break;
  }
  case LOG_INF: {
    log_line += "[INF] ";
    break;
  }
default:
    break;
  }

  log_line += file_;
  log_line += "(";
  log_line += std::to_string(line_);
  log_line += ") ";
  log_line += func_;
  log_line += ": ";
  log_line += log;
  log_line += color_end_;

  return printf_output(log_line.c_str());
}

int console_log::printf_output(const std::string &log)
{
  std::cout << log.c_str();
  return 0;
}

size_t console_log::append_time(std::string &out)
{
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  char time_buf[32] = {0};
  sprintf(time_buf, "[%02d:%02d:%02d] ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  out += time_buf;
  return 0;
}
