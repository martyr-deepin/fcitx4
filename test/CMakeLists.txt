SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/lib)
include_directories(${PROJECT_SOURCE_DIR}/src/im/pinyin)
include_directories(${PROJECT_SOURCE_DIR}/src/module/unicode)
include_directories(${PROJECT_SOURCE_DIR}/src/module/dbus)
include_directories(${PROJECT_SOURCE_DIR}/test)

add_executable(testmain 
          testmain.c 
          testarray.c 
          testbacktrace.c 
          testcast.c
          testhash.c
          testmessage.c
          testobjpool.c
          testpinyin.c
          ../src/im/pinyin/pyParser.c
          ../src/im/pinyin/pyMapTable.c
          ../src/im/pinyin/PYFA.c
          ../src/im/pinyin/sp.c
          testsort.c
          teststring.c
          testutf8.c
          testxdg.c
          testhandlertable.c
          testhotkey.c
          )

target_link_libraries(testmain fcitx-utils fcitx-config fcitx-core)

add_test(NAME testmain
         COMMAND testmain)

add_executable(testconfig testconfig.c)
target_link_libraries(testconfig fcitx-config)

add_executable(testunicode testunicode.c ../src/module/unicode/charselectdata.c)
target_link_libraries(testunicode fcitx-utils fcitx-config)

if(_ENABLE_DBUS)
  include_directories(${DBUS_INCLUDE_DIRS})
  link_directories(${DBUS_LIBRARY_DIRS})
  add_executable(testdbussocket testdbussocket.c
                 ../src/module/dbus/dbussocket.c)
  target_link_libraries(testdbussocket ${DBUS_LIBRARIES} fcitx-utils)
  add_test(NAME testdbussocket COMMAND testdbussocket)
endif()

add_test(NAME testconfig
         COMMAND testconfig ${CMAKE_CURRENT_SOURCE_DIR}/test.desc ${CMAKE_CURRENT_SOURCE_DIR}/test.conf ${CMAKE_CURRENT_BINARY_DIR}/test.conf)

add_test(NAME testunicode
         COMMAND testunicode ${PROJECT_SOURCE_DIR}/src/module/unicode/charselectdata)
