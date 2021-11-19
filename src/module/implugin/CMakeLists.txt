set(FCITX_IMPLUGIN_SOURCES
  implugin.c)

fcitx_add_addon_full(implugin DESC
  SOURCES ${FCITX_IMPLUGIN_SOURCES})

set(FCITX_IMPLUGIN_SERVICE_SOURCES
  implugin-service.c
  minGlue.h
  minIni.h
  minIni.c
  )

set(FCITX_GSETTINGTOOL_SOURCES
  gsettings-tool.c
  minGlue.h
  minIni.h
  minIni.c
  )

include(FindPkgConfig)
PKG_CHECK_MODULES(DBUS "dbus-1>=1.1.0" )
PKG_CHECK_MODULES(GLIB2 REQUIRED "glib-2.0" )
PKG_CHECK_MODULES(GIO2 REQUIRED "gio-2.0>=2.26")

include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO2_INCLUDE_DIRS})
link_directories(${GIO2_LIBRARY_DIRS})

add_executable(fcitx-implugin-service ${FCITX_IMPLUGIN_SERVICE_SOURCES})
target_link_libraries(fcitx-implugin-service fcitx-config fcitx-utils dbus-1 dbus-glib-1 glib-2.0 ${GIO2_LIBRARIES})
install(TARGETS fcitx-implugin-service DESTINATION "${bindir}")

add_executable(fcitx-gsettingtool ${FCITX_GSETTINGTOOL_SOURCES})
target_link_libraries(fcitx-gsettingtool fcitx-config fcitx-utils dbus-1 dbus-glib-1 glib-2.0 ${GIO2_LIBRARIES})
install(TARGETS fcitx-gsettingtool DESTINATION "${bindir}")
