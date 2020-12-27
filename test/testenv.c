#include "testenv.h"

int test_env()
{
    assert(fcitx_utils_get_boolean_env("GDMSESSION",-2) == 1);
    assert(fcitx_utils_get_boolean_env("GDMSESSIO",-2) == -2);
    return 0;
}
