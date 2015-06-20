#include "passes.h"

int32_t bf_pass_coalesce_incs(bf_ast *src, bf_ast *dest) {
  bf_vector nodes;
  int32_t err;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.bf_pass_coalesce_incs");
    return err;
  }

  for (int32_t i = 0; i < src->n_nodes; i++) {
    intptr_t total;
    bf_node *node;
    bf_ast *loop;

    switch (src->nodes[i].type) {
      case BF_INC:
        total = 0;
        int32_t j;
        for (j = i; j < src->n_nodes && src->nodes[j].type == BF_INC; j++) {
          total += (intptr_t)src->nodes[j].data;
        }
        if (total != 0) {
          if ((node = malloc(sizeof(*node))) == NULL) {
            perror("bfi.bf_pass_coalesce_incs");
            return BF_ERR_MALLOC;
          }
          node->type = BF_INC;
          node->data = (void *)total;
          bf_vector_push(&nodes, node);
        }
        i = j - 1;
        break;
      case BF_LOOP:
        if ((loop = malloc(sizeof(*loop))) == NULL) {
          perror("bfi.bf_pass_coalesce_incs");
          return BF_ERR_MALLOC;
        }
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_coalesce_incs");
          return BF_ERR_MALLOC;
        }
        if ((err = bf_pass_coalesce_incs((bf_ast *)src->nodes[i].data, loop)) != 0) {
          _bfi_perror(err, "bfi.bf_pass_coalesce_incs");
          return err;
        }
        node->type = BF_LOOP;
        node->data = (void *)loop;
        bf_vector_push(&nodes, node);
        break;
      default:
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_coalesce_incs");
          return BF_ERR_MALLOC;
        }
        memcpy(node, &src->nodes[i], sizeof(bf_node));
        bf_vector_push(&nodes, node);
        break;
    }
  }

  dest->n_nodes = nodes.size;
  if ((dest->nodes = malloc(dest->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&dest->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  return 0;
}

int32_t bf_pass_coalesce_ptrs(bf_ast *src, bf_ast *dest) {
  bf_vector nodes;
  int32_t err;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.bf_pass_coalesce_ptrs");
    return err;
  }

  for (int32_t i = 0; i < src->n_nodes; i++) {
    intptr_t total;
    bf_node *node;
    bf_ast *loop;

    switch (src->nodes[i].type) {
      case BF_PTR:
        total = 0;
        int32_t j;
        for (j = i; j < src->n_nodes && src->nodes[j].type == BF_PTR; j++) {
          total += (intptr_t)src->nodes[j].data;
        }
        if (total != 0) {
          if ((node = malloc(sizeof(*node))) == NULL) {
            perror("bfi.bf_pass_coalesce_ptrs");
            return BF_ERR_MALLOC;
          }
          node->type = BF_PTR;
          node->data = (void *)total;
          bf_vector_push(&nodes, node);
        }
        i = j - 1;
        break;
      case BF_LOOP:
        if ((loop = malloc(sizeof(*loop))) == NULL) {
          perror("bfi.bf_pass_coalesce_ptrs");
          return BF_ERR_MALLOC;
        }
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_coalesce_ptrs");
          return BF_ERR_MALLOC;
        }
        if ((err = bf_pass_coalesce_ptrs((bf_ast *)src->nodes[i].data, loop)) != 0) {
          _bfi_perror(err, "bfi.bf_pass_coalesce_ptrs");
          return err;
        }
        node->type = BF_LOOP;
        node->data = (void *)loop;
        bf_vector_push(&nodes, node);
        break;
      default:
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_coalesce_ptrs");
          return BF_ERR_MALLOC;
        }
        memcpy(node, &src->nodes[i], sizeof(bf_node));
        bf_vector_push(&nodes, node);
        break;
    }
  }

  dest->n_nodes = nodes.size;
  if ((dest->nodes = malloc(dest->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&dest->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  return 0;
}

int32_t bf_pass_detect_clear(bf_ast *src, bf_ast *dest) {
  bf_vector nodes;
  int32_t err;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.bf_pass_detect_clear");
    return err;
  }

  for (int32_t i = 0; i < src->n_nodes; i++) {
    bf_node *node;
    bf_ast *loop, *clear_expr;

    switch (src->nodes[i].type) {
      case BF_LOOP:
        clear_expr = (bf_ast *)src->nodes[i].data;
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_clear");
          return BF_ERR_MALLOC;
        }

        if (clear_expr->n_nodes == 1 &&
          clear_expr->nodes[0].type == BF_INC &&
          (int32_t)(clear_expr->nodes[0].data) == -1) {
          // This is indeed a clear expression. Push a Clear node.
          node->type = BF_CLEAR;
          node->data = NULL;
          bf_vector_push(&nodes, node);
        } else {
          if ((loop = malloc(sizeof(*loop))) == NULL) {
            perror("bfi.bf_pass_detect_clear");
            return BF_ERR_MALLOC;
          }
          if ((err = bf_pass_detect_clear((bf_ast *)src->nodes[i].data, loop)) != 0) {
            _bfi_perror(err, "bfi.bf_pass_detect_clear");
            return err;
          }
          node->type = BF_LOOP;
          node->data = (void *)loop;
          bf_vector_push(&nodes, node);
        }
        break;
      default:
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_clear");
          return BF_ERR_MALLOC;
        }
        memcpy(node, &src->nodes[i], sizeof(bf_node));
        bf_vector_push(&nodes, node);
        break;
    }
  }

  dest->n_nodes = nodes.size;
  if ((dest->nodes = malloc(dest->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&dest->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  return 0;
}

int32_t bf_pass_detect_scan_left(bf_ast *src, bf_ast *dest) {
  bf_vector nodes;
  int32_t err;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.bf_pass_detect_scan_left");
    return err;
  }

  for (int32_t i = 0; i < src->n_nodes; i++) {
    bf_node *node;
    bf_ast *loop, *scan_left_expr;

    switch (src->nodes[i].type) {
      case BF_LOOP:
        scan_left_expr = (bf_ast *)src->nodes[i].data;
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_scan_left");
          return BF_ERR_MALLOC;
        }

        if (scan_left_expr->n_nodes == 1 &&
          scan_left_expr->nodes[0].type == BF_PTR &&
          (int32_t)(scan_left_expr->nodes[0].data) == -1) {
          // This is indeed a scan left expression. Push a ScanLeft node.
          node->type = BF_SCAN_LEFT;
          node->data = NULL;
          bf_vector_push(&nodes, node);
        } else {
          if ((loop = malloc(sizeof(*loop))) == NULL) {
            perror("bfi.bf_pass_detect_scan_left");
            return BF_ERR_MALLOC;
          }
          if ((err = bf_pass_detect_scan_left((bf_ast *)src->nodes[i].data, loop)) != 0) {
            _bfi_perror(err, "bfi.bf_pass_detect_scan_left");
            return err;
          }
          node->type = BF_LOOP;
          node->data = (void *)loop;
          bf_vector_push(&nodes, node);
        }
        break;
      default:
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_scan_left");
          return BF_ERR_MALLOC;
        }
        memcpy(node, &src->nodes[i], sizeof(bf_node));
        bf_vector_push(&nodes, node);
        break;
    }
  }

  dest->n_nodes = nodes.size;
  if ((dest->nodes = malloc(dest->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&dest->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  return 0;
}

int32_t bf_pass_detect_scan_right(bf_ast *src, bf_ast *dest) {
  bf_vector nodes;
  int32_t err;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.bf_pass_detect_scan_right");
    return err;
  }

  for (int32_t i = 0; i < src->n_nodes; i++) {
    bf_node *node;
    bf_ast *loop, *scan_left_expr;

    switch (src->nodes[i].type) {
      case BF_LOOP:
        scan_left_expr = (bf_ast *)src->nodes[i].data;
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_scan_right");
          return BF_ERR_MALLOC;
        }

        if (scan_left_expr->n_nodes == 1 &&
          scan_left_expr->nodes[0].type == BF_PTR &&
          (int32_t)(scan_left_expr->nodes[0].data) == 1) {
          // This is indeed a scan right expression. Push a ScanRight node.
          node->type = BF_SCAN_RIGHT;
          node->data = NULL;
          bf_vector_push(&nodes, node);
        } else {
          if ((loop = malloc(sizeof(*loop))) == NULL) {
            perror("bfi.bf_pass_detect_scan_right");
            return BF_ERR_MALLOC;
          }
          if ((err = bf_pass_detect_scan_right((bf_ast *)src->nodes[i].data, loop)) != 0) {
            _bfi_perror(err, "bfi.bf_pass_detect_scan_right");
            return err;
          }
          node->type = BF_LOOP;
          node->data = (void *)loop;
          bf_vector_push(&nodes, node);
        }
        break;
      default:
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.bf_pass_detect_scan_right");
          return BF_ERR_MALLOC;
        }
        memcpy(node, &src->nodes[i], sizeof(bf_node));
        bf_vector_push(&nodes, node);
        break;
    }
  }

  dest->n_nodes = nodes.size;
  if ((dest->nodes = malloc(dest->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&dest->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  return 0;
}

int32_t bf_run_optimization_passes(bf_ast *src, bf_ast *dest) {
  bf_ast tmp1, tmp2;
  int32_t err;

  if ((err = bf_pass_coalesce_incs(src, &tmp1))) {
    _bfi_perror(err, "bfi.run_optimization_passes");
    return err;
  }
  if ((err = bf_pass_coalesce_ptrs(&tmp1, &tmp2))) {
    _bfi_perror(err, "bfi.run_optimization_passes");
    return err;
  }
  bf_ast_delete(&tmp1);
  if ((err = bf_pass_detect_clear(&tmp2, &tmp1))) {
    _bfi_perror(err, "bfi.run_optimization_passes");
    return err;
  }
  bf_ast_delete(&tmp2);
  if ((err = bf_pass_detect_scan_left(&tmp1, &tmp2))) {
    _bfi_perror(err, "bfi.run_optimization_passes");
    return err;
  }
  bf_ast_delete(&tmp1);
  if ((err = bf_pass_detect_scan_right(&tmp2, dest))) {
    _bfi_perror(err, "bfi.run_optimization_passes");
    return err;
  }
  bf_ast_delete(&tmp2);
  return 0;
}
