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

add_executable(fcitx-implugin-service ${FCITX_IMPLUGIN_SERVICE_SOURCES})
target_link_libraries(fcitx-implugin-service fcitx-config fcitx-utils)
install(TARGETS fcitx-implugin-service DESTINATION "${bindir}")
