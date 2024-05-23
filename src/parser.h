/**
 * @file parser.h
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 21-12-2023
 *
 */

#ifndef PARSER_H
#define PARSER_H

typedef struct Option {
    char* path;
    char* output;
    int flag_show_tree; /*<
        if true, only show the AST, and exit
    */
    int flag_only_tree; /*<
       If true, we stop execution after parsing the source code. */
    int flag_symtabs; /*<
        Show symbol tables
    */
    int flag_semantic; /*<
        Show only semantic errors, and exit.
    */
} Option;

/**
 * @brief Parse the command line arguments using getopt_long.
 *
 * @param argc
 * @param argv
 * @return Option
 */
Option parser(int argc, char** argv);

#endif  // PARSER_H
