#include "bencode_encoder.h"

bencode_encoder::bencode_encoder()
{
}

bencode_encoder::~bencode_encoder()
{
}

bencode_encoder::bencode_encoder(std::shared_ptr<bencode_value_base> sp_bvb) :
  sp_bvb_(sp_bvb),
  bvb_(0)
{
}

bencode_encoder::bencode_encoder(bencode_value_base *bvb) :
  sp_bvb_(0),
  bvb_(bvb)
{
}

void bencode_encoder::encode()
{
  if (sp_bvb_ != NULL) {
    sp_bvb_->crawl(this);
  }
  else if (bvb_ != NULL) {
    bvb_->crawl(this);
  }
  else {
    std::cout << "encode nothing" << std::endl;
  }
}

void bencode_encoder::print_result()
{
  std::cout << "encoded string length: " << encoded_str_.size() << std::endl;
}

const std::string &bencode_encoder::get_value()
{
  return encoded_str_;
}

void bencode_encoder::crawl(bencode_string *p)
{
  encoded_str_ += std::to_string(p->get_value().size());
  encoded_str_ += ':';
  encoded_str_ += p->get_value();
}

void bencode_encoder::crawl(bencode_integer *p)
{
  encoded_str_ += 'i';
  encoded_str_ += std::to_string(p->get_value());
  encoded_str_ += 'e';
}

void bencode_encoder::crawl(bencode_list *p)
{
  encoded_str_ += 'l';

  for (const auto &e : *p) {
    e.get()->crawl(this);
  }

  encoded_str_ += 'e';
}

void bencode_encoder::crawl(bencode_dictionary *p)
{
  encoded_str_ += 'd';

  for (const auto &e : *p) {
    auto key = e.first;
    encoded_str_ += std::to_string(key.size());
    encoded_str_ += ':';
    encoded_str_ += key.c_str();
    e.second.get()->crawl(this);
  }

  encoded_str_ += 'e';
}
