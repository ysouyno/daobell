#include "tracker_resp_parser.h"
#include "bencode_parser.h"
#include "bencode_value_safe_cast.h"
#include "peer_info.h"
#include <iostream>
#include <string.h>
#include <list>
#include <assert.h>

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

int parse_peerlist_string(const bencode_string *value,
                          std::list<peer_info> &peers)
{
  std::cout << "enter parse_peerlist_string" << std::endl;

  std::string peers_str = value->get_value();
  assert(peers_str.size() % 6 == 0);

  for (size_t i = 0; i < peers_str.size(); i += 6) {
    uint32_t ip = 0;
    memcpy(&ip,
           peers_str.substr(i, sizeof(uint32_t)).c_str(),
           sizeof(uint32_t)
           );
    std::cout << "ip: " << ip << std::endl;

    uint16_t port = 0;
    memcpy(&port,
           peers_str.substr(i + sizeof(uint32_t), sizeof(uint16_t)).c_str(),
           sizeof(uint16_t)
           );
    std::cout << "port: " << port << std::endl;

    peer_info peer;
    peer.addr.sas.ss_family = AF_INET;
    peer.addr.sa_in.sin_addr.s_addr = ip;
    peer.addr.sa_in.sin_port = port;
    memset(peer.addr.sa_in.sin_zero, 0, sizeof(peer.addr.sa_in.sin_zero));
    memset(peer.peer_id, 0, sizeof(peer.peer_id));

    peers.push_back(peer);
  }

  return 0;
}

std::shared_ptr<tracker_announce_resp>
tracker_resp_parse(const std::string &resp_str)
{
  bencode_parser bp(resp_str.c_str());
  bencode_value_ptr meta = bp.get_value();
  assert(meta);

  bencode_dict *dict = down_cast<bencode_dict>(meta.get());
  assert(dict);

  std::shared_ptr<tracker_announce_resp> ret =
    std::make_shared<tracker_announce_resp>();

  dict_map dictmap = dict->get_value();

  for (dict_map::iterator it = dictmap.begin(); it != dictmap.end(); ++it) {
    if (it->first == "failure reason") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        ret->failure_reason = value->get_value();
        SET_HAS(ret, RESPONSE_HAS_FAILURE_REASON);
        std::cout << "failure reason: " << ret->failure_reason << std::endl;
      }
    }

    if (it->first == "warning message") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        ret->warning_message = value->get_value();
        SET_HAS(ret, RESPONSE_HAS_WARNING_MESSAGE);
        std::cout << "warning message: " << ret->warning_message << std::endl;
      }
    }

    if (it->first == "interval") {
      bencode_int *value = down_cast<bencode_int>(it->second.get());
      if (value) {
        ret->interval = value->get_value();
        std::cout << "interval: " << ret->interval << std::endl;
      }
    }

    if (it->first == "min interval") {
      bencode_int *value = down_cast<bencode_int>(it->second.get());
      if (value) {
        ret->min_interval = value->get_value();
        std::cout << "min interval: " << ret->min_interval << std::endl;
        SET_HAS(ret, RESPONSE_HAS_MIN_INTERVAL);
      }
    }

    if (it->first == "tracker id") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        ret->tracker_id = value->get_value();
        SET_HAS(ret, RESPONSE_HAS_TRACKER_ID);
        std::cout << "tracker id" << ret->tracker_id << std::endl;
      }
    }

    if (it->first == "complete") {
      bencode_int *value = down_cast<bencode_int>(it->second.get());
      if (value) {
        ret->complete = value->get_value();
        std::cout << "complete: " << ret->complete << std::endl;
      }
    }

    if (it->first == "incomplete") {
      bencode_int *value = down_cast<bencode_int>(it->second.get());
      if (value) {
        ret->incomplete = value->get_value();
        std::cout << "incomplete: " << ret->incomplete << std::endl;
      }
    }

    if (it->first == "peers") {
      std::cout << "find peers" << std::endl;
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        std::cout << "binary model" << std::endl;
        parse_peerlist_string(value, ret->peers);
        std::cout << "ret->peers's size(): " << ret->peers.size() << std::endl;
      }
      else {
        // TODO
        std::cout << "dictionary model" << std::endl;
      }
    }
  }

  return ret;
}
