#ifndef CONSOLE_LOG_H
#define CONSOLE_LOG_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <memory>
#include <iostream>
#include <string>

class console_log
{
public:
  enum log_level{
    LOG_CRY, LOG_ERR, LOG_WRN, LOG_TRC, LOG_INF
  };

  console_log(log_level level, const std::string &file, size_t line, const std::string &func) :
    color_beg_(),
    color_end_(),
    level_(level),
    file_(file),
    line_(line),
    func_(func)
  {
    init_color(level_);
  }

  ~console_log() = default;

  int operator()(const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    int rt = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (rt <= 0) {
      return 0;
    }

    char *fmt_buf = (char *)malloc(++rt);
    if (fmt_buf == NULL) {
      return 0;
    }

    va_start(ap, fmt);
    rt = vsnprintf(fmt_buf, rt, fmt, ap);
    va_end(ap);
    if (rt < 0) {
      free(fmt_buf);
      return 0;
    }

    rt = format_output(fmt_buf, level_);

    free(fmt_buf);
    return rt;
  }

private:
  void init_color(log_level level);
  int format_output(const char *log, log_level level);
  int printf_output(const std::string &log);
  size_t append_time(std::string &out);

private:
  std::string color_beg_;
  std::string color_end_;
  log_level level_;
  std::string file_;
  size_t line_;
  std::string func_;
};

#endif /* CONSOLE_LOG_H */
