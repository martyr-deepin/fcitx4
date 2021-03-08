
set(FCITX_IMPLUGIN_SOURCES
  implugin.c)

fcitx_add_addon_full(implugin DESC
  SOURCES ${FCITX_IMPLUGIN_SOURCES})

set(FCITX_IMPLUGIN_SERVICE_SO_SOURCES
  implugin-service-so.c
  minGlue.h
  minIni.h
  minIni.c
  )

set(FCITX_IMPLUGIN_SERVICE_MB_SOURCES
  implugin-service-mb.c
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

add_executable(fcitx-implugin-service-so ${FCITX_IMPLUGIN_SERVICE_SO_SOURCES})
target_link_libraries(fcitx-implugin-service-so fcitx-config fcitx-utils dbus-1 dbus-glib-1)
install(TARGETS fcitx-implugin-service-so DESTINATION "${bindir}")

add_executable(fcitx-implugin-service-mb ${FCITX_IMPLUGIN_SERVICE_MB_SOURCES})
target_link_libraries(fcitx-implugin-service-mb fcitx-config fcitx-utils dbus-1 dbus-glib-1)
install(TARGETS fcitx-implugin-service-mb DESTINATION "${bindir}")
