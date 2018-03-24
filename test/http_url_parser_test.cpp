#include <cppunit/extensions/HelperMacros.h>
#include "http_url_parser.h"

class http_url_parser_test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(http_url_parser_test);
  CPPUNIT_TEST(test_parse);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_parse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(http_url_parser_test);

void http_url_parser_test::test_parse()
{
  http_url_parser hup("http://www.baidu.com/test/test.html");
  CPPUNIT_ASSERT("http" == hup.scheme_);
  CPPUNIT_ASSERT("www.baidu.com" == hup.domain_);
}
