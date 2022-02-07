#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "tree.h"

#define MEMORY   'a'
#define PERC     'p'
#define DEPTH     'd'
#define ASCII    'U'
#define NO_COLOR 'C'
#define DEVICE   'x'
#define SORT     's'
#define HELP     'h'

void printHelp(void)
{
    fprintf(stderr, "Usage: [-sadpxUC] [-d DEPTH] [-h] DIRECTORY\n"
           "\n"
           "OPTIONS:\n"
           "\t-s\t\tSorts printed files by size instead of by name.\n"
           "\t-a\t\tPrints apparent size instead of the size in blocks.\n"
           "\t-d [DEPTH]\tLimits the depth of output, but scans the whole tree.\n"
           "\t-p\t\toptions size format to percents instead of the size itself.\n"
           "\t-x\t\tDoes not cross filesystem boundaries when performing scan.\n"
           "\t-U\t\tPrints ascii characters instead of unicode ones.\n"
           "\t-C\t\tDisable colors.\n"
           "\t-h\t\tPrints this help and ends the program.\n");
}


bool isValidInteger(const char *string)
{
    for (size_t i = 0; string[i] != '\0'; ++i) {
        if (!isdigit(string[i])) {
            return false;
        }
    }
    return true;
}


int parseOptions(int size, const char **input, int *depth)
{
    int options = 0;
    bool expectingDepth = false;

    for (int i = 1; i < size; ++i) {
        if (input[i][0] == '-') {
            const char *option = input[i];
            for (int j = 1; option[j] != '\0'; ++j) {
                switch (option[j]) {
                case MEMORY:
                    options |= MEMORY_MASK;
                    break;
                case PERC:
                    options |= PERC_MASK;
                    break;
                case DEPTH:
                    expectingDepth = true;
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
                case NO_COLOR:
                    options |= NO_COLOR_MASK;
                    break;
                case HELP:
                    printHelp();
                    exit(0);
                default:
                    fprintf(stderr, "Unrecognized option \"-%c\".\n", option[j]);
                    printHelp();
                    exit(1);
                }
            }
        } else if (expectingDepth) {
            if (!isValidInteger(input[i])) {
                fprintf(stderr, "Depth must be positive integer!\n");
                exit(2);
            }
            sscanf(input[i], "%u", depth);
            expectingDepth = false;
        } else {
            fprintf(stderr, "Unrecognized option \"%s\".\n", input[i]);
            printHelp();
            exit(1);
        }
    }

    if (expectingDepth) {
        fprintf(stderr, "Depth must be provided!\n");
        exit(2);
    }
    return options;
}


int main(int argc, const char **argv)
{
    int depth = INT_MAX;
    int options = parseOptions(argc - 1, argv, &depth);
    const char *path = argv[argc - 1];
    return printTree(path, options, depth);
}
