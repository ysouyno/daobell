#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include "log_wrapper.h"

#define BUFFER_SIZE 1024
#define FTP_HOST "192.168.1.109"
#define FTP_PORT 21
#define FTP_USER "anonymous"
#define FTP_PASS ""

int ftp_downloader()
{
  int ctrl_sock = 0;
  int data_sock = 0;
  struct hostent *p;
  struct sockaddr_in sa;
  char temp_buf[BUFFER_SIZE] = {0};
  char send_buf[BUFFER_SIZE] = {0};
  char read_buf[BUFFER_SIZE] = {0};

  memset(&sa, 0, sizeof(struct sockaddr_in));

  ctrl_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == ctrl_sock) {
    log_e("socket error: %d\n", errno);
    return -1;
  }

  p = gethostbyname(FTP_HOST);
  if (0 == p) {
    log_e("gethostbyname error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  memcpy(&sa.sin_addr, p->h_addr, p->h_length);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(FTP_PORT);

  // connect to ftp server
  int rt = connect(ctrl_sock, (struct sockaddr *)&sa, sizeof(sa));
  if (-1 == rt) {
    log_e("connect error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // receive welcome message
  rt = recv(ctrl_sock, temp_buf, sizeof(temp_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // print welcome message
  std::cout << "welcome message: \n" << temp_buf << std::endl;

  // send "USER username\r\n"
  sprintf(send_buf, "USER %s\r\n", FTP_USER);
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // server respond "331 User name okay, need password."
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // send "PASS password\r\n"
  sprintf(send_buf, "PASS %s\r\n", FTP_PASS);
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // server respond "230 User logged in, proceed."
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // send "PASV\r\n" to use passive mode
  sprintf(send_buf, "PASV\r\n");
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // server respond "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)"
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  // get port
  size_t port = 0;
  std::string str(read_buf);
  if (0 == str.find("227 Entering Passive Mode")) {
    log_t("server entering passive mode\n");
    size_t pos1 = str.find_first_of('(');
    size_t pos2 = str.find_first_of(')');
    std::string ip_port = str.substr(pos1 + 1, pos2 - pos1 - 1);
    std::cout << ip_port << std::endl;

    // split
    std::vector<size_t> vec_ip_port;
    char *split_p = strtok(const_cast<char *>(ip_port.c_str()), ",");
    while (split_p) {
      size_t temp = 0;
      std::string temp_str(split_p);
      std::stringstream(temp_str) >> temp;
      vec_ip_port.push_back(temp);
      split_p = strtok(NULL, ",");
    }

    port = vec_ip_port.at(4) * 256 + vec_ip_port.at(5);
    log_t("server port: %d\n", port);
  }

  // init data socket
  data_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == data_sock) {
    log_e("socket error: %d\n", errno);
    close(ctrl_sock);
    return -1;
  }

  memcpy(&sa.sin_addr, p->h_addr, p->h_length);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);

  // connect server data port
  rt = connect(data_sock, (struct sockaddr *)&sa, sizeof(sa));
  if (-1 == rt) {
    log_e("connect error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // send "CWD directory\r\n"
  sprintf(send_buf, "CWD %s\r\n", "/");
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // server respond "250 Command okay."
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // send "SIZE filename\r\n"
  sprintf(send_buf, "SIZE %s\r\n", "underscore_filename_demo.txt");
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // server respond "213 <size>"
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // send "RETR filename\r\n"
  sprintf(send_buf, "RETR %s\r\n", "underscore_filename_demo.txt");
  rt = send(ctrl_sock, send_buf, strlen(send_buf), 0);
  if (-1 == rt) {
    log_e("send error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // server respond "150 Opening data connection."
  rt = recv(ctrl_sock, read_buf, sizeof(read_buf) - 1, 0);
  if (-1 == rt) {
    log_e("recv error: %d\n", errno);
    close(ctrl_sock);
    close(data_sock);
    return -1;
  }

  // download
  FILE *file = fopen("ftp_down.txt", "wb+");
  size_t downloaded = 0;

  while ((rt = recv(data_sock, read_buf, sizeof(read_buf) - 1, 0)) > 0) {
    downloaded += rt;
    fwrite(read_buf, sizeof(char), rt, file);
  }

  fclose(file);

  std::cout << "get file done, " << downloaded << " bytes received" << std::endl;

  close(ctrl_sock);
  close(data_sock);

  return 0;
}
