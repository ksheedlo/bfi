#ifndef BF_PASSES_H
#define BF_PASSES_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "ast.h"
#include "bf_vector.h"
#include "logging.h"

int32_t bf_pass_coalesce_incs(bf_ast *src, bf_ast *dest);

int32_t bf_pass_coalesce_ptrs(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_clear(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_scan_left(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_scan_right(bf_ast *src, bf_ast *dest);

int32_t bf_run_optimization_passes(bf_ast *src, bf_ast *dest);

#endif
