#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

/**
* @file InterfaceManager.h
* @brief Contains the base class of an interface manager
*  Requires an event loop to be passed as a constructor parameter
*  as the update timer uses it to periodically refresh interfaces information
*/

#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#ifdef __linux__
    #include "InterfaceManagerImplLinux.h"
#elif defined (_WIN32) || defined (_WIN64)
    #error "Windows impl is yet to be done"
#else
    #error "Unknown platform"
#endif

using namespace boost::asio;

typedef boost::posix_time::millisec msec;
typedef std::unique_ptr<InterfaceManagerImpl> ImplPtr;
typedef std::unique_ptr<io_service::work> WorkPtr;

////////////////////////////////////////////////////////////
///////            InterfaceManager               //////////
////////////////////////////////////////////////////////////

class InterfaceManager
{
public:
    InterfaceManager(io_service& io);
    virtual ~InterfaceManager();

    void startListening();
    void stopListening();
    void updateDevices();
    InterfaceInfoStorage getInterfaceData() const;

private:   
    /**< Slots */ 
    void onUpdateFailedSlot();
    void onInterfaceUpdateSlot(const InterfaceInfo& info, const bool& action);

     /**< The functions below are used to force signals call slots in the main thread */
    void sendUpdateFailedSignal();    
    void sendInterfaceUpdateSignal(const InterfaceInfo& info, const bool& action);  

private:   
    ImplPtr mImpl;                             /**<  An implementation depends on the platform */   
    io_service& mEventLoop;
    io_service mImplService;
    boost::thread_group mThreadGroop;
    WorkPtr mWork;

public: 
    updateSignal interfaceUpdateSignal;          /**< Emitted if an interface is added or removed */
    errorSignal  updateFailedSignal;             /**< Emitted on update error */
};

#endif // INTERFACEMANAGER_H
