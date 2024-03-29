cmake_minimum_required(VERSION 3.6)

project(fcitx)

set(version 4.2.9.32.32)

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${ECM_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

set_property(GLOBAL PROPERTY "__FCITX_INTERNAL_BUILD" ON)

option(ENABLE_X11 "Enable X11 as dependency" ON)
option(ENABLE_GLIB2 "Enable GLib2 as depenedency" ON)
option(ENABLE_CAIRO "Enable Cairo" ON)
option(ENABLE_DBUS "Enable DBus Support" ON)
option(ENABLE_PANGO "Enable Pango Support" ON)
option(ENABLE_XKB "Enable xkb support" ON)
option(ENABLE_DEBUG "Enable Debug" OFF)
option(ENABLE_PINYIN "Enable Pinyin" ON)
option(ENABLE_TABLE "Enable Table IM" ON)
option(ENABLE_GTK2_IM_MODULE "Enable GTK2 IM Module" ON)
option(ENABLE_GTK3_IM_MODULE "Enable GTK3 IM Module" ON)
option(ENABLE_QT "Enable Qt-based lib" ON)
option(ENABLE_QT_IM_MODULE "Enable Qt IM Module" ON)
option(ENABLE_QT_GUI "Enable Qt-based configuration GUI" ON)
option(ENABLE_OPENCC "Enable OpenCC for Chinese Transform" ON)
option(FORCE_OPENCC
  "Enable opencc even if the library is not found at compile time" OFF)
option(ENABLE_LUA "Enable Lua support" ON)
option(ENABLE_STATIC "Build Static Library" OFF)
option(ENABLE_TEST "Test build" ON)
option(ENABLE_SNOOPER "Enable Key Snooper for gtk app" ON)
option(ENABLE_GIR "Enable GObject Introspection" ON)
option(ENABLE_ENCHANT "Enable enchant for word predication" ON)
option(ENABLE_PRESAGE "Enable presage for word predication" ON)
option(FORCE_ENCHANT
  "Enable enchant even if the library is not found at compile time" OFF)
option(FORCE_PRESAGE
  "Enable presage even if the library is not found at compile time" OFF)
option(ENABLE_BACKTRACE "Enable backtrace support" ON)
option(ENABLE_XDGAUTOSTART "Enable xdg autostart desktop file installation" ON)
option(ENABLE_GETTEXT "Enable gettext support." ON)
option(ENABLE_SPELL "Enable Spell Addon" ON)

if(NOT NO_SNOOPER_APPS)
  set(NO_SNOOPER_APPS ".*chrome.*,.*chromium.*,firefox.*,Do.*"
    CACHE STRING "Disable Key Snooper for following app")
endif()

if(NOT NO_PREEDIT_APPS)
  set(NO_PREEDIT_APPS "gvim"
    CACHE STRING "Disable Preedit for the following apps")
endif()

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(FindPkgConfig)
include(UseDoxygen OPTIONAL)
include(FcitxMacro)

_fcitx_add_uninstall_target()
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(malloc.h HAVE_MALLOC_H)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_function_exists(asprintf HAVE_ASPRINTF)

find_package(Libintl REQUIRED)
find_package(Libiconv REQUIRED)
find_package(Pthread REQUIRED)
find_package(Libexecinfo)
find_package(GettextPo)
find_package(XKBCommon 0.5.0 REQUIRED COMPONENTS XKBCommon)

if(NOT ENABLE_GETTEXT OR NOT GETTEXTPO_FOUND)
  fcitx_disable_gettext()
endif()

if(LIBEXECINFO_LIB_FOUND)
  set(ENABLE_BACKTRACE 1)
else()
  set(ENABLE_BACKTRACE 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "BSD")
  find_package(Libkvm REQUIRED)
endif(${CMAKE_SYSTEM_NAME} MATCHES "BSD")
find_package(DL REQUIRED)
if(ENABLE_ENCHANT)
  find_package(Enchant)
  if(NOT (ENCHANT_FOUND OR FORCE_ENCHANT))
    set(ENABLE_ENCHANT Off)
  endif()
endif()
if (ENABLE_XKB)
    # Parse xkb xml file
    find_package(LibXml2 REQUIRED)
    find_package(IsoCodes REQUIRED)
    find_package(XkbFile REQUIRED)
    # Parse isocodes json
    pkg_check_modules(JsonC "json-c" IMPORTED_TARGET REQUIRED)
endif (ENABLE_XKB)
if(ENABLE_PRESAGE)
  find_package(Presage)
  if(NOT (PRESAGE_FOUND OR FORCE_PRESAGE))
    set(ENABLE_PRESAGE Off)
  endif()
endif()

if(ENABLE_QT)
  find_package(Qt4 4.8 COMPONENTS QtCore QtGui QtDBus REQUIRED)
endif()

if(ENABLE_X11)
find_package(X11 REQUIRED)

if (NOT X11_Xkb_FOUND)
    message(FATAL_ERROR "Xkb is needed")
endif (NOT X11_Xkb_FOUND)

find_package(XKeyboardConfig REQUIRED)

endif()

if (ENABLE_TEST)
    enable_testing()
endif ()

set(_ENABLE_CAIRO 0)
set(_ENABLE_PANGO 0)
if (ENABLE_X11 AND ENABLE_CAIRO)
    PKG_CHECK_MODULES(CAIRO_XLIB REQUIRED "cairo-xlib>=1.0" )
    if (CAIRO_XLIB_FOUND)
        set(_ENABLE_CAIRO 1)
    endif (CAIRO_XLIB_FOUND)
endif (ENABLE_X11 AND ENABLE_CAIRO)

set(_ENABLE_PANGO 0)
if (_ENABLE_CAIRO AND ENABLE_GLIB2)
    if (ENABLE_PANGO)
        PKG_CHECK_MODULES(PANGOCAIRO "pangocairo>=1.0" )
        if (PANGOCAIRO_FOUND)
            set(_ENABLE_PANGO 1)
        endif (PANGOCAIRO_FOUND)
    endif (ENABLE_PANGO)
    if (NOT _ENABLE_PANGO)
        PKG_CHECK_MODULES(FONTCONFIG REQUIRED "fontconfig")
    endif (NOT _ENABLE_PANGO)
endif(_ENABLE_CAIRO AND ENABLE_GLIB2)

set(_ENABLE_DBUS 0)
if (ENABLE_DBUS)
    PKG_CHECK_MODULES(DBUS "dbus-1>=1.1.0" )
    if (DBUS_FOUND)
        set(_ENABLE_DBUS 1)
    endif (DBUS_FOUND)
endif (ENABLE_DBUS)

if(ENABLE_OPENCC)
    find_package(OpenCC)
    if(NOT (OPENCC_FOUND OR FORCE_OPENCC))
        set(ENABLE_OPENCC Off)
    endif()
endif()

set(HAVE_XINERAMA 0)
if (X11_Xinerama_FOUND)
    set(HAVE_XINERAMA 1)
endif (X11_Xinerama_FOUND)

if (X11_Xfixes_FOUND)
    set(HAVE_XFIXES 1)
endif (X11_Xfixes_FOUND)

if(ENABLE_DEBUG)
    set(_DEBUG ON)
endif(ENABLE_DEBUG)

set(package     fcitx)
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(bindir ${exec_prefix}/bin)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "${exec_prefix}/lib" CACHE PATH "Library install directory [default=PREFIX/lib/]")
endif()
if(NOT DEFINED LIBDATA_INSTALL_DIR)
    set(LIBDATA_INSTALL_DIR "${LIB_INSTALL_DIR}"  CACHE PATH "Lib data install directory [default=LIB_INSTALL_DIR]")
