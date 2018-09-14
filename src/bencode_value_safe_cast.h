#ifndef BENCODE_VALUE_SAFE_CAST_H
#define BENCODE_VALUE_SAFE_CAST_H

#include "bencode_value_base.h"

class empty_bencode_crawler : public bencode_crawler
{
 public:
  virtual void crawl(bencode_string *p) {}
  virtual void crawl(bencode_integer *p) {}
  virtual void crawl(bencode_list *p) {}
  virtual void crawl(bencode_dict *p) {}
};

template <typename T>
class bencode_value_safe_cast : public empty_bencode_crawler
{
 public:
  bencode_value_safe_cast();

 public:
  virtual void crawl(T *t)
  {
    // the type of parameter 't' is a pointer pointed to 'this', so it will
    // be bencode_dict object and the type of return value 't_' is to's TO
    // type, it is bencode_dict type so here just assignment, it is safe,
    // like dynamic_cast
    t_ = t;
  }

  T *get_cast_result() const
  {
    return t_;
  }

 private:
  T *t_;
};

template <typename T>
bencode_value_safe_cast<T>::bencode_value_safe_cast() : t_(NULL)
{}

template <typename TO, typename FROM>
  TO *down_cast(FROM *from)
{
  if (!from) {
    return 0;
  }

  // because the parameter 'from' is the abstract class pointer type so visit
  // function will call it's derived class's function, here the 'from' pointer
  // pointed to bencode_dict, it will invoke bencode_dict's visit funciton,
  // and because parameter 'to' is a derived class inherited from
  // bencode_crawler pointer also, so when call bencode_dict's visit funciton,
  // it will call bencode_value_safe_cast's crawl function, and in that function
  // the parameter type is this pointer that is bencode_dict class type, see
  // above to continue
  bencode_value_safe_cast<TO> to;
  from->crawl(&to);

  return to.get_cast_result();
}

#endif /* BENCODE_VALUE_SAFE_CAST_H */
