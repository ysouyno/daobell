#ifndef BENCODE_CRAWLER_H
#define BENCODE_CRAWLER_H

class bencode_string;
class bencode_integer;
class bencode_list;
class bencode_dict;

class bencode_crawler
{
public:
  virtual void crawl(bencode_string *p) = 0;
  virtual void crawl(bencode_integer *p) = 0;
  virtual void crawl(bencode_list *p) = 0;
  virtual void crawl(bencode_dict *p) = 0;
};

#endif /* BENCODE_CRAWLER_H */
