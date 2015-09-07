#include "InterfaceMonitor.h"

////////////////////////////////////////////////////////////
///////            InterfaceMonitor               //////////
////////////////////////////////////////////////////////////

InterfaceMonitor::InterfaceMonitor(io_service& io, const uint& printPeriodMsec, std::ostream* stream) :
                   mPrintPeriodMsec(printPeriodMsec),
                   mPrintTimer(io, msec(printPeriodMsec)),
                   mOutputStream(stream)

{      
    try{
       mManager = InterfaceManagerPtr(new InterfaceManager(io));
    }
    catch(...)
    {
        /**< Handling is incumbent upon a user */
        throw;    
    }  

    mManager->interfaceUpdateSignal.connect(boost::bind(&InterfaceMonitor::onInterfaceListUpdate, this, _1, _2));
    mManager->updateFailedSignal.connect(boost::bind(&InterfaceMonitor::onUpdateFailed, this));
}

void InterfaceMonitor::start()
{
   unique_lock(mMutex);

   mManager->updateDevices();
   mManager->startListening();
   startTimer();
}

void InterfaceMonitor::stop()
{
    unique_lock(mMutex);

    mManager->stopListening();
    mPrintTimer.cancel();
}

void InterfaceMonitor::printInterfaces() const
{
    unique_lock(mMutex);
    const InterfaceInfoStorage interfaceData = mManager->getInterfaceData();

    for(auto& interface : interfaceData)
    {
        const InterfaceInfo& info = interface.second;
        std::string message = (boost::format("%s %s")
                              % IFACE
                              % serializeInterfaceInfo(info)).str();

        (*mOutputStream)<<message<<std::endl;
    }
}

void InterfaceMonitor::onInterfaceListUpdate(const InterfaceInfo &info, const bool& action) const
{
    unique_lock(mMutex);

    std::string message = (boost::format("%s %s")
                          % (action? IFACE_ADDED : IFACE_GONE)
                          % (action? serializeInterfaceInfo(info) : info.name)).str();

    (*mOutputStream)<<message<<std::endl;
}

void InterfaceMonitor::onUpdateFailed()
{
   unique_lock(mMutex);

   stop();

   /**< Handling is incumbent upon a user */
   throw std::runtime_error("Update failed");
}

void InterfaceMonitor::startTimer(uint timeout)
{
    /**< Updating the timer */
    mPrintTimer.expires_from_now(msec(timeout));
    mPrintTimer.async_wait(boost::bind(&InterfaceMonitor::onTimeout, this, boost::asio::placeholders::error));
}

void InterfaceMonitor::onTimeout(const boost::system::error_code &ec)
{
    if(!ec)
    {
        printInterfaces();
        startTimer(mPrintPeriodMsec);
    }
}

void InterfaceMonitor::setOutputStream(std::ostream* stream)
{
    unique_lock(mMutex);

    mOutputStream->flush();
    mOutputStream = stream;
}

std::string InterfaceMonitor::serializeInterfaceInfo(const InterfaceInfo &info) const
{
   return  (boost::format("%s %s %s")
            % info.name
            % info.hwAddr
            % typeTostring(info.type)).str();
}

std::string InterfaceMonitor::typeTostring(const InterfaceType& type) const
{
    std::string strType;

    switch(type)
    {
    case IF_TYPE_ETH:        strType = IFACE_ETH_NAME;      break;
    case IF_TYPE_TUN:        strType = IFACE_TUN_NAME;      break;
    default:                 strType = IFACE_UNKNOWN_NAME;  break;
    }

    return strType;
}

InterfaceMonitor::~InterfaceMonitor()
{

}
