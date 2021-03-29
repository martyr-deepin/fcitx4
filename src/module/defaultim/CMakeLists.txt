set(FCITX_DEFAULTIM_SOURCES
  defaultim.c)

fcitx_add_addon_full(defaultim DESC
  SOURCES ${FCITX_DEFAULTIM_SOURCES})

set(FCITX_SWITCH_DEFAULTIM_SOURCES
  minIni.c
  minIni.h
  minGlue.h
  fcitx-switch-default.c)

add_executable(fcitx-switch-default ${FCITX_SWITCH_DEFAULTIM_SOURCES})
target_link_libraries(fcitx-switch-default fcitx-config fcitx-utils)
install(TARGETS fcitx-switch-default DESTINATION "${bindir}")
