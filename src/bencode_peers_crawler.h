#ifndef BENCODE_PEERS_CRAWLER_H
#define BENCODE_PEERS_CRAWLER_H

#include <string>
#include <vector>
#include <memory>
#include "bencode_crawler.h"
#include "torrent_info.h"
#include "bencode_string.h"
#include "utils.h"

class bencode_peers_crawler : public bencode_crawler
{
public:
  bencode_peers_crawler();
  ~bencode_peers_crawler();

  bencode_peers_crawler(torrent_info *ti, std::shared_ptr<bencode_value_base> sp_bvb);
  bencode_peers_crawler(torrent_info *ti, bencode_value_base *bvb);

  void parse();

private:
  virtual void crawl(bencode_string *p);
  virtual void crawl(bencode_integer *p);
  virtual void crawl(bencode_list *p);
  virtual void crawl(bencode_dictionary *p);

private:
  torrent_info *ti_;
  std::shared_ptr<bencode_value_base> sp_bvb_;
  bencode_value_base *bvb_;
};

#endif /* BENCODE_PEERS_CRAWLER_H */
