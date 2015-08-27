#include "InterfaceManager.h"

////////////////////////////////////////////////////////////
///////            InterfaceManager               //////////
////////////////////////////////////////////////////////////

InterfaceManager::InterfaceManager(io_service& io, const uint& updatePeriodMsec) :
                                  mEventLoop (io),
                                  mUpdatePeriodMsec(updatePeriodMsec),
                                  mUpdateTimer(io, msec(updatePeriodMsec))
{       
    try{
       mImpl = ImplPtr(new InterfaceManagerImpl);
    }
    catch(...)
    {       
        throw;
        return;
    } 

    // Creating a thread to update information about ifaces asynchronously
    mWork = std::unique_ptr<io_service::work> (new io_service::work(mThreadService));
    mThreadGroup.create_thread(boost::bind(&io_service::run, &mThreadService));

    mImpl->statusChangedSignal.connect(boost::bind(&InterfaceManager::onStatusChangedSlot, this, _1, _2));
    mImpl->interfaceListUpdateSignal.connect(boost::bind(&InterfaceManager::onInterfaceUpdateSlot, this, _1, _2));
    mImpl->updateFailedSignal.connect(boost::bind(&InterfaceManager::onUpdateFailedSlot, this));
}

void InterfaceManager::start()
{
    unique_lock lock(mMutex);

    startTimer(0);
}

void InterfaceManager::stop()
{
    unique_lock lock(mMutex);  

    mUpdateTimer.cancel();
}

const InterfaceInfoStorage &InterfaceManager::getInterfaceData() const
{
    return mImpl->getInterfacesData();
}

uint InterfaceManager::getUpdateTimeout() const
{
    return mUpdatePeriodMsec;
}

void InterfaceManager::updateInterfaces()
{
    // Calling update fucntion in another thread
    mThreadService.dispatch(boost::bind(&InterfaceManagerImpl::update, mImpl.get()));
}

void InterfaceManager::startTimer(uint timeout)
{   
    mUpdateTimer.expires_from_now(msec(timeout));
    mUpdateTimer.async_wait(boost::bind(&InterfaceManager::onTimeout, this, boost::asio::placeholders::error));
}

void InterfaceManager::onTimeout(const boost::system::error_code &ec)
{
    if(!ec)
    {
        updateInterfaces();
        startTimer(mUpdatePeriodMsec);
    }
}

void InterfaceManager::onUpdateFailedSlot()
{   
   stop();
   mEventLoop.post(boost::bind(&InterfaceManager::sendUpdateFailedSignal, this));
}

void InterfaceManager::onStatusChangedSlot(const std::string& name, const bool& status)
{
    mEventLoop.post(boost::bind(&InterfaceManager::sendStatusChangedSignal, this, name, status));
}

void InterfaceManager::onInterfaceUpdateSlot(const InterfaceInfo& info, const bool& action)
{
    mEventLoop.post(boost::bind(&InterfaceManager::sendInterfaceUpdateSignal, this, info, action));
}

void InterfaceManager::sendUpdateFailedSignal()
{
    updateFailedSignal();
}

void InterfaceManager::sendStatusChangedSignal(const std::string& name, const bool& status)
{
    statusChangedSignal(name, status);
}

void InterfaceManager::sendInterfaceUpdateSignal(const InterfaceInfo& info, const bool& action)
{
    interfaceUpdateSignal(info, action);
}

InterfaceManager::~InterfaceManager()
{    
    mThreadService.stop();
    mThreadGroup.join_all();    
}
