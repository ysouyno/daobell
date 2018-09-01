#include "peer_connection.h"
#include "peer_msg.h"
#include <iostream>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define PEER_CONNECT_TIMEOUT_SEC 5

void get_peer_ip(const peer_info *peer, char *out_str, size_t out_len)
{
  assert(peer);
  assert(out_str);

  if (peer->addr.sas.ss_family == AF_INET) {
    inet_ntop(AF_INET, &peer->addr.sa_in.sin_addr, out_str, out_len);
  }
  else {
    inet_ntop(AF_INET6, &peer->addr.sa_in6.sin6_addr, out_str, out_len);
  }
}

int peer_connect(peer_arg *arg)
{
  std::cout << "enter peer_connect" << std::endl;

  char ipstr[INET6_ADDRSTRLEN] = {0};
  get_peer_ip(&arg->peer, ipstr, sizeof(ipstr));

  int sockfd = 0;
  sockfd = socket(arg->peer.addr.sas.ss_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (sockfd < 0) {
    perror("socket");
    return sockfd;
  }

  socklen_t addrlen = 0;
  if (arg->peer.addr.sas.ss_family == AF_INET) {
    addrlen = sizeof(arg->peer.addr.sa_in);
  }
  else {
    addrlen = sizeof(arg->peer.addr.sa_in6);
  }

  int ret = 0;
  ret = connect(sockfd, &arg->peer.addr.sa, addrlen);
  if (!(0 == ret || EINPROGRESS == errno)) {
    perror("connect");
    close(sockfd);
    return -1;
  }

  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  struct timeval timeout;
  timeout.tv_sec = PEER_CONNECT_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  if (select(sockfd + 1, NULL, &fdset, NULL, &timeout) > 0) {
    int so_error = 0;
    socklen_t optlen = sizeof(so_error);

    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &optlen);
    if (so_error) {
      perror("getsockopt");
      close(sockfd);
      return -1;
    }
  }
  else {
    std::cout << "peer (" << ipstr << ") connection attempt timed out after "
              << PEER_CONNECT_TIMEOUT_SEC << " seconds, sockfd: " << sockfd
              << std::endl;
    close(sockfd);
    return -1;
  }

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

  int opts = fcntl(sockfd, F_GETFL);
  opts &= ~O_NONBLOCK;
  fcntl(sockfd, F_SETFL, opts);

  std::cout << "successfully established connection to peer at: "
            << ipstr << ", sockfd: " << sockfd << std::endl;

  return sockfd;
}

int handshake(int sockfd, const peer_arg *parg, char peer_id[20],
              char info_hash[20], torrent_info2 **out)
{
  std::cout << "enter handshake" << std::endl;
  assert(parg);

  if (parg->has_torrent) { // download from peer
    *out = parg->torrent;

    if (peer_send_handshake(sockfd, (*out)->info_hash)) {
      std::cout << "peer_send_handshake failed" << std::endl;
      return -1;
    }

    if (peer_recv_handshake(sockfd, info_hash, peer_id, true)) {
      std::cout << "peer_recv_handshake failed" << std::endl;
      return -1;
    }
  }
  else { // upload to peer
    // TODO
  }

  return 0;
}

static void *peer_connection(void *arg)
{
  std::cout << "enter peer_connection" << std::endl;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  peer_arg *parg = (peer_arg *)arg;
  char ipstr[INET6_ADDRSTRLEN] = {0};
  get_peer_ip(&parg->peer, ipstr, sizeof(ipstr));

  int sockfd = 0;

  if (parg->has_sockfd) {
    sockfd = parg->sockfd;
  }
  else {
    if ((sockfd = peer_connect((peer_arg *)arg)) < 0) {
      std::cout << "peer_connect failled" << std::endl;
      pthread_exit(NULL);
    }

    parg->sockfd = sockfd;
    parg->has_sockfd = true;
  }

  if (sockfd < 0) {
    std::cout << "sockfd is invalid" << std::endl;
    pthread_exit(NULL);
  }

  char peer_id[20] = {0};
  char info_hash[20] = {0};
  torrent_info2 *torrent = NULL;

  if (handshake(sockfd, parg, peer_id, info_hash, &torrent)) {
    std::cout << "handshake failed" << std::endl;
    pthread_exit(NULL);
  }
  std::cout << "handshake with peer: " << ipstr
            << " successful" << std::endl;

  return NULL;
}

int peer_connection_create(pthread_t *thread, peer_arg *arg)
{
  std::cout << "enter peer_connection_create" << std::endl;

  if (pthread_create(thread, NULL, peer_connection, (void *)arg)) {
    std::cout << "pthread_create peer_connection failed" << std::endl;
    return -1;
  }

  pthread_join(*thread, NULL);

  return 0;
}
