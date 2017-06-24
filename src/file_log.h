#include <cstdarg>
#include <memory>
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <error.h>

class file_log;

#define log_file(...)                                                   \
  { file_log fl(file_log::log_level::LOG_CRY, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }

class file_log_writer
{
public:
  file_log_writer() : file_name_()
  {
    char cwd[512] = {0};
    getcwd(cwd, sizeof(cwd));
    file_name_ += cwd;
    file_name_ += "/";
    file_name_ += program_invocation_short_name;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char time_buf[32] = {0};
    sprintf(time_buf, "_%04d%02d%02d%02d%02d%02d",
            timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1,
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec);

    file_name_ += time_buf;
    file_name_ += ".log";

    fp_ = fopen(file_name_.c_str(), "w");
  }

  ~file_log_writer()
  {
    fclose(fp_);
    fp_ = NULL;
  }

  void write_file_log(const std::string &log)
  {
    fputs(log.c_str(), fp_);
  }

private:
  FILE *fp_;
  std::string file_name_;
};


class file_log
{
public:
  enum log_level{
    LOG_CRY, LOG_ERR, LOG_WRN, LOG_TRC, LOG_INF
  };

  file_log(log_level level, const std::string &file, size_t line, const std::string &func) :
    level_(level),
    file_(file),
    line_(line),
    func_(func)
  {}

  ~file_log() = default;

  int operator()(const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    int rt = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (rt <= 0) {
      return 0;
    }

    char *fmt_buf = (char*)malloc(++rt);
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

  file_log_writer &get_flw()
  {
    static file_log_writer flw;
    return flw;
  }

private:
  int format_output(const char *log, log_level level);
  int printf_output(const std::string &log);
  size_t append_time(std::string &out);

private:
  log_level level_;
  std::string file_;
  size_t line_;
  std::string func_;
};

int file_log::printf_output(const std::string &log)
{
  get_flw().write_file_log(log);
  return 0;
}

int file_log::format_output(const char *log, log_level level)
{
  std::string log_line;
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

  return printf_output(log_line.c_str());
}

size_t file_log::append_time(std::string &out)
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
