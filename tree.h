#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include <stdlib.h>

#define MEMORY_MASK   32  // 0b100000
#define PERC_MASK     16  // 0b010000
#define ASCII_MASK    8   // 0b001000
#define DEVICE_MASK   4   // 0b000100
#define SORT_MASK     2   // 0b000010
#define NO_COLOR_MASK 1   // 0b000001


struct Node;

typedef struct Node {
    char name[256];
    char path[4096];
    size_t size;
    size_t numChildren;
    struct Node *children;
    bool nonSuccessOpen;
    mode_t type;
    char link[256];
} Node;


typedef struct Tree {
    int options;
    int depth;
    dev_t device;
    Node *root;
} Tree;


/**
 * @brief recount size from bytes to human readible form
 * @param size
 * @param pointer to string where will be stored suffix of correct unit
 * @return recounted size
 */
double convertSize(size_t size, char *unit);


/**
 * @brief prints the directory tree
 * @param path to the root directory
 * @param options
 * @param depth
 * @return zero on success, non-zero otherwise
 */
int printTree(const char *path, const int options, const int depth);


/**
 * @brief loads directory tree and stores it in Tree struct
 * @param pointer to the Node struct of root directory
 * @param tree
 * @return  zero on success, non-zero otherwise
 */
int loadTree(Node *root, Tree *tree);


/**
 * @brief adds Node struct into array of Nodes inside another Node struct
 * @param Nodes struct
 * @param Node
 */
void addChildren(Node *root, Node *node);


/**
 * @brief initializes Tree struct
 * @param tree
 * @param Node struct of root directory
 */
void initTree(Tree *tree, Node *root);


/**
 * @brief initializes Node struct
 * @param node
 */
void initNode(Node *node);


/**
 * @brief initChildren
 * @param children
 */
void initChildren(Node *node);

#endif // TREE_H
