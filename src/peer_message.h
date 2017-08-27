#ifndef PEER_MESSAGE_H
#define PEER_MESSAGE_H

#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include "torrent_info.h"
#include "utils.h"
#include "log_wrapper.h"

enum peer_msg_type{
  PEER_KEEP_ALIVE,
  PEER_CHOKE,
  PEER_UNCHOKE,
  PEER_INTERESTED,
  PEER_NOT_INTERESTED,
  PEER_HAVE,
  PEER_BITFIELD,
  PEER_REQUEST,
  PEER_PIECE,
  PEER_CANCEL,
  PEER_PORT
};

typedef struct _peer_request_msg
{
  uint32_t index_;
  uint32_t begin_;
  uint32_t length_;
} peer_request_msg, *ppeer_request_msg;

typedef struct _peer_piece_msg
{
  uint32_t index_;
  uint32_t begin_;
  size_t block_length_;
} peer_piece_msg, *ppeer_piece_msg;

// compiler doesn't allow std::string inside union, so here it is
typedef struct _byte_str
{
  size_t size_;
  unsigned char str_[];
} byte_str, *pbyte_str;

typedef struct _peer_msg
{
  _peer_msg();
  ~_peer_msg();

  peer_msg_type type_;
  union {
    uint32_t have_;
    pbyte_str bitfield_;
    peer_request_msg request_;
    peer_piece_msg piece_;
    uint16_t listen_port_;
  } payload_;
} peer_msg, *ppeer_msg;

byte_str *new_byte_str(size_t size, const unsigned char *str);

void free_byte_str(byte_str *str);

void print_byte_str(byte_str *str);

void print_byte_str(char *str, size_t size);

int send_buff(int sockfd, const char *buff, size_t length);

int send_handshake(int sockfd, torrent_info *ti);

int recv_buff(int sockfd, char *buff, size_t length);

int recv_handshake(int sockfd, char out_info_hash[20], char out_peer_id[20], bool peer_id);

int peer_send_msg(int sockfd, peer_msg *msg, torrent_info *ti);

int peer_msg_recv(int sockfd, peer_msg *out_msg, torrent_info *ti);

#endif /* PEER_MESSAGE_H */
