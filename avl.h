#pragma once

#include <cstddef>
#include <cstdint>

struct AVLNode {
    AVLNode* parent = nullptr;
    AVLNode* left = nullptr;
    AVLNode* right = nullptr;
    uint32_t height = 1;  // Height of the subtree rooted at this node
    uint32_t count = 1;   // Total number of nodes in the subtree (including self)
};

// Initializes a given node as a standalone AVL tree node
inline void avl_init(AVLNode* node) {
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    node->count = 1;
}

// Helper to get height of a node (0 for null)
inline uint32_t avl_height(const AVLNode* node) {
    return node ? node->height : 0;
}

// Helper to get size of subtree rooted at node (0 for null)
inline uint32_t avl_count(const AVLNode* node) {
    return node ? node->count : 0;
}

// API Declarations
AVLNode* avl_fix(AVLNode* node);                  // Recalculates and rebalances
AVLNode* avl_del(AVLNode* node);                  // Deletes node and returns new root
AVLNode* avl_offset(AVLNode* node, int64_t k);    // Returns k-th node in in-order traversal
