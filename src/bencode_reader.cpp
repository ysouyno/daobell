#include "bencode_reader.h"

bencode_reader::bencode_reader(std::shared_ptr<bencode_value_base> sp_bvb) :
  sp_bvb_(sp_bvb)
{
}

const std::string &bencode_reader::get_announce()
{
  bencode_dict *bd = dynamic_cast<bencode_dict *>(sp_bvb_.get());
  std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict = bd->get_value();
  std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = dict.find("announce");
  bencode_string *bs = dynamic_cast<bencode_string *>(it->second.get());

  return bs->get_value();
}
