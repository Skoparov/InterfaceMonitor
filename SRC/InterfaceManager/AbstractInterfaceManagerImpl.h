#ifndef ABSTRACTIMTERFACEMANAGERIMPL_H
#define ABSTRACTIMTERFACEMANAGERIMPL_H

/**
* @file AbstractInterfaceManagerImpl.h
* @brief Contains an abstract base class of InterfaceManager implementation
*  and a structure to store interface parameters
*/

#include <map>
#include <string.h>
#include <sstream>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

struct InterfaceInfo;

typedef std::map<std::string, InterfaceInfo> InterfaceInfoStorage;
typedef std::pair<std::string, InterfaceInfo> InterfaceInfoPair;
typedef boost::signals2::signal<void (const std::string& name, const bool& status)> statusSignal;
typedef boost::signals2::signal<void (const InterfaceInfo& info, const bool& action)> updateSignal;
typedef boost::signals2::signal<void ()> errorSignal;
typedef boost::unique_lock<boost::mutex> unique_lock;

// Platform - independant interface types
enum InterfaceType
{
    IF_TYPE_ETH,
    IF_TYPE_TUN,
    IF_TYPE_LO,
    IF_TYPE_UNKNOWN
};

////////////////////////////////////////////////////////////
///////       AbstractInterfaceManagerImpl        //////////
////////////////////////////////////////////////////////////

/**
* @class AbstractInterfaceManagerImpl
* @brief An abstract implementation for InterfaceManager.
* All platform-dependant implementations should inherit this class
*/

class AbstractInterfaceManagerImpl
{      
public:
      AbstractInterfaceManagerImpl();
      virtual ~AbstractInterfaceManagerImpl(){};

      virtual bool update() = 0;
      const InterfaceInfoStorage& getInterfacesData();

protected:
     InterfaceInfoStorage mInterfaces;         /**< All gathered interface data is stored here */
     boost::mutex mMutex;
     bool mFirstUpdate;

public:
      statusSignal statusChangedSignal;        /**< Emitted if an interface changes it's status */
      updateSignal interfaceListUpdateSignal;  /**< Emitted if an interface is added or removed */
      errorSignal  updateFailedSignal;         /**< Emitted on update error */
};

////////////////////////////////////////////////////////////
///////             InterfaceInfo                 //////////
////////////////////////////////////////////////////////////

/**
* @class InterfaceInfo
* @brief Stores interface data such as it's name, MAC, type and current state
*/

struct InterfaceInfo
{
    std::string name;
    unsigned char L2_addr[6];
    InterfaceType type;
    bool isActive;
    bool isVirtual;

    InterfaceInfo(std::string interfaceName);
};

#endif // ABSTRACTIMTERFACEMANAGERIMPL_H
