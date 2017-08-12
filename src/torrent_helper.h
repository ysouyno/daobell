#ifndef TORRENT_HELPER_H
#define TORRENT_HELPER_H

#include <assert.h>
#include "torrent_info.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dictionary.h"

void get_announce(torrent_info *ti, bencode_dictionary *root);

long long get_piece_length(torrent_info *ti, bencode_dictionary *root);

long long get_creation_date(torrent_info *ti, bencode_dictionary *root);

#endif /* TORRENT_HELPER_H */