#include "InterfaceManagerImplLinux.h"

////////////////////////////////////////////////////////////
///////            InterfaceManagerImpl           //////////
////////////////////////////////////////////////////////////

InterfaceManagerImpl::InterfaceManagerImpl() : mLoop (nullptr), mNetManagerProxy(nullptr)
{   
    GError* error = nullptr;  

    try
    {
        mNetManagerProxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                          G_DBUS_PROXY_FLAGS_NONE,
                                                          NULL,
                                                          NM_IFACE_NETWORKMANAGER,
                                                          NM_IFACE_NETWORKMANAGER_PATH,
                                                          NM_IFACE_NETWORKMANAGER,
                                                          NULL,
                                                          &error);

        if(mNetManagerProxy == nullptr || error != nullptr){
            throw std::runtime_error("Error init network manager proxy");
        }        

        g_signal_connect(G_OBJECT(mNetManagerProxy), NM_SIGNAL_G_SIGNAL, G_CALLBACK(onNetManagerSignal), (gpointer)this);
    }
    catch(const std::exception& e)
    {
        std::string errorText = e.what();

        if(error != nullptr)
        {
            errorText = error->message;
            g_error_free(error);
        }

        if(mNetManagerProxy != nullptr){
            g_object_unref(mNetManagerProxy);
        }

        throw std::runtime_error(errorText);
    }
}

void InterfaceManagerImpl::startListening()
{           
    if(mLoop == nullptr){       
        mLoop = g_main_loop_new (NULL, FALSE);        
    }

    if(!g_main_is_running(mLoop)){
        g_main_loop_run (mLoop);
    }
}

void InterfaceManagerImpl::stopListening()
{
    if(mLoop != nullptr){
        g_main_loop_quit(mLoop);        
    }        
}

void InterfaceManagerImpl::onNetManagerSignal(GDBusProxy *proxy, gchar *sender, gchar *signal, GVariant *params, gpointer data)
{
    /**< where data is a pointer to the instance of InterfaceManagerImpl */
    if(data != nullptr)
    {
        InterfaceManagerImpl* impl = (InterfaceManagerImpl*)data;
        impl->handleNetManagerSignal(signal, params);
    }
}

void InterfaceManagerImpl::handleNetManagerSignal(const std::string &signalName, GVariant *params)
{
     unique_lock lock(mMutex);

     try
     {             
         if(signalName == NM_SIGNAL_DEVICE_ADDED || signalName == NM_SIGNAL_DEVICE_REMOVED )
         {
             const char *devPath;
             g_variant_get_child (params, 0, "&o", &devPath);

             if(signalName == NM_SIGNAL_DEVICE_ADDED)
             {
                 InterfaceInfo info = getDeviceInfo(devPath);
                 mInterfaces.insert(InterfaceInfoPair(devPath, info));
                 interfaceListUpdateSignal(info, true);
             }
             else if(signalName == NM_SIGNAL_DEVICE_REMOVED)
             {

                 auto info = mInterfaces.find(devPath);
                 if(info != mInterfaces.end())
                 {
                     InterfaceInfo devInfo = info->second;
                     mInterfaces.erase(devPath);
                     interfaceListUpdateSignal(devInfo, false);
                 }
             }
         }
     }
     catch(const std::exception& e){
         updateFailedSignal();
     }
}

void InterfaceManagerImpl::updateDevices()
{
    unique_lock(mMutex);

    GVariant* deviceList = nullptr;
    GError* error = nullptr;
    GCancellable* c = nullptr;

    try
    {
        if(mNetManagerProxy == nullptr){
            throw std::runtime_error("Network Manager proxy not initialized");
        }

        deviceList = g_dbus_proxy_call_sync(mNetManagerProxy,
                                            NM_METHOD_GET_DEVICES,
                                            NULL,
                                            G_DBUS_CALL_FLAGS_NONE,
                                            1000, //timout of operation
                                            c,
                                            &error);


        if (deviceList == NULL && error != NULL){
            throw std::runtime_error(error->message);
        }

        /**< iteration through the list */
        GVariantIter deviceIter1, deviceIter2;
        GVariant *deviceNode1, *deviceNode2;

        g_variant_iter_init(&deviceIter1, deviceList);
        while ((deviceNode1 = g_variant_iter_next_value(&deviceIter1)))
        {
            g_variant_iter_init(&deviceIter2, deviceNode1);
            while ((deviceNode2 = g_variant_iter_next_value(&deviceIter2)))
            {
                gsize strlength = 256;
                const gchar* devicePath = g_variant_get_string(deviceNode2, &strlength);
                InterfaceInfo info = getDeviceInfo(devicePath);
                mInterfaces.insert(InterfaceInfoPair(devicePath, info));
            }
        }                
    }
    catch(const std::exception& e)
    {
        if(deviceList != nullptr){
            g_variant_unref(deviceList);
        }

        if(error != nullptr){
            g_error_free(error);
        }

        if(c != nullptr){
            g_cancellable_release_fd(c);
        }

        std::cout<<e.what()<<std::endl;
        updateFailedSignal();
    }
}

