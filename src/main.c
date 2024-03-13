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
#include <string.h>

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
                       child->firstChild->att.key_word[0] == 'i' ? INT : CHAR,
                       0);
        }
        if (child->label == Type) {
            STable_add(table,
                       child->firstChild->att.ident,
                       child->att.key_word[0] == 'i' ? INT : CHAR,
                       1);
            // printf("child->label: %s\n", NODE_STRING[child->label]);
            // printf("child->label: %s\n", NODE_STRING[child->firstChild->label]);
            // printf("%s", child->att.ident);
        }
    }
}

void is_minus(Node *node, FILE *nasm) {
    if (!node) {
        return;
    }

    if (node->label == Addsub) {
        // appel sur les fils
        is_minus(node->firstChild, nasm);
        is_minus(node->firstChild->nextSibling, nasm);

        // on suppose que les deux valeurs sont sur la pile
        fprintf(nasm, "pop rcx\n");
        fprintf(nasm, "pop rax\n");
        fprintf(nasm, "sub rax, rcx\n");
        fprintf(nasm, "push rax\n");
    } else if (node->label == Num) {
        fprintf(nasm, "push %d\n", node->att.num);
    } else if (node->label == Ident) {
        printf("Not implemented yet for ident only number\n");
    } else {
        fprintf(stderr, "Who are you (in is_minus) : %s!\n", NODE_STRING[node->label]);
    }
}

void _is_main(Node *node, Symbol_Table *table, FILE *nasm, bool already_declared) {
    // printf("node->label: %s\n", NODE_STRING[node->label]);
    bool is_decl_key_word = node->label == Addsub;

    if (is_decl_key_word && node->att.key_word[0] == '-' && !already_declared) {
        is_minus(node, nasm);
        already_declared = true;
    }
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        _is_main(child, table, nasm, already_declared);
    }
}

static void _print_vars(Node *node, Symbol_Table *table, FILE *nasm) {
    bool is_decl_local_vars = node->label == DeclVars;
    bool is_decl_func_vars = node->label == ListTypVar;
    bool is_decl_func = node->label == DeclFonct;
    // printf("node->label: %s\n", NODE_STRING[node->label]);

    if (is_decl_local_vars) {
        for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
            _vars(child, table);
        }
    } else if (is_decl_func_vars) {
        // STable_print(table);
        printf("\n");
        STable_set_empty(table);
        _func(node, table);
    } else if (is_decl_func) {
        if (strcmp(node->firstChild->firstChild->nextSibling->att.ident, "main") == 0) {
            _is_main(node->firstChild->nextSibling, table, nasm, false);
        }
    }

    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        _print_vars(child, table, nasm);
    }
}

void fill_asm_entry(FILE *nasm) {
    fprintf(nasm, "global _start\n");
    fprintf(nasm, "section .text\n");
    fprintf(nasm, "extern show_registers\n");
    fprintf(nasm, "_start:\n");
}

void fill_asm_exit(FILE *nasm) {
    fprintf(nasm, "pop rbx\n");
    fprintf(nasm, "call show_registers\n");
    fprintf(nasm, "mov rax, 60\n");
    fprintf(nasm, "mov rdi, 0\n");
    fprintf(nasm, "syscall\n");
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

    // open _anonymous.asm
    FILE *nasm = fopen("_anonymous.asm", "w");
    fill_asm_entry(nasm);
    if (!nasm) {
        perror("fopen");
        fprintf(stderr, "End of execution.\n");
        return 1;
    }

    int r_val = parser_bison(yyin, &abr);
    fclose(yyin);
    if (!r_val && opt.flag_tree) {
        printTree(abr);
    }
    _print_vars(abr, &table, nasm);
    // STable_print(&table);
    STable_free(&table);
    if (opt.flag_tree) {
        deleteTree(abr);
    }
    fill_asm_exit(nasm);

    fclose(nasm);

    return r_val;
}
