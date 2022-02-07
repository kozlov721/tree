#include <string.h>
#include <limits.h>
#include <stdio.h>
#include "tree.h"


void printHelp(void)
{
    fprintf(stderr, "Usage: [OPTIONS] [-d DEPTH] DIRECTORY\n"
           "\n"
           "OPTIONS:\n"
           "\t-s\t\tSorts printed files by size instead of by name.\n"
           "\t-a\t\tPrints apparent size instead of the size in blocks.\n"
           "\t-d [DEPTH]\tLimits the depth of output, but scans the whole tree.\n"
           "\t-p\t\toptions size format to percents instead of the size itself.\n"
           "\t-U\t\tPrints ascii characters instead of unicode ones.\n"
           "\t-x\t\tDoes not cross filesystem boundaries when performing scan.\n"
           "\t-h\t\tPrints help and ends the program.\n");
}


bool isValidDepth(const char *string)
{
    for (size_t i = 0; i < strlen(string); i++) {
        if (string[i] < '0' || string[i] > '9') {
            return false;
        }
    }
    return true;
}


int getOptions(int size, const char **input, int *depth)
{
    int options = 0;
    char used[24] = { 0 };
    int i = 1;
    do {
        if (input[i][0] != '-' || input[i][2] != '\0' || strchr(used, input[i][1])) {
            if (i == size) {
                continue;
            }
            return -1;
        }
        switch (input[i][1]) {
        case MEMORY:
            options |= MEMORY_MASK;
            break;
        case PERC:
            options |= PERC_MASK;
            break;
        case DEEP:
            if (i == size - 1 || !isValidDepth(input[i + 1])) {
                return -3;
            } else {
                sscanf(input[i + 1], "%d", depth);
            }
            options |= DEEP_MASK;
            i++;
            break;
        case ASCII:
            options |= ASCII_MASK;
            break;
        case DEVICE:
            options |= DEVICE_MASK;
            break;
        case SORT:
            options |= SORT_MASK;
            break;
        case HELP:
            printHelp();
            return -2;
        default:
            return -1;
        }
        used[i - 1] = input[i][1];
        i++;
    } while (i < size);
    return options;
}


int main(int argc, const char **argv)
{
    int depth = INT_MAX;
    int options = getOptions(argc - 1, argv, &depth);
    if (options == -3) {
        fprintf(stderr, "Depth must be a positive integer!\n");
        printHelp();
        return 3;
    }
    if (options == -1) {
        fprintf(stderr, "Unrecognized option!\n");
        printHelp();
        return 2;
    }
    if (options == -2) {
        return 0;
    }
    const char *path = argv[argc - 1];
    return printTree(path, options, depth);
}