InterfaceInfo InterfaceManagerImpl::getDeviceInfo(const std::string& deviceAddr)
{
    GDBusProxy* nmDeviceProxy = nullptr;
    GError* error = nullptr;
    InterfaceInfo info;

    try
    {              
        nmDeviceProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                      G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL,
                                                      NM_IFACE_NETWORKMANAGER,
                                                      deviceAddr.c_str(),
                                                      NM_IFACE_DEVICE,
                                                      NULL,
                                                      &error);


        if (nmDeviceProxy == NULL && error != NULL){
            throw std::runtime_error(error->message);
        }      

        guint deviceType = getDeviceType(nmDeviceProxy);

        info.type = nmDevTypeToLocalDevType(deviceType);
        info.name = getDeviceName(nmDeviceProxy);

        const std::string nmModule = getNmInterface(deviceType);
        if(nmModule.length()){
            info.hwAddr = getDeviceHwAddress(deviceAddr, nmModule);
        }     
    }
    catch(...)
    {
        if(nmDeviceProxy != nullptr){
            g_object_unref(nmDeviceProxy);
        }

        if(error != nullptr){
            g_error_free(error);
        }

        throw;
    }

    return info;
}

std::string InterfaceManagerImpl::getDeviceName(GDBusProxy* proxy) const
{
    std::string deviceName;
    gsize strlength = 256;

    GVariant* variant = g_dbus_proxy_get_cached_property(proxy, NM_IFACE_DEVICE_PROPERTY_NAME);
    if (variant != nullptr)
    {
        deviceName = g_variant_get_string(variant, &strlength);
        g_variant_unref(variant);
    }
    else{
        throw std::runtime_error("Error reading device name");
    }

    return deviceName;
}

guint InterfaceManagerImpl::getDeviceType(GDBusProxy* proxy) const
{
    guint deviceType = NM_DEVICE_TYPE_UNKNOWN;
    GVariant* variant = g_dbus_proxy_get_cached_property(proxy, NM_IFACE_DEVICE_PROPERTY_TYPE);

    if (variant != nullptr)
    {
        deviceType = g_variant_get_uint32(variant);
        g_variant_unref(variant);
    }
    else{
        throw std::runtime_error("Error reading device type");
    }

    return deviceType;
}

std::string InterfaceManagerImpl::getDeviceHwAddress(const std::string& deviceAddr, const std::string& nmModuleName) const
{
    std::string hwAddress;

    GError* error = nullptr;
    GDBusProxy* ethProxy = nullptr;

    try
    {
        ethProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 NM_IFACE_NETWORKMANAGER,
                                                 deviceAddr.c_str(),
                                                 nmModuleName.c_str(),
                                                 NULL,
                                                 &error);

        if (ethProxy == nullptr || error != nullptr){
            throw std::runtime_error("Failed to init proxy");
        }

        GVariant* variant = g_dbus_proxy_get_cached_property(ethProxy, NM_IFACE_DEVICE_PROPERTY_HWADDR);
        if (variant != nullptr)
        {
            gsize strLength = 255;
            hwAddress = g_variant_get_string(variant, &strLength);
            g_variant_unref(variant);
        }
        else{
            throw std::runtime_error("Failed to get hw addr");
        }
    }
    catch(const std::exception& e)
    {
        std::string errorText = e.what();

        if(error != nullptr)
        {
            errorText = error->message;
            g_error_free(error);
        }

        if(ethProxy != nullptr){
            g_error_free(error);
        }

        throw std::runtime_error(errorText);
    }

    return hwAddress;
}

InterfaceType InterfaceManagerImpl::nmDevTypeToLocalDevType(const guint &deviceType) const
{
    InterfaceType type =  IF_TYPE_UNKNOWN;

    if(deviceType == NM_DEVICE_TYPE_WIFI || deviceType == NM_DEVICE_TYPE_ETH){
        type = IF_TYPE_ETH;
    }
    else if(deviceType == NM_DEVICE_TYPE_VLAN){
        type = IF_TYPE_TUN;
    }

    return type;
}

std::string InterfaceManagerImpl::getNmInterface(const guint &deviceType) const
{
    std::string type;

    switch(deviceType)
    {
        case NM_DEVICE_TYPE_ETH:
                    type = NM_IFACE_DEVICE_WIRED; break;
        case NM_DEVICE_TYPE_VLAN:
                    type = NM_IFACE_DEVICE_VLAN; break;
        case NM_DEVICE_TYPE_WIFI:
                    type = NM_IFACE_DEVICE_WIFI; break;   
    }

    return type;
}

InterfaceManagerImpl::~InterfaceManagerImpl()
{   
     if(mNetManagerProxy != nullptr){
         g_object_unref (mNetManagerProxy);
     }

     if(mLoop != nullptr)
     {
        stopListening();
        g_main_loop_unref (mLoop);
     }
}
