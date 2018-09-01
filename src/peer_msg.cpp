#include "peer_msg.h"
#include "peer_id.h"
#include <iostream>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <memory>

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
