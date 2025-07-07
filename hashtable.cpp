#include "hashtable.h"
#include <cassert>
#include <cstdlib>

void h_init(HTab* htab, size_t n){
    assert(n > 0 && (n & n-1) == 0);
    htab->tab = (HNode**) calloc(n, sizeof(HNode));
    htab->mask = n-1;
    htab->size = 0;
}

void h_insert(HTab* htab, HNode* node){
    assert(htab->tab && node);
    size_t pos = node->hcode & htab->mask;
    HNode* next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}

void hm_trigger_rehashing(HMap* hmap){
    assert(hmap->older.size == 0);
    hmap->older = hmap->newer;
    h_init(&hmap->newer, (hmap->older.mask + 1) * 2);
    hmap->migrate_pos = 0;
}

size_t k_rehash_work = 128;

HNode* h_detach(HTab* htab, HNode** from){
    assert(htab->tab && *from);
    HNode* node = *from;
    *from = node->next;
    htab->size--;
    return node;
}

void hm_help_rehashing(HMap* hmap){
    assert(hmap->older.tab && hmap->newer.tab);
    size_t work = 0;
    while(hmap->older.size && work < k_rehash_work){ // constant work
        HNode** from = &hmap->older.tab[hmap->migrate_pos];
        if(!*from){
            hmap->migrate_pos++;
            continue;
        }
        h_insert(&hmap->newer, h_detach(&hmap->older, from));
        work++;
    }
    if(!hmap->older.size && hmap->older.tab){
        free(hmap->older.tab);
        hmap->older = HTab{};
    }
}

int K_MAX_LOAD_FACTOR = 8;

void hm_insert(HMap* hmap, HNode* node){
    if(!hmap->newer.tab){
        h_init(&hmap->newer, 4);
    }
    h_insert(&hmap->newer, node);

    if(!hmap->older.tab){
        int threshold = (hmap->newer.mask + 1) * K_MAX_LOAD_FACTOR;
        if(hmap->newer.size > threshold)
            hm_trigger_rehashing(hmap);
    }

    hm_help_rehashing(hmap);
}
