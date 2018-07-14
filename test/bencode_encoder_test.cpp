#include <cppunit/extensions/HelperMacros.h>
#include "bencode_parser.h"
#include "bencode_encoder.h"

class bencode_encoder_test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(bencode_encoder_test);
  CPPUNIT_TEST(test_get_length);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_get_length();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bencode_encoder_test);

void bencode_encoder_test::test_get_length()
{
  std::string torrent_path =
    DAOBELL_TEST_DIR "/debian-9.4.0-amd64-netinst.iso.torrent";

  unsigned long file_size = 0;
  FILE *fp = fopen(torrent_path.c_str(), "r");
  if (NULL != fp) {
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
  }

  std::ifstream file(torrent_path, std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_encoder be(sp_bvb);
  be.encode();

  CPPUNIT_ASSERT(file_size == be.get_length());
}
