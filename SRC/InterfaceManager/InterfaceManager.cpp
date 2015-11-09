#include "InterfaceManager.h"

////////////////////////////////////////////////////////////
///////            InterfaceManager               //////////
////////////////////////////////////////////////////////////

InterfaceManager::InterfaceManager(io_service& io) :
    mEventLoop(io),
    mWork(new io_service::work(mImplService))
{           
    mImpl = ImplPtr(new InterfaceManagerImpl);       
    mThreadGroop.create_thread(boost::bind(&io_service::run, &mImplService));

    /**< Connecting signals */
    mImpl->interfaceListUpdateSignal.connect(boost::bind(&InterfaceManager::onInterfaceUpdateSlot, this, _1, _2));
    mImpl->updateFailedSignal.connect(boost::bind(&InterfaceManager::onUpdateFailedSlot, this));
}


void InterfaceManager::startListening()
{
    mImplService.dispatch(boost::bind(&InterfaceManagerImpl::startListening, mImpl.get()));
}

void InterfaceManager::stopListening()
{
    mImpl->stopListening();
}

void InterfaceManager::updateDevices()
{
    mImpl->updateDevices();
}

InterfaceInfoStorage InterfaceManager::getInterfaceData() const
{
    return mImpl->getInterfacesData();
}

void InterfaceManager::onUpdateFailedSlot()
{     
   mEventLoop.post(boost::bind(&InterfaceManager::sendUpdateFailedSignal, this));
}

void InterfaceManager::onInterfaceUpdateSlot(const InterfaceInfo& info, const bool& action)
{
    mEventLoop.post(boost::bind(&InterfaceManager::sendInterfaceUpdateSignal, this, info, action));
}

void InterfaceManager::sendUpdateFailedSignal()
{
    updateFailedSignal();
}

void InterfaceManager::sendInterfaceUpdateSignal(const InterfaceInfo& info, const bool& action)
{
    interfaceUpdateSignal(info, action);
}

InterfaceManager::~InterfaceManager()
{    
    stopListening();
    mImplService.stop();
    mThreadGroop.join_all();
}
