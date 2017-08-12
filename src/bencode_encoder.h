#ifndef BENCODE_ENCODER_H
#define BENCODE_ENCODER_H

#include <iostream>
#include <string>
#include <memory>
#include "bencode_crawler.h"
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dictionary.h"

class bencode_encoder : public bencode_crawler
{
public:
  bencode_encoder();
  ~bencode_encoder();

  explicit bencode_encoder(std::shared_ptr<bencode_value_base> sp_bm);

  virtual void crawl(bencode_string *p);
  virtual void crawl(bencode_integer *p);
  virtual void crawl(bencode_list *p);
  virtual void crawl(bencode_dictionary *p);

  void crawl_all();
  void print_all();

private:
  std::string encoded_str_;
  std::shared_ptr<bencode_value_base> sp_bm_;
};

#endif /* BENCODE_ENCODER_H */
