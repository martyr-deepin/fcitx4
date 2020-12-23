#include "fcitx-utils/utils.h"
#include <assert.h>

struct my_struct {
    int id; /* key */
    char name[10];
    UT_hash_handle hh; /* makes this structure hashable */
};

struct my_struct *users = NULL;

void add_user(int user_id, char *name) {
    struct my_struct *s;

    HASH_FIND_INT(users, &user_id, s); /* id already in the hash? */
    if (s == NULL) {
        s = (struct my_struct *)malloc(sizeof(struct my_struct));
        s->id = user_id;
        HASH_ADD_INT(users, id, s); /* id: name of key field */
    }
    strcpy(s->name, name);
}

struct my_struct *find_user(int user_id) {
    struct my_struct *s;

    HASH_FIND_INT(users, &user_id, s); /* s: output pointer */
    return s;
}

void delete_user(struct my_struct *user) {
    HASH_DEL(users, user); /* user: pointer to deletee */
    free(user);
}

void delete_all() { HASH_CLEAR(hh, users); }

void print_users() {
    struct my_struct *s;

    for (s = users; s != NULL; s = (struct my_struct *)(s->hh.next)) {
        printf("user id %d: name %s\n", s->id, s->name);
    }
}

int name_sort(struct my_struct *a, struct my_struct *b) {
    return strcmp(a->name, b->name);
}

int id_sort(struct my_struct *a, struct my_struct *b) {
    return (a->id - b->id);
}

void sort_by_name() { HASH_SORT(users, name_sort); }

void sort_by_id() { HASH_SORT(users, id_sort); }

int main(int argc, char *argv[]) {
    int id;
    int i;
    char *c;
    struct my_struct *s;
    unsigned num_users;

    // add user
    add_user(1, "a");

    // add/rename user by id
    add_user(1, "b");
    add_user(2, "c");
    add_user(4, "d");
    add_user(3, "e");

    // find user
    s = find_user(1);
    assert(strcmp(s->name, "b") == 0);

    // delete user
    s = find_user(1);
    if (s)
        delete_user(s);

    // sort items by id
    sort_by_id();

    // verify users id sort
    for (s = users, id = 2; s != NULL;
         s = (struct my_struct *)(s->hh.next), id++) {
        assert(id == s->id);
    }

    // sort items by name
    sort_by_name();

    // verify users name sort
    for (s = users, i = 0; s != NULL;
         s = (struct my_struct *)(s->hh.next), i++) {
        switch (i) {
        case 0:
            c = "c";
            break;
        case 1:
            c = "d";
            break;
        case 2:
            c = "e";
            break;
        }
        assert(strcmp(s->name, c) == 0);
    }

    // count users
    num_users = HASH_COUNT(users);
    assert(num_users == 3);

    // delete all users
    delete_all();
    num_users = HASH_COUNT(users);
    assert(num_users == 0);

    return 0;
}
