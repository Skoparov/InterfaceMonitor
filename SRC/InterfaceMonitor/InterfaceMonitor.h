#ifndef INTERFACEMONITOR_H
#define INTERFACEMONITOR_H

/**
* @file InterfaceMonitor.h
* @brief Contains the class of an interface monitor
*  The class periodically prints all available interfaces
*  and informs about interface additions/removals
*  or status changes
*/

#include "InterfaceManager.h"
#include <boost/format.hpp>
#include <fstream>

typedef std::unique_ptr<InterfaceManager> InterfaceManagerPtr;
typedef unsigned int uint;

using namespace boost::asio;

#define IFACE_GONE              "GONE"
#define IFACE_ADDED             "NEW"
#define IFACE                   "IFACE"
#define IFACE_ETH_NAME          "Ethernet"
#define IFACE_TUN_NAME          "Tunnel"
#define IFACE_UNKNOWN_NAME      "Unknown"

////////////////////////////////////////////////////////////
///////            InterfaceMonitor               //////////
////////////////////////////////////////////////////////////

class InterfaceMonitor
{
private:    
    void startTimer(uint timeout = 0);

    //slots
    void onTimeout(const boost::system::error_code &ec);    
    void onInterfaceListUpdate (const InterfaceInfo& info, const bool& action) const;
    void onUpdateFailed();

    std::string serializeInterfaceInfo(const InterfaceInfo& info) const;  /**< Iface info -> string */  
    std::string typeTostring(const InterfaceType& type) const; 

public:
    InterfaceMonitor(io_service& io, const uint& printPeriodMsec, std::ostream* stream = &std::cout);
    ~InterfaceMonitor();

    void start();                                 /**< Starts printing ifaces */
    void stop();                                  /**< Stops printing ifaces */
    void printInterfaces() const;
    void setOutputStream(std::ostream* stream);

private:
    InterfaceManagerPtr mManager;

    boost::mutex mMutex;
    std::ostream* mOutputStream;
    uint mPrintPeriodMsec;                         /**< Interface info print period in msec */    
    deadline_timer mPrintTimer;  
};

#endif // INTERFACEMANAGER_H
