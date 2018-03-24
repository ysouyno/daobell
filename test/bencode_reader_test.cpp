#include <cppunit/extensions/HelperMacros.h>
#include "bencode_parser.h"
#include "bencode_reader.h"

class bencode_reader_test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(bencode_reader_test);
  CPPUNIT_TEST(test_get_announce);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_get_announce();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bencode_reader_test);

void bencode_reader_test::test_get_announce()
{
  std::string announce = "http://bttracker.debian.org:6969/announce";
  std::string torrent_path =
    DAOBELL_TEST_DIR "/debian-9.4.0-amd64-netinst.iso.torrent";

  std::ifstream file(torrent_path, std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_reader br(sp_bvb);

  CPPUNIT_ASSERT(announce == br.get_announce());
}
