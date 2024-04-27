#ifndef ERROR_H
#define ERROR_H

typedef enum ErrorType {
    // 4 bits
    ERR_NONE = 0ULL,
    ERR_PARSE_FLAG = 1ULL << 0,
    ERR_SEMANTIC_FLAG = 1ULL << 1,
    ERR_CRITIAL_FLAG = 1ULL << 2,
    ERR_WARN_FLAG = 1ULL << 3,

    // 15 bits err de parse (5 à 15)
    ERR_PARSE_SYNTAX = ERR_PARSE_FLAG | 1ULL << 4,

    // 15 bits err de semantic (17 à 32)
    ERR_SEM_REDECLARED_SYMBOL  = ERR_SEMANTIC_FLAG | 1ULL << 16,
    ERR_SEM_IS_NOT_CALLABLE    = ERR_SEMANTIC_FLAG | 1ULL << 17,
    ERR_UNDECLARED_SYMBOL      = ERR_SEMANTIC_FLAG | 1ULL << 18,
    ERR_FUNCTION_AS_RVALUE     = ERR_SEMANTIC_FLAG | 1ULL << 19,
    ERR_RETURN_TYPE_NON_VOID   = ERR_SEMANTIC_FLAG | 1ULL << 20,
    ERR_RETURN_TYPE_VOID       = ERR_SEMANTIC_FLAG | 1ULL << 21,
    ERR_MAIN_UNAVAILABLE       = ERR_SEMANTIC_FLAG | 1ULL << 22,
    ERR_USE_UNDEFINED_FUNCTION = ERR_SEMANTIC_FLAG | 1ULL << 23,
    ERR_MAIN_RETURN_TYPE       = ERR_SEMANTIC_FLAG | 1ULL << 24,
    ERR_MAIN_PARAM             = ERR_SEMANTIC_FLAG | 1ULL << 25,

    // 15 bits err critical (33 à 48)
    ERR_NO_MEMORY = ERR_CRITIAL_FLAG | 1ULL << 32,
    ERR_FILE_OPEN = ERR_CRITIAL_FLAG | 1ULL << 33,

    // 15 bits Warn (49 à 63)
    WARN_IMPLICIT_INT_TO_CHAR = ERR_WARN_FLAG | 1ULL << 48,

} ErrorType;

#define IS_PARSE_ERROR(err) ((err & ERR_PARSE_FLAG) == ERR_PARSE_FLAG)
#define IS_SEMANTIC(err)    ((err & ERR_SEMANTIC_FLAG) == ERR_SEMANTIC_FLAG)
#define IS_WARNING(err)     ((err & ERR_WARN_FLAG) == ERR_WARN_FLAG)
#define IS_CRITICAL(err)    ((err & ERR_CRITIAL_FLAG) == ERR_CRITIAL_FLAG)
#define LEVEL_MASK(err)     ((1ULL << 4) - 1)

#define EXIT_CODE(err) (                   \
    (err) == ERR_NONE || IS_WARNING(err) ? 0 \
    : IS_PARSE_ERROR(err) ? 1              \
    : IS_SEMANTIC(err) ? 2                 \
    : IS_CRITICAL(err) ? 3                 \
    : -1                                   \
)

typedef struct CodeError {
    ErrorType err;
    int line;
    int column;
} CodeError;

#define COLOR_ERR_RED "\e[1;31m"
#define COLOR_WARN_YELLOW "\e[1;33m"
#define RESET_COLOR "\e[0m"


__attribute__ ((format (printf, 2, 3)))
void CodeError_print(CodeError err, const char *msg, ...);

#endif
