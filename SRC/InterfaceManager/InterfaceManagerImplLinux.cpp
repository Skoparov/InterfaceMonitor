#include "InterfaceManagerImplLinux.h"

////////////////////////////////////////////////////////////
///////            InterfaceManagerImpl           //////////
////////////////////////////////////////////////////////////

InterfaceManagerImpl::InterfaceManagerImpl()
{
    //creating a udp socket
    mSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (mSock == -1) {
        throw std::runtime_error("Error creating socket");
    };
}

bool InterfaceManagerImpl::update()
{  
    unique_lock lock(mMutex);
    bool result = true;

    try
    {
      const InterfacesData& interfaces = getCurrInterfaceList();

       removeGoneInterfaces(interfaces);
       updateInterfaces(interfaces);
    }
    catch(const std::exception& e)
    {       
        std::cout<<e.what()<<std::endl;        
        mInterfaces.clear();
        mFirstUpdate = true;
        result = false;
        updateFailedSignal();
    }  

    if(result && mFirstUpdate){
        mFirstUpdate = false;
    }

    return result;
}

InterfacesData InterfaceManagerImpl::getCurrInterfaceList() const
{
    std::vector<ifaddrs> interfacesData;
    ifaddrs* interfaceList = nullptr;

    try
    {
        if(mSock == -1 ){
             throw std::runtime_error("Socket not initialized");
        }

        // getting a linked list comprised of ifaddrs structs
        ifaddrs* interface;
        if (getifaddrs(&interfaceList) == -1){
            throw std::runtime_error("Error getting interfaces info");
        }

        // iterating thorough the list
        interface = interfaceList;
        while(interface != nullptr)
        {
            interfacesData.push_back(*interface);
            interface = interface->ifa_next;
        }
    }
    catch(...)
    {
        interfacesData.clear();
        throw;
    }

    // If the operation was successful, clear the linked list
    if(interfaceList != nullptr){
        freeifaddrs(interfaceList);
    }

    return interfacesData;
}

void InterfaceManagerImpl::removeGoneInterfaces(const InterfacesData& interfacesData)
{
    // Removing gone interfaces from the map
    for(auto interface = mInterfaces.begin(); interface != mInterfaces.end();)
    {
        std::string name = interface->second.name;
        auto ifIter = std::find_if(interfacesData.begin(), interfacesData.end(),
                                   [&name]( const ifaddrs& item ){
                                        return name == item.ifa_name;
                                    });

        if(ifIter == interfacesData.end())
        {
            InterfaceInfo info = interface->second;
            mInterfaces.erase(interface++);            
            interfaceListUpdateSignal(info, false); //sending a notification to the application
        }
        else{
            ++interface;
        }
    }
}

void InterfaceManagerImpl::updateInterfaces(const InterfacesData& interfaceList)
{
    // Updating avilable interfaces info
    for (auto& interface : interfaceList)
    {
        bool isNewInterface = !mInterfaces.count(interface.ifa_name);
        if(isNewInterface)
        {
            InterfaceInfo info(interface.ifa_name);
            info.isVirtual = info.name.find(":");
            mInterfaces.insert(InterfaceInfoPair(info.name, info));
        }

        // updating the current interface info
        updateInterfaceInfo(interface.ifa_name, isNewInterface);

        if(isNewInterface)
        {
            auto infoIter = mInterfaces.find(interface.ifa_name);

            //if not first update, sending a notification to the application
            if(!mFirstUpdate){
             interfaceListUpdateSignal(infoIter->second, true);
            }
        }
    }
}

void InterfaceManagerImpl::updateInterfaceInfo(std::string interfaceName, const bool& isNewInterface)
{
    auto infoIter = mInterfaces.find(interfaceName);
    InterfaceInfo& info = infoIter->second;
    bool isActive;

    if(info.isVirtual)
    {
        std::vector<std::string> parts;
        boost::split(parts,interfaceName,boost::is_any_of(":"));
        interfaceName = parts.front();
    }

    if(!getInterfaceType(interfaceName, info.type)){
        throw std::runtime_error("Error getting interface type");
    }

    if(!getMacAndStatus(interfaceName, info.type, info.L2_addr, isActive)){
        throw std::runtime_error("Error getting mac address");
    }

    if(isActive != info.isActive)
    {
        info.isActive = isActive;
        if(!isNewInterface){
            statusChangedSignal(info.name, isActive);
        }
    }
}

bool InterfaceManagerImpl::getInterfaceType(const std::string &interfaceName, InterfaceType &type) const
{
    // Executing the interface type query
    std::string str = "cat /sys/class/net/" + interfaceName + "/type";

    FILE* pipe = popen(str.c_str(), "r");
    if (!pipe){
        return false;
    }

    // Reading the result from the pipe
    std::string result;
    while(!feof(pipe))
    {
        char buffer[4];
        if(fgets(buffer, 128, pipe) != NULL){
            result += buffer;
        }
    }

    pclose(pipe);

    // Converting the gathered type to the lib's platform-independant type
    const int iftype = std::stoi(result);
    switch(iftype)
    {
        case ARPHRD_ETHER :    type = IF_TYPE_ETH;        break;
        case ARPHRD_TUNNEL :   type = IF_TYPE_TUN;        break;
        case ARPHRD_TUNNEL6 :  type = IF_TYPE_TUN;        break;
        case ARPHRD_SIT :      type = IF_TYPE_TUN;        break;
        case ARPHRD_IPGRE :    type = IF_TYPE_TUN;        break;
        case ARPHRD_LOOPBACK : type = IF_TYPE_LO;         break;
        default :              type = IF_TYPE_UNKNOWN;    break;
    }

    return true;
}

bool InterfaceManagerImpl::getMacAndStatus(const std::string &interfaceName, const InterfaceType& type,
                                           unsigned char (&mac)[6], bool &isActive) const
{
    ifreq ifr;
    strcpy(ifr.ifr_name, interfaceName.c_str());

    if (ioctl(mSock, SIOCGIFFLAGS, &ifr)  != 0){
        return false;
    }

    // Status
    isActive = (ifr.ifr_flags & ( IFF_UP | IFF_RUNNING )) == ( IFF_UP | IFF_RUNNING );

    // If the interface is not a loopback or a tunnel, reading it's mac address
    int macAddrSize = 6;
    if(!(ifr.ifr_flags & IFF_LOOPBACK && type != IF_TYPE_TUN) &&
       ioctl(mSock, SIOCGIFHWADDR, &ifr) == 0)
    {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, macAddrSize);
    }

    return true;
}

InterfaceManagerImpl::~InterfaceManagerImpl()
{
    if(mSock != -1){
        close(mSock);
    }
}
