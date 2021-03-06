/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file boostTest.cpp
 * @author Dimitry Khokhlov <dimitry@ethdev.com>
 * @date 2015
 * Stub for generating main boost.test module.
 * Original code taken from boost sources.
 */

#define BOOST_TEST_MODULE EthereumTests
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
//#define BOOST_DISABLE_WIN32 //disables SEH warning
#define BOOST_TEST_NO_MAIN
#include <boost/test/included/unit_test.hpp>
#pragma GCC diagnostic pop
#include <test/TestHelper.h>

using namespace boost::unit_test;

std::vector<char*> originalArgs;
static std::ostringstream strCout;
std::streambuf* oldCoutStreamBuf;

//Custom Boost Initialization
test_suite* init_func(int argc, char* argv[])
{
	//Required for boost. -nowarning
	(void) argc;
	(void) argv;

	//restore output for creating test
	std::cout.rdbuf(oldCoutStreamBuf);
	std::cerr.rdbuf(oldCoutStreamBuf);
	const auto& opt = dev::test::Options::get();
	if (opt.createRandomTest)
	{
		//For no reason BOOST tend to remove valuable arg -t "TestSuiteName", so using original parametrs instead
		if (dev::test::createRandomTest(originalArgs))
			throw framework::internal_error("Create Random Test Error!");
		else
		{
			//disable post output so the test json would be clean
			std::cout.rdbuf(strCout.rdbuf());
			std::cerr.rdbuf(strCout.rdbuf());
			throw framework::nothing_to_test();
		}
	}
	return 0;
}

//Custom Boost Unit Test Main
int main( int argc, char* argv[] )
{
	for (int i = 0; i < argc; i++)
		originalArgs.push_back(argv[i]);

	//disable initial output
	oldCoutStreamBuf = std::cout.rdbuf();
	std::cout.rdbuf(strCout.rdbuf());
	std::cerr.rdbuf(strCout.rdbuf());

	try
	{
		framework::init(init_func, argc, argv);

		if( !runtime_config::test_to_run().is_empty() )
		{
			test_case_filter filter(runtime_config::test_to_run());

			traverse_test_tree(framework::master_test_suite().p_id, filter);
		}

		framework::run();

		results_reporter::make_report();

		return runtime_config::no_result_code()
					? boost::exit_success
					: results_collector.results(framework::master_test_suite().p_id).result_code();
	}
	catch (framework::nothing_to_test const&)
	{
		return boost::exit_success;
	}
	catch (framework::internal_error const& ex)
	{
		results_reporter::get_stream() << "Boost.Test framework internal error: " << ex.what() << std::endl;

		return boost::exit_exception_failure;
	}
	catch (framework::setup_error const& ex)
	{
		results_reporter::get_stream() << "Test setup error: " << ex.what() << std::endl;

		return boost::exit_exception_failure;
	}
	catch (...)
	{
		results_reporter::get_stream() << "Boost.Test framework internal error: unknown reason" << std::endl;

		return boost::exit_exception_failure;
	}
}
