#pragma once

#include "defines.h"

#ifndef DARRAY_DEFAULT_CAPACITY
    #define DARRAY_DEFAULT_CAPACITY 3
#endif

#ifndef DARRAY_RESIZE_FACTOR
    #define DARRAY_RESIZE_FACTOR 3
#endif

typedef enum DArrayHeaderField {
    DARRAY_CAPACITY,
    DARRAY_SIZE,
    DARRAY_STRIDE,
    DARRAY_MAX_FIELDS
} DArrayHeaderField;

void *impl_darray_create(u64 capacity, u64 stride);

void impl_darray_destroy(void *arr);

bool impl_darray_resize(void **arr, u64 capacity);

u64 impl_darray_get_header_field(void *arr, DArrayHeaderField field);

bool impl_darray_push(void **arr, void *element);

bool impl_darray_pop(void **arr, void *element);

bool impl_darray_push_at(void **arr, u64 index, void *element);

bool impl_darray_pop_at(void **arr, u64 index, void *element);

void impl_darray_clear(void *arr);

/**
 * @brief Create darray with given type and capacity.
 *
 * @param capacity The initial capacity of the array
 * @param type Type of the element
 *
 * @return The array or NULL on failure.
 */
#define darray_create_with_capacity(capacity, type) \
    (type *)impl_darray_create(capacity, sizeof(type))

/**
 * @brief Create a darray of given type.
 *
 * @param type Type of the elements of the array
 *
 * @return The array or NULL on failure.
 */
#define darray_create(type) \
    darray_create_with_capacity(DARRAY_DEFAULT_CAPACITY, type)

/**
 * @brief Destroy the darray.
 *
 * @param arr The array
 */
#define darray_destroy(arr) impl_darray_destroy(arr)

/**
 * @brief Resize the darray to given capacity.
 *
 * @param parr Pointer to the array
 * @param capacity The new capacity (number of elements)
 *
 * @return Returns true on success else false.
 */
#define darray_resize(parr, capacity) \
    impl_darray_resize((void **)parr, capacity)

/**
 * @brief Get the size of the darray (number of elements).
 *
 * @param arr The array
 *
 * @return The size of the array.
 */
#define darray_get_size(arr) impl_darray_get_header_field(arr, DARRAY_SIZE)

/**
 * @brief Get the current capacity of the darray (number of elements).
 *
 * @param arr The array
 *
 * @return The capacity of the array.
 */
#define darray_get_capacity(arr) \
    impl_darray_get_header_field(arr, DARRAY_CAPACITY)

/**
 * @brief Clear the darray.
 *
 * Just sets the size of the array to zero. The elements should be considered
 * garbage values.
 *
 * @param arr The array
 */
#define darray_clear(arr) impl_darray_clear(arr)

/**
 * @brief Push the given element to the end of the array.
 *
 * @param parr Pointer to the array
 * @param element The element
 * @param res Result
 *
 * @return Returns true on success, else false.
 */
#define darray_push(parr, element) \
    impl_darray_push((void **)parr, (__typeof__(element)[]){element})

/**
 * @brief Pop the element from the end of the array.
 *
 * @param parr Pointer to the array
 * @param element Pointer to store poped element (can be NULL)
 *
 * @return Returns true on success, else false.
 */
#define darray_pop(parr, element) impl_darray_pop((void **)parr, element)

/**
 * @brief Push the element to given index of the array.
 *
 * @param parr Pointer to the array
 * @param index Index to insert the element to
 * @param element The element to insert
 *
 * @return Returns true on success, else false.
 */
#define darray_push_at(parr, index, element) \
    impl_darray_push_at((void **)parr, index, (__typeof__(element)[]){element})

/**
 * @brief Pop the element at given index of the array.
 *
 * @param parr Pointer to the array
 * @param index Index to pop the element
 * @param element Pointer to store element (can be NULL)
 *
 * @return Returns true on success, else false.
 */
#define darray_pop_at(parr, index, element) \
    impl_darray_pop_at((void **)parr, index, element)
