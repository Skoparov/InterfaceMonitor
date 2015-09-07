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
    ARG_COUNT
};

void setInterfaceState(bool state, const std::string& ifName)
{
#ifdef __linux__
    // executing ip link command
    std::string command = state?
                (boost::format("sudo ip link add link %s name test type vlan id 0") % ifName).str() :
                "sudo ip link delete test";

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
        std::string infoMessage  = (boost::format("\n%s\n%s\n%s\n%s\n")
                                % "Invalid parameters count. Parameters:"
                                % "1. Iface list output pattern file path."
                                    " Must contain a valid interface list output"
                                     "for the current interface settings"
                                % "2. Iface add/remove output pattern file path."
                                    " Must contain a valid output from both addition"
                                     "AND removal of an interface (both in the same file)"
                                % "3. Base iface name to create vlan").str();

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

        /**< The printing period is big enough to avoid unnecessary printing */
        uint printTimeout = 600000;

        /**< Starting the class in another thread to be able to handle it's output
          The class prints all interfaces on start, so we don't need to dont anything else */

        io_service eventLoop;
        io_service::work work(eventLoop);
        InterfaceMonitor mon(eventLoop, printTimeout, &output);
        boost::thread t(boost::bind(&boost::asio::io_service::run, &eventLoop));
        eventLoop.dispatch(boost::bind(&InterfaceMonitor::start, &mon));

        boost::this_thread::sleep(msec(1000));

        eventLoop.stop();
        t.join();

        BOOST_CHECK(output.match_pattern());
    }    
}

BOOST_FIXTURE_TEST_CASE( interface_add_remove_print_check, ArgsFixture )
{
    bool argsOk = (argc == ARG_COUNT);
    BOOST_CHECK(argsOk);

    if(argsOk)
    {
        std::string ifName = argv[ARG_IF_NAME];

        output_test_stream output( argv[ARG_IF_STATE_PATTERN_FILENAME], true);
        stream< null_sink> nullOstream((null_sink())); // To skip the initial ifase list output

        /**< The printing period is big enough to avoid unnecessary printing */
        uint printTimeout = 500000;

        io_service eventLoop;
        io_service::work work(eventLoop);

        InterfaceMonitor mon(eventLoop, printTimeout, &nullOstream);
        boost::thread t(boost::bind(&boost::asio::io_service::run, &eventLoop));
        eventLoop.dispatch(boost::bind(&InterfaceMonitor::start, &mon));        

        // To skip the initial ifase list output
        boost::this_thread::sleep(msec(1000));

        eventLoop.dispatch(boost::bind(&InterfaceMonitor::setOutputStream, &mon, &output)); //setting test output

        std::cout<<"Adding test iface"<<std::endl;
        setInterfaceState(true,  ifName);
        boost::this_thread::sleep_for(boost::chrono::seconds(1)); // To give the system some time to add an iface

        std::cout<<"Removing test iface"<<std::endl;
        setInterfaceState(false, ifName);
        boost::this_thread::sleep_for(boost::chrono::seconds(1));

        eventLoop.stop();
        t.join();

        BOOST_CHECK( output.match_pattern());
    }
}

#endif //TESTS_H
