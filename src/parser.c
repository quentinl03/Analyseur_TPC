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
#include <stdbool.h>
#include <linux/limits.h>
#include <string.h>

static void print_help(char* path, int exitcode) {
    printf(
        "\nTPCC helper :\n\n"
        "%s [-t] [-h] file\n\n"
        "file :\n"
        "\t Path to the file to read.\n\n"
        "-t / --tree :\n"
        "\t Generate an abstract tree from the file.\n\n"
        "-h / --help :\n"
        "\t Prints this menu.\n\n"
        "-s / --symtabs :\n"
        "\t Prints symbol tables.\n\n",
        path
    );
    exit(exitcode);
}

static Option init_option(void) {
    return (Option){
        .path = NULL,
        .flag_tree = false,
        .flag_symtabs = false,
        .output = "_anonymous.asm",
    };
}

/**
 * @brief Names output file with the same name as the input file,
 * but with a .asm extension.
 * 
 * @param path Path to the input file.
 * @return char* Statcally allocated string containing the output file name.
 */
static char* default_output_name(char* path) {
    static char result[PATH_MAX];
    char* extension = strrchr(path, '.');

    if (extension) {
        // We replace the extension by ".asm"
        snprintf(result, PATH_MAX, "%.*s.asm", (int) (extension - path), path);
    }
    else { // File have no extension
        snprintf(result, PATH_MAX, "%s.asm", path);
    }

    return result;
}

Option parser(int argc, char** argv) {
    Option option = init_option();
    int option_index = 0, opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"tree", no_argument, 0, 't'},
        {"symtabs", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "sht", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                option.flag_tree = true;
                break;

            case 's':
                option.flag_symtabs = true;
                break;

            case 'h':
                print_help(argv[0], EXIT_SUCCESS);
                break;

            case '?':
            default:
                print_help(argv[0], EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        if (optind == argc - 1) {
            option.path = argv[optind];
            option.output = default_output_name(option.path);
        } else {
            printf(
                "Too much arguments %s (1 for path) (no file loaded)\n",
                argv[optind]
            );
        }
    }

    return option;
}
