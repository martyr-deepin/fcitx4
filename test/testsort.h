#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include "fcitx-utils/utarray.h"

typedef struct {
    int a;
    int b;
} SortItem;

int cmp(const void* a, const void* b, void* thunk);

int intcmp(const void* a, const void* b, void* thunk);

int test_sort();