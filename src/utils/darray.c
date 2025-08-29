#include "darray.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE (sizeof(u64) * DARRAY_MAX_FIELDS)

/**
 * @brief Create a darray.
 *
 * @param stride Size of each element
 *
 * @return Pointer to the array or NULL on failure.
 */
void *impl_darray_create(u64 capacity, u64 stride) {
    u64 total = (capacity * stride) + HEADER_SIZE;

    // Allocate memeory
    u64 *ptr = (u64 *)malloc(total);
    if (!ptr) return NULL;

    memset(ptr, 0, total);

    // Store the header field values
    ptr[DARRAY_CAPACITY] = capacity;
    ptr[DARRAY_STRIDE] = stride;
    ptr[DARRAY_SIZE] = 0;

    return (void *)((u64)ptr + HEADER_SIZE);
}

/**
 * @brief Destroy the darray.
 *
 * @param arr The array
 */
void impl_darray_destroy(void *arr) {
    u64 ptr = (u64)arr;
    ptr -= HEADER_SIZE;

    free((void *)ptr);
}

/**
 * @brief Resize the array to hold the given number of elements.
 *
 * After successfull execution of this function the array might be pointing to
 * some other address since it is using realloc.
 *
 * @param arr Pointer to the array.
 * @param capacity The new capacity
 */
bool impl_darray_resize(void **arr, u64 capacity) {
    u64 ptr = (u64)(*arr);
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;

    p = (u64 *)realloc(p, (capacity * p[DARRAY_STRIDE]) + HEADER_SIZE);
    if (!p) return false;

    p[DARRAY_CAPACITY] = capacity;

    ptr = (u64)p;
    ptr += HEADER_SIZE;

    *arr = (void *)ptr;

    return true;
}

/**
 * @brief Get the value in the header field.
 *
 * @param arr The array
 * @param field The header field
 *
 * @return The value in the header field of array.
 */
u64 impl_darray_get_header_field(void *arr, DArrayHeaderField field) {
    u64 ptr = (u64)arr;
    ptr -= HEADER_SIZE;
    return ((u64 *)ptr)[field];
}

/**
 * @brief Push an element to the end of the array.
 *
 * Might call darray_resize.
 *
 * @param arr Pointer to the array
 * @param element Pointer to the element to insert
 *
 * @return True on success else fail.
 */
bool impl_darray_push(void **arr, void *element) {
    u64 ptr = (u64)(*arr);
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;

    if (p[DARRAY_CAPACITY] <= p[DARRAY_SIZE]) {
        if (!darray_resize(arr, p[DARRAY_CAPACITY] + DARRAY_RESIZE_FACTOR))
            return false;

        ptr = (u64)(*arr);
        ptr -= HEADER_SIZE;
        p = (u64 *)ptr;
    }

    void *dest = ((u8 *)*arr) + (p[DARRAY_SIZE] * p[DARRAY_STRIDE]);
    memcpy(dest, element, p[DARRAY_STRIDE]);
    ++p[DARRAY_SIZE];

    return true;
}

/**
 * @brief Pop the element at the end of the array.
 *
 * @param arr Pointer to the array
 * @param element Pointer to store the poped element (Can be NULL)
 *
 * @return Returns true on success or false on fail.
 */
bool impl_darray_pop(void **arr, void *element) {
    u64 ptr = (u64)(*arr);
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;

    if (p[DARRAY_SIZE] == 0) return false;

    --p[DARRAY_SIZE];
    if (element) {
        void *src = ((u8 *)*arr) + (p[DARRAY_SIZE] * p[DARRAY_STRIDE]);
        memcpy(element, src, p[DARRAY_STRIDE]);
    }

    return true;
}

/**
 * @brief Push the given element to given index.
 *
 * Uninitialized indices will be initialized to zero.
 *
 * @param arr Pointer to the array
 * @param index Index to push the element to
 * @param element Pointer to the element to insert
 *
 * @return Returns true on success else false.
 */
bool impl_darray_push_at(void **arr, u64 index, void *element) {
    u64 ptr = (u64)(*arr);
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;

    if (index > p[DARRAY_SIZE]) {
        // No need to move any elements, just put the given element
        if (p[DARRAY_CAPACITY] <= index + 1) {
            if (!darray_resize(arr, index + 1)) return false;

            ptr = (u64)(*arr);
            ptr -= HEADER_SIZE;
            p = (u64 *)ptr;
        }

        // Set things to zero
        memset(((u8 *)*arr) + (p[DARRAY_SIZE] * p[DARRAY_STRIDE]), 0,
               (index - p[DARRAY_SIZE]) * p[DARRAY_STRIDE]);

        // copy the element
        void *dest = ((u8 *)*arr) + (index * p[DARRAY_STRIDE]);
        memcpy(dest, element, p[DARRAY_STRIDE]);

        // update the size
        p[DARRAY_SIZE] = index + 1;

        return true;
    }

    if (p[DARRAY_CAPACITY] <= p[DARRAY_SIZE]) {
        if (!darray_resize(arr, index + 1)) return false;

        ptr = (u64)(*arr);
        ptr -= HEADER_SIZE;
        p = (u64 *)ptr;
    }

    // Make the space
    void *dest = (u8 *)(*arr) + (p[DARRAY_STRIDE] * (index + 1));
    void *src = (u8 *)(*arr) + (p[DARRAY_STRIDE] * (index));
    u64 size = p[DARRAY_STRIDE] * (p[DARRAY_SIZE] - index);
    memmove(dest, src, size);

    // copy the element
    dest = (u8 *)(*arr) + (index * p[DARRAY_STRIDE]);
    memcpy(dest, element, p[DARRAY_STRIDE]);

    ++p[DARRAY_SIZE];

    return true;
}

/**
 * @brief Pops the element at the given index.
 *
 * @param arr Pointer to the array
 * @param idnex The index to pop element from
 * @param element Pointer to store the poped element (can be NULL)
 *
 * @return Returns true on success, else false.
 */
bool impl_darray_pop_at(void **arr, u64 index, void *element) {
    u64 ptr = (u64)(*arr);
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;

    assert(index < p[DARRAY_SIZE]);

    void *dest = ((u8 *)*arr) + (index * p[DARRAY_STRIDE]);
    void *src = ((u8 *)*arr) + ((index + 1) * p[DARRAY_STRIDE]);
    u64 size = (p[DARRAY_SIZE] - index - 1) * p[DARRAY_STRIDE];

    if (element) memcpy(element, dest, p[DARRAY_STRIDE]);
    memmove(dest, src, size);

    --p[DARRAY_SIZE];

    return true;
}

/**
 * @brief Clear the darray.
 *
 * Just sets the size of the array to zero. The elements should be considered
 * garbage values.
 *
 * @param arr The array
 */
void impl_darray_clear(void *arr) {
    u64 ptr = (u64)arr;
    ptr -= HEADER_SIZE;

    u64 *p = (u64 *)ptr;
    p[DARRAY_SIZE] = 0;
}
