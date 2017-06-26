#include "http_multi_threads_downloader.h"

int http_multi_threads_downloader::init()
{
  if (url_.empty()) {
    log_e("url is null\n");
    return -1;
  }

  up_(url_);
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0) {
    log_e("socket() error\n");
    return -1;
  }

  sockaddr_in sa;
  bzero(&sa, sizeof(sa));
  sa.sin_family = AF_INET;

  int rt = bind(sock_, (sockaddr *)&sa, sizeof(sa));
  if (rt < 0) {
    log_e("bind() error\n");
    return -1;
  }

  hostent *p = gethostbyname(up_.domain_.c_str());
  if (NULL == p) {
    log_e("gethostbyname() error\n");
    return -1;
  }

  sa.sin_port = htons(80);
  memcpy(&sa.sin_addr, p->h_addr, 4);

  rt = connect(sock_, (sockaddr *)&sa, sizeof(sa));
  if (rt < 0) {
    log_e("connect() error\n");
    return -1;
  }

  // send request to get file size
  std::string request_str("GET ");
  request_str.append(up_.path_);
  request_str.append(" HTTP/1.1\r\nHost: ");
  request_str.append(up_.domain_);
  request_str.append("\r\nConnection: Close\r\n\r\n");

  rt = send(sock_, request_str.c_str(), request_str.size(), 0);
  if (rt < 0) {
    log_e("send() error\n");
    return -1;
  }

  // recv buff to parse 200 or 302 for file size
  char buff[1024] = {0};
  rt = recv(sock_, buff, sizeof(buff) - 1, 0);
  if (rt < 0) {
    log_e("recv() error\n");
    return -1;
  }

  std::cout << "--------------------------------\n";
  std::cout << "recv buff:\n";
  std::cout << buff;
  std::cout << "--------------------------------\n";

  http_header_parse hhp(buff);
  file_size_ = hhp.get_file_size();
  log_i("file_size_: %d\n", file_size_);

  // get current thread download offset form beg_ to end_
  thread_file_size_ = file_size_ / thread_count_;
  thread_offset_beg_ = current_thread_index_ * thread_file_size_;
  thread_offset_end_ = thread_offset_beg_ + thread_file_size_ - 1;
  log_i("thread_offset_beg_: %d\n", thread_offset_beg_);
  log_i("thread_offset_end_: %d\n", thread_offset_end_);

  // generate request header which include range value
  std::string request_range_header("GET ");
  request_range_header += up_.path_;
  request_range_header += " HTTP/1.1\r\nHost: ";
  request_range_header += up_.domain_;
  request_range_header += "\r\nConnection: Keep-Alive\r\n\r\n";
  request_range_header += "Range: bytes=";
  request_range_header += std::to_string(thread_offset_beg_);
  request_range_header += "-";
  request_range_header += std::to_string(thread_offset_end_);
  request_range_header += "\r\n\r\n";

  std::cout << "--------------------------------\n";
  std::cout << "recv request_range_header:\n";
  std::cout << request_range_header.c_str();
  std::cout << "--------------------------------\n";

  // send request header with range value
  rt = send(sock_, request_range_header.c_str(), request_range_header.size(), 0);
  if (rt < 0) {
    log_e("send() request header with range value error\n");
    return -1;
  }

  // generate destination file name
  // TODO: get file name from url
  char file_name_temp[512] = {0};
  if (0 == current_thread_index_) {
    sprintf(file_name_temp, "%s", "just_test_file_name.jpg");
  }
  else {
    // %zu for size_t
    sprintf(file_name_temp, "just_test_file_name.%zu.temp.jpg", current_thread_index_);
  }

  // recv the part of file in thread
  fp_ = fopen(file_name_temp, "wb+");
  char recv_buff[4096] = {0};

  while (true) {
    rt = recv(sock_, recv_buff, 4096, 0);
    if (rt < 0) {
      log_e("recv() recv_buff error\n");
      break;
    }
    else if (rt == 0) {
      log_i("recv() done\n");
      break;
    }

    if (fwrite(recv_buff, sizeof(char), rt, fp_) < 0) {
      log_e("fwrite() error\n");
      break;
    }

    bzero(recv_buff, 4096);
  }

  return 0;
}
