/**
 * @file analyseur.c
 * @author Laborde Quentin
 * @brief
 * @date 31-01-2024
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "table.h"
#include "tpc_bison.h"
#include "tree.h"

static const char *NODE_STRING[] = {
    FOREACH_NODE(GENERATE_STRING)};

static void _vars(Node *node, Symbol_Table *table) {
    if (!node) {
        return;
    }
    Symbol_Type type = node->att.key_word[0] == 'i' ? INT : CHAR;
    int size = 1;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == DeclArray) {
            size = child->firstChild->att.num;
        }
        STable_add(table, child->att.ident, type, size);
    }
}

static void _func(Node *node, Symbol_Table *table) {
    // Symbol_Type type = node->att.key_word[0] == 'i' ? INT : CHAR;
    // int size = 1;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == DeclFonctArray) {  // un cran de plus en profondeur
            STable_add(table,
                       child->firstChild->firstChild->att.ident,
                       node->att.key_word[0] == 'i' ? INT : CHAR,
                       0);
        }
        if (child->label == Type) {
            STable_add(table,
                       child->firstChild->att.ident,
                       node->att.key_word[0] == 'i' ? INT : CHAR,
                       1);
            // printf("child->label: %s\n", NODE_STRING[child->label]);
            // printf("child->label: %s\n", NODE_STRING[child->firstChild->label]);
            // printf("%s", child->att.ident);
        }
    }
}

static void _print_vars(Node *node, Symbol_Table *table) {
    bool is_decl_vars = node->label == DeclVars;
    bool is_decl_func = node->label == ListTypVar;
    // printf("node->label: %s\n", NODE_STRING[node->label]);

    if (is_decl_vars) {
        for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
            _vars(child, table);
        }
    } else if (is_decl_func) {
        STable_print(table);
        printf("\n");
        STable_set_empty(table);
        _func(node, table);
    }

    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        _print_vars(child, table);
    }
}

int main(int argc, char **argv) {
    extern FILE *yyin;
    Node *abr;
    Option opt;
    Symbol_Table table;
    STable_init(&table);
    opt = parser(argc, argv);
    if (opt.flag_help) {
        return 0;
    }

    if (opt.path) {
        yyin = fopen(opt.path, "r");
        if (!yyin) {
            perror("fopen");
            fprintf(stderr, "End of execution.\n");
            return 1;
        }
    }

    int r_val = parser_bison(yyin, &abr);
    fclose(yyin);
    if (!r_val && opt.flag_tree) {
        printTree(abr);
    }
    _print_vars(abr, &table);
    STable_print(&table);
    STable_free(&table);
    if (abr) {
        deleteTree(abr);
    }
    return r_val;
}
