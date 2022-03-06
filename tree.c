#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tree.h"

#define BRBLUE "\033[1;34m"
#define BRCYAN "\033[1;36m"
#define BRGREEN "\033[1;32m"
#define BRMAGENTA "\033[1;35m"
#define BRRED "\033[1;31m"
#define BRYELLOW "\033[1;33m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"
#define YELLOW "\033[0;33m"


static void toUpper(char *dest, const char *source)
{
    size_t size = strlen(source);
    for (size_t i = 0; i < size; ++i) {
        if (!(source[i] >= 'A' && source[i] <= 'Z')) {
            dest[i] = source[i] - 32;
        } else {
            dest[i] = source[i];
        }
    }
}


static void printNode(mode_t type, Node *node, bool useColor)
{
    int link = false;
    if (useColor) {
        if (node->name[0] == '.') {
            printf(BRRED);
        } else {
            switch (type & S_IFMT) {
            case S_IFREG:
                if (type & S_IXUSR || type & S_IXGRP || type & S_IXOTH)
                    printf(BRGREEN);
                break;
            case S_IFDIR:
                printf(BRBLUE);
                break;
            case S_IFBLK:
                printf(BRCYAN);
                break;
            case S_IFCHR:
                printf(CYAN);
                break;
            case S_IFIFO:
                printf(YELLOW);
                break;
            case S_IFLNK:
                printf(BRMAGENTA);
                link = true;
                break;
            case S_IFSOCK:
                printf(BRYELLOW);
                break;
            }
        }
    }
    printf("%s", node->name);
    if (useColor) {
        printf(RESET);
    }
    if (link) {
        printf(" -> %s", node->link);
    }
    if (node->nonSuccessOpen) {
        printf(" [unable to open dir]");
    }
    printf("\n");
}


static int nameCmp(const void *node1, const void *node2)
{
    const Node *l1 = (const Node *) node1;
    const Node *l2 = (const Node *) node2;

    char *str1Upper = calloc(strlen(l1->name) + 1, sizeof(char));
    char *str2Upper = calloc(strlen(l2->name) + 1, sizeof(char));

    toUpper(str1Upper, l1->name);
    toUpper(str2Upper, l2->name);

    int result = strcmp(str1Upper, str2Upper);
    if (!result) {
        result = strcmp(l1->name, l2->name);
    }

    free(str1Upper);
    free(str2Upper);

    return result;
}


static int sizeCmp(const void *node1, const void *node2)
{
    const Node *l1 = (const Node *) node1;
    const Node *l2 = (const Node *) node2;

    int result = l1->size > l2->size ? -1 : 1;
    result = l1->size == l2->size ? 0 : result;

    if (!result) {
        result = nameCmp(node1, node2);
    }

    return result;
}


double convertSize(size_t size, char *unit)
{
    char *suffixes = "_BKMGTPEZY";
    int iterator = 10;
    double newsize = 0;
    while (1) {
        if (pow(2, iterator) > size) {
            if (iterator == 10) {
                strcpy(unit, "B  ");
            } else {
                strcpy(unit, " iB");
                unit[0] = suffixes[iterator / 10];
            }
            newsize = size / pow(2, iterator - 10);
            return newsize;
        }
        iterator += 10;
    }
}


void addChildren(Node *root, Node *node)
{
    root->children = realloc(root->children, (unsigned long) (root->numChildren + 1) * sizeof(Node));
    root->children[root->numChildren] = *node;
    root->numChildren++;
}


void initTree(Tree *tree, Node *root)
{
    tree->depth = INT_MAX;
    tree->options = 0;
    tree->root = root;
    tree->device = 0;
}


void initNode(Node *node)
{
    node->size = 0;
    node->type = 0;
    memset(node->link, '\0', 256);
    node->nonSuccessOpen = false;
    node->numChildren = 0;
    node->children = NULL;
}


void destroyTree(Node *root)
{
    for (size_t i = 0; i < root->numChildren; ++i) {
        if (root->children[i].numChildren > 0) {
            destroyTree(&root->children[i]);
        }
    }
    free(root->children);
}


static void printPrefix(int level, size_t i, size_t size, int *lastDirs, int options)
{
    int ascii = options & ASCII_MASK;
    for (int i = 0; i < level; ++i) {
        if (!lastDirs[i]) {
            if (ascii) {
                printf("|   ");
            } else {
                printf("│   ");
            }
        } else {
            printf("    ");
        }
    }

    if (ascii) {
        if (i == size - 1) {
            printf("\\-- ");
        } else {
            printf("|-- ");
        }
    } else {
        if (i == size - 1) {
            printf("└── ");
        } else {
            printf("├── ");
        }
    }
}


