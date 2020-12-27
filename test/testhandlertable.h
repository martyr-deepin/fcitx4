#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "fcitx-utils/handler-table.h"

typedef struct {
    int id;
    char *str;
} HandlerObj;

static void
test_free_handler_obj(void *obj);

int test_handlertable();