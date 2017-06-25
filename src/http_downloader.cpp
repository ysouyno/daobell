#include "http_downloader.h"

http_downloader::~http_downloader()
{
  if (sock_ > 0) {
    close(sock_);
  }
}

http_downloader::http_downloader(const std::string &url) :
  url_(url),
  request_(),
  dest_file_name_(),
  sock_(0)
{
  init();
}

int http_downloader::init()
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

  return 0;
}

int http_downloader::gen_request()
{
  request_.clear();

  request_ += "GET ";
  request_ += up_.path_;
  request_ += " HTTP/1.1\r\nHost: ";
  request_ += up_.domain_;
  request_ += "\r\nConnection:Close\r\n\r\n";

  return 0;
}

int http_downloader::gen_dest_file_name()
{
  dest_file_name_.clear();
  dest_file_name_ = "just_for_temp_test";

  return 0;
}

int http_downloader::download_it()
{
  if (sock_ <= 0) {
    log_e("sock_ is invalid\n");
    return -1;
  }

  sockaddr_in sa;
  bzero(&sa, sizeof(sockaddr_in));
  sa.sin_family = AF_INET;

  int rt = bind(sock_, (sockaddr*)&sa, sizeof(sa));
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

  rt = connect(sock_, (sockaddr*)&sa, sizeof(sa));
  if (rt < 0) {
    log_e("connect() error\n");
    return -1;
  }

  if (gen_request() < 0) {
    log_e("gen_request() error\n");
    return -1;
  }

  rt = send(sock_, request_.c_str(), request_.size(), 0);
  if (rt < 0) {
    log_e("send() error\n");
    return -1;
  }

  if (gen_dest_file_name() < 0) {
    log_e("gen_dest_file_name() error\n");
    return -1;
  }

  std::fstream file;

  file.open(dest_file_name_.c_str(), std::ios::out | std::ios::binary);
  char buff[1024] = {0};
  rt = recv(sock_, buff, sizeof(buff) - 1, 0);
  if (rt < 0) {
    log_e("recv() error\n");
    return -1;
  }

  http_header_parse hhp(buff);
  hhp.print();
  std::cout << "file size: " << hhp.get_file_size() << " bytes" << std::endl;

  size_t downloaded = 0;
  double file_size = hhp.get_file_size();
  size_t size_count = hhp.get_file_size_number_count();

  char *str = strstr(buff, "\r\n\r\n");
  file.write(str + strlen("\r\n\r\n"), rt - (str - buff) - strlen("\r\n\r\n"));
  while ((rt = recv(sock_, buff, sizeof(buff) - 1, 0)) > 0) {
    downloaded += rt;
    std::cout << std::setw(size_count) << downloaded << " bytes downloaded ";
    std::cout << std::setprecision(3) << (downloaded / file_size) * 100 << "%" << std::endl;
    file.write(buff, rt);
  }

  file.close();

  return 0;
}
