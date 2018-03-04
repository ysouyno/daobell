#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include "../src/ftp_url_parser.h"

class ftp_url_parser_test : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(ftp_url_parser_test);
    CPPUNIT_TEST(test_parse);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_parse()
    {
        ftp_url_parser fup("ftp://127.0.0.1:21/w@w/1024_lnk.txt");
        CPPUNIT_ASSERT("anonymous" == fup.user_);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ftp_url_parser_test);

int main(int argc, char *argv[])
{
    CppUnit::TestResult result;
    CppUnit::TestResultCollector result_collector;
    result.addListener(&result_collector);

    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(result);

    CppUnit::TextOutputter output(&result_collector, std::cout);
    output.write();

    return result_collector.wasSuccessful() ? 0 : -1;
}
