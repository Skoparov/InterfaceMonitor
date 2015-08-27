cmake_policy (SET CMP0015 NEW)

IF (UNIX)
    set(IMPL_SOURCES InterfaceManagerImplLinux.cpp InterfaceManagerImplLinux.h)
ELSEIF(WIN32)
    set(IMPL_SOURCES )
ENDIF()

add_project (interfaceManager
             SHARED_LIB             
             InterfaceManager.cpp
             InterfaceManager.h
             AbstractInterfaceManagerImpl.cpp
             AbstractInterfaceManagerImpl.h
             ${IMPL_SOURCES})
