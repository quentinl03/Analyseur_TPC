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
    int flag_tree;
    int flag_symtabs;
} Option;

Option parser(int argc, char** argv);

#endif  // PARSER_H
