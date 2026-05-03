# Platform Configuration Utilities 
# Ewan Burnett (EwanBurnettSK@Outlook.com)

# Windows 
function(Configure_Win32 CONFIG_TARGET_NAME CONFIG_SOURCE_PATH CONFIG_TARGET_PATH CONFIG_TARGET_DESCRIPTION IS_DEBUG IS_PRERELEASE CONFIG_ICON_PATH)
    set(CONFIG_TARGET_ROOT_DIR ${CMAKE_SOURCE_DIR})

    # Configure and copy the Resources.rc file to the target directory. 
    message(STATUS "[Win32] Configuring Resources.rc...")
    configure_file(
        "${CMAKE_SOURCE_DIR}/Platform/Windows/Resources.rc.in"
        "${CONFIG_TARGET_PATH}/Resources.rc"
    )

    # Clean up .in files...
    file(GLOB_RECURSE TEMP_IN_FILES 
        "${CONFIG_TARGET_PATH}/Windows/*.in"
    )

    message(STATUS "Removing Files...")
    foreach(TEMP_FILE ${TEMP_IN_FILES})
        message(STATUS ">${TEMP_FILE}")
    endforeach()

    file(REMOVE ${TEMP_IN_FILES})

endfunction()
