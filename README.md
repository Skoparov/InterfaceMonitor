# InterfaceMonitor
A simple network interface monitor for Linux, based on signals from NetworkManager. 
Updates interface information in real time.

Requires the following packets: dbus libdbus-1-dev libdbus-glib-1-dev libdbus-glib-1-2

Tests are fully automatic, but require the following start parameters:
- File with a sample of an interface list actual for the current system 
- File with a sample of correct program output after adding\deleting an interface
- Name of the base interface that will be used to create and delete a test vlan interface

Test exec example:
./interfaceMonitorTests list_pattern state_pattern eth0.0
