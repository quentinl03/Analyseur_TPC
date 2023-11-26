/**
 * @file parser.c
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 26-11-2023
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

char* parser(int argc, char** argv, int* flag_h, int* flag_t) {
    int option_index = 0, opt;
    char* map = NULL;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"tree", no_argument, 0, 't'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "ht:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                *flag_t = 1;
                break;

            case 'h':
                *flag_h = 1;
                printf(
                    "\nTPC helper :\n\n"
                    "./bin/tpc [-t] [-h] [file]\n\n"
                    "file :\n"
                    "\t Path to the file to real and ...\n\n"
                    "-t / --tree :\n"
                    "\t Generate an abstract tree from the file.\n\n"
                    "-h / --help :\n"
                    "\t Prints this menu.\n\n");
                break;

            default:
                break;
        }
    }
    if (optind < argc) {
        if (optind == argc - 1) {
            map = argv[optind];
        } else {
            printf("Too much arguments (1 for map)\n");
        }
    }
}