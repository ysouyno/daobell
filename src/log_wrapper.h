#include "console_log.h"
#include "file_log.h"

#define LOG_CONSOLE
// #define LOG_FILE

class console_log;
class file_log;

#if defined(LOG_CONSOLE)
#define log_y(...)                                                      \
  { console_log cl(console_log::log_level::LOG_CRY, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }
#define log_e(...)                                                      \
  { console_log cl(console_log::log_level::LOG_ERR, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }
#define log_w(...)                                                      \
  { console_log cl(console_log::log_level::LOG_WRN, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }
#define log_t(...)                                                      \
  { console_log cl(console_log::log_level::LOG_TRC, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }
#define log_i(...)                                                      \
  { console_log cl(console_log::log_level::LOG_INF, __FILE__, __LINE__, __FUNCTION__); cl(__VA_ARGS__); }
#elif defined(LOG_FILE)
#define log_y(...)                                                      \
  { file_log fl(file_log::log_level::LOG_CRY, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }
#define log_e(...)                                                      \
  { file_log fl(file_log::log_level::LOG_ERR, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }
#define log_w(...)                                                      \
  { file_log fl(file_log::log_level::LOG_WRN, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }
#define log_t(...)                                                      \
  { file_log fl(file_log::log_level::LOG_TRC, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }
#define log_i(...)                                                      \
  { file_log fl(file_log::log_level::LOG_INF, __FILE__, __LINE__, __FUNCTION__); fl(__VA_ARGS__); }
#else
#define log_y(...)
#define log_e(...)
#define log_w(...)
#define log_t(...)
#define log_i(...)
#endif
