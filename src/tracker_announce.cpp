#include "tracker_announce.h"
#include "http_url_parser.h"
#include "utils.h"
#include "tracker_resp_parser.h"
#include <iostream>

int build_http_request(http_url_parser *url, tracker_announce_req *req,
                       std::string &out_str)
{
  std::cout << "enter build_http_request" << std::endl;

  if (!req) {
    std::cout << "param req is nullptr" << std::endl;
    return -1;
  }

  out_str = "GET ";
  out_str += url->path_;
  out_str += "?info_hash=";
  out_str += utils::percent_encode((const unsigned char *)req->info_hash, 20);
  out_str += "&peer_id=";
  out_str += utils::percent_encode((const unsigned char *)req->peer_id, 20);
  out_str += "&port=";
  out_str += std::to_string(req->port);
  out_str += "&uploaded=";
  out_str += std::to_string(req->uploaded);
  out_str += "&downloaded=";
  out_str += std::to_string(req->downloaded);
  out_str += "&left=";
  out_str += std::to_string(req->left);

  if (HAS(req, REQUEST_HAS_COMPACT)) {
    out_str += "&compact=";
    out_str += std::to_string(!!(req->compact));
  }

  if (HAS(req, REQUEST_HAS_EVENT)) {
    std::string event_str;

    switch (req->event) {
    case TORRENT_EVENT_STARTED: {
      event_str = "started";
      break;
    }
    case TORRENT_EVENT_COMPLETED: {
      event_str = "completed";
      break;
    }
    case TORRENT_EVENT_STOPPED: {
      event_str = "stopped";
      break;
    }
    }

    out_str += "&event=";
    out_str += event_str;
  }

  if (HAS(req, REQUEST_HAS_IP)) {
  }

  if (HAS(req, REQUEST_HAS_NUMWANT)) {
    out_str += "&numwant=";
    out_str += std::to_string(req->numwant);
  }

  if (HAS(req, REQUEST_HAS_KEY)) {
    out_str += "&key=";
    out_str += req->key;
  }

  if (HAS(req, REQUEST_HAS_TRACKER_ID)) {
    out_str += "&trackerid=";
    out_str += req->trackerid;
  }

  out_str += " HTTP/1.1\r\nHost: ";
  out_str += url->domain_;
  out_str +=  "\r\n\r\n";

  return 0;
}

