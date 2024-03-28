#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum ArrayListError {
    ARRAYLIST_ERR_NONE = 0,
    ARRAYLIST_ERR_ALLOC = -1,
} ArrayListError;

typedef struct ArrayList {
    size_t len;
    size_t capacity;
    size_t element_size;
    int (*cmp)(const void*, const void*);
    uint8_t* arr;
} ArrayList;

#define ARRAYLIST_REALLOC_BLOCK 10

// https://stackoverflow.com/questions/28524896/casting-pointer-to-memory-buffer-to-pointer-to-vla
// int (*arrPointer)[ARRSIZE] = (int(*)[ARRSIZE]) pointer;
#define ARRAYLIST_DECLARE_VLA(self, typename, array_name) \
    typename(*array_name)[self.len] = ((typename(*)[self.len])(typename*)self.arr)

#define ARRAYLIST_DECLARE_ARRAY(self, typename, array_name) \
    typename* array_name = (typename*)(self).arr

// #define ArrayList_pop_t(self, elem_type) ((elem_type) ArrayList_pop(self))

/**
 * @brief Create a new vector object
 *
 * @param self ArrayList to initialize
 * @param element_size Size of one element
 * @param initial_capacity Preallocation size
 * @param cmp Comparison function used to sort array if needed,
 * if sorting is not used, set to NULL
 * @return ArrayListError
 * if error while allocation : ARRAYLIST_ERR_ALLOC
 */
ArrayListError ArrayList_init(
    ArrayList* self,
    size_t element_size,
    size_t initial_capacity,
    int (*cmp)(const void*, const void*));

/**
 * @brief Add to element end of the vector
 *
 * @param self ArrayList object
 * @param elem Element to add
 * @return ArrayListError
 */
ArrayListError ArrayList_append(ArrayList* self, void* elem);

/**
 * @brief Add element to arraylist in sorted order
 * A comparison function must be provided on initialization !
 * @param self ArrayList object
 * @param elem Element to add
 * @return ArrayListError
 */
ArrayListError ArrayList_sorted_insert(ArrayList* self, void* elem);

/**
 * @brief Add element at index i if positive,
 * len - i, if negative
 *
 * @param self
 * @param i
 * @param elem
 * @return ArrayListError
 */
ArrayListError ArrayList_insert(ArrayList* self, int64_t i, const void* elem);

/**
 * @brief Extend self ArrayList with other vector
 *
 * @param self List to extend
 * @param other List to add to self
 * @return ArrayListError
 */
ArrayListError ArrayList_extend(ArrayList* self, const ArrayList* other);

/**
 * @brief Get element's address at index i if positive,
 * len - i, if negative
 * @param self ArrayList object
 * @param i index
 * @return void* Value's address
 */
void* ArrayList_get(const ArrayList* self, int64_t i);

/**
 * @brief Get element's value at index i if positive,
 * len - i, if negative
 * @param self ArrayList object
 * @param i index
 * @param type value's type
 */
#define ArrayList_get_v(self, i, type) (*((type*)ArrayList_get(self, i)))

/**
 * @brief Pop last element address
 *
 * @param self ArrayList object
 */
void* ArrayList_pop(ArrayList* self);

/**
 * @brief Pop last element's value
 *
 * @param self ArrayList object
 * @param type value's type
 */
#define ArrayList_pop_v(self, type) (*((type*)ArrayList_pop(self)))

/**
 * @brief Pop element at index i if positive,
 * len - i, if negative
 *
 * @param self ArrayList object
 * @param i index
 * @return void* Value's address
 */
void* ArrayList_pop_index(ArrayList* self, int64_t i);

/**
 * @brief Pop element's value at index i if positive,
 * len - i, if negative
 *
 * @param self ArrayList object
 * @param i index
 * @param type value's type
 */
#define ArrayList_pop_index_v(self, i, type) (*((type*)ArrayList_pop_index(self, i)))

/**
 * @brief Resize vector
 * old_size <= new_size : Nothing
 * old_size > new_size : Removes elements to fit
 *
 * @param self
 * @param new_size new size
 */
void ArrayList_resize(ArrayList* self, size_t new_size);

/**
 * @brief Reduce memory usage by reducing capacity
 * to vector's length
 *
 * @param self
 */
ArrayListError ArrayList_shrink_to_fit(ArrayList* self);

/**
 * @brief Free ArrayList object
 *
 * @param self
 */
void ArrayList_free(ArrayList* self);

/**
 * @brief Clear ArrayList object
 *
 * @param self
 */
void ArrayList_clear(ArrayList* self);

/**
 * @brief Get ArrayList's length
 *
 * @param self
 * @return size_t
 */
size_t ArrayList_get_length(const ArrayList* self);

/**
 * @brief Check if ArrayList is empty
 *
 * @param self
 * @return true
 * @return false
 */
bool ArrayList_is_empty(const ArrayList* self);

/**
 * @brief Sort's vector
 *
 * @param self
 */
void ArrayList_sort(ArrayList* self);

/**
 * @brief Search value in vector
 * A comparison function must be provided on initialization
 * @param self
 * @param searched_value
 * @return void*
 */
void* ArrayList_search(const ArrayList* self, const void* searched_value);

/**
 * @brief Check if the ArrayList contains a value
 * A comparison function must be provided on initialization
 * @param self ArrayList object
 * @param searched_value Value to search
 * @return true Value is present
 * @return false Value is not present
 */
bool ArrayList_contains(const ArrayList* self, const void* searched_value);

#endif
