#include "peer_connection.h"
#include "peer_msg.h"
#include "piece_request.h"
#include "bitfield_macro.h"
#include <iostream>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <boost/dynamic_bitset.hpp>
#include <queue>

#define PEER_CONNECT_TIMEOUT_SEC 5
#define PEER_NUM_OUTSTANDING_REQUESTS 1

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
  std::queue<request_msg> peer_requests;  // requests from peer
  std::list<piece_request *> local_requests; // requests from piece
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
  ret->peer_have.resize(ret->bitlen, false);
  ret->peer_wants.resize(ret->bitlen, false);

  pthread_mutex_lock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));
  torrent_make_bitfield(torrent, &(ret->local_have));
  pthread_mutex_unlock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));

  ret->blocks_sent = 0;
  ret->blocks_recvd = 0;

  return ret;
}

void unchoke(int sockfd, const torrent_info2 *torrent, conn_state *state)
{
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

void show_interested(int sockfd, const torrent_info2 *torrent,
                     conn_state *state)
{
  peer_msg2 interested_msg;
  interested_msg.type = MSG_INTERESTED;

  if (peer_msg_send(sockfd, torrent, &interested_msg)) {
    std::cout << "peer_msg_send MSG_INTERESTED failed" << std::endl;
    return;
  }

  state->local.interested = true;
}

void show_not_interested(int sockfd, const torrent_info2 *torrent,
                         conn_state *state)
{
  std::cout << "enter show_not_interested" << std::endl;

  peer_msg2 not_interested_msg;
  not_interested_msg.type = MSG_NOT_INTERESTED;

  if (peer_msg_send(sockfd, torrent, &not_interested_msg)) {
    std::cout << "peer_msg_send MSG_NOT_INTERESTED failed" << std::endl;
    return;
  }

  state->local.interested = false;
}

void handle_piece_dnld_completion(int sockfd, torrent_info2 *torrent,
                                  unsigned index)
{
  assert(index < torrent->pieces.size());

  bool completed = false;
  unsigned pieces_left = 0;

  pthread_mutex_lock(&torrent->sh_mutex);

  if (torrent->sh.pieces_state[index] != PIECE_STATE_HAVE) {
    torrent->sh.pieces_state[index] = PIECE_STATE_HAVE;
    torrent->sh.pieces_left--;

    assert(torrent->sh.pieces_left < torrent->pieces.size());

    if (0 == torrent->sh.pieces_left) {
      torrent->sh.completed = true;
      completed = true;
    }
  }
  pieces_left = torrent->sh.pieces_left;

  pthread_mutex_unlock(&torrent->sh_mutex);

  std::cout << "pieces left: " << pieces_left << std::endl;

  if (completed) {
    torrent_complete(torrent);
  }

  peer_msg2 have_msg;
  have_msg.type = MSG_HAVE;
  have_msg.payload.have = index;
  peer_msg_send(sockfd, torrent, &have_msg);
}

void process_piece_msg(int sockfd, const piece_msg *msg,
                       torrent_info2 *torrent, conn_state *state)
{
  std::cout << "enter process_piece_msg" << std::endl;

  typedef std::list<piece_request *>::iterator piece_req_it;
  typedef std::list<block_request *>::iterator block_req_it;

  for (piece_req_it piece_it = state->local_requests.begin();
       piece_it != state->local_requests.end();
       ++piece_it) {
    if ((*piece_it)->piece_index == msg->index) {
      for (block_req_it block_it = (*piece_it)->block_requests.begin();
           block_it != (*piece_it)->block_requests.end();
           ++block_it) {
        if ((*block_it)->len == msg->block_len &&
            (*block_it)->begin == msg->begin) {
          (*block_it)->completed = true;
          (*piece_it)->blocks_left--;
          break;
        }
      }

      if ((*piece_it)->blocks_left == 0) {
        // got the entire piece
        std::cout << "got a piece fam" << std::endl;

        bool valid = torrent_sha1_verify(torrent, (*piece_it)->piece_index);
        if (!valid) {
          std::cout << "piece download does not have an expected SHA1 hash"
                    << std::endl;
          pthread_mutex_lock(&torrent->sh_mutex);
          torrent->sh.pieces_state[(*piece_it)->piece_index] =
            PIECE_STATE_NOT_REQUESTED;
          pthread_mutex_unlock(&torrent->sh_mutex);
        }
        else {
          std::cout << "successfully downloaded a piece: "
                    << (*piece_it)->piece_index << std::endl;
          handle_piece_dnld_completion(sockfd,
                                       torrent,
                                       (*piece_it)->piece_index
                                       );
        }

        state->local_requests.remove(*piece_it++); // fix segmentation fault
      }
    }
  }
}

void process_msg(int sockfd, torrent_info2 *torrent, const peer_msg2 *msg,
                 conn_state *state)
{
  bool interested = false;

  switch (msg->type) {
  case MSG_KEEPALIVE: {
    break;
  }
  case MSG_CHOKE: {
    state->local.choked = true;
    std::cout << "i'm choked" << std::endl;
    break;
  }
  case MSG_UNCHOKE: {
    state->local.choked = false;
    std::cout << "i'm unchoked" << std::endl;
    break;
  }
  case MSG_INTERESTED: {
    state->remote.interested = true;
    std::cout << "the remote peer is interested in us" << std::endl;
    break;
  }
  case MSG_NOT_INTERESTED: {
    state->remote.interested = false;
    std::cout << "the remote peer is not interested in us" << std::endl;
    break;
  }
  case MSG_HAVE: {
    if (!state->local.interested && state->local_have[msg->payload.have]) {
      show_interested(sockfd, torrent, state);
    }

    state->peer_have[msg->payload.have] = 1;
    break;
  }
  case MSG_BITFIELD: {
    std::cout << "conn_state.peer_have: " << state->peer_have << std::endl;
    /*assert(msg->payload.bitfield.num_blocks() ==
      BITFIELD_NUM_BYTES(state->bitlen));*/
    std::cout << "state->bitlen: " << state->bitlen << std::endl;
    for (unsigned i = 0; i < state->peer_have.size(); ++i) {
      state->peer_have[i] = msg->payload.bitfield[i];
    }
    std::cout << "conn_state.peer_have: " << state->peer_have << std::endl;

    pthread_mutex_lock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));
    for (unsigned i = 0; i < torrent->pieces.size(); ++i) {
      if (torrent->sh.pieces_state.at(i) != PIECE_STATE_HAVE &&
          state->peer_have[i]) {
        interested = true;
        std::cout << "torrent.sh.pieces_state [" << i << "] interested"
                  << std::endl;
        break;
      }
    }
    pthread_mutex_unlock(&((const_cast<torrent_info2 *>(torrent))->sh_mutex));

    if (interested) {
      show_interested(sockfd, torrent, state);
    }

    break;
  }
  case MSG_REQUEST: {
    std::cout << "push remote peer request:" << std::endl;
    std::cout << "  index: " << msg->payload.request.index << std::endl;
    std::cout << "  begin: " << msg->payload.request.begin << std::endl;
    std::cout << " length: " << msg->payload.request.length << std::endl;
    state->peer_requests.push(msg->payload.request);
    break;
  }
  case MSG_PIECE: {
    process_piece_msg(sockfd, &msg->payload.piece, torrent, state);
    state->blocks_recvd++;
    break;
  }
  case MSG_CANCEL: {
    // TODO
    break;
  }
  case MSG_PORT: {
    // TODO
    break;
  }
  default:
    break;
  }
}