static void printLineHeader(Node *node, Tree *tree)
{
    char unit[4];
    if (tree->options & PERC_MASK) {
        printf("%5.1f%% ", trunc((((double) node->size) / tree->root->size) * 1000.0) / 10.0);
    } else {
        printf("%6.1f %3s ", trunc(convertSize(node->size, unit) * 10.0) / 10.0, unit);
    }
}


static void printRoot(Tree *tree, Node *root, int level, int *lastDirs, int checkParent)
{
    for (size_t i = 0; i < root->numChildren; ++i) {
        Node *r = &root->children[i];
        if (i == 0 && !checkParent) {
            r = root;
            i--;
            checkParent = true;
        }

        printLineHeader(r, tree);
        lastDirs[level] = (i == root->numChildren - 1);
        if (r != root) {
            printPrefix(level, i, root->numChildren, lastDirs, tree->options);
        }

        printNode(r->type, r, !(tree->options & NO_COLOR_MASK));

        if (tree->depth == 0)
            return;
        if (r->numChildren > 0 && level + 1 < tree->depth && r->path != root->path) {

            printRoot(tree, r, level + 1, lastDirs, checkParent);
        }
    }
}


int printTree(const char *path, const int options, const int depth)
{
    Tree tree;
    Node root;
    struct stat st;
    if ((lstat(path, &st))) {
        fprintf(stderr, "Wrong or missing path!\n");
        return 3;
    }

    if ((st.st_mode & S_IFMT) != S_IFREG &&
            (st.st_mode & S_IFMT) != S_IFDIR &&
            (st.st_mode & S_IFMT) != S_IFLNK) {
        return 0;
    }


    initNode(&root);
    strcpy(root.name, path);
    strcpy(root.path, path);
    initTree(&tree, &root);
    tree.device = st.st_dev;
    strcpy(tree.root->path, path);
    tree.options = options;
    tree.depth = depth;

    if (tree.options & MEMORY_MASK) {
        root.size = (size_t) st.st_size;
    } else {
        root.size = (size_t) st.st_blocks * 512;
    }

    DIR *cwd;

    if (path[strlen(path) - 1] != '/') {
        if (!(cwd = opendir(path))) {
            fclose(stderr);
            loadTree(&root, &tree);
        }
    } else {
        if (!(cwd = opendir(path))) {
            tree.root->nonSuccessOpen = true;
        }
    }

    closedir(cwd);
    loadTree(&root, &tree);
    int lastDirs[2050];
    if (!tree.root->numChildren) {
        printLineHeader(tree.root, &tree);
        printf("%s\n", tree.root->name);
        return 0;
    }
    printRoot(&tree, tree.root, 0, lastDirs, false);
    destroyTree(tree.root);
    return 0;
}


int loadTree(Node *root, Tree *tree)
{
    DIR *cwd;
    struct dirent *dir;
    struct stat st;
    Node node;

    if (!(cwd = opendir(root->path))) {
        perror(root->path);
        return 1;
    }

    while ((dir = readdir(cwd)) != NULL) {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
            continue;
        }

        initNode(&node);

        sprintf(node.path, "%s/%s", root->path, dir->d_name);
        sprintf(node.name, "%s", dir->d_name);

        if (lstat(node.path, &st)) {
            perror(node.path);
            node.nonSuccessOpen = true;
            return 1;
        }

        node.type = st.st_mode;

        if ((st.st_mode & S_IFMT) == S_IFLNK) {
            readlink(node.path, node.link, 256);
        }

        if (tree->options & DEVICE_MASK && st.st_dev != tree->device) {
            continue;
        }

        if (tree->options & MEMORY_MASK) {
            node.size = (size_t) st.st_size;
        } else {
            node.size = (size_t) st.st_blocks * 512;
        }

        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            if (loadTree(&node, tree)) {
                node.nonSuccessOpen = true;
            }
        }
        root->size += node.size;
        addChildren(root, &node);
    }

    if (tree->options & SORT_MASK) {
        qsort(root->children, root->numChildren, sizeof(Node), sizeCmp);
    } else {
        qsort(root->children, root->numChildren, sizeof(Node), nameCmp);
    }

    closedir(cwd);
    return 0;
}
