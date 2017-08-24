#include "http_multi_threads_downloader.h"

http_multi_threads_downloader::~http_multi_threads_downloader()
{
  if (sock_ > 0) {
    close(sock_);
  }

  if (fp_ > 0) {
    fclose(fp_);
  }
}

http_multi_threads_downloader::http_multi_threads_downloader(const std::string &url, size_t thread_count, size_t thread_index) :
  url_(url),
  thread_count_(thread_count),
  current_thread_index_(thread_index),
  fp_(NULL),
  downloaded_size_(0)
{
  if (1 == thread_count_) {
    log_t("single thread\n");
    current_thread_index_ = 0;
  }
}

int http_multi_threads_downloader::init()
{
  hup_.parse(url_);

  if (connect_server() < 0) {
    log_e("connect_server() error\n");
    return -1;
  }
  else {
    return  0;
  }
}

int http_multi_threads_downloader::connect_server()
{
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    log_e("socket() error: %d\n", errno);
    return -1;
  }

  sockaddr_in sa;
  bzero(&sa, sizeof(sockaddr_in));
  sa.sin_family = AF_INET;

  int rt = bind(sock_, (sockaddr *)&sa, sizeof(sa));
  if (sock_ < 0) {
    log_e("bind() error: %d\n", errno);
    return -1;
  }

  hostent *p = gethostbyname(hup_.domain_.c_str());
  if (sock_ < 0) {
    log_e("gethostbyname() error: %d\n", errno);
    return -1;
  }

  sa.sin_port = htons(80);
  memcpy(&sa.sin_addr, p->h_addr, 4);

  rt = connect(sock_, (sockaddr *)&sa, sizeof(sa));
  if (rt < 0) {
    log_e("connect() error: %d\n", errno);
    return -1;
  }

  return 0;
}

int http_multi_threads_downloader::send_request_query_http_header()
{
  if (0 == sock_) {
    log_e("sock_ is null\n");
    return -1;
  }

  std::string header;
  if (gen_request_header(header) < 0) {
    log_e("gen_request_header() error\n");
    return -1;
  }

  int rt = send(sock_, header.c_str(), header.size(), 0);
  if (rt < 0) {
    log_e("send() error: %d\n", errno);
    return -1;
  }

  // TODO: recv buff to parse 200 or 302 or file size
  char buff[1024] = { 0 };
  rt = recv(sock_, buff, sizeof(buff) - 1, 0);
  if (rt < 0) {
    log_e("recv() error: %d\n", errno);
    return -1;
  }

  http_header_parser hhp(buff);
  file_size_ = hhp.get_content_length();

  // here need to close socket
  close(sock_);

  return 0;
}

int http_multi_threads_downloader::gen_request_header(std::string &out)
{
  if (!hup_.valid()) {
    log_e("hup_ is invalid\n");
    return -1;
  }

  out.clear();

  out += "GET ";
  out += hup_.path_;
  out += " HTTP/1.1\r\nHost: ";
  out += hup_.domain_;
  out += "\r\nConnection: Close\r\n\r\n";

  return 0;
}

int http_multi_threads_downloader::gen_range_header(std::string &out)
{
  if (!hup_.valid()) {
    log_e("hup_ is invalid\n");
    return -1;
  }

  // get current thread download offset from start to end
  thread_file_size_ = file_size_ / thread_count_;
  thread_offset_beg_ = current_thread_index_ * thread_file_size_;
  thread_offset_end_ = thread_offset_beg_ + thread_file_size_ - 1;
  if (current_thread_index_ == thread_count_ - 1) {
    thread_offset_end_ = file_size_;
  }

  out.clear();

  out = "GET ";
  out += hup_.path_;
  out += " HTTP/1.1\r\nHost: ";
  out += hup_.domain_;
  out += "\r\nConnection: Close\r\n";
  out += "Range: bytes=";
  out += std::to_string(thread_offset_beg_);
  out += "-";
  out += std::to_string(thread_offset_end_);
  out += "\r\n\r\n";

  return 0;
}

int http_multi_threads_downloader::send_range_request(const std::string &header)
{
  if (init() < 0) {
    log_e("init() error\n");
    return -1;
  }

  // send request header with range value
  int rt = send(sock_, header.c_str(), header.size(), 0);
  if (rt < 0) {
    log_e("send() error: %d\n", errno);
    return -1;
  }
  else {
    return 0;
  }
}

int http_multi_threads_downloader::get_dest_file_name(std::string &out)
{
  if (!hup_.valid()) {
    log_e("hup_ is invalid\n");
    return -1;
  }

  size_t pos = hup_.path_.find_last_of('/');
  if (std::string::npos != pos) {
    out = hup_.path_.substr(pos + 1, hup_.path_.size());
  }

  return 0;
}

int http_multi_threads_downloader::gen_file_name_temp(std::string &out)
{
  out.clear();

  if (get_dest_file_name(dest_file_name_) < 0) {
    log_e("get_dest_file_name() error\n");
    return -1;
  }

  if (0 == current_thread_index_ && 1 == thread_count_) {
    out = dest_file_name_;
  }
  else {
    out += dest_file_name_;
    out += ".";
    out += std::to_string(current_thread_index_);
    out += ".dao";
  }

  return 0;
}

int http_multi_threads_downloader::download_it()
{
  if (init() < 0) {
    log_e("init() error\n");
    return -1;
  }

  send_request_query_http_header();
  std::string range_header;
  gen_range_header(range_header);
  send_range_request(range_header);

  gen_file_name_temp(dest_file_name_temp_);

  // recv the whole file or the part of it in thread
  std::fstream file;
  file.open(dest_file_name_temp_.c_str(), std::ios::out | std::ios::binary);
  char buff[1024] = { 0 };
  int rt = recv(sock_, buff, sizeof(buff) - 1, 0);
  if (rt < 0) {
    log_e("recv() error: %d\n", errno);
    return -1;
  }

  char *str = strstr(buff, "\r\n\r\n");
  file.write(str + strlen("\r\n\r\n"), rt - (str - buff) - strlen("\r\n\r\n"));
  downloaded_size_ = rt - (str - buff) - strlen("\r\n\r\n");
  while ((rt = recv(sock_, buff, sizeof(buff) - 1, 0)) > 0) {
    downloaded_size_ += rt;
    file.write(buff, rt);
  }

  file.close();

  return 0;
}
