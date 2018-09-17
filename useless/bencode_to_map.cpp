#include "bencode_to_map.h"

bencode_to_map::bencode_to_map(std::shared_ptr<bencode_value_base> sp_bencode_parser) :
  sp_bencode_parser_(sp_bencode_parser)
{
}

void bencode_to_map::crawl(bencode_string *p)
{
  crawl_str_ = p->get_value();
}

void bencode_to_map::crawl(bencode_int *p)
{
  crawl_str_ = std::to_string(p->get_value());
}

void bencode_to_map::crawl(bencode_list *p)
{
  std::vector<std::shared_ptr<bencode_value_base> > value = p->get_value();

  for (std::vector<std::shared_ptr<bencode_value_base> >::iterator it = value.begin();
       it != value.end(); ++it) {
    std::string key = get_list_name();
    (*it)->crawl(this);
    // comment: is there a dictionary in a list?
    multimap_dictionary_.insert(std::make_pair(key, crawl_str_));
  }
}

void bencode_to_map::crawl(bencode_dict *p)
{
  std::multimap<std::string, std::shared_ptr<bencode_value_base> > value = p->get_value();

  for (std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = value.begin();
       it != value.end(); ++it) {
    std::string key = it->first;
    std::string value = "";

    bencode_dict *bd = dynamic_cast<bencode_dict *>(it->second.get());
    bencode_list *bl = dynamic_cast<bencode_list *>(it->second.get());

    if (NULL == bd && NULL == bl) {
      // neither dictionary nor list
      // string or integer
      it->second.get()->crawl(this);
      value = crawl_str_;
      multimap_dictionary_.insert(std::make_pair(key, value));
    }
    else if (NULL != bd && NULL == bl) {
      // dictionary
      it->second.get()->crawl(this);
      multimap_dictionary_.insert(std::make_pair(key, ""));
    }
    else if (NULL == bd && NULL != bl) {
      // list
      list_name_ = crawl_str_;
      it->second.get()->crawl(this);
    }
  }
}

const std::string &bencode_to_map::get_list_name() const
{
  return list_name_;
}

void bencode_to_map::print_all()
{
  crawl_all();
  std::cout << crawl_str_ << std::endl;
}

void bencode_to_map::crawl_all()
{
  sp_bencode_parser_->crawl(this);
}

void bencode_to_map::print_multimap()
{
  crawl_all();

  for (std::multimap<std::string, std::string>::iterator it = multimap_dictionary_.begin();
       it != multimap_dictionary_.end(); ++it) {
    if (0 != strcmp(it->first.c_str(), "pieces")) {
      std::cout << it->first << " : " << it->second << std::endl;
    }
    else {
      std::cout << it->first << " : (" << it->second.size() << ")" << std::endl;
    }
  }
}
