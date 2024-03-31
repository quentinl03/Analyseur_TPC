#ifndef ERROR_H
#define ERROR_H

typedef enum Error {
    ERR_NONE = 0,
    ERR_SYNTAX = -1,
    ERR_NO_MEMORY = -2,

    ERR_SEM_REDECLARED_SYMBOL = -3,
} Error;

#endif
