#ifndef PEER_MSG_H
#define PEER_MSG_H

#include <stdint.h>

#define HANDSHAKE_LEN 128

struct request_msg
{
  uint32_t index;  // specifying the zero-based piece index
  uint32_t begin;  // specifying the zero-based byte offset within the piece
  uint32_t length; // integer specifying the requested length
};

int peer_send_handshake(int sockfd, char info_hash[20]);
int peer_recv_handshake(int sockfd, char out_info_hash[20],
                        char out_peer_id[20], bool peer_id);

#endif /* PEER_MSG_H */
