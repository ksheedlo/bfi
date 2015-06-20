#ifndef BF_LOGGING_H
#define BF_LOGGING_H

#include<stdarg.h>
#include<stdio.h>

#include "ast.h"

void _bfi_perror(int32_t code, char *scope);

void _bfi_debug(const char *fmt, ...);

void _bf_print_ast(bf_ast *ast);

#endif