int process_queued_msgs(int sockfd, torrent_info2 *torrent, conn_state *state)
{
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

int send_requests(int sockfd, torrent_info2 *torrent, conn_state *state)
{
  int n = PEER_NUM_OUTSTANDING_REQUESTS - state->local_requests.size();
  std::cout << "n = " << n << std::endl;
  if (n <= 0) {
    return 0;
  }

  bool not_interested = false;

  for (int i = 0; i < n; ++i) {
    unsigned req_index = 0;

    if (torrent_next_request(torrent, &state->peer_have, &req_index)) {
      std::cout << "not find a piece we can request" << std::endl;
      not_interested = true;
      break;
    }

    std::cout << "sending request for piece " << req_index << std::endl;

    piece_request *request = new piece_request; // TODO: need delete
    piece_request_create(torrent, req_index, request);
    state->local_requests.push_back(request);

    typedef std::list<block_request *>::iterator block_request_it;

    for (block_request_it it = request->block_requests.begin();
         it != request->block_requests.end();
         ++it) {
      peer_msg2 req_msg;
      req_msg.type = MSG_REQUEST;
      req_msg.payload.request.index = request->piece_index;
      req_msg.payload.request.begin = (*it)->begin;
      req_msg.payload.request.length = (*it)->len;

      if (peer_msg_send(sockfd, torrent, &req_msg)) {
        std::cout << "peer_msg_send MSG_REQUEST failed" << std::endl;
        return -1;
      }
    }
  }

  if (state->local.interested && not_interested) {
    show_not_interested(sockfd, torrent, state);
  }

  return 0;
}

static void *peer_connection(void *arg)
{
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

  while (true) {
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    usleep(250 * 1000);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // TODO: mq_receive

    if (process_queued_msgs(sockfd, torrent, state.get())) {
      std::cout << "process_queued_msgs failed" << std::endl;
      pthread_exit(NULL);
    }

    if (state->peer_requests.size() > 0) {
      // TODO
    }
    else {
      if (!state->local.choked && state->local.interested) {
        if (send_requests(sockfd, torrent, state.get())) {
          std::cout << "send_requests failed" << std::endl;
          pthread_exit(NULL);
        }
      }
    }
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
  }

  return NULL;
}

int peer_connection_create(pthread_t *thread, peer_arg *arg)
{
  if (pthread_create(thread, NULL, peer_connection, (void *)arg)) {
    std::cout << "pthread_create peer_connection failed" << std::endl;
    return -1;
  }

  pthread_join(*thread, NULL);

  return 0;
}
