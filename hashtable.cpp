#include <cassert>
#include <cstdlib>
#include <functional>
#include "hashtable.h"
#include "utils.h"

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

HNode* h_detach(HTab* htab, HNode** from){
    assert(htab->tab && *from);
    HNode* node = *from;
    *from = node->next;
    htab->size--;
    return node;
}

size_t k_rehash_work = 128;

void hm_help_rehashing(HMap* hmap){
    //msg("hm_help_rehashing");
    size_t work = 0;
    while(hmap->older.size > 0 && work < k_rehash_work){ // constant work
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

HNode** h_lookup(HTab* tab, HNode* key, std::function<bool(HNode*, HNode*)> eq){
    //msg("h_lookup");
    if(!tab->tab) return nullptr;
    size_t pos = tab->mask & key->hcode;
    //msg("pos: %d\n", pos);
    HNode** from = &tab->tab[pos];
    for( ; (*from) != nullptr; from = &(*from)->next){
        //msg("hcode: %d", (*from)->hcode);
        if(key->hcode == (*from)->hcode && eq(*from, key)) return from;
    }
    //msg("not found");
    return nullptr;
}

HNode* hm_lookup(HMap* hmap, HNode* key, std::function<bool(HNode*, HNode*)> eq){
    //msg("hm_lookup");
    hm_help_rehashing(hmap);
    HNode** from = h_lookup(&hmap->newer, key, eq);
    if(!from)
    {
        from = h_lookup(&hmap->older, key, eq);
    }
    return from ? *from : nullptr;
}

HNode* hm_delete(HMap* hmap, HNode* node, std::function<bool(HNode*, HNode*)> eq){
    hm_help_rehashing(hmap);
    ;
    if(HNode** from = h_lookup(&hmap->newer, node, eq)){
        return h_detach(&hmap->newer, from);
    }
    if(HNode** from = h_lookup(&hmap->older, node, eq)){
        return h_detach(&hmap->older, from);
    }
    return nullptr;
}

void hm_clear(HMap* hmap){
    free(hmap->older.tab);
    free(hmap->newer.tab);
    *hmap = HMap{};
}

size_t hm_size(HMap* hmap){
    return hmap->newer.size + hmap->older.size;
}

void h_foreach(HTab* htab, std::function<bool(HNode*)> cb){
    for(int pos = 0; pos < htab->mask; pos++){
        HNode* curr = htab->tab[pos];
        if(curr){
            while(curr){
                cb(curr);
                curr = curr->next;
            }
        }
    }
}

void hm_foreach(HMap* hmap, std::function<bool(HNode* node)> cb){
    if(hmap->newer.tab)
        h_foreach(&hmap->newer, cb);
    if(hmap->older.tab)
        h_foreach(&hmap->older, cb);
}
