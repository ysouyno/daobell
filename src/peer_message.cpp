#include "peer_message.h"

_peer_msg::_peer_msg()
{
}

_peer_msg::~_peer_msg()
{
  free_byte_str(payload_.bitfield_);
}

byte_str *new_byte_str(const unsigned char *str, size_t size)
{
  byte_str *bs = NULL;

  bs = (byte_str *)malloc(sizeof(byte_str) + size + 1);
  if (bs) {
    memcpy(bs->str_, str, size);
    bs->str_[size] = '\0';
    bs->size_ = size;
  }

  return bs;
}

void free_byte_str(byte_str *str)
{
  free(str);
}

void print_byte_str(byte_str *str)
{
  assert(str != NULL);

  for (size_t i = 0; i < str->size_; ++i) {
    printf("%02x ", str->str_[i]);
  }

  printf("\n");
}

void print_byte_str(char *str, size_t size)
{
  assert(str != NULL);

  for (size_t i = 0; i < size; ++i) {
    printf("%02x ", str[i]);
  }

  printf("\n");
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

  std::string hex_info_hash;
  utils::hex_string_to_hex(ti->info_hash_, hex_info_hash);

  std::string hex_peer_id;
  utils::hex_string_to_hex(ti->peer_id_, hex_peer_id);

  size_t length =
    1 +
    pstrlen +
    sizeof(reserved) +
    hex_info_hash.size() +
    hex_peer_id.size();

  off_t off = 0;
  char buff[length] = {0};

  buff[0] = pstrlen;
  off++;

  memcpy(buff + off, pstr, pstrlen);
  off += pstrlen;

  memcpy(buff + off, reserved, sizeof(reserved));
  off += sizeof(reserved);

  memcpy(buff + off, hex_info_hash.c_str(), hex_info_hash.size());
  off += hex_info_hash.size();

  memcpy(buff + off, hex_peer_id.c_str(), hex_peer_id.size());

  return send_buff(sockfd, buff, length);
}

int recv_buff(int sockfd, char *buff, size_t length)
{
  size_t total_recv = 0;
  int rt = 0;

  do
    {
      rt = recv(sockfd, buff + total_recv, length - total_recv, 0);
      if (rt < 0) {
        log_e("recv() error %s\n", strerror(errno));
        close(sockfd);
        return -1;
      }

      total_recv += rt;
    } while (rt > 0 && total_recv < length);

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

uint32_t get_msg_len(peer_msg *msg)
{
  uint32_t msg_len = 0;

  switch (msg->type_) {
  case PEER_KEEP_ALIVE: {
    msg_len = 0;
    break;
  }
  case PEER_PIECE: {
    msg_len =
      1 +
      2 * sizeof(uint32_t) +
      msg->payload_.piece_.block_length_;
    break;
  }
  case PEER_BITFIELD: {
    msg_len =
      1 +
      msg->payload_.bitfield_->size_;
    break;
  }
  case PEER_REQUEST: {
    msg_len =
      1 +
      3 * sizeof(uint32_t);
    break;
  }
  case PEER_HAVE:
  case PEER_PORT: {
    msg_len =
      1 +
      sizeof(uint32_t);
    break;
  }
  default:
    msg_len = 1;
    break;
  }

  return msg_len;
}

int peer_send_msg(int sockfd, peer_msg *msg, torrent_info *ti)
{
  uint32_t msg_send_len = get_msg_len(msg);
  msg_send_len = htonl(msg_send_len);

  log_t("message type: %d, msg_send_len: %d\n", msg->type_, msg_send_len);

  if (send_buff(sockfd, (char *)&msg_send_len, sizeof(uint32_t))) {
    return -1;
  }

  if (PEER_KEEP_ALIVE == msg->type_) {
    return 0;
  }

  char msg_type = msg->type_;
  if (send_buff(sockfd, &msg_type, 1)) {
    return -1;
  }

  switch (msg->type_) {
  case PEER_CHOKE:
  case PEER_UNCHOKE:
  case PEER_INTERESTED:
  case PEER_NOT_INTERESTED:
  case PEER_CANCEL: {
    assert(1 == msg_send_len);
    return 0;
  }
  case PEER_PIECE: {
    //
  }
  case PEER_BITFIELD: {
    assert(msg->payload_.bitfield_->str_);

    if (send_buff(sockfd, (const char *)(msg->payload_.bitfield_->str_), msg->payload_.bitfield_->size_)) {
      return -1;
    }

    return 0;
  }
  case PEER_REQUEST: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload_.request_.index_);
    if (send_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }

    u32 = htonl(msg->payload_.request_.begin_);
    if (send_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }

    u32 = htonl(msg->payload_.request_.length_);
    if (send_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }

    return 0;
  }
  case PEER_HAVE: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload_.have_);
    if (send_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }

    return 0;
  }
  case PEER_PORT: {
    uint32_t u32 = 0;

    u32 = htonl(msg->payload_.listen_port_);
    if (send_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }

    return 0;
  }
  default:
    return -1;
  }
}

