#ifndef TORRENT_HELPER_H
#define TORRENT_HELPER_H

#include <assert.h>
#include "torrent_info.h"
#include "bencode_string.h"
#include "bencode_int.h"
#include "bencode_list.h"
#include "bencode_dict.h"
#include "bencode_encoder.h"
#include "sha1.h"
#include "bencode_peers_crawler.h"

void get_announce(torrent_info *ti, bencode_dict *root);

long long get_piece_length(torrent_info *ti, bencode_dict *root);

long long get_creation_date(torrent_info *ti, bencode_dict *root);

void get_info_hash(torrent_info *ti, bencode_dict *root);

void get_files_and_size(torrent_info *ti, bencode_dict *root);

void get_peers(torrent_info *ti, bencode_dict *root);

void get_peer_id(torrent_info *ti);

#endif /* TORRENT_HELPER_H */
