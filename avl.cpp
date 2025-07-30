#include "avl.h"
#include <algorithm>
#include <stddef.h>
#include <stdint.h>

void fill_height_cnt(AVLNode* node){
    node->height = 1 + std::max(avl_height(node->left), avl_height(node->right));
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}

AVLNode* avl_rot_right(AVLNode* node){
    AVLNode* new_root = node->left;
    AVLNode* parent = node->parent;
    AVLNode** from = nullptr;
    if(parent){
        from = parent->left == node ? &parent->left : &parent->right;
        *from = new_root;
    }
    AVLNode* dangler = new_root->right;
    new_root->right = node;
    new_root->parent = parent;
    node->parent = new_root;
    node->left = dangler;

    if(dangler){
        dangler->parent = node;
    }

    fill_height_cnt(node);
    fill_height_cnt(new_root);

    return new_root;
}

AVLNode* avl_rot_left(AVLNode* node){
    AVLNode* new_root = node->right;
    AVLNode* parent = node->parent;
    AVLNode** from = nullptr;
    if(parent){
        from = parent->left == node ? &parent->left : &parent->right;
        *from = new_root;
    }
    AVLNode* dangler = new_root->left;
    new_root->left = node;
    new_root->parent = parent;

    node->parent = new_root;
    node->right = dangler;

    if(dangler){
        dangler->parent = node;
    }

    fill_height_cnt(node);
    fill_height_cnt(new_root);

    return new_root;
}

AVLNode* avl_fix_left(AVLNode* node){
    if(avl_height(node->left->left) < avl_height(node->left->right)){
        avl_rot_left(node->left);
    }
    return avl_rot_right(node);
}

AVLNode* avl_fix_right(AVLNode* node){
    if(avl_height(node->right->right) < avl_height(node->right->left)){
        avl_rot_right(node->right);
    }
    return avl_rot_left(node);
}

AVLNode* avl_fix(AVLNode* node){
    while(true){
        AVLNode* curr = node;
        AVLNode* parent = curr->parent;
        AVLNode** from = parent->left == curr ? &parent->left : &parent->right;
        uint32_t l = avl_height(curr->left);
        uint32_t r = avl_height(curr->right);
        if(l==r+2){
            curr = avl_fix_left(curr);
        }
        else if(l+2==r){
            curr = avl_fix_right(curr);
        }
        else{
            node = curr->parent;
            continue;
        }
        if(!parent){
            return curr;
        }
        *from = curr;
        if(curr->parent){
            node = curr->parent;
        }
        else{
            break;
        }
    }
    return node;
}

AVLNode* avl_zero_or_one_child_del(AVLNode* node){
    AVLNode* child = node->left ? node->left : node->right;
    AVLNode* parent = node->parent;
    if(parent){
        AVLNode** from = parent->left == node ? &parent->left : &parent->right;
        *from = child;
    }
    child->parent = parent;

    fill_height_cnt(child);

    if(parent){
        return avl_fix(parent);
    }
    return child;
}

AVLNode* avl_two_children_del(AVLNode* node){
    AVLNode* parent = node->parent;
    if(parent){
        AVLNode** from = parent->left == node ? &parent->left : &parent->right;
        *from = node->left;
    }
    node->left->parent = parent;

    AVLNode* right_most = node->left;
    while(right_most->right){
        right_most = right_most->right;
    }
    right_most->right = node->right;
    node->right->parent = right_most;

    return avl_fix(right_most);
}

AVLNode* avl_del(AVLNode* node){
    if(!node->left || !node->right){
        return avl_zero_or_one_child_del(node);
    }
    return avl_two_children_del(node);
}
