#include "avl.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stddef.h>
#include <stdint.h>
#include <algorithm>

struct Data {
    uint32_t val;
    AVLNode node;
};


//template<typename T, typename M>
//T* container_of(M* ptr){
    //return reinterpret_cast<T*>((reinterpret_cast<char*>(ptr) - offsetof(T, M)));
//}
#define container_of(ptr, T, member)    \
    ((T*)((char*)ptr - offsetof(T, member)))

uint32_t get_val(AVLNode* curr){
    return container_of(curr, Data, node)->val;
}

struct Container {
    AVLNode* root;
};

void avl_verify(AVLNode* parent, AVLNode* node){
    if(!node){
        return;
    }
    avl_verify(node, node->left);
    avl_verify(node, node->right);

    assert(node->parent == parent);

    assert(node->cnt == 1 + avl_cnt(node->left) + avl_cnt(node->right));

    uint32_t l = avl_height(node->left);
    uint32_t r = avl_height(node->right);
    assert(node->height == 1 + std::max(l, r));

    assert((l > r ? l - r : r - l) < 2);
    
    uint32_t node_val = get_val(node);
    if(node->left){
        assert(node->left->parent == node); // redundant
        uint32_t left_val = get_val(node->left);
        assert(left_val <= node_val);
    }
    if(node->right){
        assert(node->right->parent == node);
        uint32_t right_val = get_val(node->right);
        assert(right_val >= node_val);
    }
}

void extract(AVLNode* node, std::multiset<uint32_t> &ms){
    if(!node){
        return;
    }
    extract(node->left, ms);
    ms.insert(get_val(node));
    extract(node->right, ms);
}

void container_verify(Container &c, const std::multiset<uint32_t> &ref){
    avl_verify(nullptr, c.root);
    assert(avl_cnt(c.root) == ref.size());
    std::multiset<uint32_t> extracted;
    extract(c.root, extracted);
    assert(extracted == ref);
}

void add(Container &c, uint32_t val, std::multiset<uint32_t> &ref){
    std::cout << "adding val: " << val << std::endl;
    ref.insert(val);
    Data *d;
    d->val = val;
    avl_init(&d->node);
    AVLNode** from = &c.root;
    AVLNode* parent = nullptr;
    while(from){
        parent = *from;
        if(get_val(*from) > val){
            from = &(*from)->right;
        }
        else{
            from = &(*from)->left;
        }
    }
    *from = &d->node;
    d->node.parent = parent;
    avl_fix(*from);
}

void del(Container &c, uint32_t val, std::multiset<uint32_t> &ref){
    ref.erase(val);
    AVLNode** from = &c.root;
    while(*from){
        uint32_t from_val = get_val(*from);
        if(from_val > val){
            from = &(*from)->left;
        }
        else if(from_val < val){
            from = &(*from)->right;
        }
        else{
            break;
        }
    }
    if(!*from){
        return;
    }
    avl_del(*from);
}

int main(){

    std::cout << "starting test" << '\n';

    Container c { nullptr };
    std::multiset<uint32_t> ref;
    container_verify(c, ref);
    add(c, 10, ref);
    container_verify(c, ref);
    add(c, 20, ref);
    container_verify(c, ref);
    del(c, 30, ref);
    container_verify(c, ref);
    del(c, 20, ref);
    container_verify(c, ref);
    del(c, 10, ref);
    container_verify(c, ref);
    del(c, 10, ref);
    container_verify(c, ref);

    std::cout << "finished test" << '\n';
    

}

