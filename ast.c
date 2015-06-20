#include "ast.h"

void bf_ast_delete(bf_ast *ast) {
  for (int32_t i = 0; i < ast->n_nodes; i++) {
    if (ast->nodes[i].type == BF_LOOP) {
      bf_ast_delete((bf_ast *)ast->nodes[i].data);
      free(ast->nodes[i].data);
    }
  }

  ast->n_nodes = 0;
  free(ast->nodes);
}
