//
// Created by daniil on 15.06.19.
//

#ifndef VECTOR_VECTOR_H
#define VECTOR_VECTOR_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <memory.h>

template<typename T>
struct vector {
private:
    struct buffer {

        buffer() : size(0), capacity(0), ref_count(0), data(nullptr){};

        ~buffer() {
            ::operator delete(data);
        }

        buffer(size_t _size, T *other, size_t cap) {
            data = static_cast<T *>(::operator new(cap * sizeof(T)));
            mempcpy(data, other, _size * sizeof(T));
        }

        buffer(size_t _size) {
            data = static_cast<T *>(::operator new(_size * sizeof(T)));
        }

        size_t size;
        size_t capacity;
        size_t ref_count;
        T *data;
    };

public:

    ~vector() {
        if (is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
        }
    }

    typedef T value_type;

    //typedef Iterator iterator;
     vector() noexcept : my_flags(0) {
        set_empty(1);
        set_big(0);
    }

    void clear() {
        if (is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
            set_big(0);
        }
        set_empty(1);
    }

    //strong
    vector(vector const &other) : my_flags(other.my_flags) {
        if (!is_big()) {
            new(&any_obj.small) T(other.any_obj.small);
        } else {
            any_obj.big = other.any_obj.big;
            any_obj.big->ref_count++;
        }
    }

    //strong
    vector &operator=(vector const &other) {
        if (!is_big()) {
            new(&any_obj.small) T(other.any_obj.small);
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

    void push_back(T const && val) {
        // new (data + size_) T(val)
        if (empty() && !is_big()) {
            new(&any_obj.small) T(val);
            set_empty(0);
        } else {
            if (!is_big()) {
                //T tmp = any_obj.small;
                any_obj.big = new buffer(5);
                any_obj.big->size = 2;
                any_obj.big->ref_count = 1;
                any_obj.big->capacity = 5;
                new(any_obj.big->data) T(any_obj.small);
                //any_obj.big->data[0] = tmp;
                new(any_obj.big->data + 1) T(val);
                //any_obj.big->data[1] = val;
                set_big(1);
                set_empty(0);
            } else {
                make_uniq();
                ensure_capacity();
                new(any_obj.big->data +size()) T(val);
                //any_obj.big->data[size()] = val;
                any_obj.big->size++;
                set_empty(0);
            }
        }
    }


    bool is_big() const noexcept {
        return (my_flags >> 1u) & 1;
    }

    bool empty() const noexcept {
        return (my_flags & 1u);
    }

    void resize(size_t n) {
        if (is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                any_obj.big->~buffer();
                delete (any_obj.big);
            }
        }
        buffer *tmp = new buffer(n);
        tmp->size = n;
        tmp->capacity = n;
        tmp->ref_count = 1;
        any_obj.big = tmp;

    }

    void reserve(size_t cap) {
        if (is_big()) {
            make_uniq();
            buffer *tmp = new buffer(size(), any_obj.big->data, cap);
            tmp->size = size();
            tmp->ref_count = 1;
            tmp->capacity = cap;
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
            any_obj.big = tmp;
        } else {
            buffer *tmp = new buffer(cap);
            tmp->size = size();
            tmp->ref_count = 1;
            new(tmp->data) T(any_obj.small);
            tmp->capacity = cap;
            ~any_obj.small;
            any_obj.big = tmp;
            set_big(1);
        }

    }


    T *data() {
        if (is_big()) {
            return any_obj.big->data;
        } else {
            return &any_obj.small;
        }
    }

    T &front() {
        if (is_big()) {
            make_uniq();
            return any_obj.big->data[0];
        }
        return any_obj.small;
    }

    T &back() {
        if (is_big()) {
            make_uniq();
            return any_obj.big->data[size()];
        }
        return any_obj.small;
    }

    void pop_back() {
        if (!is_big()) {
            set_empty(1);
            ~any_obj.small;
        } else {
            any_obj.big->size--;
            if (any_obj.big->size == 0) {
                set_empty(1);
            }
        }
    }

    void shrink_to_fit() {
        if (is_big()) {
            buffer *tmp = new buffer(size(), any_obj.big->data, size());
            tmp->size = size();
            tmp->capacity = size();
            tmp->ref_count = 1;
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
            any_obj.big = tmp;
        }

    }

    size_t capacity() const {
        if (!is_big()) {
            return 1;
        }
        return any_obj.big->capacity;

    }

private:


    void ensure_capacity() {
        if (size() == any_obj.big->capacity) {
            buffer *tmp = new buffer(size(), any_obj.big->data, size() * 2 + 10);
            tmp->capacity = size() * 2 + 10;
            tmp->size = size();
            tmp->ref_count = 1;
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
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
        buffer *tmp = new buffer(size(), any_obj.big->data, size() * 2);
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
    union cur{
        buffer * big;
        T small;
        cur(){}
        ~cur(){};
    } any_obj;
//fitst bit - empty, second bit - big
    uint8_t my_flags;
};


#endif //VECTOR_VECTOR_H
