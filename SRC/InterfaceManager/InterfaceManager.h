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
typedef unsigned int uint;

////////////////////////////////////////////////////////////
///////            InterfaceManager               //////////
////////////////////////////////////////////////////////////

class InterfaceManager
{
public:
    InterfaceManager(io_service& io, const uint& updatePeriodMsec);
    virtual ~InterfaceManager();

    void start();
    void stop();
    InterfaceInfoStorage getInterfaceData() const;
    uint getUpdateTimeout() const;

private:
    void updateInterfaces();
    void startTimer(uint timeout);

    /**< Slots */
    void onTimeout(const boost::system::error_code &ec);
    void onUpdateFailedSlot();
    void onStatusChangedSlot(const std::string& name, const bool& status);
    void onInterfaceUpdateSlot(const InterfaceInfo& info, const bool& action);

     /**< The functions below are used to force signals call slots in the main thread */
    void sendUpdateFailedSignal();
    void sendStatusChangedSignal(const std::string& name, const bool& status);
    void sendInterfaceUpdateSignal(const InterfaceInfo& info, const bool& action);  

private:   
    ImplPtr mImpl;                             /**<  An implementation depends on the platform */
    uint mUpdatePeriodMsec;                    /**< Interface info update period in msec */
    deadline_timer mUpdateTimer;               /**< Interface data is updaten upon timeout */

    io_service& mEventLoop;
    io_service mThreadService;
    boost::thread_group mThreadGroup;          /**< Used to update interface data in async way */
    WorkPtr mWork;

    boost::mutex mMutex;

public:
    statusSignal statusChangedSignal;            /**< Emitted if an interface has changed it's status */
    updateSignal interfaceUpdateSignal;          /**< Emitted if an interface is added or removed */
    errorSignal  updateFailedSignal;             /**< Emitted on update error */
};

#endif // INTERFACEMANAGER_H
