# Pre-initialize the QPC SDK
# This file must be included prior to the project() call

# Note: this file is perhaps named badly, as it provides a method qpc_sdk_init which
# the enclosing project calls LATER to actually "initialize" the SDK (by including the CMakeLists.txt from this
# same directory)

if (NOT TARGET _qpc_sdk_pre_init_marker)
    add_library(_qpc_sdk_pre_init_marker INTERFACE)

    function(qpc_is_top_level_project VAR)
        string(TOLOWER ${CMAKE_CURRENT_LIST_DIR} __list_dir)
        string(TOLOWER ${CMAKE_SOURCE_DIR} __source_dir)
        if (__source_dir STREQUAL __list_dir)
            set(${VAR} 1 PARENT_SCOPE)
        else()
            set(${VAR} 0 PARENT_SCOPE)
        endif()
    endfunction()

    function(qpc_message_debug MESSAGE)
        # The log-level system was added in CMake 3.15.
        if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
            message(${MESSAGE})
        else()
            message(DEBUG ${MESSAGE})
        endif()
    endfunction()

    if (NOT QPC_SDK_PATH)
        set(QPC_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
    endif ()

    file(REAL_PATH "${QPC_SDK_PATH}" QPC_SDK_PATH BASE_DIRECTORY "${CMAKE_BINARY_DIR}")

    set(QPC_SDK_PATH ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "Path to the QP/C SDK" FORCE)

    list(APPEND CMAKE_MODULE_PATH ${QPC_SDK_PATH}/cmake)

    message("QPC_SDK_PATH is ${CMAKE_CURRENT_LIST_DIR}")

    function(qpc_sdk_init)
        if (NOT CMAKE_PROJECT_NAME)
            message(WARNING "qpc_sdk_init() should be called after the project is created (and languages added)")
        endif()

        add_subdirectory(${QPC_SDK_PATH} qpc-sdk)
    endfunction()
endif()
