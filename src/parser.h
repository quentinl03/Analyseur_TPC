/**
 * @file parser.h
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 21-12-2023
 *
 */

#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char* path;
    int flag_help;
    int flag_tree;
} Option;

void print_help(char* path);

Option init_option(void);

Option parser(int argc, char** argv);

#endif  // PARSER_H