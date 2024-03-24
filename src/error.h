#ifndef ERROR_H
#define ERROR_H

typedef enum Error {
    ERR_NONE = 0,
    ERR_SYNTAX = -1,
    ERR_NO_MEMORY = -2,
} Error;

typedef enum SemWarning {
    WARN_NONE = 0,
    WARN_UNUSED_VARIABLE = -1,
    WARN_UNUSED_FUNCTION = -2,
} SemWarning;

typedef enum SemError {
    SEM_ERR_NONE = 0,
    SEM_ERR_UNDECLARED_SYMBOL = -1,
    SEM_ERR_REDECLARED_SYMBOL = -2,
    SEM_ERR_TYPE_MISMATCH = -3,
    SEM_ERR_INVALID_RETURN_TYPE = -4,
    SEM_ERR_NEGATIVE_ARRAY_SIZE = -5,
} SemError;

#endif
