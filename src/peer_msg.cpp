#include "peer_msg.h"
#include "peer_id.h"
#include "bitfield_macro.h"
#include "piece_request.h"
#include <iostream>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <memory>
#include <sys/ioctl.h>

int peer_send_buff(int sockfd, const char *buff, size_t len)
{
  std::cout << "enter peer_send_buff" << std::endl;

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

  if (total_sent == len) {
    return 0;
  }
  else {
    return -1;
  }
}

int peer_recv_buff(int sockfd, char *buff, size_t len)
{
  std::cout << "enter peer_recv_buff" << std::endl;

  unsigned total_recv = 0;
  ssize_t nb = 0;

  if (!len) {
    return -1;
  }

  do {
    assert(len - total_recv > 0);

    nb = recv(sockfd, buff + total_recv, len - total_recv, 0);
    if (-1 == nb) {
      perror("recv");
      return -1;
    }

    total_recv += nb;
  } while (nb > 0 && total_recv < len);

  if (total_recv == len) {
    return 0;
  }
  else {
    return -1;
  }
}

// https://wiki.theory.org/index.php/BitTorrentSpecification#Handshake
int peer_send_handshake(int sockfd, char info_hash[20])
{
  std::cout << "enter peer_send_handshake" << std::endl;

  const char *pstr = "BitTorrent protocol";
  unsigned char pstrlen = strlen(pstr);
  const char reserved[8] = {0};

  size_t bufflen =
    1 + pstrlen + sizeof(reserved) + 20 + sizeof(g_local_peer_id);
  off_t off = 0;
  char buff[HANDSHAKE_LEN] = {0};

  buff[0] = pstrlen;
  off++;

  memcpy(buff + off, pstr, pstrlen);
  off += pstrlen;
  assert(off == 20);

  memcpy(buff + off, reserved, sizeof(reserved));
  off += sizeof(reserved);
  assert(off == 28);

  memcpy(buff + off, info_hash, 20);
  off += 20;

  memcpy(buff + off, g_local_peer_id, sizeof(g_local_peer_id));

  return peer_send_buff(sockfd, buff, bufflen);
}

int peer_recv_handshake(int sockfd, char out_info_hash[20],
                        char out_peer_id[20], bool peer_id)
{
  std::cout << "enter peer_recv_handshake" << std::endl;

  const char *pstr = "BitTorrent protocol";
  unsigned char pstrlen = strlen(pstr);
  const char reserved[8] = {0};

  size_t bufflen =
    1 + pstrlen + sizeof(reserved) + 20 +
    (peer_id ? sizeof(g_local_peer_id) : 0);

  char *buff = new char[bufflen];
  memset(buff, 0, bufflen);

  if (peer_recv_buff(sockfd, buff, bufflen)) {
    std::cout << "peer_recv_handshake failed" << std::endl;
    delete[] buff;
    return -1;
  }

  off_t off = 0;
  if (buff[off] != pstrlen) {
    delete[] buff;
    return -1;
  }

  off++;
  if (strncmp(buff + off, pstr, pstrlen)) {
    delete[] buff;
    return -1;
  }

  off += pstrlen;

  off += sizeof(reserved);

  memcpy(out_info_hash, buff + off, 20);

  if (peer_id) {
    off += 20;
    memcpy(out_peer_id, buff + off, sizeof(g_local_peer_id));
  }

  delete[] buff;
  return 0;
}

uint32_t msg_length(const msg_type type, const torrent_info2 *torrent)
{
  std::cout << "enter msg_length" << std::endl;
  assert(torrent);

  uint32_t ret = 0;

  switch (type) {
  case MSG_KEEPALIVE: {
    ret = 0;
    std::cout << "MSG_KEEPALIVE length: " << ret << std::endl;
    break;
  }
  case MSG_PIECE: {
    ret = 1 + 2 * sizeof(uint32_t) + PEER_REQUEST_SIZE;
    std::cout << "MSG_PIECE length: " << ret << std::endl;
    break;
  }
  case MSG_BITFIELD: {
    ret = 1 + BITFIELD_NUM_BYTES(torrent->pieces.size());
    std::cout << "MSG_BITFIELD length: " << ret << std::endl;
    break;
  }
  case MSG_REQUEST: {
    ret = 1 + 3 * sizeof(uint32_t);
    std::cout << "MSG_REQUEST length: " << ret << std::endl;
  }
  case MSG_HAVE:
  case MSG_PORT: {
    ret = 1 + sizeof(uint32_t);
    std::cout << "MSG_HAVE or MSG_PORT length: " << ret << std::endl;
    break;
  }
  default: {
    ret = 1;
    std::cout << "MSG_* length: " << ret << std::endl;
    break;
  }
  }

  return ret;
}

