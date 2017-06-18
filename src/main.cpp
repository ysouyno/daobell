#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <iomanip>
#include "url_parse.h"
#include "http_header_parse.h"

int main(int argc, char *argv[])
{
  if (argc <= 1) {
    std::cout << "usage: xxx.exe url" << std::endl;
    return 1;
  }

  std::string url(argv[1]);
  url_parse up(url);

  int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_cli < 0) {
    std::cout << "socket error" << std::endl;
    return 1;
  }

  sockaddr_in sa;
  bzero(&sa, sizeof(sockaddr_in));
  sa.sin_family = AF_INET;

  int n = bind(sock_cli, (sockaddr*)&sa, sizeof(sa));
  if (n < 0) {
    std::cout << "bind error" << std::endl;
    close(sock_cli);
    return 1;
  }

  hostent *p = gethostbyname(up.domain_.c_str());
  if (0 == p) {
    std::cout << "gethostbyname error" << std::endl;
    close(sock_cli);
    return 1;
  }

  sa.sin_port = htons(80);
  memcpy(&sa.sin_addr, p->h_addr, 4);

  n = connect(sock_cli, (sockaddr*)&sa, sizeof(sa));
  if (n < 0) {
    std::cout << "connect error" << std::endl;
    close(sock_cli);
    return 1;
  }

  std::string request_str("GET ");
  request_str.append(up.path_);
  request_str.append(" HTTP/1.1\r\nHost: ");
  request_str.append(up.domain_);
  request_str.append("\r\nConnection:Close\r\n\r\n");

  n = send(sock_cli, request_str.c_str(), request_str.size(), 0);
  if (n < 0) {
    std::cout << "send error" << std::endl;
    close(sock_cli);
    return 1;
  }

  std::string file_name("test_download");
  std::fstream file;

  file.open(file_name.c_str(), std::ios::out | std::ios::binary);
  char buff[1024] = {0};
  n = recv(sock_cli, buff, sizeof(buff) - 1, 0);
  if (n < 0) {
    std::cout << "recv error" << std::endl;
    close(sock_cli);
    return 1;
  }

  http_header_parse hhp(buff);
  hhp.print();
  std::cout << "file size: " << hhp.get_file_size() << " bytes" << std::endl;

  size_t downloaded = 0;
  double file_size = hhp.get_file_size();
  size_t size_count = hhp.get_file_size_number_count();

  char *str = strstr(buff, "\r\n\r\n");
  file.write(str + strlen("\r\n\r\n"), n - (str - buff) - strlen("\r\n\r\n"));
  while ((n = recv(sock_cli, buff, sizeof(buff) - 1, 0)) > 0) {
    downloaded += n;
    std::cout << std::setw(size_count) << downloaded << " bytes downloaded ";
    std::cout << std::setprecision(3) << (downloaded / file_size) * 100 << "%" << std::endl;
    file.write(buff, n);
  }

  file.close();

  return 0;
}
