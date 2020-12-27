#include "fcitx-utils/utils.h"
#include <assert.h>

struct my_struct {
    int id; /* key */
    char name[10];
    UT_hash_handle hh; /* makes this structure hashable */
};


void add_user(int user_id, char *name);

struct my_struct *find_user(int user_id);

void delete_user(struct my_struct *user);

void delete_all();

void print_users();

int name_sort(struct my_struct *a, struct my_struct *b);

int id_sort(struct my_struct *a, struct my_struct *b);

void sort_by_name();

void sort_by_id();

int test_hash();