int peer_msg_recv(int sockfd, peer_msg *out_msg, torrent_info *ti)
{
  uint32_t msg_recv_len = 0;
  if (recv_buff(sockfd, (char *)&msg_recv_len, sizeof(uint32_t))) {
    return -1;
  }

  msg_recv_len = ntohl(msg_recv_len);

  if (0 == msg_recv_len) {
    out_msg->type_ = PEER_KEEP_ALIVE;
    return 0;
  }

  char msg_type = 0;
  if (recv_buff(sockfd, &msg_type, 1)) {
    return -1;
  }

  if (msg_type > PEER_PORT) {
    return -1;
  }

  out_msg->type_ = (peer_msg_type)msg_type;

  log_w("[conversion] char to peer_msg_type result: %d\n", out_msg->type_);

  unsigned char left = msg_recv_len - 1;

  // when we get a piece, write it to file directly
  if (PEER_PIECE == msg_type) {
    /*
    assert(left > 0);
    uint32_t u32 = 0;

    if (recv_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }
    out_msg->payload_.piece_.index_ = ntohl(u32);
    left -= sizeof(uint32_t);

    if (recv_buff(sockfd, (char *)&u32, sizeof(uint32_t))) {
      return -1;
    }
    out_msg->payload_.piece_.begin_ = ntohl(u32);
    left -= sizeof(uint32_t);
    */
  }
  else if (left > 0) {
    char buff[left] = {0};

    if (recv_buff(sockfd, buff, left)) {
      return -1;
    }

    print_byte_str(buff, left);

    switch (msg_type) {
    case PEER_BITFIELD: {
      out_msg->payload_.bitfield_ = new_byte_str((const unsigned char *)"", left);
      if (out_msg->payload_.bitfield_) {
        log_e("new_byte_str malloc error\n");
        return -1;
      }

      memcpy(out_msg->payload_.bitfield_->str_, buff, left);
      break;
    }
    case PEER_REQUEST: {
      assert(sizeof(buff) == 4 * sizeof(uint32_t));
      uint32_t u32 = 0;

      memcpy(&u32, buff + 0 * sizeof(uint32_t), sizeof(uint32_t));
      out_msg->payload_.request_.index_ = ntohl(u32);

      memcpy(&u32, buff + 1 * sizeof(uint32_t), sizeof(uint32_t));
      out_msg->payload_.request_.begin_ = ntohl(u32);

      memcpy(&u32, buff + 2 * sizeof(uint32_t), sizeof(uint32_t));
      out_msg->payload_.request_.length_ = ntohl(u32);
      break;
    }
    case PEER_HAVE: {
      assert(sizeof(buff) == sizeof(uint32_t));
      uint32_t u32 = 0;

      memcpy(&u32, buff, sizeof(uint32_t));
      out_msg->payload_.have_ = ntohl(u32);
      break;
    }
    case PEER_PORT: {
      assert(sizeof(buff) == sizeof(uint32_t));
      uint32_t u32 = 0;

      memcpy(&u32, buff, sizeof(uint32_t));
      out_msg->payload_.listen_port_ = ntohl(u32);
      break;
    }
    default:
      return -1;
    }
  }

  log_t("receive message from peer successfully, type: %d\n", msg_type);

  return 0;
}
