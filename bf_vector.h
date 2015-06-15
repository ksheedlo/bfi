#ifndef BF_VECTOR_H
#define BF_VECTOR_H

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
  void **data;
  int32_t size;
  int32_t capacity;
} bf_vector;

int32_t bf_vector_init(bf_vector *vector);

int32_t bf_vector_push(bf_vector *vector, void *data);

void bf_vector_delete(bf_vector *vector);

#endif
