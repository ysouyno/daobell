#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

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
