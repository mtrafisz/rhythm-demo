#include "car.h"

#include <assert.h>
#include <string.h>

#define UNUSED(thing) ((void)thing)

static void* __glibc_realloc(void* ptr, size_t old_size, size_t new_size) {
    UNUSED(old_size);
    return realloc(ptr, new_size);
}

const Allocator STD_ALLOCATOR = {
    .alloc = malloc,
    .dealloc = free,
    .realloc = __glibc_realloc
};

Array __array_new(size_t sizeof_element, uint32_t capacity, Allocator allocator) {
    assert(capacity != 0);

    Array a = {0};

    a.data = allocator.alloc(sizeof_element * capacity);
    assert(a.data != NULL);

    a.size = 0;
    a.capacity = capacity;
    a.sizeof_element = sizeof_element;
    a.allocator = allocator;

    return a;
}

void array_append(ArrayPtr a, const void* p) {
    assert(a != NULL && a->data != NULL && p != NULL);

    if (a->size == a->capacity) {
        a->data = a->allocator.realloc(a->data, a->capacity * a->sizeof_element, (a->capacity << 1) * a->sizeof_element);
        assert(a->data != NULL);
        a->capacity *= 2;
    }

    memcpy(
        (char*)a->data + (a->size * a->sizeof_element),
        p,
        a->sizeof_element
    );

    a->size++;
}

void* array_at(ArrayPtr a, uint32_t i) {
    assert(a != NULL && a->data != NULL);
    if (i > a->size) return NULL;
    return (char*)a->data + (i * a->sizeof_element);
}

uint32_t array_size(ArrayPtr a) {
    assert(a != NULL);
    return a->size;
}

void array_resize(ArrayPtr a, uint32_t new_size) {
    assert(a != NULL);
    // this should also assert(new_size <= a->size), but who am I to prevent
    // You from doing wacky shit?

    if (a->capacity < new_size) {
        a->data = a->allocator.realloc(a->data, a->capacity, new_size);
        a->capacity = new_size;
    }

    a->size = new_size;
}

void array_destroy(ArrayPtr a) {
    assert(a != NULL);

    if (a->data) a->allocator.dealloc(a->data);
    a->size = 0;
    a->capacity = 0;
    a->sizeof_element = 0;
}

Array array_clone(const ArrayPtr a) {
    assert(a != NULL && a->data != NULL);

    Array a_clone = {
        .allocator = a->allocator,
        .capacity = a->capacity,
        .size = a->size,
        .sizeof_element = a->sizeof_element
    };

    a_clone.data = a_clone.allocator.alloc(a_clone.size * a_clone.sizeof_element);
    assert(a_clone.data != NULL);

    memcpy(
        a_clone.data,
        a->data,
        a->sizeof_element * a->size
    );

    return a_clone;
}

ArrayIterator iterator(const ArrayPtr a) {
    return (ArrayIterator) {
        .current = a->data,
        .end = (char*)a->data + (a->size * a->sizeof_element),
        .sizeof_element = a->sizeof_element
    };
}

void* iterator_next(ArrayIteratorPtr i) {
    if (i->current >= i->end) return NULL;
    void* p = i->current;
    i->current = (char*)i->current + i->sizeof_element;
    return p;
}

void array_set(ArrayPtr a, uint32_t i, const void* p) {
    assert(a != NULL && a->data != NULL && p != NULL);
    assert(i <= a->size);

    memcpy(
        (char*)a->data + (i * a->sizeof_element),
        p,
        a->sizeof_element
    );
}

void array_shrink(ArrayPtr a) {
    assert(a != NULL && a->data != NULL);

    a->capacity = a->size;
    a->data = a->allocator.realloc(a->data, a->capacity, a->size);
    assert(a->data != NULL);
}

void array_combine(ArrayPtr a, const ArrayPtr b) {
    assert(a != NULL && b != NULL);
    assert(a->sizeof_element == b->sizeof_element);

    uint32_t asz = a->size;

    ArrayIterator bit = iterator(b);
    void* v;
    while ((v = iterator_next(&bit))) {
        array_append(a, v);
    }

    assert(a->size == asz + b->size);
}