int tracker_connect(http_url_parser *url)
{
  std::cout << "enter tracker_connect" << std::endl;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *head;
  int ret = 0;

  ret = getaddrinfo(url->domain_.c_str(), url->port_.c_str(), &hints, &head);
  if (ret) {
    perror("getaddrinfo");
    return -1;
  }

  int sockfd = 0;
  struct addrinfo *i = NULL;

  for (i = head; i; i = i->ai_next) {
    if (-1 == (sockfd = socket(i->ai_family, i->ai_socktype, i->ai_protocol))) {
      continue;
    }

    if (-1 == connect(sockfd, i->ai_addr, i->ai_addrlen)) {
      perror("connect");
      close(sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(head);

  if (!i) {
    std::cout << "unable to connect tracker: " << url->domain_ << std::endl;
    return -1;
  }

  errno = 0;
  std::cout << "successfully connected(socket: " << sockfd << ") to tracker: "
            << url->domain_ << std::endl;

  return sockfd;
}

int tracker_send_all(int sockfd, const char *buff, size_t len)
{
  unsigned total_sent = 0;

  while (total_sent < len) {
    ssize_t sent = send(sockfd, buff, len - total_sent, 0);

    if (-1 == sent) {
      perror("send");
      return -1;
    }

    total_sent += sent;
    buff += sent;
  }

  return 0;
}

int content_from_tracker_resp(char *buff, size_t len, std::string &out_str)
{
  std::cout << "enter content_from_tracker_resp" << std::endl;

  char *line = NULL;
  char *saveptr = NULL;
  bool chuncked = false;
  unsigned content_length = 0;

  line = strtok_r(buff, "\n", &saveptr);

  if (strncmp(line, "HTTP/1.0 200 OK", strlen("HTTP/1.0 200 OK")) &&
      strncmp(line, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK"))) {
    std::cout << "can not find HTTP/1.x 200 OK" << std::endl;
    return -1;
  }

  char *token = NULL;
  char *saveptrtoken = NULL;

  do {
    line = strtok_r(NULL, "\n", &saveptr);

    if (!strncmp(line, "Transfer-Encoding: chuncked",
                 strlen("Transfer-Encoding: chuncked"))) {
      chuncked = true;
    }

    if (!strncmp(line, "Content-Length:", strlen("Content-Length:"))) {
      token = strtok_r(line, ":", &saveptrtoken);
      token = strtok_r(NULL, ":", &saveptrtoken);

      content_length = strtoul(token, NULL, 0);
    }
  } while (strlen(line) != 1);

  if (chuncked) {
    std::cout << "chuncked is true" << std::endl;
    // TODO
  }
  else {
    std::cout << "chuncked is false" << std::endl;
    std::string str(line + strlen(line) + 1, content_length);
    out_str = str;
  }

  return 0;
}

int tracker_recv_resp(int sockfd, std::string &out_str)
{
  std::cout << "enter tracker_recv_resp" << std::endl;

  char buff[1024] = {0};
  ssize_t total_recv = 0;
  ssize_t ret = 0;

  do {
    ret = recv(sockfd, buff + total_recv, sizeof(buff) - total_recv, 0);

    if (-1 == ret) {
      perror("recv");
      return -1;
    }

    total_recv += ret;
  } while (ret > 0);

  std::cout << buff << std::endl;

  if (content_from_tracker_resp(buff, total_recv, out_str)) {
    std::cout << "content_from_tracker_resp failed" << std::endl;
    return -1;
  }

  std::cout << "out_str: " << out_str << std::endl;

  return 0;
}

std::shared_ptr<tracker_announce_resp>
tracker_announce(const std::string &url, tracker_announce_req *req)
{
  std::cout << "enter tracker_announce" << std::endl;

  std::cout << "announce url: " << url << std::endl;

  std::shared_ptr<http_url_parser> url_parser =
    std::make_shared<http_url_parser>(url);
  std::cout << "url's scheme: " << url_parser->scheme_ << std::endl;

  if (url_parser->scheme_ == "https" ||
      url_parser->scheme_ == "udp") {
    std::cout << "not support for https and udp tracker protocols" << std::endl;
    return NULL;
  }

  std::string req_str;
  build_http_request(url_parser.get(), req, req_str);

  std::cout << req_str << std::endl;

  int sockfd = 0;
  if ((sockfd = tracker_connect(url_parser.get())) < 0) {
    std::cout << "tracker_announce failed" << std::endl;
    return NULL;
  }

  if (tracker_send_all(sockfd, req_str.c_str(), req_str.length())) {
    std::cout << "tracker_sendall failed" << std::endl;
    return NULL;
  }

  std::string resp_str;
  if (tracker_recv_resp(sockfd, resp_str)) {
    std::cout << "tracker_recv_resp failed" << std::endl;
    return NULL;
  }

  std::shared_ptr<tracker_announce_resp> ret = tracker_resp_parse(resp_str);
  if (!ret) {
    std::cout << "tracker_resp_parse failed" << std::endl;
    close(sockfd);
    return NULL;
  }

  return ret;
}

void print_tracker_announce_resp(const tracker_announce_resp *resp)
{
  std::cout << "enter print_tracker_announce_resp" << std::endl;
  assert(resp);

  std::cout << "tracker announce response:" << std::endl;

  if (HAS(resp, RESPONSE_HAS_FAILURE_REASON)) {
    std::cout << "  failure reason: " << resp->failure_reason << std::endl;
  }

  if (HAS(resp, RESPONSE_HAS_WARNING_MESSAGE)) {
    std::cout << "  warning message: " << resp->warning_message << std::endl;
  }

  if (HAS(resp, RESPONSE_HAS_TRACKER_ID)) {
    std::cout << "  tracker id: " << resp->tracker_id << std::endl;
  }

  if (HAS(resp, RESPONSE_HAS_MIN_INTERVAL)) {
    std::cout << "  min interval: " << resp->min_interval << std::endl;
  }

  std::cout << "  interval: " << resp->interval << std::endl;
  std::cout << "  complete: " << resp->complete << std::endl;
  std::cout << "  incomplete: " << resp->incomplete << std::endl;
  std::cout << "  peers's size: " << resp->peers.size() << std::endl;

  for (std::list<peer_info>::const_iterator it = resp->peers.begin();
       it != resp->peers.end(); ++it) {
    char buff[INET6_ADDRSTRLEN] = {0};
    uint16_t port = 0;

    if (it->addr.sas.ss_family == AF_INET) {
      inet_ntop(AF_INET, &it->addr.sa_in.sin_addr, buff, INET_ADDRSTRLEN);
      port = ntohs(it->addr.sa_in.sin_port);
    }
    else {
      inet_ntop(AF_INET6, &it->addr.sa_in6.sin6_addr, buff, INET6_ADDRSTRLEN);
      port = ntohs(it->addr.sa_in6.sin6_port);
    }

    std::cout << "    peer: " << buff << " [port: " << port << "]" << std::endl;
  }
}
