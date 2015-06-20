#include "parser.h"

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

int32_t bf_parse(bf_ast *ast, char *str) {
  int32_t len = strlen(str);
  int32_t err;
  bf_vector nodes;

  if ((err = bf_vector_init(&nodes)) != 0) {
    _bfi_perror(err, "parser.bf_parse");
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
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_INC;
        node->data = (void *) 1;
        assert(((int32_t)node->data) == 1);
        bf_vector_push(&nodes, node);
        break;
      case '-':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_INC;
        node->data = (void *) -1;
        assert(((int32_t)node->data) == -1);
        bf_vector_push(&nodes, node);
        break;
      case '>':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PTR;
        node->data = (void *) 1;
        assert(((int32_t)node->data) == 1);
        bf_vector_push(&nodes, node);
        break;
      case '<':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PTR;
        node->data = (void *) -1;
        assert(((int32_t)node->data) == -1);
        bf_vector_push(&nodes, node);
        break;
      case '.':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        node->type = BF_PUT;
        node->data = NULL;
        bf_vector_push(&nodes, node);
        break;
      case ',':
        if ((node = malloc(sizeof(*node))) == NULL) {
          perror("parser.bf_parse");
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
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        if ((loop = malloc(sizeof(*loop))) == NULL) {
          perror("parser.bf_parse");
          return BF_ERR_MALLOC;
        }
        str[i + 1 + body_len] = '\0';
        _bfi_debug("loop body: %s\n", &str[i+1]);
        if ((err = bf_parse(loop, &str[i+1])) != 0) {
          str[i + 1 + body_len] = ']';
          _bfi_perror(err, "parser.bf_parse");
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
