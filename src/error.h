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
    ERR_SEM_REDECLARED_SYMBOL = ERR_SEMANTIC_FLAG | 1ULL << 16,
    ERR_SEM_VAR_IS_NOT_A_FUNC = ERR_SEMANTIC_FLAG | 1ULL << 17,
    ERR_UNDECLARED_SYMBOL     = ERR_SEMANTIC_FLAG | 1ULL << 18,
    ERR_FUNCTION_AS_RVALUE    = ERR_SEMANTIC_FLAG | 1ULL << 19,

    // 15 bits err critical (33 à 48)
    ERR_NO_MEMORY = ERR_CRITIAL_FLAG | 1ULL << 32,
    ERR_FILE_OPEN = ERR_CRITIAL_FLAG | 1ULL << 33,

    // 15 bits Warn (49 à 63)
    WARN_IMPLICIT_INT_TO_CHAR = ERR_WARN_FLAG | 1ULL << 48,
    WARN_RETURN_WITHOUT_VALUE = ERR_WARN_FLAG | 1ULL << 49,

} ErrorType;

#define IS_PARSE_ERROR(err) ((err & ERR_PARSE_FLAG) == ERR_PARSE_FLAG)
#define IS_SEMANTIC(err)    ((err & ERR_SEMANTIC_FLAG) == ERR_SEMANTIC_FLAG)
#define IS_WARNING(err)     ((err & ERR_WARN_FLAG) == ERR_WARN_FLAG)
#define IS_CRITICAL(err)    ((err & ERR_CRITIAL_FLAG) == ERR_CRITIAL_FLAG)

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
