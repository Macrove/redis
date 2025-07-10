#pragma once

#include <cstddef>
#include <functional>
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

void hm_insert(HMap* hmap, HNode* node);
void hm_clear(HMap* hmap);
size_t hm_size(HMap* hmap);
HNode* hm_lookup(HMap* hmap, HNode* key, std::function<bool(HNode*, HNode*)> eq);
HNode* hm_delete(HMap* hmap, HNode* node, std::function<bool(HNode*, HNode*)> eq);
void hm_foreach(HMap* hmap, std::function<bool(HNode* node)>);
