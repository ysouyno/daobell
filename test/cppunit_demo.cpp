#include <cppunit/TestCase.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>

class simple_test : public CppUnit::TestCase
{
public:
    simple_test() {}
    virtual ~simple_test() {}

    void runTest()
    {
        int i = 0;
        CPPUNIT_ASSERT_EQUAL(0, i);
    }
};

int main(int argc, char *argv[])
{
    CppUnit::TestResult result;
    CppUnit::TestResultCollector collector;
    result.addListener(&collector);

    simple_test st;
    st.run(&result);

    CppUnit::TextOutputter output(&collector, std::cout);
    output.write();

    return 0;
}
