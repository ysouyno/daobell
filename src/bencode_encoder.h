#ifndef BENCODE_ENCODER_H
#define BENCODE_ENCODER_H

#include <iostream>
#include <string>
#include <memory>
#include "bencode_crawler.h"
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_int.h"
#include "bencode_list.h"
#include "bencode_dict.h"

class bencode_encoder : public bencode_crawler
{
public:
  bencode_encoder();
  ~bencode_encoder();

  explicit bencode_encoder(std::shared_ptr<bencode_value_base> sp_bvb);
  explicit bencode_encoder(bencode_value_base *bvb);

  void encode();
  void print_result();
  const std::string &get_value();
  unsigned long get_length();

private:
  virtual void crawl(bencode_string *p);
  virtual void crawl(bencode_int *p);
  virtual void crawl(bencode_list *p);
  virtual void crawl(bencode_dict *p);

private:
  std::string encoded_str_;
  std::shared_ptr<bencode_value_base> sp_bvb_;
  bencode_value_base *bvb_;
};

#endif /* BENCODE_ENCODER_H */
