#ifndef BFI_H
#define BFI_H

#include<fcntl.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>

#include "ast.h"
#include "logging.h"
#include "parser.h"
#include "passes.h"

intptr_t bf_interpret(bf_ast *ast, uint8_t *memory, intptr_t dptr);

#endif
