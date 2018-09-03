#include "peer_connection.h"
#include "peer_msg.h"
#include "piece_request.h"
#include <iostream>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <boost/dynamic_bitset.hpp>
#include <queue>

#define PEER_CONNECT_TIMEOUT_SEC 5

struct peer_state
{
  bool choked;
  bool interested;
};

struct conn_state
{
  peer_state local;
  peer_state remote;
  boost::dynamic_bitset<> peer_have;
  boost::dynamic_bitset<> peer_wants;
  boost::dynamic_bitset<> local_have;
  size_t bitlen;
  unsigned blocks_sent;
  unsigned blocks_recvd;
  std::queue<request_msg> peer_requests;   // requests from peer
  std::list<piece_request> local_requests; // requests from piece
};

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

void peer_connection_queue_name(pthread_t thread, char *out, size_t len)
{
  assert(len >= strlen("/") + 2 * sizeof(pthread_t) + strlen("_queue") + 1);

  size_t plen = 0;
  plen += snprintf(out, len - plen, "/");

  for(unsigned char *cp = (unsigned char *)thread;
      cp < ((unsigned char *)thread) + sizeof(pthread_t); cp++) {
    plen += snprintf(out + plen, len - plen, "%02X", *cp);
    if (plen == len) {
      return;
    }
  }

  snprintf(out + plen, len - plen, "_queue");
}

mqd_t peer_queue_open(int flags)
{
  std::cout << "enter peer_queue_open" << std::endl;

  char queue_name[64] = {0};
  peer_connection_queue_name(pthread_self(), queue_name, sizeof(queue_name));

  struct mq_attr attr;
  attr.mq_flags = O_NONBLOCK;
  attr.mq_maxmsg = 10; // TODO: get max value for this with getrlimit
  attr.mq_msgsize = sizeof(unsigned);
  attr.mq_curmsgs = 0;

  mqd_t ret = mq_open(queue_name, flags, 0600, &attr);
  if ((mqd_t)-1 != ret) {
    std::cout << "successfully opened message queue from receiver thread: "
              << queue_name << std::endl;
  }
  else {
    perror("mq_open");
    std::cout << "failed to open queue in receiver thread: "
              << queue_name << std::endl;
  }

  return ret;
}

std::shared_ptr<conn_state> conn_state_init(const torrent_info2 *torrent)
{
  std::cout << "enter conn_state_init" << std::endl;
  assert(torrent);

  std::shared_ptr<conn_state> ret = std::make_shared<conn_state>();
  if (!ret) {
    return ret;
  }

  ret->local.choked = true;
  ret->local.interested = false;
  ret->remote.choked = true;
  ret->remote.interested = false;

  ret->bitlen = torrent->pieces.size();
  std::cout << "conn_state.bitlen: " << ret->bitlen << std::endl;

  ret->peer_have.resize(ret->bitlen, false);
  std::cout << "conn_state.peer_have:" << ret->peer_have << std::endl;

  ret->peer_wants.resize(ret->bitlen, false);
  std::cout << "conn_state.peer_wants: " << ret->peer_wants << std::endl;

  pthread_mutex_lock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));
  torrent_make_bitfield(torrent, &(ret->local_have));
  pthread_mutex_unlock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));
  std::cout << "conn_state.local_have: " << ret->local_have << std::endl;

  ret->blocks_sent = 0;
  ret->blocks_recvd = 0;

  return ret;
}

void unchoke(int sockfd, const torrent_info2 *torrent, conn_state *state)
{
  std::cout << "enter unchoke" << std::endl;
  assert(state);
  assert(torrent);

  peer_msg2 unchoke_msg;
  unchoke_msg.type = MSG_UNCHOKE;

  if (peer_msg_send(sockfd, torrent, &unchoke_msg)) {
    std::cout << "peer_msg_send MSG_UNCHOKE failed" << std::endl;
    return;
  }

  state->remote.choked = false;
  std::cout << "unchoked peer" << std::endl;
}

void process_msg(int sockfd, const torrent_info2 *torrent,
                 const peer_msg2 *msg, conn_state *state)
{
  std::cout << "enter process_msg" << std::endl;
}

int process_queued_msgs(int sockfd, const torrent_info2 *torrent,
                        conn_state *state)
{
  std::cout << "process_queued_msgs" << std::endl;

  while (peer_msg_buff_nonempty(sockfd)) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    peer_msg2 msg;

    if (peer_msg_recv(sockfd, torrent, &msg)) {
      std::cout << "peer_msg_recv failed" << std::endl;
      return -1;
    }

    process_msg(sockfd, torrent, &msg, state);
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

  mqd_t queue = 0;
  queue = peer_queue_open(O_RDONLY | O_CREAT | O_NONBLOCK);
  if ((mqd_t)-1 == queue) {
    std::cout << "peer_queue_open failed" << std::endl;
    pthread_exit(NULL);
  }

  // TODO: cleanup peer queue, why build error if use pthread_cleanup_push
  std::shared_ptr<conn_state> state = conn_state_init(torrent);
  if (!state) {
    std::cout << "conn_state_init failed" << std::endl;
    pthread_exit(NULL);
  }

  // send the initial bitfield
  peer_msg2 bitmsg;
  bitmsg.type = MSG_BITFIELD;
  bitmsg.payload.bitfield.resize(torrent->pieces.size(), false);
  std::cout << "peer_msg2.payload.bitfield: " << bitmsg.payload.bitfield
            << std::endl;
  if (peer_msg_send(sockfd, torrent, &bitmsg)) {
    std::cout << "peer_msg_send failed" << std::endl;
    pthread_exit(NULL);
  }

  unchoke(sockfd, torrent, state.get());

  int i = 0;
  while (true && i++ < 1) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    usleep(250 * 1000);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // TODO: mq_receive

    if (process_queued_msgs(sockfd, torrent, state.get())) {
      std::cout << "process_queued_msgs failed" << std::endl;
      pthread_exit(NULL);
    }
  }

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
