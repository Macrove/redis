#pragma once

#include <cstddef>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

// hashtable node. this should be embeded in the payload
struct HNode {
    HNode *next = nullptr;
    uint64_t hcode = 0;
};

struct HTab {
    HNode** tab; // array of slots.
    size_t mask; // power of 2. 2^n - 1
    size_t size; // num elements in tab
};

// hastable interface. need to use 2 tabs
// newer + older for progressive rehashing
struct HMap {
    HTab older;
    HTab newer;
    size_t migrate_pos = 0;
};

HNode* hm_lookup(HMap* hmap, HNode* key, bool (*eq)(HNode* , HNode*));
void hm_insert(HMap* hmap, HNode* node);
HNode* hm_delete(HMap* hmap, HNode* node, bool (*eq)(HNode*, HNode*));
void hm_clear(HMap* hmap);
void hm_size(HMap* hmap);
