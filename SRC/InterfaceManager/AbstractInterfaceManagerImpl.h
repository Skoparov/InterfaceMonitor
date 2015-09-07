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
typedef boost::signals2::signal<void (const InterfaceInfo& info, const bool& action)> updateSignal;
typedef boost::signals2::signal<void ()> errorSignal;
typedef boost::unique_lock<boost::mutex> unique_lock;

// Platform - independent interface types
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
* All platform-dependent implementations should inherit this class
*/

class AbstractInterfaceManagerImpl
{      
public:
      AbstractInterfaceManagerImpl();
      virtual ~AbstractInterfaceManagerImpl(){};

      virtual void startListening() = 0; /**< Begin listening to system notifications */
      virtual void stopListening() = 0;  /**< Stop listening to system notifications */
      virtual void updateDevices() = 0;  /**< Directly updates devices data */

      const InterfaceInfoStorage& getInterfacesData();

protected:
     InterfaceInfoStorage mInterfaces;         /**< All gathered interface data is stored here */
     boost::mutex mMutex; 

public:
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
    std::string hwAddr;
    InterfaceType type;

    InterfaceInfo();
};

#endif // ABSTRACTIMTERFACEMANAGERIMPL_H
