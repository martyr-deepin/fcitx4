if(_ENABLE_DBUS)
  include_directories(
    ${DBUS_INCLUDE_DIRS})
  link_directories(${DBUS_LIBRARY_DIRS})
else()
  set(uosnotificationitem_noinstall NO_INSTALL)
endif()

set(FCITX_UOSNOTIFICATIONITEM_SOURCES
    uosnotificationitem.c
    dbusmenu.c
    ../dbusstuff/property.c)
fcitx_add_addon_full(uosnotificationitem ${uosnotificationitem_noinstall}
  SCAN SCAN_PRIV
  HEADERS uosnotificationitem.h
  SOURCES ${FCITX_UOSNOTIFICATIONITEM_SOURCES}
  LINK_LIBS ${DBUS_LIBRARIES})
