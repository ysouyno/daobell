#include "bencode_peers_crawler.h"

bencode_peers_crawler::bencode_peers_crawler() :
  ti_(0),
  sp_bvb_(0),
  bvb_(0)
{
}

bencode_peers_crawler::~bencode_peers_crawler()
{
}

bencode_peers_crawler::bencode_peers_crawler(torrent_info *ti, std::shared_ptr<bencode_value_base> sp_bvb) :
  ti_(ti),
  sp_bvb_(sp_bvb),
  bvb_(0)
{
}

bencode_peers_crawler::bencode_peers_crawler(torrent_info *ti, bencode_value_base *bvb) :
  ti_(ti),
  sp_bvb_(0),
  bvb_(bvb)
{
}

void bencode_peers_crawler::parse()
{
  if (sp_bvb_ != NULL) {
    sp_bvb_->crawl(this);
  }
  else if (bvb_ != NULL) {
    bvb_->crawl(this);
  }
  else {
    std::cout << "peers crawl nothing" << std::endl;
  }
}

// reference from aria2 (https://github.com/aria2/aria2)
void bencode_peers_crawler::crawl(bencode_string *p)
{
  size_t uint = 6; // only for AF_INET
  size_t length = (p->get_value()).size();

  if (length % uint == 0) {
    const unsigned char *beg = reinterpret_cast<const unsigned char *>(p->get_value().data());
    const unsigned char *end = beg + length;

    for (; beg != end; beg += uint) {
      std::pair<std::string, uint16_t> peer = utils::unpack_compact(beg);
      if (peer.first.empty()) {
        continue;
      }

      ti_->peers_.push_back(peer);
    }
  }
}

void bencode_peers_crawler::crawl(bencode_integer *p)
{
}

void bencode_peers_crawler::crawl(bencode_list *p)
{
}

void bencode_peers_crawler::crawl(bencode_dict *p)
{
}
