#ifndef TRACKER_RESP_PARSER_H
#define TRACKER_RESP_PARSER_H

#include "tracker_announce.h"

std::shared_ptr<tracker_announce_resp>
tracker_resp_parse(const std::string &resp_str);

#endif /* TRACKER_RESP_PARSER_H */
