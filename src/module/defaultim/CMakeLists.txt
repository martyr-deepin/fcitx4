set(FCITX_DEFAULTIM_SOURCES
  defaultim.c)

fcitx_add_addon_full(defaultim DESC
  SOURCES ${FCITX_DEFAULTIM_SOURCES})

set(SWITCH_FCITX_DEFAULTIM_SOURCES
  minIni.c
  minIni.h
  minGlue.h
  switch-fcitx-default.c)

add_executable(switch-fcitx-default ${SWITCH_FCITX_DEFAULTIM_SOURCES})
target_link_libraries(switch-fcitx-default fcitx-config fcitx-utils)
install(TARGETS switch-fcitx-default DESTINATION "${bindir}")