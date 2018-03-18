#include <cppunit/extensions/HelperMacros.h>
#include "ftp_url_parser.h"

class ftp_url_parser_test : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(ftp_url_parser_test);
    CPPUNIT_TEST(test_parse);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_parse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ftp_url_parser_test);

void ftp_url_parser_test::test_parse()
{
    ftp_url_parser fup("ftp://127.0.0.1:21/w@w/1024_lnk.txt");
    CPPUNIT_ASSERT("anonymous" == fup.user_);
    CPPUNIT_ASSERT("anonymous" == fup.password_);
    CPPUNIT_ASSERT("127.0.0.1" == fup.domain_);
    CPPUNIT_ASSERT("21" == fup.port_);
}
