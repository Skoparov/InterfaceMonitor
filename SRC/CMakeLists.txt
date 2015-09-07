cmake_policy (SET CMP0015 NEW)

function (add_project NAME TYPE ALLSOURCES)		

    set (ALLSOURCES ${ALLSOURCES} ${ARGN})    
    add_definitions("-std=c++11")

    foreach (CUR_SRC_FILE ${ALLSOURCES})
        if (CUR_SRC_FILE MATCHES "\\.h$")
            set (HEADERS ${HEADERS} ${CUR_SRC_FILE})                   
        elseif(CUR_SRC_FILE MATCHES "\\.cpp$")
            set(CPP_FILES ${CPP_FILES} ${CUR_SRC_FILE})
        endif()
    endforeach()				             

    if( TYPE STREQUAL "SHARED_LIB")         
        add_library(${NAME} SHARED ${ALLSOURCES})
        link_directories (${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${GLIB2_LINK_DIR})
        target_link_libraries(${NAME}
                             ${DBUS_LIBRARY}
                             ${GLIB2_LIBRARIES})

    elseif(TYPE STREQUAL "BIN")		    	       
        add_executable(${NAME} ${ALLSOURCES})
        target_link_libraries(${NAME} interfaceManager)

    elseif( TYPE STREQUAL "TEST")        
	add_definitions (-DBOOST_TEST_DYN_LINK)       
        add_executable(${NAME} ${ALLSOURCES})
	target_link_libraries(${NAME} interfaceManager)
	link_directories (${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        set_target_properties(${NAME} PROPERTIES LINKER_LANGUAGE CXX)

    else()
        message(FATAL_ERROR "Unknown project type '${TYPE}'")
    endif()

    target_link_libraries(${NAME} ${Boost_LIBRARIES})

    source_group ("Header Files" FILES ${HEADERS})
    source_group ("Source Files" FILES ${CPP_FILES})
endfunction(add_project)

add_subdirectory (InterfaceManager)
add_subdirectory (InterfaceMonitor)
add_subdirectory (Tests)

