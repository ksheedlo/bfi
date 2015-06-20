#include "logging.h"

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