endif()
if(NOT DEFINED MAN_INSTALL_DIR)
    set(MAN_INSTALL_DIR "${prefix}/share/man"  CACHE PATH "man install directory [default=PREFIX/share/man]")
endif()
if (NOT DEFINED DBUS_SERVICE_DIR)
    set(DBUS_SERVICE_DIR "${CMAKE_INSTALL_PREFIX}/share/dbus-1/services" CACHE PATH "dbus session service directory [default=PREFIX/share/dbus-1/services]")
endif()
if (NOT DEFINED SYSCONFDIR)
    set(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "sys config directory [default=PREFIX/etc/]")
endif()

set(includedir "${CMAKE_INSTALL_PREFIX}/include")
set(libdir ${LIB_INSTALL_DIR})
set(pkgconfdir ${LIBDATA_INSTALL_DIR}/pkgconfig)
set(mandir ${MAN_INSTALL_DIR})
set(addondir ${libdir}/${package} )
set(pkgdatadir ${prefix}/share/${package})
set(configdescdir ${pkgdatadir}/configdesc)
set(addonconfigdir ${pkgdatadir}/addon)
set(imconfigdir ${pkgdatadir}/inputmethod)
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${LIB_INSTALL_DIR}" _isSystemPlatformLibDir)
list(FIND CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "${LIB_INSTALL_DIR}" _isSystemCxxLibDir)
if("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")
    set(CMAKE_SKIP_BUILD_RPATH  FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif("${_isSystemPlatformLibDir}" STREQUAL "-1" AND "${_isSystemCxxLibDir}" STREQUAL "-1")

# For FcitxMacro
set(FCITX4_ADDON_INSTALL_DIR ${addondir})
set(FCITX4_CONFIGDESC_INSTALL_DIR ${configdescdir})
set(FCITX4_ADDON_CONFIG_INSTALL_DIR ${addonconfigdir})
set(FCITX4_INPUTMETHOD_CONFIG_INSTALL_DIR ${imconfigdir})
set(FCITX4_PACKAGE_NAME ${package})

function(_RESOLVE_LIBRARY_PATH _FILE_NAME _LIBRARY_PATH)
    if (CMAKE_OBJDUMP)
        execute_process (
            COMMAND ${CMAKE_OBJDUMP} -p "${_LIBRARY_PATH}"
            RESULT_VARIABLE STATUS
            OUTPUT_VARIABLE OBJDUMP_RESULT
            ERROR_QUIET
        )
        string(REGEX REPLACE ".*SONAME +([^ ]+)\n.*$" "\\1" SONAME_OUT "${OBJDUMP_RESULT}")
        if (SONAME_OUT)
            set(_FILE_NAME_OUT "${SONAME_OUT}")
        endif ()
    endif()
    if (NOT _FILE_NAME_OUT)
        get_filename_component(_FILE_NAME_OUT "${_LIBRARY_PATH}" NAME)
    endif()
    set(${_FILE_NAME} ${_FILE_NAME_OUT} PARENT_SCOPE)
endfunction()


if (NOT DEFINED OPENCC_LIBRARY_FILENAME)
    if (OPENCC_LIBRARIES)
        _RESOLVE_LIBRARY_PATH(OPENCC_LIBRARY_FILENAME "${OPENCC_LIBRARIES}")
    else ()
        set(OPENCC_LIBRARY_FILENAME
            "${CMAKE_SHARED_LIBRARY_PREFIX}opencc${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif ()
endif()

if(NOT DEFINED ENCHANT_LIBRARY_FILENAME)
    if (ENCHANT_LIBRARIES)
        _RESOLVE_LIBRARY_PATH(ENCHANT_LIBRARY_FILENAME "${ENCHANT_LIBRARIES}")
    else ()
        set(ENCHANT_LIBRARY_FILENAME
            "${CMAKE_SHARED_LIBRARY_PREFIX}enchant${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif ()
endif()
if(NOT DEFINED PRESAGE_LIBRARY_FILENAME)
    if (PRESAGE_LIBRARIES)
        _RESOLVE_LIBRARY_PATH(PRESAGE_LIBRARY_FILENAME "${PRESAGE_LIBRARIES}")
    else ()
        set(PRESAGE_LIBRARY_FILENAME
            "${CMAKE_SHARED_LIBRARY_PREFIX}presage${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif ()
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")
add_definitions("-D_GNU_SOURCE -DFCITX_HAVE_CONFIG_H")

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)
configure_file("${PROJECT_SOURCE_DIR}/cmake/FcitxConfigVersion.cmake.in"
               "${PROJECT_BINARY_DIR}/cmake/FcitxConfigVersion.cmake"
                IMMEDIATE @ONLY
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/src/lib/)

# must be the first
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(skin)
add_subdirectory(tools)
add_subdirectory(data)
add_subdirectory(cmake)

if(ENABLE_TEST)
  add_subdirectory(test)
endif()
