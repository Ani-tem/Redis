#include <assert.h>
#include <stdlib.h>     // calloc(), free()
#include "hashtable.h"

// Ensure `n` is a power of 2
static void h_init(HTab *htab, size_t n) {
    assert(n > 0 && ((n - 1) & n) == 0);
    htab->tab = (HNode **)calloc(n, sizeof(HNode *));
    htab->mask = n - 1;
    htab->size = 0;
}

// Insert node into hashtable
static void h_insert(HTab *htab, HNode *node) {
    size_t pos = node->hcode & htab->mask;
    node->next = htab->tab[pos];
    htab->tab[pos] = node;
    htab->size++;
}

// Find node by key and return pointer to pointer to it (for deletion)
static HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *)) {
    if (!htab->tab) return NULL;

    size_t pos = key->hcode & htab->mask;
    HNode **from = &htab->tab[pos];

    for (HNode *cur = *from; cur; cur = *from) {
        if (cur->hcode == key->hcode && eq(cur, key)) {
            return from;
        }
        from = &cur->next;
    }
    return NULL;
}

// Detach a node from the hashtable (removal helper)
static HNode *h_detach(HTab *htab, HNode **from) {
    HNode *node = *from;
    *from = node->next;
    htab->size--;
    return node;
}

const size_t k_rehashing_work = 128;

// Migrate some entries from older table to newer during rehashing
static void hm_help_rehashing(HMap *hmap) {
    size_t work = 0;

    while (work < k_rehashing_work && hmap->older.size > 0) {
        HNode **from = &hmap->older.tab[hmap->migrate_pos];

        if (!*from) {
            hmap->migrate_pos++;
            continue;
        }

        HNode *moved = h_detach(&hmap->older, from);
        h_insert(&hmap->newer, moved);
        work++;
    }

    if (hmap->older.size == 0 && hmap->older.tab) {
        free(hmap->older.tab);
        hmap->older = (HTab){0};
    }
}

// Start rehashing by doubling table size
static void hm_trigger_rehashing(HMap *hmap) {
    assert(hmap->older.tab == NULL);
    hmap->older = hmap->newer;
    h_init(&hmap->newer, (hmap->older.mask + 1) * 2);
    hmap->migrate_pos = 0;
}

// Lookup a key in the hashmap
HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
    hm_help_rehashing(hmap);

    HNode **from = h_lookup(&hmap->newer, key, eq);
    if (!from) {
        from = h_lookup(&hmap->older, key, eq);
    }

    return from ? *from : NULL;
}

const size_t k_max_load_factor = 8;

// Insert a key into the hashmap
void hm_insert(HMap *hmap, HNode *node) {
    if (!hmap->newer.tab) {
        h_init(&hmap->newer, 4);
    }

    h_insert(&hmap->newer, node);

    if (!hmap->older.tab) {
        size_t threshold = (hmap->newer.mask + 1) * k_max_load_factor;
        if (hmap->newer.size >= threshold) {
            hm_trigger_rehashing(hmap);
        }
    }

    hm_help_rehashing(hmap);
}

// Delete a key from the hashmap
HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
    hm_help_rehashing(hmap);

    HNode **from = h_lookup(&hmap->newer, key, eq);
    if (from) return h_detach(&hmap->newer, from);

    from = h_lookup(&hmap->older, key, eq);
    return from ? h_detach(&hmap->older, from) : NULL;
}

// Clear all hashmap memory
void hm_clear(HMap *hmap) {
    free(hmap->newer.tab);
    free(hmap->older.tab);
    *hmap = (HMap){0};
}

// Total number of entries in hashmap
size_t hm_size(HMap *hmap) {
    return hmap->newer.size + hmap->older.size;
}

// Apply a function to all elements in a table
static bool h_foreach(HTab *htab, bool (*f)(HNode *, void *), void *arg) {
    if (!htab->tab) return true;

    for (size_t i = 0; i <= htab->mask; i++) {
        for (HNode *node = htab->tab[i]; node; node = node->next) {
            if (!f(node, arg)) return false;
        }
    }
    return true;
}

// Apply function to all elements in hashmap (newer and older)
void hm_foreach(HMap *hmap, bool (*f)(HNode *, void *), void *arg) {
    h_foreach(&hmap->newer, f, arg);
    h_foreach(&hmap->older, f, arg);
}
