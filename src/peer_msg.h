#ifndef PEER_MSG_H
#define PEER_MSG_H

#include "torrent_info.h"
#include <stdint.h>
#include <stddef.h>
#include <boost/dynamic_bitset.hpp>

#define HANDSHAKE_LEN 128

#define KB (1 << 10)
#define PEER_REQUEST_SIZE (16 * KB)

enum msg_type {
  MSG_KEEPALIVE = -1,
  MSG_CHOKE,
  MSG_UNCHOKE,
  MSG_INTERESTED,
  MSG_NOT_INTERESTED,
  MSG_HAVE,
  MSG_BITFIELD,
  MSG_REQUEST,
  MSG_PIECE,
  MSG_CANCEL,
  MSG_PORT,
  MSG_MAX
};

struct request_msg
{
  uint32_t index;  // specifying the zero-based piece index
  uint32_t begin;  // specifying the zero-based byte offset within the piece
  uint32_t length; // integer specifying the requested length
};

struct piece_msg
{
  uint32_t index;
  uint32_t begin;
  size_t block_len;
};

// Using std::vector or boost::dynamic_bitset in union will cause compilation
// errors, the compiler can't find which constructor to call, because it does
// not know what type is in the union. All function calls must be known at
// compiletime or virtual, and a union is neither polymorphic nor compile-time
// constant. So the compiler can never know what to do. But using std::string
// and std::vector in union is fine, you can find example here.
// https://en.cppreference.com/w/cpp/language/union
struct peer_msg
{
  msg_type type;
  struct {
    uint32_t have;
    boost::dynamic_bitset<> bitfield;
    request_msg request;
    piece_msg piece;
    unsigned listen_port;
  } payload;
};

int peer_send_handshake(int sockfd, char info_hash[20]);
int peer_recv_handshake(int sockfd, char out_info_hash[20],
                        char out_peer_id[20], bool peer_id);
int peer_msg_send(int sockfd, const torrent_info *torrent, peer_msg *msg);
int peer_msg_recv(int sockfd, const torrent_info *torrent, peer_msg *out);
bool peer_msg_buff_nonempty(int sockfd);
int peer_msg_send_piece(int sockfd, const torrent_info *torrent,
                        const piece_msg *pmsg);
int peer_msg_recv_piece(int sockfd, const torrent_info *torrent,
                        uint32_t len, peer_msg *out);
std::string query_msg_type_text(int type);

#endif /* PEER_MSG_H */
