#include <stdio.h>
#include "testarray.h"
#include "testbacktrace.h"
#include "testcast.h"
#include "testenv.h"
#include "testhash.h"
#include "testmessage.h"
#include "testobjpool.h"
#include "testpinyin.h"
#include "testsort.h"
#include "teststring.h"
#include "testutf8.h"
#include "testxdg.h"
#include "testhandlertable.h"
#include "testhotkey.h"

//#include "testdbussocket.h"
//#include "testconfig.h"
//#include "testrewritefile.h"
//#include "testunicode.h"

int main(int argc, char **argv)
{
    test_array();
    test_backtrace();
    test_cast();
    test_env();
    test_hash();
    test_message();
    test_objpool();
    test_pinyin();
    test_sort();
    test_string();
    test_string_hash_set();
    test_string_map();
    test_utf8();
    test_xdg();
    test_handlertable();
    test_hotkey();

// test_dbussocket();
// test_config(int argc, char* argv[]);
// test_rewritefile();
// test_unicode(int argc, char* argv[]);

    return EXIT_SUCCESS;
}
