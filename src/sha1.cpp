#include "sha1.h"

#define CIRCULAR_SHIFT_32(X, n) (htobe32((be32toh(X) << (n)) | (be32toh(X) >> (32 - (n)))))
#define ADD_32(a, b) (htobe32(be32toh(a) + be32toh(b)))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static uint32_t f(int t, uint32_t b, uint32_t c, uint32_t d)
{
  if (0 <= t && t <= 19) {
    return ((b & c) | ((~b) & d));
  }
  else if (20 <= t && t <= 39) {
    return (b ^ c ^ d);
  }
  else if (40 <= t && t <= 59) {
    return ((b & c) | (b & d) | (c & d));
  }
  else if (60 <= t && t <= 79) {
    return (b ^ c ^ d);
  }

  // warning: control reaches end of non-void function [-Wreturn-type]
  return 0;
}

static uint32_t K(int t)
{
  if (0 <= t && t <= 19) {
    return htobe32(0x5A827999);
  }
  else if (20 <= t && t <= 39) {
    return htobe32(0x6ED9EBA1);
  }
  else if (40 <= t && t <= 59) {
    return htobe32(0x8F1BBCDC);
  }
  else if (60 <= t && t <= 79) {
    return htobe32(0xCA62C1D6);
  }

  // warning: control reaches end of non-void function [-Wreturn-type]
  return 0;
}

static void pad_msg_block(char *out, size_t to_pad, size_t len)
{
  size_t footer_len = 1 + sizeof(uint64_t);

  if (footer_len >= to_pad) {
    memset(out + 64, 0, to_pad);
  }
  else {
    unsigned to_zero = to_pad - footer_len;
    memset(out + 64 - to_pad, 0x80, 1);
    memset(out + 64 - to_pad + 1, 0, to_zero);

    uint64_t len64 = htobe64(len * 8);
    memcpy(out + 64 - to_pad + to_zero + 1, &len64, sizeof(uint64_t));
  }
}

static unsigned num_msg_blocks(size_t len)
{
  return (len / 64) + (64 - (len % 64) >= 1 + sizeof(uint64_t) ? 1 : 2);
}

static int block_at_index(const char *msg, size_t len, int i, char out_block[64])
{
  unsigned left = MAX(len - 64 * i, 0);
  const char *next = msg + i * 64;

  if (left < 64) {
    memcpy(out_block, next, left);
    pad_msg_block(out_block, 64 - left, len);
  }
  else {
    memcpy(out_block, next, 64);
  }

  return 0;
}

int sha1_compute(const char *msg, size_t len, char out_digest[DIGEST_LEN])
{
  uint32_t A, B, C, D, E;

  union {
    unsigned char bytes[DIGEST_LEN];
    uint32_t H[5];
  } digest;

  uint32_t W[80];

  digest.H[0] = htobe32(0x67452301);
  digest.H[1] = htobe32(0xEFCDAB89);
  digest.H[2] = htobe32(0x98BADCFE);
  digest.H[3] = htobe32(0x10325476);
  digest.H[4] = htobe32(0xC3D2E1F0);

  uint32_t msg_block[16];

  for (unsigned i = 0; i < num_msg_blocks(len); i++) {
    block_at_index(msg, len, i, (char*)msg_block);

    for (int t = 0; t <= 15; t++) {
      W[t] = msg_block[t];
    }

    for (int t = 16; t <= 79; t++) {
      W[t] = CIRCULAR_SHIFT_32(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
    }

    A = digest.H[0];
    B = digest.H[1];
    C = digest.H[2];
    D = digest.H[3];
    E = digest.H[4];

    for (int t = 0; t <= 79; t++) {
      uint32_t temp =
        ADD_32(ADD_32(ADD_32(ADD_32(CIRCULAR_SHIFT_32(A, 5), f(t, B, C, D)), E), W[t]), K(t));

      E = D;
      D = C;
      C = CIRCULAR_SHIFT_32(B, 30);
      B = A;
      A = temp;
    }

    digest.H[0] = ADD_32(digest.H[0], A);
    digest.H[1] = ADD_32(digest.H[1], B);
    digest.H[2] = ADD_32(digest.H[2], C);
    digest.H[3] = ADD_32(digest.H[3], D);
    digest.H[4] = ADD_32(digest.H[4], E);
  }

  memcpy(out_digest, digest.bytes, DIGEST_LEN);

  return 0;
}
