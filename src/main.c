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

static void _types(Node *node, Symbol_Table *table) {
    Symbol_Type type = node->att.key_word[0] == 'i' ? INT : CHAR;
    int size = 1;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == DeclArray) {
            size = child->firstChild->att.num;
        }
        STable_add(table, child->att.ident, type, size);
    }
}

static void _print_vars(Node *node, Symbol_Table *table) {
    bool is_decl = node->label == DeclVars;

    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        if (is_decl) {
            STable_set_empty(table);
            _types(child, table);
            STable_print(table);
            printf("\n");
        } else {
            _print_vars(child, table);
        }
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
