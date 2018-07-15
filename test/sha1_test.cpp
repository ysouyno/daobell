#include <cppunit/extensions/HelperMacros.h>
#include "sha1.h"

class sha1_test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(sha1_test);
  CPPUNIT_TEST(test_sha1_compute);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_sha1_compute();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sha1_test);

void sha1_test::test_sha1_compute()
{
  std::string sha1_str = "test";
  char sha1_temp[DIGEST_LEN] = {0};
  sha1_compute(sha1_str.c_str(), sha1_str.size(), sha1_temp);

  std::string sha1_ret;
  for (size_t i = 0; i < DIGEST_LEN; ++i) {
    char temp[8] = {0};
    sprintf(temp, "%02x", (unsigned char)sha1_temp[i]);
    sha1_ret.append(temp);
  }

  CPPUNIT_ASSERT(sha1_ret == "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3");
}
