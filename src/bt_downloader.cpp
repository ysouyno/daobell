#include "bt_downloader.h"
#include "bencode_parser.h"
#include "bencode_reader.h"
#include "tracker_announce.h"
#include "peer_info.h"
#include "dnld_file.h"
#include "torrent_info2.h"
#include "peer_info.h"
#include "peer_connection.h"
#include "peer_id.h"
#include <string.h>

#define TRACKER_RETRY_INTERVAL 15

static const uint16_t g_port = 6889;

struct tracker_arg
{
  struct torrent_info2 *torrent;
  uint16_t port;
};

std::shared_ptr<tracker_announce_req> create_tracker_request(const void *arg)
{
  const tracker_arg *targ = (tracker_arg *)arg;
  std::shared_ptr<tracker_announce_req> ret =
    std::make_shared<tracker_announce_req>();
  if (ret) {
    ret->has = 0;
    memcpy(ret->info_hash, targ->torrent->info_hash, sizeof(ret->info_hash));
    memcpy(ret->peer_id, g_local_peer_id, sizeof(ret->peer_id));
    ret->port = targ->port;
    ret->compact = true;
    SET_HAS(ret, REQUEST_HAS_COMPACT);

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    unsigned num_conns = targ->torrent->sh.peer_connections.size();
    pthread_mutex_unlock(&targ->torrent->sh_mutex);

    ret->numwant = targ->torrent->max_peers - num_conns;
    SET_HAS(ret, REQUEST_HAS_NUMWANT);

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    ret->uploaded = targ->torrent->sh.uploaded;
    ret->downloaded = targ->torrent->sh.downloaded;
    ret->left = 0; // TODO
    pthread_mutex_unlock(&targ->torrent->sh_mutex);
  }

  return ret;
}

int create_peer_connection(peer_info *peer, torrent_info2 *torrent)
{
  assert(peer);
  assert(torrent);

  std::shared_ptr<peer_conn> conn = std::make_shared<peer_conn>();
  std::shared_ptr<peer_arg> arg = std::make_shared<peer_arg>();

  arg->torrent = torrent;
  arg->has_torrent = true;
  arg->has_sockfd = false;
  arg->peer = *peer;

  if (peer_connection_create(&conn->peer_tid, arg.get())) {
    std::cout << "peer_connection_create failed" << std::endl;
    return -1;
  }

  return 0;
}

static void *periodic_announce(void *arg)
{
  const tracker_arg *targ = (tracker_arg *)arg;
  bool completed = false;
  unsigned interval = 0;

  pthread_mutex_lock(&targ->torrent->sh_mutex);
  completed = targ->torrent->sh.completed;
  pthread_mutex_unlock(&targ->torrent->sh_mutex);

  bool started = false;
  while (true) {
    std::shared_ptr<tracker_announce_req> req = create_tracker_request(targ);

    if (!started) {
      req->event = TORRENT_EVENT_STARTED;
      SET_HAS(req, REQUEST_HAS_EVENT);
      started = true;
    }

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    bool current_completed = targ->torrent->sh.completed;
    pthread_mutex_unlock(&targ->torrent->sh_mutex);

    if (false == completed && true == current_completed) {
      req->event = TORRENT_EVENT_COMPLETED;
      SET_HAS(req, REQUEST_HAS_EVENT);
    }

    completed = current_completed;

    std::shared_ptr<tracker_announce_resp> resp =
      tracker_announce(targ->torrent->announce, req.get());

    if (resp) {
      print_tracker_announce_resp(resp.get());
      interval = resp->interval;
      std::cout << "re-announcing to tracker again in " << interval
                << " seconds" << std::endl;

      for (std::list<peer_info>::iterator it = resp->peers.begin();
           it != resp->peers.end(); ++it) {
        create_peer_connection(&(*it), targ->torrent);
      }
    }
    else {
      interval = TRACKER_RETRY_INTERVAL;
      std::cout << "retrying announcing to tracker in " << interval
                << " seconds" << std::endl;
    }

    sleep(interval);
  }

  return NULL;
}

int tracker_connection_create(pthread_t *tid, tracker_arg *arg)
{
  if (pthread_create(tid, NULL, periodic_announce, (void *)arg)) {
    return -1;
  }

  return 0;
}

void bt_download(const std::string &metafile, const std::string &destdir)
{
  create_local_peer_id(g_local_peer_id);

  std::ifstream file(metafile, std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_reader br(sp_bvb);

  torrent_info2 *torrent = torrent_init(sp_bvb, destdir);

  std::shared_ptr<tracker_arg> arg = std::make_shared<tracker_arg>();
  arg->torrent = torrent;
  arg->port = g_port;
  if (tracker_connection_create(&torrent->tracker_tid, arg.get())) {
    std::cout << "tracker_connection_create failed" << std::endl;
  }

  pthread_join(torrent->tracker_tid, NULL);

  delete torrent;
}
