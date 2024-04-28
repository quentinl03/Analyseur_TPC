#include "error.h"

#include <stdarg.h>
#include <stdio.h>

/**
 * @brief Get the error level as a string.
 *
 * @param err ErrorType with only a level flag set
 * @return const char*
 */
static const char* get_str_error_level(ErrorType err) {
    switch (err & LEVEL_MASK(err)) {
        case ERR_SEMANTIC_FLAG:
        case ERR_PARSE_FLAG:
            return "error";
            break;
        case ERR_CRITIAL_FLAG:
            return "critical";
            break;
        case ERR_WARN_FLAG:
            return "warning";
            break;

        default:
            return "Unknown error";
    }
}

void CodeError_print(CodeError err, const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);
    fprintf(stderr, "%s: ", get_str_error_level(err.err));
    fprintf(stderr, "position: %d:%d: ", err.line, err.column);
    vfprintf(stderr, format, arguments);
    putc('\n', stderr);
    va_end(arguments);
}
