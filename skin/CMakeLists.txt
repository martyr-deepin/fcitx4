
foreach(skin 默认 经典 黑暗)
    install(DIRECTORY ${skin} DESTINATION ${pkgdatadir}/skin
             PATTERN .* EXCLUDE
             PATTERN *~ EXCLUDE)
endforeach(skin 默认 经典 黑暗)

install(DIRECTORY imicon DESTINATION ${pkgdatadir}
            PATTERN .* EXCLUDE
            PATTERN *~ EXCLUDE)
