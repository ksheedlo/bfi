#include "bfi.h"

intptr_t bf_interpret(bf_ast *ast, uint8_t *memory, intptr_t dptr) {
  for (int32_t i = 0; i < ast->n_nodes; i++) {
    switch (ast->nodes[i].type) {
      case BF_INC:
        memory[dptr] = (uint8_t)(memory[dptr] + (intptr_t)(ast->nodes[i].data));
        break;
      case BF_PTR:
        dptr += (intptr_t)(ast->nodes[i].data);
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

void usage(char *name) {
  printf("Usage: %s PROGRAM.b\n", name);
}

int main(int argc, char **argv) {
  uint8_t *memory;
  char *program;
  int32_t err;
  bf_ast ast, optimized;

  if (argc < 2) {
    usage(argv[0]);
    return 0;
  }

  if ((program = bf_read_file(argv[1])) == NULL) {
    perror("bfi.main");
    return 3;
  }

  if ((memory = malloc(32768 * sizeof(*memory))) == NULL) {
    perror("bfi.main");
    return 1;
  }
  memset(memory, 0, 32768 * sizeof(*memory));

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
