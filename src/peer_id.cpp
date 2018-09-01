#include "peer_id.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char g_local_peer_id[20];

void create_local_peer_id(char outbuff[20])
{
  int offset = 0;
  const char *id = "ys";

  memset(outbuff, 0, 20);
  offset += snprintf(outbuff, 20, "-%.*s%02u%02u-", 2, id, 1, 0);

  for (unsigned i = 0; i < 12 / (sizeof(int32_t)); i++) {
    int32_t r = rand();
    memcpy(outbuff + offset, &r, sizeof(r));
    offset += sizeof(r);
  }
}
