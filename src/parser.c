/**
 * @file parser.c
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 26-11-2023
 *
 */

#include "parser.h"

#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Print the help menu and exit the program.
 *
 * @param path path to the executable
 * @param exitcode exit code
 */
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
        "\t Prints symbol tables.\n\n"
        "-a / --only-tree :\n"
        "\t Only generate the syntax tree, and stop the execution.\n\n"
        "-w / --only-semantic :\n"
        "\t Only generate the semantics errors/warnings, and stop the execution.\n\n",
        path);
    exit(exitcode);
}

/**
 * @brief Initialize the option structure with default values.
 *
 * @return Option initialized.
 */
static Option init_option(void) {
    return (Option){
        .path = NULL,
        .flag_show_tree = false,
        .flag_only_tree = false,
        .flag_symtabs = false,
        .flag_semantic = true,
        .output = "_anonymous.asm",
    };
}

/**
 * @brief Names output file with the same name as the input file,
 * but with a .asm extension.
 * if the input file has no extension, we add ".asm" at the end.
 *
 * @param path Path to the input file.
 * @return char* Statically allocated string containing the output file name.
 */
static char* default_output_name(char* path) {
    static char result[PATH_MAX];
    // last index of '.' and return NULL when the character is not found
    char* extension = strrchr(path, '.');
    char* last_slash = strrchr(path, '/');

    // We want the name of the file, not the path
    last_slash = last_slash ? last_slash + 1 : path;

    if (extension) {
        // We replace the extension by ".asm"
        snprintf(result, PATH_MAX, "%.*s.asm", (int)(extension - last_slash), last_slash);
    } else {  // File have no extension
        snprintf(result, PATH_MAX, "%s.asm", last_slash);
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
        {"only-tree", no_argument, 0, 'a'},
        {"only-semantic", no_argument, 0, 'w'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "asht", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                option.flag_show_tree = true;
                break;

            case 's':
                option.flag_symtabs = true;
                break;

            case 'h':
                print_help(argv[0], EXIT_SUCCESS);
                break;

            case 'a':
                option.flag_only_tree = true;
                break;

            case 'w':
                option.flag_semantic = true;
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
                argv[optind]);
        }
    }

    return option;
}
