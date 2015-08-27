#ifndef INTERFACEMANAGERIMPLLINUX_H
#define INTERFACEMANAGERIMPLLINUX_H

/**
* @file AbstractInterfaceManagerImpl.h
* @brief Contains a linux-based concrete class of InterfaceManager implementation
*/

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "AbstractInterfaceManagerImpl.h"

typedef std::vector<ifaddrs> InterfacesData;

////////////////////////////////////////////////////////////
///////            InterfaceManagerImpl           //////////
////////////////////////////////////////////////////////////

class InterfaceManagerImpl : public AbstractInterfaceManagerImpl
{    
public:
    InterfaceManagerImpl();
    ~InterfaceManagerImpl();

    bool update();   /**< Removes gone, adds new and updates ifaces, sends corresponding signals */

private:
    InterfacesData getCurrInterfaceList() const; /**< Returns a vector of ifaddrs for all avilable interfaces */

    void updateInterfaces(const InterfacesData& interfaceList);
    void removeGoneInterfaces(const InterfacesData& interfaceList);

    void updateInterfaceInfo(std::string interfaceName, const bool& isNewInterface = false);
    bool getInterfaceType(const std::string& interfaceName, InterfaceType& type) const;
    bool getMacAndStatus(const std::string& interfaceName, const InterfaceType& type,
                         unsigned char (&mac)[6], bool& isActive) const;

private:   
    int mSock;       /**< Used to gather L2 information and the status of an interface */
};

#endif // INTERFACEMANAGERIMPLLINUX_H
