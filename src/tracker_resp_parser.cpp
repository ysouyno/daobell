#include "tracker_resp_parser.h"
#include "bencode_parser.h"
#include "bencode_value_safe_cast.h"
#include <iostream>
#include <assert.h>

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

std::shared_ptr<tracker_announce_resp>
tracker_resp_parse(const std::string &resp_str)
{
  std::cout << "enter tracker_resp_parse" << std::endl;

  bencode_parser bp(resp_str.c_str());
  bencode_value_ptr meta = bp.get_value();
  assert(meta);

  bencode_dictionary *dict = down_cast<bencode_dictionary>(meta.get());
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
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        ret->interval = value->get_value();
        std::cout << "interval: " << ret->interval << std::endl;
      }
    }

    if (it->first == "min interval") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        ret->min_interval = value->get_value();
        std::cout << "min interval: " << ret->min_interval << std::endl;
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
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        ret->complete = value->get_value();
        std::cout << "complete: " << ret->complete << std::endl;
      }
    }

    if (it->first == "incomplete") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        ret->incomplete = value->get_value();
        std::cout << "incomplete: " << ret->incomplete << std::endl;
      }
    }

    if (it->first == "peers") {
      std::cout << "find peers" << std::endl;
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        std::cout << "dictionary model" << std::endl;
      }
      else {
        std::cout << "binary model" << std::endl;
      }
    }
  }

  return NULL;
}
