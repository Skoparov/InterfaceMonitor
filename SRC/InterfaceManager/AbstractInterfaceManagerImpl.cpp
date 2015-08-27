#include "AbstractInterfaceManagerImpl.h"

////////////////////////////////////////////////////////////
///////         AbstractInterfaceManagerImpl      //////////
////////////////////////////////////////////////////////////

AbstractInterfaceManagerImpl::AbstractInterfaceManagerImpl() :
                              mFirstUpdate (true)
{

}

const InterfaceInfoStorage& AbstractInterfaceManagerImpl::getInterfacesData()
{
    unique_lock lock(mMutex);
    return mInterfaces;
}

////////////////////////////////////////////////////////////
///////             InterfaceInfo                 //////////
////////////////////////////////////////////////////////////

InterfaceInfo::InterfaceInfo(std::string interfaceName) :
    name(interfaceName),
    type(IF_TYPE_UNKNOWN),
    isActive(false),
    isVirtual(false)
{

}
