#include "torrent_downloader.h"

torrent_downloader::torrent_downloader()
{
}

torrent_downloader::~torrent_downloader()
{
}

// TODO: multi files mode connection
int get_tracker_socket(const http_url_parser &hup)
{
  struct addrinfo hints, *tracker, *head;
  int sockfd;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (0 != getaddrinfo(hup.domain_.c_str(), hup.port_.c_str(), &hints, &head)) {
    return -1;
  }

  for (tracker = head; tracker; tracker = tracker->ai_next) {
    if ((sockfd = socket(tracker->ai_family, tracker->ai_socktype, tracker->ai_protocol)) < 0) {
      continue;
    }

    if (connect(sockfd, tracker->ai_addr, tracker->ai_addrlen) < 0) {
      close(sockfd);
      continue;
    }

    break;
  }

  if (!tracker) {
    log_w("cannot to connect to tracker: %s\n", hup.domain_.c_str());
    freeaddrinfo(head);
    close(sockfd);
    return -1;
  }

  freeaddrinfo(head);

  log_t("%s connected (socket: %d)\n", hup.domain_.c_str(), sockfd);

  return sockfd;
}

int send_tracker_request(int sockfd, const char *request, size_t length)
{
  size_t total_sent = 0;

  while (total_sent < length) {
    size_t sent = send(sockfd, request, length - total_sent, 0);
    if (sent < 0) {
      return -1;
    }

    total_sent += sent;
    request += sent;
  }

  return 0;
}

int get_tracker_response_peers(int sockfd, std::string &response_peers)
{
  size_t total_recv = 0;
  int temp = 0;
  char buff[2048] = {0};

  response_peers.clear();

  do
    {
      temp = recv(sockfd, buff + total_recv, sizeof(buff) - total_recv, 0);
      if (temp < 0) {
        return -1;
      }

      total_recv += temp;
    } while (temp > 0);

  std::string response(buff, total_recv); // need total_recv or segmentation fault
  http_header_parser hhp(response.c_str());

  if (std::string::npos == hhp.get_status().find("200 OK")) {
    log_w("cannot to connect to tracker\n");
    return -1;
  }

  const char *http_header_end = "\r\n\r\n";
  size_t pos = response.find(http_header_end);
  if (std::string::npos == pos) {
    return -1;
  }

  response_peers = response.substr(pos + strlen(http_header_end), total_recv);
  // std::cout << response_peers << std::endl;

  return 0;
}

int parse_tracker_response(torrent_info *ti, const std::string &response_peers)
{
  bencode_parser bp(response_peers.c_str());
  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();

  get_peers(ti, dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  // print all peers (format ip:port)
  if (!ti->peers_.empty()) {
    for (std::vector<std::pair<std::string, uint16_t> >::iterator it = ti->peers_.begin();
         it != ti->peers_.end(); ++it) {
      std::cout << it->first << ":" << it->second << std::endl;
    }
  }

  return 0;
}

// try connect a peer and get socket
int get_peer_socket(std::pair<std::string, uint16_t> *peer)
{
  int sockfd = 0;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    return sockfd;
  }

  sockaddr_in sa;
  bzero(&sa, sizeof(sockaddr_in));
  sa.sin_family = AF_INET;

  int rt = bind(sockfd, (sockaddr *)&sa, sizeof(sa));
  if (sockfd < 0) {
    log_e("bind() error [%s] %s\n", peer->first.c_str(), strerror(errno));
    close(sockfd);
    return -1;
  }

  hostent *p = gethostbyname(peer->first.c_str());
  if (sockfd < 0) {
    log_e("gethostbyname() error [%s] %s\n", peer->first.c_str(), strerror(errno));
    close(sockfd);
    return -1;
  }

  sa.sin_port = htons(peer->second);
  memcpy(&sa.sin_addr, p->h_addr, 4);

  rt = connect(sockfd, (sockaddr *)&sa, sizeof(sa));
  if (rt < 0) {
    log_e("connect() error [%s] %s\n", peer->first.c_str(), strerror(errno));
    close(sockfd);
    return -1;
  }

  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  struct timeval timeout;
  timeout.tv_sec = 5; // 5 seconds
  timeout.tv_usec = 0;

  rt = select(sockfd + 1, NULL, &fdset, NULL, &timeout);
  if (rt > 0) {
    int so_error = 0;
    socklen_t len = sizeof(so_error);

    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error) {
      close(sockfd);
      return -1;
    }
  }
  else {
    log_w("peer connection time out\n");
    close(sockfd);
    return -1;
  }

  log_t("connect [%s] success\n", peer->first.c_str());

  return sockfd;
}

