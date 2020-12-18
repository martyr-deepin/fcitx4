#include "fcitx-utils/utils.h"
#include "fcitx/fcitx.h"
#include <assert.h>

typedef struct {
    int a;
    int b;
} intpair_t;

UT_icd intpair_icd = {sizeof(intpair_t), NULL, NULL, NULL};

int main() {
    UT_array array;
    utarray_init(&array, &ut_int_icd);
    int test1[] = {3, 1, 2};
    int test2[] = {1, 2, 3};
    int test3[] = {1, 2};
    int test4[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    char *test5[] = {"Free ", "Chinese ", "Input ", "Toy ", "for ", "X"};
    int test6a[] = {1, 10};
    int test6b[] = {2, 20};
    FCITX_UNUSED(test1);
    FCITX_UNUSED(test2);
    FCITX_UNUSED(test3);
    FCITX_UNUSED(test4);
    FCITX_UNUSED(test5);
    int i1;
    i1 = 1;
    utarray_push_back(&array, &i1);
    i1 = 2;
    utarray_push_back(&array, &i1);
    i1 = 3;
    utarray_push_back(&array, &i1);
    utarray_move(&array, 2, 0);
    {
        utarray_foreach(p, &array, int) {
            assert(*p == test1[utarray_eltidx(&array, p)]);
        }
    }
    utarray_move(&array, 0, 2);

    {
        utarray_foreach(p, &array, int) {
            assert(*p == test2[utarray_eltidx(&array, p)]);
        }
    }
    utarray_pop_back(&array);
    {
        utarray_foreach(p, &array, int) {
            assert(*p == test3[utarray_eltidx(&array, p)]);
        }
    }
    utarray_done(&array);

    UT_array *nums;
    int i2, *p2;

    utarray_new(nums, &ut_int_icd);
    for (i2 = 0; i2 < 10; i2++)
        utarray_push_back(nums, &i2);

    for (p2 = (int *)utarray_front(nums); p2 != NULL;
         p2 = (int *)utarray_next(nums, p2)) {
        assert(*p2 == test4[utarray_eltidx(nums, p2)]);
    }

    utarray_free(nums);

    UT_array *strs;
    char *s, **p3;

    utarray_new(strs, &ut_str_icd);

    s = "Free ";
    utarray_push_back(strs, &s);
    s = "Chinese ";
    utarray_push_back(strs, &s);
    s = "Input ";
    utarray_push_back(strs, &s);
    s = "Toy ";
    utarray_push_back(strs, &s);
    s = "for ";
    utarray_push_back(strs, &s);
    s = "X";
    utarray_push_back(strs, &s);
    p3 = NULL;
    while ((p3 = (char **)utarray_next(strs, p3))) {
        assert(strcmp(*p3, test5[utarray_eltidx(strs, p3)]) == 0);
    }

    utarray_free(strs);

    UT_array *pairs;
    intpair_t ip, *p4;
    utarray_new(pairs, &intpair_icd);

    ip.a = 1;
    ip.b = 2;
    utarray_push_back(pairs, &ip);
    ip.a = 10;
    ip.b = 20;
    utarray_push_back(pairs, &ip);

    for (p4 = (intpair_t *)utarray_front(pairs); p4 != NULL;
         p4 = (intpair_t *)utarray_next(pairs, p4)) {
        assert(p4->a == test6a[utarray_eltidx(pairs, p4)] &&
               p4->b == test6b[utarray_eltidx(pairs, p4)]);
    }

    utarray_free(pairs);
    return 0;
}
