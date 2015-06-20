#include "bf_vector.h"
#include "bfi.h"

void _bfi_perror(int32_t code, char *scope) {
  char *msg = "An unknown error occurred.";

  switch (code) {
    case BF_ERR_MALLOC: msg = "Memory allocation failed."; break;
    case BF_ERR_BRACKET: msg = "Brackets didn't match."; break;
  }

  fprintf(stderr, "%s: %s\n", scope, msg);
}

void _bfi_debug(const char *fmt, ...) {
#ifndef NDEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

int32_t _seek_length_to_close_bracket(char *str) {
  int32_t max_len = strlen(str);
  int32_t level = 1;
  int32_t i = 0;

  for (; i < max_len; i++) {
    if (str[i] == '[') {
      level++;
    }
    if (str[i] == ']') {
      if (--level == 0) {
        return i;
      }
    }
  }

  return -1;
}

void _bf_print_ast(bf_ast *ast) {
#ifndef NDEBUG
  int32_t val;

  for (int32_t i = 0; i < ast->n_nodes; i++) {
    switch (ast->nodes[i].type) {
      case BF_INC:
        val = (int32_t)ast->nodes[i].data;
        if (abs(val) < 2) {
          for (int i = 0; i < val; i++) {
            fputc('+', stderr);
          }
          for (int i = 0; i < -val; i++) {
            fputc('-', stderr);
          }
        } else if (val > 0) {
          fprintf(stderr, "+(%d)", val);
        } else {
          fprintf(stderr, "-(%d)", -val);
        }
        break;
      case BF_PTR:
        val = (int32_t)ast->nodes[i].data;
        if (abs(val) < 2) {
          for (int i = 0; i < val; i++) {
            fputc('>', stderr);
          }
          for (int i = 0; i < -val; i++) {
            fputc('<', stderr);
          }
        } else if (val > 0) {
          fprintf(stderr, ">(%d)", val);
        } else {
          fprintf(stderr, "<(%d)", -val);
        }
        break;
      case BF_PUT:
        fputc('.', stderr);
        break;
      case BF_GET:
        fputc(',', stderr);
        break;
      case BF_LOOP:
        fputc('[', stderr);
        _bf_print_ast((bf_ast *)ast->nodes[i].data);
        fputc(']', stderr);
        break;
      case BF_CLEAR:
        fputc('C', stderr);
        break;
      case BF_SCAN_LEFT:
        fputc('L', stderr);
        break;
      case BF_SCAN_RIGHT:
        fputc('R', stderr);
        break;
    }
  }
#endif
}

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

int32_t bf_parse(bf_ast *ast, char *str) {
  int32_t len = strlen(str);
  int32_t err;
  bf_vector nodes;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "bfi.parse");
    return err;
  }

  for (int32_t i = 0; i < len; i++) {
    bf_node *node;
    bf_ast *loop;
    int32_t body_len;

    char ch = str[i];
    switch(ch) {
      case '+':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_INC;
        node->data = (void *) 1;
        assert(((int32_t)node->data) == 1);
        bf_vector_push(&nodes, node);
        break;
      case '-':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_INC;
        node->data = (void *) -1;
        assert(((int32_t)node->data) == -1);
        bf_vector_push(&nodes, node);
        break;
      case '>':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PTR;
        node->data = (void *) 1;
        assert(((int32_t)node->data) == 1);
        bf_vector_push(&nodes, node);
        break;
      case '<':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PTR;
        node->data = (void *) -1;
        assert(((int32_t)node->data) == -1);
        bf_vector_push(&nodes, node);
        break;
      case '.':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PUT;
        node->data = NULL;
        bf_vector_push(&nodes, node);
        break;
      case ',':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_GET;
        node->data = NULL;
        bf_vector_push(&nodes, node);
        break;
      case '[':
        body_len = _seek_length_to_close_bracket(&str[i+1]);
        if (body_len == -1) {
          return BF_ERR_BRACKET;
        }
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        if ((loop = malloc(sizeof(*loop))) == NULL) {
          perror("bfi.parse");
          return BF_ERR_MALLOC;
        }
        str[i + 1 + body_len] = '\0';
        _bfi_debug("loop body: %s\n", &str[i+1]);
        if ((err = bf_parse(loop, &str[i+1])) != 0) {
          str[i + 1 + body_len] = ']';
          _bfi_perror(err, "bfi.parse");
          return err;
        }
        str[i + 1 + body_len] = ']';
        _bfi_debug("loop: ");
        _bf_print_ast(loop);
        _bfi_debug("\n");
        node->type = BF_LOOP;
        node->data = (void *)loop;
        loop = NULL;
        bf_vector_push(&nodes, node);

        // Skip over the entire loop body.
        i += 1 + body_len;
        break;
      case ']':
        // This is an error condition that we should not see because we
        // skip the loop body when we see a loop.
        return BF_ERR_BRACKET;
    }
  }

  ast->n_nodes = nodes.size;
  _bfi_debug("ast->n_nodes: %d\n", ast->n_nodes);
  if ((ast->nodes = malloc(ast->n_nodes * sizeof(bf_node))) == NULL) {
    return BF_ERR_MALLOC;
  }
  int32_t i;
  for (i = 0; i < nodes.size; i++) {
    memcpy(&ast->nodes[i], nodes.data[i], sizeof(bf_node));
    free(nodes.data[i]);
  }
  bf_vector_delete(&nodes);
  _bfi_debug("i: %d\n", i);
  _bfi_debug("ast: ");
  _bf_print_ast(ast);
  _bfi_debug("\n");
  return 0;
}

