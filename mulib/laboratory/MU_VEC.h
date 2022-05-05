/**
 * MIT License
 *
 * Copyright (c) 2020 R. Dunbar Poor <rdpoor@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * A sketch for a "meta vector" generator.
 */

#ifndef _MU_VEC_H_
#define _MU_VEC_H_

// *****************************************************************************
// includes

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// types and definitions

typedef enum {
  MU_VEC_ERR_NONE,
  MU_VEC_ERR_EMPTY,
  MU_VEC_ERR_FULL,
  MU_VEC_ERR_INDEX,
  MU_VEC_ERR_NOT_FOUND,
} mu_vec_err_t;

/**
 * @brief MU_VEC_DECL creates declarations for a vector that contains elements
 * of _element_type.  Sample usage:
 *
 *   typedef struct {
 *     float x;
 *     float y;
 *   } pixel_t;
 *
 *   MU_VEC_DECL(pixel_vector, pixel_t)
 *
 * will generate declarations of the form:
 *
 *   pixel_vector_t *pixel_vector_init(pixel_vector_t *v,
 *                                     pixel_t *store,
 *                                     size_t capacity);
 *   pixel_vector_t *pixel_vector_clear(pixel_vector_t *v);
 *   size_t pixel_vector_capacity(pixel_vector_t *v);
 *   size_t pixel_vector_count(pixel_vector_t *v);
 *   mu_vec_err_t pixel_vector_push(pixel_vector_t *v, pixel_t e);
 *   mu_vec_err_t pixel_vector_pop(pixel_vector_t *v, pixel_t *e);
 */
#define MU_VEC_DECL(_name, _element_type)                                  \
#include <stddef.h>                                                            \
#include <stdbool.h>                                                           \
                                                                               \
typedef struct {                                                               \
    size_t capacity;                                                           \
    size_t count;                                                              \
    _element_type *store;                                                      \
  } _name##_t;                                                             \
                                                                               \
_name##_t *_name##_init(                                               \
    _name##_t *vec, _element_type *store, int capacity);                   \
_name##_t *_name##_clear(_name##_t *vec);                          \
size_t _name##_capacity(_name##_t *vec);                               \
size_t _name##_count(_name##_t *vec);                                  \
size_t _name##_element_size(_name##_t *vec);                           \
bool _name##_is_empty(_name##_t *v);                                   \
bool _name##_is_full(_name##_t *v);                                    \
mu_vec_err_t _name##_push(_name##_t *vec, _element_type e);            \
mu_vec_err_t _name##_pop(_name##_t *vec, _element_type *e);            \


_name##_t *_name##_init(_name##_t *vect,
                                _element_t  *elements,
                                size_t capacity);
_name##_t *_name##_reset(_name##_t *vect);
_element_t  *_name##_elements(_name##_t *vect);
size_t _name##_capacity(_name##_t *vect);
size_t _name##_count(_name##_t *vect);
size_t _name##_element_size(_name##_t *vect);
bool _name##_is_empty(_name##_t *vect);
bool _name##_is_full(_name##_t *vect);
_element_t *_name##_ref(_name##_t *vect, size_t idx);
mu_vect_err_t _name##_push(_name##_t *vect, _element_t  *e);
mu_vect_err_t _name##_pop(_name##_t *vect, _element_t  *e);
mu_vect_err_t _name##_peek(_name##_t *vect, _element_t  *e);
mu_vect_err_t _name##_insert(_name##_t *vect, size_t idx, _element_t  *e);
mu_vect_err_t _name##_remove(_name##_t *vect, size_t idx, _element_t  *e);
size_t _name##_index(_name##_t *vect, _element_t e);
size_t _name##_rindex(_name##_t *vect, _element_t e);
bool _name##_contains(_name##_t *vect, _element_t e, void *arg);
void *_name##_traverse(_name##_t *vect,
                       _name##_find_fn find_fn,
                       void *arg);
mu_vect_err_t _name##_insert_sorted(_name##_t *vect,
                                    _element_t e,
                                    _name##_cmp_fn cmp);
mu_vect_err_t _name##_sort(_name##_t *vect, _name##_cmp_fn cmp);

mu_vect_t *mu_vect_init(mu_vect_t *v, void *store, size_t n_items) {
  v->items = items;
  v->capacity = n_itmes;
  return mu_vect_reset(v);
}

mu_vect_t *mu_vect_reset(mu_vect_t *v) {
  v->count = 0;
  return v;
}

size_t mu_vect_capacity(mu_vect_t *v) {
  return v->capacity;
}

size_t mu_vect_count(mu_vect_t *v) {
  return v->count;
}

