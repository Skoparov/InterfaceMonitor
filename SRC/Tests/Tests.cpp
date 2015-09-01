#ifndef TESTS_H
#define TESTS_H

/**
* @file Tests.h
* @brief Contains InterfaceMonitor tests
*/

#define BOOST_TEST_MODULE InterfaceMonitorTests

#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
#include "boost/iostreams/stream.hpp"
#include "boost/iostreams/device/null.hpp"
#include <boost/chrono.hpp>

#include <stdlib.h>

#include "InterfaceMonitor.cpp"

using boost::test_tools::output_test_stream;
using namespace boost::iostreams;

enum ArgsPos
{
    ARG_IF_LIST_PATTERN_FILENAME = 1, //since 0th arg is the path to a binary file
    ARG_IF_STATE_PATTERN_FILENAME,
    ARG_IF_NAME,
    ARG_IF_ADDR,
    ARG_COUNT
};

void setInterfaceState(bool state, const std::string& ifName,
                       const std::string& ifAddr = std::string() )
{
#ifdef __linux__
    // executing ifconfig command
    std::string stateStr = state? " up" : " down";
    std::string command  = (boost::format(" sudo ifconfig %s %s %s")
                            % ifName
                            % ifAddr
                            % stateStr).str();

     system(command.c_str());
#elif defined (_WIN32) || defined (_WIN64)
    #error "Windows impl is yet to be done"
#else
    #error "Unknown platform"
#endif
}

struct ArgsFixture
{
   ArgsFixture():
            argc(boost::framework::master_test_suite().argc),
            argv(boost::framework::master_test_suite().argv){}

   int argc;
   char **argv;
};

BOOST_FIXTURE_TEST_CASE( args_check, ArgsFixture )
{
    bool argsOk = argc > 1 && argc <= ARG_COUNT;

    if(!argsOk)
    {
        std::string infoMessage  = (boost::format("\n%s\n%s\n%s\n%s\n%s\n")
                                % "Invalid parameters count. Parameters:"
                                % "1. Iface list output pattern file path."
                                    " Must contain a valid interface list output"
                                     "for the current interface settings"
                                % "2. Iface add/remove output pattern file path."
                                    " Must contain a valid output from both addition"
                                     "AND removal of an interface (both in the same file)"
                                % "3. Iface name to add/remove"
                                % "4. IP address of the aforementioned iface").str();

        std::cout<< infoMessage<<std::endl;
    }

    BOOST_CHECK(argsOk);
}

BOOST_FIXTURE_TEST_CASE( interface_list_print_check, ArgsFixture)
{
    bool argsOk = (argc == ARG_COUNT);
    BOOST_CHECK(argsOk);

    if(argsOk)
    {
        /**< Loading the pattern */
        output_test_stream output(argv[ARG_IF_LIST_PATTERN_FILENAME], true);

        /**< Regular iface update time, though the printing period is big enough to avoid unnecessary printing */
        uint updateTimeout = 500;
        uint printTimeout = 600000;

        io_service eventLoop;
        InterfaceMonitor mon(eventLoop, updateTimeout, printTimeout);

        /**< Starting the class in another thread to be able to handle it's output
          The class prints all interfaces on start, so we don't need to don anything else */
        boost::thread t(boost::bind(&boost::asio::io_service::run, &eventLoop));
        io_service::work work(eventLoop);
        eventLoop.dispatch(boost::bind(&InterfaceMonitor::start, &mon));

        eventLoop.stop();
        t.join();

        BOOST_CHECK( output.match_pattern() );
    }
}

BOOST_FIXTURE_TEST_CASE( interface_add_remove_print_check, ArgsFixture )
{
    bool argsOk = (argc == ARG_COUNT);
    BOOST_CHECK(argsOk);

    if(argsOk)
    {
        std::string ifName = argv[ARG_IF_NAME];
        std::string ifAddr;
        if(argc > ARG_IF_ADDR){
            ifAddr = argv[ARG_IF_ADDR];
        }

        output_test_stream output( argv[ARG_IF_STATE_PATTERN_FILENAME], true);
        stream< null_sink> nullOstream((null_sink())); // To skip the initial ifase list output

        /**< Regular iface update time, though the printing period is big enough to avoid unnecessary printing */
        uint updateTimeout = 500;
        uint printTimeout = 500000;

        io_service eventLoop;
        InterfaceMonitor mon(eventLoop, updateTimeout, printTimeout, &nullOstream);

        boost::thread t(boost::bind(&boost::asio::io_service::run, &eventLoop));
        io_service::work work(eventLoop);
        eventLoop.dispatch(boost::bind(&InterfaceMonitor::start, &mon));

        // To skip the initial ifase list output
        boost::this_thread::sleep(msec(2*updateTimeout));

        eventLoop.dispatch(boost::bind(&InterfaceMonitor::setOutputStream, &mon, &output)); //setting test output

        std::cout<<"Adding " + ifName<<std::endl;
        setInterfaceState(true,  ifName, ifAddr);

        boost::this_thread::sleep_for(boost::chrono::seconds(3)); // To give the system some time to add an iface

        std::cout<<"Removing " + ifName<<std::endl;
        setInterfaceState(false, ifName);

        boost::this_thread::sleep_for(boost::chrono::seconds(3));

        eventLoop.stop();
        t.join();

        BOOST_CHECK( output.match_pattern());
    }
}

#endif //TESTS_H