int peer_msg_send(int sockfd, const torrent_info2 *torrent, peer_msg2 *msg)
{
  // https://wiki.theory.org/index.php/BitTorrentSpecification#Messages
  // messages take the form of <length prefix><message ID><payload>
  //   length prefix: is a four byte big-endian value
  //   message ID   : is a single decimal byte
  //   payload      : is message dependent

  std::cout << "enter peer_msg_send" << std::endl;
  assert(msg);
  assert(torrent);

  uint32_t len = msg_length(msg->type, torrent);
  std::cout << "sending message (type: " << msg->type << "), len: "
            << len << std::endl;
  len = htonl(len);

  // send <length prefix>
  if (peer_send_buff(sockfd, (char *)&len, sizeof(uint32_t))) {
    std::cout << "peer_send_buff <length prefix> failed" << std::endl;
    return -1;
  }

  if (MSG_KEEPALIVE == msg->type) {
    return 0;
  }

  // send <message ID>
  if (peer_send_buff(sockfd, (const char *)&msg->type, 1)) {
    std::cout << "peer_send_buff <message ID> failed" << std::endl;
    return -1;
  }

  // send <payload>
  switch (msg->type) {
  case MSG_CHOKE:
  case MSG_UNCHOKE:
  case MSG_INTERESTED:
  case MSG_NOT_INTERESTED: {
    assert(1 == ntohl(len));
    return 0;
  }
  case MSG_PIECE: {
    piece_msg *pmsg = &msg->payload.piece;
    return peer_msg_send_piece(sockfd, torrent, pmsg);
  }
  case MSG_BITFIELD: {
    unsigned num_bytes = BITFIELD_NUM_BYTES(msg->payload.bitfield.size());
    std::cout << "num_bytes: " << num_bytes << std::endl;
    char *bitfield_payload = new char[num_bytes];
    memset(bitfield_payload, 0, num_bytes);

    for (unsigned i = 0; i < msg->payload.bitfield.size(); ++i) {
      if (msg->payload.bitfield[i]) {
        std::cout << "bit: " << i << " set" << std::endl;
        BITFIELD_SET(i, bitfield_payload);
      }
      else {
        std::cout << "bit: " << i << " clr" << std::endl;
        BITFIELD_CLR(i, bitfield_payload);
      }
    }

    if (peer_send_buff(sockfd, (const char *)bitfield_payload, num_bytes)) {
      std::cout << "peer_send_buff MSG_BITFIELD <payload> failed"
                << std::endl;
      delete[] bitfield_payload;
      return -1;
    }

    delete[] bitfield_payload;
    return 0;
  }
  case MSG_REQUEST: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload.request.index);
    if (peer_send_buff(sockfd, (const char *)&u32, sizeof(uint32_t))) {
      std::cout << "peer_send_buff MSG_REQUEST index failed" << std::endl;
      return -1;
    }

    u32 = htonl(msg->payload.request.begin);
    if (peer_send_buff(sockfd, (const char *)&u32, sizeof(uint32_t))) {
      std::cout << "peer_send_buff MSG_REQUEST begin failed" << std::endl;
      return -1;
    }

    u32 = htonl(msg->payload.request.length);
    if (peer_send_buff(sockfd, (const char *)&u32, sizeof(uint32_t))) {
      std::cout << "peer_send_buff MSG_REQUEST length failed" << std::endl;
      return -1;
    }

    return 0;
  }
  case MSG_HAVE: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload.have);
    if (peer_send_buff(sockfd, (const char *)&u32, sizeof(uint32_t))) {
      std::cout << "peer_send_buff MSG_HAVE failed" << std::endl;
      return -1;
    }

    return 0;
  }
  case MSG_PORT: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload.listen_port);
    if (peer_send_buff(sockfd, (const char *)&u32, sizeof(uint32_t))) {
      std::cout << "peer_send_buff MSG_PORT failed" << std::endl;
      return -1;
    }

    return 0;
  }
  case MSG_CANCEL: {
    assert(0);
    return -1;
  }
  default: {
    return -1;
  }
  }

  return 0;
}

inline bool valid_len(msg_type type, const torrent_info2 *torrent, uint32_t len)
{
  if (MSG_PIECE == type) {
    return
      (len >= (1 + 2 * sizeof(uint32_t) + 1)) &&
      (len <= (1 + 2 * sizeof(uint32_t) + PEER_REQUEST_SIZE));
  }

  return (len == msg_length(type, torrent));
}

