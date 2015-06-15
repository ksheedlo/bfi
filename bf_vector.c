#include "bf_vector.h"

int32_t bf_vector_init(bf_vector *vector) {
  if ((vector->data = malloc(20 * sizeof(*(vector->data)))) == NULL) {
    perror("bf_vector.init");
    return 1;
  }
  vector->size = 0;
  vector->capacity = 20;
  return 0;
}

int32_t bf_vector_push(bf_vector *vector, void *data) {
  if (vector->size < vector->capacity) {
    vector->data[vector->size++] = data;
  } else {
    if ((vector->data = realloc(vector->data, 2 * vector->capacity * sizeof(*(vector->data)))) == NULL) {
      perror("bf_vector.push");
      return 1;
    }
    vector->capacity = 2*vector->capacity;
    vector->data[vector->size++] = data;
  }
  return 0;
}

void bf_vector_delete(bf_vector *vector) {
  free(vector->data);
  vector->data = NULL;
  vector->size = 0;
  vector->capacity = 0;
}
