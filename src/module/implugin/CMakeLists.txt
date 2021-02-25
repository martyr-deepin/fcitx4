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

add_executable(fcitx-implugin-service-so ${FCITX_IMPLUGIN_SERVICE_SO_SOURCES})
target_link_libraries(fcitx-implugin-service-so fcitx-config fcitx-utils)
install(TARGETS fcitx-implugin-service-so DESTINATION "${bindir}")

add_executable(fcitx-implugin-service-mb ${FCITX_IMPLUGIN_SERVICE_MB_SOURCES})
target_link_libraries(fcitx-implugin-service-mb fcitx-config fcitx-utils)
install(TARGETS fcitx-implugin-service-mb DESTINATION "${bindir}")
