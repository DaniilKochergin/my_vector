//
// Created by daniil on 15.06.19.
//

#ifndef VECTOR_VECTOR_H
#define VECTOR_VECTOR_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <memory.h>

template<typename T>
struct vector {
    typedef T value_type;

    //typedef Iterator iterator;
    vector() noexcept : my_flags(0) {
        set_empty(1);
        set_big(0);
        any_obj.big;
    }

    //strong
    vector(vector const &other) : my_flags(other.my_flags) {
        if (!is_big()) {
            any_obj.small = other.any_obj.small;
        } else {
            any_obj.big = other.any_obj.big;
            any_obj.big->ref_count++;
        }
    }

    //strong
    vector &operator=(vector const &other) {
        if (!is_big()) {
            any_obj.small = other.any_obj.small;
        } else {
            any_obj.big = other.any_obj.big;
            any_obj.big->ref_count++;
        }
        return *this;
    }

    //strong
    T &operator[](size_t i) {
        if (!is_big()) {
            return any_obj.small;
        } else {
            make_uniq();
            return any_obj.big->data[i];
        }
    }

    T const &operator[](size_t i) const {
        if (!is_big()) {
            return any_obj.small;
        } else {
            return any_obj.big->data[i];
        }
    }

    size_t size() const noexcept {
        if (empty()) {
            return 0;
        }
        if (!is_big()) {
            return 1;
        } else {
            return any_obj.big->size;
        }
    }

    void push_back(T const &val) {
        if (empty()) {
            any_obj.small = val;
            set_empty(0);
        } else {
            if (!is_big()) {
                T tmp = any_obj.small;
                ~any_obj.small;
                any_obj.big = new buffer(5);
                any_obj.big->size = 2;
                any_obj.big->ref_count = 1;
                any_obj.big->capacity = 5;
                any_obj.big->data[0] = tmp;
                any_obj.big->data[1] = val;
                set_big(1);
            } else {
                make_uniq();
                ensure_capacity();
                any_obj.big->data[size()] = val;
                any_obj.big->size++;
            }
        }
    }


    bool is_big() const noexcept {
        return (my_flags >> 1u) & 1;
    }

    bool empty() const noexcept {
        return (my_flags&1u);
    }

private:

    struct buffer {

        ~buffer() {
            free(data);
        }

        buffer(size_t _size, T *other) {
            data = static_cast<T *>(malloc(_size * sizeof(T)));
            mempcpy(data, other, _size * sizeof(T));
        }

        buffer(size_t _size) {
            data = (T *) (malloc(_size * sizeof(T)));
        }

        size_t size;
        size_t capacity;
        size_t ref_count;
        T *data;
    };

    void ensure_capacity() {
        if (size() == any_obj.big->capacity) {
            buffer *tmp = new buffer(any_obj.big->size, any_obj.big->data);
            tmp->capacity = size() * 2;
            tmp->size = size();
            tmp->ref_count = 1;
            if (is_uniq()) {
                any_obj.big->~buffer();
            } else {
                any_obj.big->capacity--;
            }
            any_obj.big = tmp;
        }
    }

    bool is_uniq() {
        if (!is_big()) {
            return true;
        }
        return any_obj.big->ref_count == 1;
    }

    void make_uniq() {
        if (is_uniq()) {
            return;
        }
        buffer *tmp = new buffer(size() * 2, any_obj.big->data);
        tmp->capacity = size() * 2;
        tmp->size = size();
        tmp->ref_count = 1;
        any_obj.big->ref_count--;
        any_obj.big = tmp;
    }

    void set_empty(bool x) noexcept {
        if (x) {
            my_flags |= x;
        } else {
            my_flags >>= 1u;
            my_flags <<= 1u;
        }
    };


    void set_big(bool x) noexcept {
        if (x) {
            my_flags |= 2u;
        } else {
            my_flags &= 1u;
        }
    };
    //0 - size
    //1 - counter links
    //2 - capacity
    union {
        buffer *big;
        T small;
    } any_obj;
    //fitst bit - empty, second bit - big
    uint8_t my_flags;
};


#endif //VECTOR_VECTOR_H
