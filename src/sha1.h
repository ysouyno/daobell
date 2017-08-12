#ifndef SHA1_H
#define SHA1_H

// from the BitFiend (https://github.com/eduard-permyakov/BitFiend.git)

#include <stdint.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#define DIGEST_LEN 20

int sha1_compute(const char *msg, size_t len, char out_digest[DIGEST_LEN]);

#endif /* SHA1_H */
