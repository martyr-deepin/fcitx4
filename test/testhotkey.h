#include <assert.h>
#include "fcitx-config/hotkey.h"

#define TEST_HOTKEY_UNIFICATION(ORGSYM, ORGSTATE, OBJSYM, OBJSTATE) \
    do { \
        FcitxKeySym sym = ORGSYM; \
        unsigned int state = ORGSTATE; \
        FcitxHotkeyGetKey(sym, state, &sym, &state); \
        assert(OBJSYM == sym); \
        assert(OBJSTATE == state); \
    } while(0)

int test_hotkey();