#include "error.h"
#include <stdio.h>
#include <stdarg.h>

void CodeError_print(CodeError err, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    printf("Position: %d:%d: ", err.line, err.column);
    vprintf(format, arguments);
    putchar('\n');
    va_end(arguments);
}
