#ifndef INTERFACEMANAGERIMPLLINUX_H
#define INTERFACEMANAGERIMPLLINUX_H

/**
* @file AbstractInterfaceManagerImpl.h
* @brief Contains a linux-based concrete class of InterfaceManager implementation
*/

#include <iostream>
#include "dbus/dbus.h"
#include <dbus/dbus-glib.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "AbstractInterfaceManagerImpl.h"

enum NmDeviceType
{
    NM_DEVICE_TYPE_UNKNOWN = 0,    //The device type is unknown.
    NM_DEVICE_TYPE_ETH = 1,   //The device is wired Ethernet device.
    NM_DEVICE_TYPE_WIFI = 2,       //The device is an 802.11 WiFi device.
    NM_DEVICE_TYPE_VLAN = 11,      //The device is a VLAN interface.
};

#define NM_IFACE_NETWORKMANAGER             "org.freedesktop.NetworkManager"
#define NM_IFACE_NETWORKMANAGER_PATH        "/org/freedesktop/NetworkManager"
#define NM_IFACE_DEVICE                     "org.freedesktop.NetworkManager.Device"
#define NM_IFACE_DEVICE_WIRED               "org.freedesktop.NetworkManager.Device.Wired"
#define NM_IFACE_DEVICE_VLAN                "org.freedesktop.NetworkManager.Device.Vlan"
#define NM_IFACE_DEVICE_WIFI                "org.freedesktop.NetworkManager.Device.Wireless"
#define NM_IFACE_DEVICE_PROPERTY_NAME       "Interface"
#define NM_IFACE_DEVICE_PROPERTY_TYPE       "DeviceType"
#define NM_IFACE_DEVICE_PROPERTY_HWADDR     "HwAddress"

#define NM_SIGNAL_G_SIGNAL                  "g-signal"   /**< connecting to g-signal, we connect to all signals coming from a proxy */
#define NM_SIGNAL_DEVICE_ADDED              "DeviceAdded"
#define NM_SIGNAL_DEVICE_REMOVED            "DeviceRemoved"

#define NM_METHOD_GET_DEVICES               "GetDevices"

////////////////////////////////////////////////////////////
///////            InterfaceManagerImpl           //////////
////////////////////////////////////////////////////////////

class InterfaceManagerImpl : public AbstractInterfaceManagerImpl
{    
public:
    InterfaceManagerImpl();
    ~InterfaceManagerImpl();

    void startListening();
    void stopListening();
    void updateDevices();

private:       

    static void onNetManagerSignal(GDBusProxy *proxy, gchar *sender, gchar* signal, GVariant* params, gpointer data);
    void handleNetManagerSignal(const std::string& signalName, GVariant* params);

    InterfaceInfo getDeviceInfo(const std::string& deviceAddr);
    guint getDeviceType(GDBusProxy* proxy) const;
    std::string getDeviceName(GDBusProxy* proxy) const;
    std::string getDeviceHwAddress(const std::string& deviceAddr, const std::string& nmModuleName) const;

    InterfaceType nmDevTypeToLocalDevType(const guint& deviceType) const;
    std::string getNmInterface(const guint& deviceType) const;  /**< The name of an interface responsible for the device type */

private:     
    GMainLoop *mLoop;    /**< GLib's event loop is required to get their signal system working */
    GDBusProxy* mNetManagerProxy;  
};

#endif // INTERFACEMANAGERIMPLLINUX_H
