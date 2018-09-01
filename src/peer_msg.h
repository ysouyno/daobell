#ifndef PEER_MSG_H
#define PEER_MSG_H

#define HANDSHAKE_LEN 128

int peer_send_handshake(int sockfd, char info_hash[20]);
int peer_recv_handshake(int sockfd, char out_info_hash[20],
                        char out_peer_id[20], bool peer_id);

#endif /* PEER_MSG_H */