int send_buff(int sockfd, const char *buff, size_t length)
{
  size_t total_sent = 0;

  while (total_sent < length) {
    size_t sent = send(sockfd, buff, length - total_sent, 0);
    if (sent < 0) {
      log_e("send() error %s\n", strerror(errno));
      return -1;
    }

    total_sent += sent;
    buff += sent;
  }

  return 0;
}

int send_handshake(int sockfd, torrent_info *ti)
{
  const char *pstr = "BitTorrent protocol";
  unsigned char pstrlen = strlen(pstr);
  const char reserved[8] = {0};

  // because of info_hash_ is hex string so its size is 40
  size_t info_hash_length = ti->info_hash_.size() / 2;

  size_t length =
    1 +
    pstrlen +
    sizeof(reserved) +
    info_hash_length +
    20; // length of peer_id

  off_t off = 0;
  char buff[length] = {0};

  buff[0] = pstrlen;
  off++;

  memcpy(buff + off, pstr, pstrlen);
  off += pstrlen;

  memcpy(buff + off, reserved, sizeof(reserved));
  off += sizeof(reserved);

  memcpy(buff + off, ti->info_hash_.c_str(), info_hash_length);
  off += info_hash_length;

  memcpy(buff + off, ti->peer_id_.c_str(), 20);

  return send_buff(sockfd, buff, length);
}

int recv_buff(int sockfd, char *buff, size_t length)
{
  size_t total_recv = 0;
  int rt = 0;

  do
    {
      rt = recv(sockfd, buff + total_recv, sizeof(buff) - total_recv, 0);
      std::cout << "rt: " << rt << std::endl;
      if (rt < 0) {
        log_e("recv() error %s\n", strerror(errno));
        close(sockfd);
        return -1;
      }

      total_recv += rt;
    } while (rt > 0 && total_recv < length);

  std::cout << "total_recv: " << total_recv << " length: " << length << std::endl;
  if (total_recv == length) {
    return 0;
  }
  else {
    return -1;
  }
}

int recv_handshake(int sockfd, char out_info_hash[20], char out_peer_id[20], bool peer_id)
{
  const char *pstr = "BitTorrent protocol";
  unsigned char pstrlen = strlen(pstr);
  const char reserved[8] = {0};

  size_t length =
    1 +
    pstrlen +
    sizeof(reserved) +
    sizeof(char[20]) +
    (peer_id ? 20 : 0);

  char buff[length] = {0};
  if (recv_buff(sockfd, buff, length)) {
    log_e("recv_buff() error\n");
    return -1;
  }

  off_t off = 0;

  if (buff[off] != pstrlen) {
    log_e("buff[0] != pstrlen\n");
    return -1;
  }

  off++;

  if (strncmp(buff + off, pstr, pstrlen)) {
    log_e("buff not equal \"BitTorrent protocol\"\n");
    return -1;
  }

  off += pstrlen;

  // for size of reserved
  off += sizeof(reserved);

  memcpy(out_info_hash, buff + off, sizeof(char[20]));

  if (peer_id) {
    off += sizeof(char[20]);
    memcpy(out_peer_id, buff + off, sizeof(char[20]));
  }

  return 0;
}

// peer connection thread procedure
void *peer_connection_thread_proc(void *arg)
{
  // std::pair<std::string, uint16_t> *peer = (std::pair<std::string, uint16_t> *)arg;
  peer_thread_arg *pta = (peer_thread_arg *)arg;

  int sockfd = get_peer_socket(pta->peer_);
  if (sockfd < 0) {
    return NULL;
  }

  torrent_info *ti = NULL;
  char out_info_hash[20] = {0};
  char out_peer_id[20] = {0};

  if (pta->has_torrent_) {
    ti = pta->ti_;

    send_handshake(sockfd, ti);
    recv_handshake(sockfd, out_info_hash, out_peer_id, true);

    std::cout << "out_info_hash: " << out_info_hash << std::endl;
    std::cout << "out_peer_id: " << out_peer_id << std::endl;
  }

  // free it here
  if (pta != NULL) {
    free(pta);
  }

  return NULL;
}

