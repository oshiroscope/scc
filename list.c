/* 
 * list.c
 */

#include <assert.h>
#include <string.h>
#include "util.h"
#include "list.h"



/* 空のリストを作る */
list_t mk_list()
{
    int sz = 1;
    list_t b = (list_t)safe_malloc(sizeof(struct list));
    b->a = (void **)safe_malloc(sz * sizeof(void*));
    b->n = 0;
    b->sz = sz;
    return b;
}

/* 補助関数. b->a が n 要素持つように,
   必要ならば配列を取り直して保証する. */
static void list_ensure_sz(list_t b, int n)
{
    int sz = b->sz;
    if (n >= sz) {
        int new_sz = sz * 2;
        if (new_sz <= sz) new_sz = sz + 1;
        void ** new_a = (void **)safe_malloc(new_sz * sizeof(void *));
        memcpy(new_a, b->a, sz * sizeof(void *));
        b->a = new_a;
        b->sz = new_sz;
    }
}

/* bの後ろにcを加える */
void list_add(list_t b, void * c)
{
    list_ensure_sz(b, b->n + 1);
    int n = b->n;
    assert(n < b->sz);
    b->a[n] = c;
    b->a[n + 1] = 0;
    b->n = n + 1;
}

/* bの第i要素を返す */
void * list_get(list_t b, int i)
{
    assert(i < b->n);
    return b->a[i];
}

/* bに格納されている要素数 */
int list_sz(list_t b)
{
    return b->n;
}

