/**
 * @file parser.c
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 26-11-2023
 *
 */

#include "parser.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void print_help() {
    printf(
        "\nTPC helper :\n\n"
        "./bin/tpc [-t] [-h] [file]\n\n"
        "file :\n"
        "\t Path to the file to real and ...\n\n"
        "-t / --tree :\n"
        "\t Generate an abstract tree from the file.\n\n"
        "-h / --help :\n"
        "\t Prints this menu.\n\n");
}

Option init_option() {
    return (Option){.path = NULL,
                    .flag_help = 0,
                    .flag_tree = 0};
}

Option parser(int argc, char** argv) {
    Option option = init_option();
    int option_index = 0, opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"tree", no_argument, 0, 't'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "ht", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                option.flag_tree = 1;
                break;

            case 'h':
                option.flag_help = 1;
                print_help();
                break;

            default:
                break;
        }
    }
    if (optind < argc) {
        if (optind == argc - 1) {
            option.path = argv[optind];
        } else {
            printf("Too much arguments %s (1 for path) (no file are loaded)\n",
                   argv[optind]);
        }
    }
    return option;
}