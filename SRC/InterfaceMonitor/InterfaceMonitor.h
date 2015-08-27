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

using namespace boost::asio;

typedef std::unique_ptr<InterfaceManager> InterfaceManagerPtr;

////////////////////////////////////////////////////////////
///////            InterfaceMonitor               //////////
////////////////////////////////////////////////////////////

class InterfaceMonitor
{
private:    
    void startTimer(uint timeout);

    //slots
    void onTimeout(const boost::system::error_code &ec);
    void onInterfaceStateChanged(const std::string& name, const bool& status) const;
    void onInterfaceListUpdate (const InterfaceInfo& info, const bool& action) const;
    void onUpdateFailed();

    std::string serializeInterfaceInfo(const InterfaceInfo& info) const;  /**< Iface info -> string */
    std::string macToString(const unsigned char (&mac)[6]) const;
    std::string typeTostring(const InterfaceType& type) const; 

public:
    InterfaceMonitor(io_service& io, const uint& updatePeriodMsec,
                     const uint& printPeriodMsec, std::ostream* stream = &std::cout);
    ~InterfaceMonitor();

    void start();                                 /**< Starts iface info updating, immediately prints the output */
    void stop();                                  /**< Stops iface info updating */
    void printInterfaces() const;
    void setOutputStream(std::ostream* stream);   /**< Stops iface info updating */

private:
    InterfaceManagerPtr mManager;

    boost::mutex mMutex;
    std::ostream* mOutputStream;
    uint mPrintPeriodMsec;                         /**< Interface info print period in msec */    
    deadline_timer mPrintTimer;  
};

#endif // INTERFACEMANAGER_H