int peer_msg_recv_pastlen(int sockfd, const torrent_info2 *torrent,
                          uint32_t len, peer_msg2 *out)
{
  std::cout << "enter peer_msg_recv_pastlen" << std::endl;
  std::cout << "receiving message of length: " << len << std::endl;

  if (0 == len) {
    out->type = MSG_KEEPALIVE;
    std::cout << "MSG_KEEPALIVE" << std::endl;
    return 0;
  }

  unsigned char type = 0;
  if (peer_recv_buff(sockfd, (char *)&type, 1)) {
    std::cout << "peer_recv_buff failed" << std::endl;
    return -1;
  }

  if (type >= MSG_MAX) {
    std::cout << "type greater than MSG_MAX" << std::endl;
    return -1;
  }

  if (!valid_len((msg_type)type, torrent, len)) {
    std::cout << "valid_len failed" << std::endl;
    return -1;
  }

  out->type = (msg_type)type;
  unsigned left = len - 1;

  switch (type) {
  case MSG_CHOKE:
  case MSG_UNCHOKE:
  case MSG_INTERESTED:
  case MSG_NOT_INTERESTED: {
    assert(left == 0);
    break;
  }
  case MSG_PIECE: {
    // TODO
    break;
  }
  case MSG_BITFIELD: {
    char *buff = new char[left];
    memset(buff, 0, left);

    if (peer_recv_buff(sockfd, buff, left)) {
      std::cout << "peer_recv_buff MSG_BITFIELD failed" << std::endl;
      delete[] buff;
      return -1;
    }

    out->payload.bitfield.resize(left * CHAR_BIT, false);

    for (unsigned i = 0; i < out->payload.bitfield.size(); ++i) {
      if (BITFIELD_ISSET(i, buff)) {
        out->payload.bitfield[i] = 1;
      }
      else {
        out->payload.bitfield[i] = 0;
      }
    }

    // bit 0 at the end
    std::cout << "peer_msg2.payload.bitfield: "
              << out->payload.bitfield << std::endl;

    delete[] buff;
    break;
  }
  case MSG_REQUEST: {
    char *buff = new char[left];
    memset(buff, 0, left);

    if (peer_recv_buff(sockfd, buff, left)) {
      std::cout << "peer_recv_buff MSG_REQUEST failed" << std::endl;
      delete[] buff;
      return -1;
    }

    assert(left == 3 * sizeof(uint32_t));
    uint32_t u32 = 0;

    memcpy(&u32, buff + 0 * sizeof(uint32_t), sizeof(uint32_t));
    out->payload.request.index = ntohl(u32);
    std::cout << "request.index: " << ntohl(u32) << std::endl;

    memcpy(&u32, buff + 1 * sizeof(uint32_t), sizeof(uint32_t));
    out->payload.request.begin = ntohl(u32);
    std::cout << "request.begin: " << ntohl(u32) << std::endl;

    memcpy(&u32, buff + 2 * sizeof(uint32_t), sizeof(uint32_t));
    out->payload.request.length = ntohl(u32);
    std::cout << "request.length: " << ntohl(u32) << std::endl;

    delete[] buff;
    break;
  }
  case MSG_HAVE: {
    assert(left == sizeof(uint32_t));
    uint32_t u32 = 0;

    if (peer_recv_buff(sockfd, (char *)&u32, left)) {
      std::cout << "peer_recv_buff MSG_HAVE failed" << std::endl;
      return -1;
    }

    out->payload.have = ntohl(u32);
    std::cout << "have: " << ntohl(u32) << std::endl;
    break;
  }
  case MSG_PORT: {
    assert(left == sizeof(uint32_t));
    uint32_t u32 = 0;

    if (peer_recv_buff(sockfd, (char *)&u32, left)) {
      std::cout << "peer_recv_buff MSG_PORT failed" << std::endl;
      return -1;
    }

    out->payload.listen_port = ntohl(u32);
    std::cout << "listen_port: " << ntohl(u32) << std::endl;
    break;
  }
  case MSG_CANCEL: {
    // TODO
    assert(0);
    break;
  }
  default:
    return -1;
  }

  std::cout << "successfully received message from peer, type: "
            << (unsigned)type << std::endl;
  return 0;
}

int peer_msg_recv(int sockfd, const torrent_info2 *torrent, peer_msg2 *out)
{
  std::cout << "enter peer_msg_recv" << std::endl;
  assert(torrent);

  uint32_t len = 0;

  if (peer_recv_buff(sockfd, (char *)&len, sizeof(uint32_t))) {
    std::cout << "peer_recv_buff failed" << std::endl;
    return -1;
  }

  len = ntohl(len);

  return peer_msg_recv_pastlen(sockfd, torrent, len, out);
}

bool peer_msg_buff_nonempty(int sockfd)
{
  std::cout << "enter peer_msg_buff_nonempty" << std::endl;

  uint32_t len = 0;
  int n = recv(sockfd, (char *)&len, sizeof(uint32_t), MSG_PEEK | MSG_DONTWAIT);
  if (n < 0) {
    perror("recv");
    return false;
  }

  if ((uint32_t)n < sizeof(uint32_t)) {
    return false;
  }

  len = ntohl(len);

  int bytes_avail = 0;
  if (ioctl(sockfd, FIONREAD, &bytes_avail)) {
    perror("ioctl");
    return false;
  }

  std::cout << "bytes_avail: " << bytes_avail
            << " , len: " << len << std::endl;

  if ((unsigned)bytes_avail >= len + sizeof(uint16_t)) {
    return true;
  }
  else {
    return false;
  }
}

int peer_msg_send_piece(int sockfd, const torrent_info2 *torrent,
                        const piece_msg *pmsg)
{
  std::cout << "--- enter peer_msg_send_piece [index: ]"
            << pmsg->index << "] ---" << std::endl;

  std::shared_ptr<piece_request> sp_piece_req =
    std::make_shared<piece_request>();

  if (piece_request_create(torrent, pmsg->index, sp_piece_req.get())) {
    std::cout << "piece_request_create failed" << std::endl;
    return -1;
  }

  return 0;
}
