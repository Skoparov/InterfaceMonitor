#include "InterfaceMonitor.h"
#include <signal.h>

static boost::asio::io_service eventLoop;

void closeHandler (int sigNum)
{
   std::cout<<"Finishing the program"<<std::endl;
   eventLoop.stop();
}

int main(int argc, char **argv)
{
    /**< Close signals handling */
    struct sigaction action;
    action.sa_handler = closeHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    uint updateTimeout = 500;
    uint printTimeout = 5000;

    boost::asio::io_service::work work(eventLoop);

    try
    {
        InterfaceMonitor mon(eventLoop, updateTimeout, printTimeout);
        mon.start();
        eventLoop.run();
    }
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        eventLoop.stop();
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught\n";
    }

    return 0;
}
