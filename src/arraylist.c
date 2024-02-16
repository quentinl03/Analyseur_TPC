#include "arraylist.h"
#include <assert.h>
#include <string.h>

static ArrayListError _ArrayList_realloc(ArrayList *self, size_t new_capacity);

ArrayListError ArrayList_init(ArrayList *self, size_t element_size,
                              size_t initial_capacity,
                              int (*cmp)(const void *, const void *)) {

    *self = (ArrayList){
        .capacity = initial_capacity,
        .element_size = element_size,
        .len = 0,
        .cmp = cmp,
    };

    self->arr = malloc(element_size * initial_capacity);

    if (self->arr == NULL)
        return ARRAYLIST_ERR_ALLOC;

    return ARRAYLIST_ERR_NONE;
}

static ArrayListError _ArrayList_realloc(ArrayList *self, size_t new_capacity) {
    uint8_t *new_arr = realloc(self->arr, new_capacity * self->element_size);

    if (new_arr == NULL)
        return ARRAYLIST_ERR_ALLOC;

    self->arr = new_arr;
    self->capacity = new_capacity;

    return ARRAYLIST_ERR_NONE;
}

/**
 * @brief Realloc self ArrayList if len >= capacity
 *
 * @param self
 * @return ArrayListError
 */
static ArrayListError _ArrayList_realloc_extend(ArrayList *self) {
    if (self->len < self->capacity) {
        return ARRAYLIST_ERR_NONE;
    }
    return _ArrayList_realloc(self, self->capacity + ARRAYLIST_REALLOC_BLOCK);
}

ArrayListError ArrayList_append(ArrayList *self, void *elem) {
    ArrayListError err = _ArrayList_realloc_extend(self);
    if (err < 0)
        return err;

    memcpy(self->arr + self->len * self->element_size, (uint8_t *)elem,
           self->element_size);
    self->len++;

    return ARRAYLIST_ERR_NONE;
}

/**
 * @brief Get index of element in arraylist for insertion
 *
 * @param self
 * @param elem
 * @return int64_t
 */
static int64_t ArrayList_bsearch_index(const ArrayList *self, void *elem) {
    assert(self->cmp);
    assert(elem);

    int64_t left = 0;
    int64_t right = self->len - 1;
    int64_t mid;

    while (left <= right) {
        mid = (left + right) / 2;

        int cmp = self->cmp(elem, self->arr + mid * self->element_size);

        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return left;
}

ArrayListError ArrayList_sorted_insert(ArrayList *self, void *elem) {
    assert(self->cmp);
    assert(elem);

    ArrayListError err = _ArrayList_realloc_extend(self);

    if (err < 0)
        return err;

    int64_t index = ArrayList_bsearch_index(self, elem);

    memmove(self->arr + (index + 1) * self->element_size,
            self->arr + index * self->element_size,
            (self->len - index) * self->element_size);

    memcpy(self->arr + index * self->element_size, elem, self->element_size);

    self->len++;

    return ARRAYLIST_ERR_NONE;
}

void *ArrayList_get(const ArrayList *self, int64_t i) {

    if (i < 0)
        i = self->len + i;

    if (i >= self->len)
        return NULL;

    return self->arr + i * self->element_size;
}

void *ArrayList_pop(ArrayList *self) {
    if (self->len == 0)
        return NULL;

    uint8_t *popped = self->arr + (--(self->len)) * self->element_size;

    return popped;
}

void ArrayList_resize(ArrayList *self, size_t new_size) {
    assert(new_size >= 0);
    self->len = new_size;
}

ArrayListError ArrayList_shrink_to_fit(ArrayList *self) {
    return _ArrayList_realloc(self, self->len);
}

void ArrayList_free(ArrayList *self) {
    free(self->arr);
    *self = (ArrayList){0};
}

void ArrayList_clear(ArrayList *self) {
    self->len = 0;
}

size_t ArrayList_get_length(const ArrayList *self) {
    return self->len;
}

void ArrayList_sort(ArrayList *self) {
    assert(self->cmp);

    qsort(self->arr, self->len, self->element_size, self->cmp);
}

void *ArrayList_search(const ArrayList *self, const void *searched_value) {
    assert(self->cmp);

    return bsearch(searched_value, self->arr, self->len, self->element_size,
                   self->cmp);
}

bool ArrayList_contains(const ArrayList *self, const void *searched_value) {
    return ArrayList_search(self, searched_value) != NULL;
}