int32_t bf_interpret(bf_ast *ast, int32_t *memory, int32_t dptr) {
  for (int32_t i = 0; i < ast->n_nodes; i++) {
    switch (ast->nodes[i].type) {
      case BF_INC:
        memory[dptr] += (int32_t)(ast->nodes[i].data);
        break;
      case BF_PTR:
        dptr += (int32_t)(ast->nodes[i].data);
        break;
      case BF_PUT:
        putchar(memory[dptr]);
        break;
      case BF_GET:
        memory[dptr] = getchar();
        break;
      case BF_LOOP:
        while (memory[dptr]) {
          dptr = bf_interpret((bf_ast *)ast->nodes[i].data, memory, dptr);
        }
        break;
      case BF_CLEAR:
        memory[dptr] = 0;
        break;
      case BF_SCAN_LEFT:
        while (memory[dptr]) {
          dptr--;
        }
        break;
      case BF_SCAN_RIGHT:
        while (memory[dptr]) {
          dptr++;
        }
        break;
    }
  }

  return dptr;
}

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

void usage(char *name) {
  printf("Usage: %s PROGRAM.b\n", name);
}

int main(int argc, char **argv) {
  int32_t *memory;
  char *program;
  int32_t err;
  bf_ast ast, optimized;
  int fd;
  FILE *code;
  struct stat pstats;

  if (argc < 2) {
    usage(argv[0]);
    return 0;
  }

  if ((fd = open(argv[1], O_RDONLY)) == -1) {
    perror("bfi.main");
    return 3;
  }
  if (fstat(fd, &pstats) == -1) {
    perror("bfi.main");
    return 4;
  }

  if ((program = malloc(pstats.st_size)) == NULL) {
    perror("bfi.main");
    return 1;
  }

  if ((code = fdopen(fd, "r")) == NULL) {
    perror("bfi.main");
    return 2;
  }

  fread(program, pstats.st_size, 1, code);
  fclose(code);

  if ((memory = malloc(32768 * sizeof(*memory))) == NULL) {
    perror("bfi.main");
    return 1;
  }

  _bfi_debug("program: %s\n", program);
  err = bf_parse(&ast, program);
  if (err) {
    _bfi_perror(err, "bfi.main");
    return err;
  }
  free(program);

  if ((err = bf_run_optimization_passes(&ast, &optimized))) {
    _bfi_perror(err, "bfi.main");
    return err;
  }
  bf_ast_delete(&ast);
  _bfi_debug("optimized: ");
  _bf_print_ast(&optimized);
  _bfi_debug("\n");

  bf_interpret(&optimized, memory, 0);
  bf_ast_delete(&optimized);
  free(memory);
  return 0;
}
