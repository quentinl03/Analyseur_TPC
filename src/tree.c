#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
extern int nbline; /* from lexer */

// static const char *StringFromLabel[] = {
//     "E",
//     "T",
//     "divstar",
//     "id"
//     /* list all other node labels, if any */
//     /* The list must coincide with the label_t enum in tree.h */
//     /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
// };

static const char *NODE_STRING[] = {
    FOREACH_NODE(GENERATE_STRING)};

Node *makeNode(label_t label) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(1);
    }
    node->label = label;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = nbline;
    node->type = type_void;
    return node;
}

void addAttribut(Node *node, Attribut att, type_t type) {
    node->type = type;
    node->att = att;
}

void addSibling(Node *node, Node *sibling) {
    Node *curr = node;
    while (curr->nextSibling != NULL) {
        curr = curr->nextSibling;
    }
    curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    } else {
        addSibling(parent->firstChild, child);
    }
}

void deleteTree(Node *node) {
    if (node->firstChild) {
        deleteTree(node->firstChild);
    }
    if (node->nextSibling) {
        deleteTree(node->nextSibling);
    }
    free(node);
}

void printTree(Node *node) {
    static bool rightmost[128];        // tells if node is rightmost sibling
    static int depth = 0;              // depth of current node
    for (int i = 1; i < depth; i++) {  // 2502 = vertical line
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) {  // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }

    switch (node->type) {
        case type_byte:
            printf("%c\n", node->att.byte);
            break;
        case type_num:
            printf("%c\n", node->att.num);
            break;
        case type_ident:
            printf("%s\n", node->att.ident);
            break;
        case type_key_word:
            printf("%s\n", node->att.key_word);
            break;
        case type_void:
        default:
            printf("%s\n", NODE_STRING[node->label]);
            break;
    }
    // printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