// create new thread for peer connection
int create_peer_connection(pthread_t *tid, peer_thread_arg *arg)
{
  pthread_create(tid, NULL, peer_connection_thread_proc, (void *)(arg));

  return 0;
}

// create connection for each peer, [ti] including all peers
int peer_connect(std::pair<std::string, uint16_t> *peer, torrent_info *ti)
{
  pthread_t tid = 0;

  /*
  std::shared_ptr<peer_thread_arg> sp_pta = std::make_shared<peer_thread_arg>();
  sp_pta->ti_ = ti;
  sp_pta->peer_ = peer;

  std::cout << sp_pta.get() << std::endl;
  */

  // must use malloc or get same address
  // free it in thread procedure
  peer_thread_arg *pta = (peer_thread_arg *)malloc(sizeof(peer_thread_arg));
  pta->ti_ = ti;
  pta->peer_ = peer;
  pta->has_torrent_ = true;

  create_peer_connection(&tid, pta);

  return 0;
}

void *connect_tracker_thread(void *arg)
{
  log_t("connect_tracker_thread start\n");

  torrent_info *ti = (torrent_info *)arg;

  // connect tracker announce
  for (std::vector<std::string>::iterator it = ti->announce_list_.begin();
       it != ti->announce_list_.end(); ++it) {
    log_t("connecting %s\n", it->c_str());

    http_url_parser hup(it->c_str());

    if (hup.scheme_ == "https" ||
        hup.scheme_ == "udp") {
      log_w("not support https and udp for now\n");
      continue;
    }

    // convert a hex string to a real hex string ("d4" -> 0xd4)
    assert(!ti->info_hash_.empty());
    std::string hex;
    utils::hex_string_to_hex(ti->info_hash_, hex);

    // generate request string
    std::string request_str;
    request_str += "GET ";
    request_str += hup.path_; // already including '/'
    request_str += "?info_hash=";
    request_str += utils::percent_encode(hex);
    request_str += "&peer_id=";
    request_str += ti->peer_id_;
    request_str += "&left=";
    request_str += std::to_string(ti->files_size_);
    request_str += "&compact=1";
    request_str += " HTTP/1.1\r\nHost: ";
    request_str += hup.domain_;
    request_str += "\r\n\r\n";

    // std::cout << "request string: " << request_str << std::endl;

    // get socket
    int sockfd = get_tracker_socket(hup);

    send_tracker_request(sockfd, request_str.c_str(), request_str.size());

    std::string response_peers;
    get_tracker_response_peers(sockfd, response_peers);

    parse_tracker_response(ti, response_peers);
    assert(!ti->peers_.empty());

    // create connection for peers
    for (size_t i = 0; i < ti->peers_.size(); ++i) {
      std::pair<std::string, uint16_t> *peer = &(ti->peers_.at(i));
      peer_connect(peer, ti);
    }

    sleep(10);

    close(sockfd);
  }

  return NULL;
}

int torrent_downloader::connect_tracker(pthread_t *tid, torrent_info *ti)
{
  if (0 == pthread_create(tid, NULL, connect_tracker_thread, (void *)ti)) {
    return 0;
  }

  return -1;
}

void torrent_downloader::download_it(const std::string &torrent_file, const std::string &dest_dir)
{
  std::ifstream file(torrent_file.c_str(), std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  auto ti = std::make_shared<torrent_info>();
  get_announce(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  /*
  for (std::vector<std::string>::iterator it = ti->announce_list_.begin();
       it != ti->announce_list_.end(); ++it) {
    std::cout << "announce: " << it->c_str() << std::endl;
  }
  */

  get_info_hash(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  get_peer_id(ti.get());

  get_files_and_size(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  /*
  std::cout << "get files: " << std::endl;
  for (std::vector<std::pair<std::string, long long> >::iterator it = ti->files_.begin();
       it != ti->files_.end(); ++it) {
    std::cout << "file path + name: " << it->first.c_str() << "\nlength: " << it->second << std::endl;
  }
  */

  pthread_t tid = 0;

  connect_tracker(&tid, ti.get());

  if (0 != tid) {
    // waits for the thread specified by thread to terminate
    pthread_join(tid, NULL);
  }
}
