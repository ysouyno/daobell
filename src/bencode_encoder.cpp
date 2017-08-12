#include "bencode_encoder.h"

bencode_encoder::bencode_encoder()
{
}

bencode_encoder::~bencode_encoder()
{
}

bencode_encoder::bencode_encoder(std::shared_ptr<bencode_value_base> sp_bm) :
  sp_bm_(sp_bm)
{
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

void bencode_encoder::crawl_all()
{
  sp_bm_->crawl(this);
}

void bencode_encoder::print_all()
{
  // if the string length is equal to the seed file size,
  // the encoding is successful
  std::cout << "\n" << encoded_str_.size() << std::endl;
}
