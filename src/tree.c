#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char *NODE_STRING[] = {
    FOREACH_NODE(GENERATE_STRING)
};

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

void addAttributIdent(Node *node, char value[64]) {
    node->type = type_ident;
    // node->att.ident = value;
    strncpy(node->att.ident, value, 64);
}

void addAttributKeyWord(Node *node, char value[5]) {
    node->type = type_key_word;
    strncpy(node->att.key_word, value, 5);
}

void addAttributByte(Node *node, char value) {
    node->type = type_byte;
    node->att.byte = value;
}

void addAttributNum(Node *node, int value) {
    node->type = type_num;
    node->att.num = value;
}

void addAttribut(Node *node, Attribut attrib, type_t type) {
    node->type = type;
    // strcpy(node->att.ident, attrib.ident);
    node->att = attrib;
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
    static char *unprintable_char[32] = {
        [0] = "Null character",
        [1] = "Start of Heading",
        [2] = "Start of Text",
        [3] = "End of Text",
        [4] = "End of Transmission",
        [5] = "Enquiry",
        [6] = "Acknowledge",
        [7] = "Bell, Alert",
        [8] = "Backspace",
        [9] = "Horizontal Tab",
        [10] = "Line Feed",
        [11] = "Vertical Tabulation",
        [12] = "Form Feed",
        [13] = "Carriage Return",
        [14] = "Shift Out",
        [15] = "Shift In",
        [16] = "Data Link Escape",
        [17] = "Device Control One (XON)",
        [18] = "Device Control Two",
        [19] = "Device Control Three (XOFF)",
        [20] = "Device Control Four",
        [21] = "Negative Acknowledge",
        [22] = "Synchronous Idle",
        [23] = "End of Transmission Block",
        [24] = "Cancel",
        [25] = "End of medium",
        [26] = "Substitute",
        [27] = "Escape",
        [28] = "File Separator",
        [29] = "Group Separator",
        [30] = "Record Separator",
        [31] = "Unit Separator",
    };
    static bool rightmost[128];        // tells if node is rightmost sibling
    static int depth = 0;              // depth of current node
    for (int i = 1; i < depth; i++) {  // 2502 = vertical line
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) {  // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }

    printf("%s : ", NODE_STRING[node->label]);

    // if (node->label == DeclArray || node->label == ArrayLR) {
    //     printf("%s : ", NODE_STRING[node->label]);
    // }

    switch (node->type) {
        case type_byte:
            if (node->label == Character) {
                if (node->att.byte < 32) {
                    printf("'%s'\n", unprintable_char[(int)node->att.byte]);
                } else if (node->att.byte > 126) {
                    printf("%d\n", node->att.byte);
                } else {
                    printf("'%c'\n", node->att.byte);
                }
            } else {
                printf("%c\n", node->att.byte);
            }
            break;
        case type_num:
            printf("%d\n", node->att.num);
            break;
        case type_ident:
            printf("%s\n", node->att.ident);
            break;
        case type_key_word:
            printf("%s\n", node->att.key_word);
            break;
        case type_void:
        default:
            // printf("%s\n", NODE_STRING[node->label]);
            printf("\n");
            break;
    }
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
