#ifndef BFI_H
#define BFI_H

#include<assert.h>
#include<fcntl.h>
#include<stdarg.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>

typedef int32_t bf_node_type;

#define BF_INC        0
#define BF_PTR        1
#define BF_PUT        2
#define BF_GET        3
#define BF_LOOP       4
#define BF_CLEAR      5
#define BF_SCAN_LEFT  6
#define BF_SCAN_RIGHT 7

#define BF_ERR_MALLOC  1
#define BF_ERR_BRACKET 2

/**
 * Type: bf_node
 * Represents a node in the BF AST.
 *
 * field: data
 * This field has different meanings depending on the node type.
 *
 * - For BF_INC and BF_PTR, this is the number by which to change
 *   the value at the data pointer or the data pointer, respectively.
 * - For BF_LOOP, this is a pointer to a bf_ast representing the body
 *   of the loop.
 * - For BF_PUT and BF_GET, this field is ignored.
 *
 * field: bf_node_type
 * This field represents the node type. It will be one of BF_INC, BF_PTR,
 * BF_PUT, BF_GET, or BF_LOOP.
 */

typedef struct {
  void *data;
  bf_node_type type;
} bf_node;

/**
 * Type: bf_ast
 * Represents a BF AST.
 *
 * field: nodes
 * Pointer to the array of nodes in this AST.
 *
 * field: n_nodes
 * The number of direct child nodes in this AST. The AST may be evaluated
 * by evaluating each of these nodes in sequence.
 */
typedef struct {
  bf_node *nodes;
  int32_t n_nodes;
} bf_ast;

int32_t bf_parse(bf_ast *ast, char *str);

int32_t bf_pass_coalesce_incs(bf_ast *src, bf_ast *dest);

int32_t bf_pass_coalesce_ptrs(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_clear(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_scan_left(bf_ast *src, bf_ast *dest);

int32_t bf_pass_detect_scan_right(bf_ast *src, bf_ast *dest);

int32_t bf_interpret(bf_ast *ast, int32_t *memory, int32_t dptr);

void bf_ast_delete(bf_ast *ast);

#endif
