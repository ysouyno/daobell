#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <memory>
#include <iostream>
#include <string>

class console_log;

#define log_y(...)\
{ console_log cl(console_log::log_level::LOG_CRY, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }

class console_log
{
public:
  enum log_level{
    LOG_CRY, LOG_ERR, LOG_WRN, LOG_TRC, LOG_INF
  };

  console_log(log_level level, const std::string &file, size_t line, const std::string &func) :
    color_start_(),
    color_end_(),
    level_(level),
    file_(file),
    line_(line),
    func_(func)
  {
  }

  ~console_log() = default;

  int operator()(const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    int rt = format_output(fmt, ap, level_);
    va_end(ap);
    return rt;
  }

private:
  int format_output(const char *fmt, va_list ap, log_level level);
  int printf_output(const std::string &log);
  size_t append_time(std::string &out);

private:
  std::string color_start_;
  std::string color_end_;
  log_level level_;
  std::string file_;
  size_t line_;
  std::string func_;
};

int console_log::format_output(const char *fmt, va_list ap, log_level level)
{
  int rt = vsnprintf(NULL, 0, fmt, ap);
  if (rt <= 0) {
    return 0;
  }

  char *fmt_buf = (char*)malloc(++rt);
  if (fmt_buf == NULL) {
    return 0;
  }

  rt = vsnprintf(fmt_buf, rt, fmt, ap);
  if (rt < 0) {
    free(fmt_buf);
    return 0;
  }

  std::string log_line;

  switch (level) {
  case LOG_CRY: {
    log_line += "\033[1;31m";
    append_time(log_line);
    log_line += "[CRY] ";
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
  log_line += fmt_buf;
  log_line += "\033[0;m";

  free(fmt_buf);

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
