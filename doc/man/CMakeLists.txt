set(FCITX_MANS
fcitx.1 createPYMB.1 readPYBase.1 readPYMB.1 mb2org.1 scel2org.1 mb2txt.1 txt2mb.1 fcitx-remote.1 fcitx-implugin-service.1 fcitx-gsettingtool.1
)

install(FILES ${FCITX_MANS} DESTINATION "${mandir}/man1")
