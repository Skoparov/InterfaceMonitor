#include "InterfaceMonitor.h"

////////////////////////////////////////////////////////////
///////            InterfaceMonitor               //////////
////////////////////////////////////////////////////////////

InterfaceMonitor::InterfaceMonitor(io_service& io, const uint& updatePeriodMsec,
                                   const uint& printPeriodMsec, std::ostream* stream) :
                   mPrintPeriodMsec(printPeriodMsec),
                   mPrintTimer(io, msec(updatePeriodMsec)),
                   mOutputStream(stream)

{      
    try{
       mManager = InterfaceManagerPtr(new InterfaceManager(io, updatePeriodMsec));
    }
    catch(...)
    {
        /**< Handling is incumbent upon a user */
        throw;
        return;
    }  

    mManager->statusChangedSignal.connect(boost::bind(&InterfaceMonitor::onInterfaceStateChanged, this, _1, _2));
    mManager->interfaceUpdateSignal.connect(boost::bind(&InterfaceMonitor::onInterfaceListUpdate, this, _1, _2));
    mManager->updateFailedSignal.connect(boost::bind(&InterfaceMonitor::onUpdateFailed, this));
}

void InterfaceMonitor::start()
{
   unique_lock(mMutex);

   mManager->start();
   startTimer(mManager->getUpdateTimeout());
}

void InterfaceMonitor::stop()
{
    unique_lock(mMutex);

    mPrintTimer.cancel();
}

void InterfaceMonitor::printInterfaces() const
{
    unique_lock(mMutex);

    const InterfaceInfoStorage interfaceData = mManager->getInterfaceData();
    if(interfaceData.size())
    {
        for(auto interface : interfaceData)
        {
            InterfaceInfo& info = interface.second;
            (*mOutputStream)<<std::string("IFACE ") + serializeInterfaceInfo(info)<<std::endl;
        }
    }
}

void InterfaceMonitor::onInterfaceStateChanged(const std::string& name, const bool& status) const
{
    unique_lock(mMutex);

    std::string message = name + (status? " ON" : " OFF ");
    (*mOutputStream)<<message<<std::endl;
}

void InterfaceMonitor::onInterfaceListUpdate(const InterfaceInfo &info, const bool& action) const
{
    unique_lock(mMutex);

    std::string message = action?
                "NEW " + serializeInterfaceInfo(info) : "GONE " + info.name;

    (*mOutputStream)<<message<<std::endl;
}

void InterfaceMonitor::onUpdateFailed()
{
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
    std::string type = typeTostring(info.type);

    std::string macAddr;
    if(info.type != IF_TYPE_LO && info.type != IF_TYPE_TUN){
        macAddr = " " + macToString(info.L2_addr);
    }

    std::string result = (boost::format("%s%s %s") % info.name % macAddr % type).str();

    return result;
}

std::string InterfaceMonitor::macToString(const unsigned char (&mac)[6]) const
{
    return (boost::format("%x:%x:%x:%x:%x:%x")
            % (int)mac[0] % (int)mac[1] % (int)mac[2]
            % (int)mac[3] % (int)mac[4] % (int)mac[5]).str();
}

std::string InterfaceMonitor::typeTostring(const InterfaceType& type) const
{
    std::string strType;

    switch(type)
    {
    case IF_TYPE_ETH:        strType = "Ethernet";break;
    case IF_TYPE_LO:         strType = "Loopback";break;
    case IF_TYPE_TUN:        strType = "Tunnel";  break;
    default:                 strType = "Unknown"; break;
    }

    return strType;
}

InterfaceMonitor::~InterfaceMonitor()
{

}