float *mu_vect_float_ref(mu_vect_t *v, size_t idx) {
  return (idx >= mu_vect_count(v)) ? NULL : _mu_vect_float_ref(v, idx);
}

// use only when idx known to be safe...
float *_mu_vect_float_ref(mu_vect_t *v, size_t idx) {
  return &((float *)v->items)[idx];
}

char *mu_vect_char_ref(mu_vect_t *v, size_t idx) {
  return (idx >= mu_vect_count(v)) ? NULL : &((char *)v->items)[idx];
}

mu_vect_err_t mu_vect_float_push(mu_vect_t *v, float e) {
  if (mu_vect_count(v) == mu_vect_capacity(v)) {
    return MU_VECT_ERR_FULL;
  }
  *mu_vect_float_ref(v, v->count++) = e;
  return MU_VECT_ERR_NONE;
}

mu_vect_err_t mu_vect_char_push(mu_vect_t *v, char e) {
  if (mu_vect_count(v) == mu_vect_capacity(v)) {
    return MU_VECT_ERR_FULL;
  }
  *mu_vect_char_ref(v, v->count++) = e;
  return MU_VECT_ERR_NONE;
}

mu_vect_err_t mu_vect_float_pop(mu_vect_t *v, float *e) {
  if (mu_vect_count(v) == 0) {
    return MU_VECT_ERR_EMPTY;
  }
  *e = mu_vect_float_ref(v, --v->count);
  return MU_VECT_ERR_NONE;
}

mu_vect_err_t mu_vect_float_insert(mu_vect_t *v, float e, size_t idx) {
  if (mu_vect_is_full(v)) {
    return MU_VECT_ERR_FULL;
  }
  size_t to_move = mu_vect_count(v) - idx;
  // This comparison exploits unsigned & 2s compliment arithmetic:
  if (to_move >= mu_vect_count(v)) {
    // Arrive here if idx is negative, or if idx > mu_vect_count(v)
    return MU_VECT_ERR_INDEX;
  }
  float *src = mu_vect_float_ref(v, idx);
  if (to_move > 0) {
    // carve a hole by moving store down one slot
    float *dst = mu_vect_float_ref(v, idx + 1);
    memmove(dst, mu_vect_float_ref(v, idx), sizeof(float));
  }
  // assign new element to newly opened hole.
  *src = s;
  v->count++;
  return MU_VECT_ERR_NONE;
}

mu_vect_err_t mu_vect_char_pop(mu_vect_t *v, char *e) {
  if (mu_vect_count(v) == 0) {
    return MU_VECT_ERR_EMPTY;
  }
  *e = mu_vect_char_ref(v, --v->count);
  return MU_VECT_ERR_NONE;
}

#define MU_VEC_IMPL(_name, _element_type)                                  \
#include <stddef.h>                                                            \
#include <stdbool.h>                                                           \
#include <string.h>                                                            \
                                                                               \
_name ## _t *_name ## _init(_name ## _t *v,                        \
                                    _element_type *store,                      \
                                    int capacity) {                            \
  v->store = store;                                                            \
  v->capacity = capacity;                                                      \
  return _name##_clear(v);                                                 \
}                                                                              \
_name##_t *_name##_clear(_name##_t *v) {                           \
  v->count = 0;                                                                \
  return vec;                                                                  \
}                                                                              \
size_t _name##_capacity(_name##_t *v) { return v->capacity; }          \
size_t _name##_count(_name##_t *v) { return v->count; }                \
size_t _name##_element_size(_name##_t *v) {                            \
  (void)v;                                                                     \
  return sizeof(_element_type);                                                \
}                                                                              \
bool _name##_is_empty(_name##_t *v) { return v->count == 0; }          \
bool _name##_is_full(_name##_t *v) {                                   \
  return v->count == v->capacity;                                              \
}                                                                              \
mu_vec_err_t _name##_push(_name##_t *v, _element_type e) {             \
  if (_name##_is_full(v)) {                                                \
    return MU_VEC_ERR_FULL;                                                    \
  }                                                                            \
  v->store[v->count++] = e;                                                    \
  return MU_VEC_ERR_NONE;                                                      \
}                                                                              \
mu_vec_err_t _name##_pop(_name##_t *v, _element_type *e) {             \
  if (_name##_is_empty(v)) {                                               \
    return MU_VEC_ERR_EMPTY;                                                   \
  }                                                                            \
  *e = v->store[--v->count];                                                   \
  return MU_VEC_ERR_NONE;                                                      \
}                                                                              \

// *****************************************************************************
// declarations

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_VEC_H_ */
