#include <cppunit/extensions/HelperMacros.h>
#include "bencode_parser.h"
#include "torrent_info.h"
#include "torrent_helper.h"

class torrent_helper_test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(torrent_helper_test);
  CPPUNIT_TEST(test_get_announce);
  CPPUNIT_TEST(test_get_piece_length);
  CPPUNIT_TEST(test_get_creation_date);
  CPPUNIT_TEST(test_get_info_hash);
  CPPUNIT_TEST_SUITE_END();

  std::shared_ptr<bencode_value_base> sp_bvb_;
  std::shared_ptr<torrent_info> sp_ti_;

public:
  void setUp()
  {
    std::string torrent_path =
      DAOBELL_TEST_DIR "/debian-9.4.0-amd64-netinst.iso.torrent";

    std::ifstream file(torrent_path, std::ios::binary);
    bencode_parser bp(file);

    sp_bvb_ = bp.get_value();
    sp_ti_ = std::make_shared<torrent_info>();
  }

  void test_get_announce();
  void test_get_piece_length();
  void test_get_info_hash();
  void test_get_creation_date();
};

CPPUNIT_TEST_SUITE_REGISTRATION(torrent_helper_test);

void torrent_helper_test::test_get_announce()
{
  std::string announce = "http://bttracker.debian.org:6969/announce";

  get_announce(sp_ti_.get(), dynamic_cast<bencode_dictionary *>(sp_bvb_.get()));

  CPPUNIT_ASSERT(sp_ti_->announce_list_.size() == 1);
  CPPUNIT_ASSERT(sp_ti_->announce_list_.at(0) == announce);
}

void torrent_helper_test::test_get_piece_length()
{
  long long piece_length = 262144;

  get_piece_length(sp_ti_.get(), dynamic_cast<bencode_dictionary *>(sp_bvb_.get()));

  CPPUNIT_ASSERT(sp_ti_->piece_length_ == piece_length);
}

void torrent_helper_test::test_get_creation_date()
{
  long long creation_date = 1520682848;

  get_creation_date(sp_ti_.get(), dynamic_cast<bencode_dictionary *>(sp_bvb_.get()));

  CPPUNIT_ASSERT(sp_ti_->creation_date_ == creation_date);
}

void torrent_helper_test::test_get_info_hash()
{
  std::string info_hash = "7431A969B347E14BBA641B3517C024F7B40DFB7F";

  get_info_hash(sp_ti_.get(), dynamic_cast<bencode_dictionary *>(sp_bvb_.get()));

  CPPUNIT_ASSERT(sp_ti_->info_hash_ == info_hash);
}
