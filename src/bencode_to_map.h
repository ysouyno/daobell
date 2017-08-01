#ifndef BENCODE_TO_MAP_H
#define BENCODE_TO_MAP_H

class bencode_to_map : public bencode_crawler
{
public:
  bencode_to_map(std::shared_ptr<bencode_value_base> sp_bencode_parser) :
    sp_bencode_parser_(sp_bencode_parser)
  {
  }

  virtual void crawl(bencode_string *p)
  {
    crawl_str_ = p->get_value();
  }

  virtual void crawl(bencode_integer *p)
  {
    crawl_str_ = std::to_string(p->get_value());
  }

  virtual void crawl(bencode_list *p)
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

  virtual void crawl(bencode_dictionary *p)
  {
    std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> > value = p->get_value();

    for (std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> >::iterator it = value.begin();
         it != value.end(); ++it) {
      it->first.get()->crawl(this);
      std::string key = crawl_str_;
      std::string value = "";

      bencode_dictionary *bd = dynamic_cast<bencode_dictionary *>(it->second.get());
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

  const std::string &get_list_name() const
  {
    return list_name_;
  }

  void print_all()
  {
    crawl_all();
    std::cout << crawl_str_ << std::endl;
  }

  void crawl_all()
  {
    sp_bencode_parser_->crawl(this);
  }

  void print_multimap()
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

private:
  std::string crawl_str_;
  std::shared_ptr<bencode_value_base> sp_bencode_parser_;
  std::multimap<std::string, std::string> multimap_dictionary_;
  std::string list_name_;
};

#endif /* BENCODE_TO_MAP_H */
