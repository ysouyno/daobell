#include "file_log.h"

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
  sprintf(time_buf, "[%02d:%02d:%02d] ",
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  out += time_buf;
  return 0;
}
