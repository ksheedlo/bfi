#ifndef BF_PARSER_H
#define BF_PARSER_H

#include<assert.h>
#include<stdlib.h>
#include<string.h>

#include "ast.h"
#include "bf_vector.h"
#include "logging.h"

int32_t bf_parse(bf_ast *ast, char *str);

#endif
