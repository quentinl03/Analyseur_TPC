#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "program.h"

extern Program PROGRAM;

#define COLOR_RED "\033[0;31m"
#define COLOR_PURPLE "\033[0;35m"
#define COLOR_RESET "\033[0m"
#define BOLD "\033[1m"

/**
 * @brief Get the error level as a string.
 *
 * @param err ErrorType with only a level flag set
 * @return const char*
 */
static const char* get_str_error_level(ErrorType err) {
    switch (err & LEVEL_MASK(err)) {
        case ERR_SEMANTIC_FLAG:
        case ERR_PARSE_FLAG:    return "error";
        case ERR_CRITIAL_FLAG:  return "critical";
        case ERR_WARN_FLAG:     return "warning";

        default:
            return "Unknown error";
    }
}

static const char* get_level_color(ErrorType err) {
    switch (err & LEVEL_MASK(err)) {
        case ERR_SEMANTIC_FLAG:
        case ERR_PARSE_FLAG:    return COLOR_RED;
        case ERR_CRITIAL_FLAG:  return COLOR_RED;
        case ERR_WARN_FLAG:     return COLOR_PURPLE;

        default:
            return COLOR_RED;
    }
}

static void _go_to_line(FILE* f, int line) {
    fseek(f, 0, SEEK_SET);
    int current_line = 1;
    while (current_line < line) {
        if (fgetc(f) == '\n') {
            current_line++;
        }
    }
}

static void print_one_line_from_file(FILE* f) {
    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        putc(c, stderr);
    }
    putc('\n', stderr);
}

static void safe_color_print(const char* color, const char* format, ...) {
    if (isatty(STDERR_FILENO)) {
        fprintf(stderr, "%s", color);
    }

    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);

    if (isatty(STDERR_FILENO)) {
        fprintf(stderr, "%s", COLOR_RESET);
    }
}

static void CodeError_print_file_line(FILE* f, CodeError err) {
    _go_to_line(f, err.line);
    const int offset = fprintf(stderr, "%5d | ", err.line);
    print_one_line_from_file(f);

    if (err.line == -1) {
        return;
    }

    fprintf(stderr, "%*s|%*s^\n", offset - 2, "", err.column - 1, "");
}

void CodeError_print(CodeError err, const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);

    safe_color_print(BOLD, "%s:%d:%d: ", PROGRAM.opt.path ? PROGRAM.opt.path : "stdin", err.line, err.column);

    safe_color_print(get_level_color(err.err), "%s: ", get_str_error_level(err.err));

    vfprintf(stderr, format, arguments);
    putc('\n', stderr);

    if (PROGRAM.opt.path) {
        CodeError_print_file_line(PROGRAM.file_in, err);
    }

    va_end(arguments);
}
