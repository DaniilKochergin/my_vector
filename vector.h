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
#include <utility>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <iostream>

template<typename T>
struct vector {
private:
    struct buffer {

        buffer() : size(0), capacity(0), ref_count(1), data(nullptr) {};

        ~buffer() {
            for (size_t i = 0; i < size; ++i) {
                data[i].~T();
            }
            ::operator delete(data);
        }

        buffer(size_t _size, T *other, size_t cap) {
            data = static_cast<T *>(::operator new(cap * sizeof(T)));
            for (size_t i = 0; i < _size; ++i) {
                new(data + i) T(other[i]);
            }
            // memcpy(data, other, _size*sizeof(T));
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
    template<typename U>
    struct Iterator : std::iterator<std::random_access_iterator_tag, U> {

        friend struct vector<T>;

        Iterator() = default;

        ~Iterator() = default;

        template<typename V>
        Iterator(Iterator<V> const &other) {
            pointer = other.pointer;
        }

        template<typename V>
        Iterator &operator=(Iterator<V> const &other) {
            pointer = other.pointer;
        }

        Iterator &operator++() {
            pointer++;
            return *this;
        }

        Iterator operator++(int) {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        Iterator &operator--() {
            pointer--;
            return *this;
        }

        Iterator operator--(int) {
            iterator tmp(*this);
            --(*this);
            return tmp;
        }

        U &operator*() {
            return *pointer;
        }

        U *const operator->() {
            return pointer;
        }

        Iterator &operator+=(size_t _size) {
            pointer += _size;
            return *this;
        }

        Iterator operator+(size_t _size) {
            iterator tmp(*this);
            tmp += _size;
            return tmp;
        }

        Iterator &operator-=(size_t _size) {
            pointer -= _size;
            return *this;
        }

        Iterator operator-(size_t _size) {
            iterator tmp(*this);
            tmp -= _size;
            return tmp;
        }

        template<typename V>
        bool operator==(Iterator<V> const &b) const {
            return pointer == b.pointer;
        }

        template<typename V>
        bool operator<(Iterator<V> const &b) const {
            return pointer < b.pointer;
        }

        template<typename V>
        bool operator>(Iterator<V> const &b) const {
            return pointer > b.pointer;
        }

        template<typename V>
        bool operator!=(Iterator<V> const &b) const {
            return pointer != b.pointer;
        }

        template<typename V>
        bool operator>=(Iterator<V> const &b) const {
            return pointer >= b.pointer;
        }

        template<typename V>
        bool operator<=(Iterator<V> const &b) const {
            return pointer <= b.pointer;
        }

        template<typename V>
        Iterator(V *p) {
            pointer = p;
        }

    private:

        U *pointer;
    };


    ~vector() noexcept {
        if (is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
        } else {
            //  any_obj.small.~T();
        }
    }

    vector() noexcept : my_flags(0) {
        set_empty(1);
        set_big(1);
    }

    void clear() {
        if (is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
        } else {
            any_obj.small.~T();
        }
        new(&any_obj) cur();
        set_big(1);
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
        if (*this == other) {
            return *this;
        }
        if (!is_big() && !other.is_big()) {
            any_obj.small.~T();
            new(&any_obj.small) T(other.any_obj.small);
        } else if (is_big() && other.is_big()) {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
            any_obj.big = other.any_obj.big;
            any_obj.big->ref_count++;
        } else if (!is_big() && other.is_big()) {
            any_obj.small.~T();
            any_obj.big = other.any_obj.big;
            any_obj.big->ref_count++;
        } else {
            any_obj.big->ref_count--;
            if (any_obj.big->ref_count == 0) {
                delete (any_obj.big);
            }
            new(&any_obj.small) T(other.any_obj.small);
        }
        my_flags = other.my_flags;
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
        // new (data + size_) T(val)
        if (empty() && !is_big()) {
            new(&any_obj.small) T(val);
            set_empty(0);
        } else {
            if (!is_big()) {
                T *tmp = new T(any_obj.small);
                any_obj.small.~T();
                any_obj.big = new buffer(5);
                any_obj.big->size = 2;
                any_obj.big->ref_count = 1;
                any_obj.big->capacity = 5;
                new(any_obj.big->data) T(*tmp);
                //any_obj.big->data[0] = tmp;
                new(any_obj.big->data + 1) T(val);
                //any_obj.big->data[1] = val;
                set_big(1);
                set_empty(0);
                delete tmp;
            } else {
                T *tmp = new T(val);
                make_uniq();
                ensure_capacity();
                new(any_obj.big->data + size()) T(*tmp);
                //any_obj.big->data[size()] = val;
                any_obj.big->size++;
                set_empty(0);
                delete (tmp);
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
            any_obj.small.~T();
            any_obj.big = tmp;
            set_big(1);
        }

    }

    T const *data() const {
        if (is_big()) {
            return any_obj.big->data;
        } else {
            return &any_obj.small;
        }
    }

    T *data() {
        if (is_big()) {
            return any_obj.big->data;
        } else {
            return &any_obj.small;
        }
    }


    T const &front() const {
        if (is_big()) {
            return any_obj.big->data[0];
        }
        return any_obj.small;
    };

    T const &back() const {
        if (is_big()) {
            return any_obj.big->data[size() - 1];
        }
        return any_obj.small;
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
            return any_obj.big->data[size() - 1];
        }
        return any_obj.small;
    }

    void pop_back() {
        if (!is_big()) {
            set_empty(1);
            any_obj.small.~T();
        } else {
            any_obj.big->size--;
            any_obj.big->data[size()].~T();
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

    typedef T value_type;

    typedef Iterator<T> iterator;
    typedef Iterator<T const> const_iterator;
    typedef std::reverse_iterator<iterator>
            reverse_iterator;
    typedef std::reverse_iterator<const_iterator>
            const_reverse_iterator;

    friend iterator;
    friend const_iterator;

    iterator insert(const_iterator pos, T const &val) {
        vector tmp;
        const_iterator it = begin();
        for (; it != pos; ++it) {
            tmp.push_back(*it);
        }
        tmp.push_back(val);
        iterator it1 = tmp.end();
        --it1;
        for (; it != end(); ++it) {
            tmp.push_back(*it);
        }
        *this = tmp;
        return it1;
    }

    iterator erase(const_iterator pos) {
        vector tmp;
        const_iterator it = begin();
        for (; it != pos; ++it) {
            tmp.push_back(*it);
        }
        if (it != end()) {
            ++it;
        }
        iterator it3 = tmp.end();
        for (; it != end(); ++it) {
            tmp.push_back(*it);
        }
        *this = tmp;
        return it3;
    }

    iterator erase(const_iterator first, const_iterator last) {
        vector tmp;
        const_iterator it = begin();
        for (; it != first; ++it) {
            tmp.push_back(*it);
        }
        it = last;
        iterator it3 = tmp.end();
        for (; it != end(); ++it) {
            tmp.push_back(*it);
        }
        *this = tmp;
        return it3;
    }

    const_iterator begin() const noexcept {
        if (is_big()) {
            return const_iterator(any_obj.big->data);
        } else {
            return const_iterator(&any_obj.small);
        }
    }

    const_iterator end() const noexcept {
        if (is_big()) {
            return const_iterator(any_obj.big->data + size());
        } else {
            return const_iterator(&any_obj.small + 1);
        }
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    iterator begin() noexcept {
        if (is_big()) return iterator(any_obj.big->data);
        else {
            return iterator(&any_obj.small);
        }
    }

    iterator end() noexcept {
        if (is_big()) {
            return iterator(any_obj.big->data + size());
        } else {
            return iterator(&any_obj.small + 1);
        }
    }

    friend bool operator==(vector const &a, vector const &b) {
        if (a.is_big() != b.is_big()) {
            return false;
        }
        if (a.is_big()) {
            if (a.size() != b.size()) {
                return false;
            }
            for (size_t i = 0; i < a.size(); ++i) {
                if (a[i] != b[i]) return false;
            }
            return true;
        } else {
            if (a.empty() != b.empty()) return false;
            return !(!a.empty() && !b.empty() && a.any_obj.small != b.any_obj.small);
        }
    }

    friend bool operator<(vector const &a, vector const &b) {
        for (size_t i = 0; i < std::min(b.size(), a.size()); ++i) {
            if (a[i] > b[i]) {
                return false;
            }
            if (a[i] < b[i]) {
                return true;
            }
        }
        return a.size() < b.size();
    }

    friend bool operator<=(vector const &a, vector const &b) {
        return a < b || a == b;
    }

    friend bool operator>=(vector const &a, vector const &b) {
        return !(a < b);
    }

    friend bool operator>(vector const &a, vector const &b) {
        return (a >= b) && a != b;
    }

    friend bool operator!=(vector const &a, vector const &b) {
        return !(a == b);
    }

    friend void swap(vector &a, vector &b) {
        if (a.is_big() && b.is_big()) {
            buffer *tmp = b.any_obj.big;
            b.any_obj.big = a.any_obj.big;
            a.any_obj.big = tmp;
            //сделать поэлементное копирование!
            // swap(a.any_obj.big, b.any_obj.big);
        } else if (!a.is_big() && !b.is_big()) {
            T *tmp = new T(b.any_obj.small);
            b.any_obj.small.~T();
            new(&b.any_obj.small) T(a.any_obj.small);
            a.any_obj.small.~T();
            new(&a.any_obj.small) T(*tmp);
            delete (tmp);
        } else if (!a.is_big() && b.is_big()) {
            buffer *tmp = b.any_obj.big;
            new(&b.any_obj.big) T(a.any_obj.small);
            a.any_obj.small.~T();
            a.any_obj.big = tmp;

        } else {
            buffer *tmp = a.any_obj.big;
            new(&a.any_obj.big) T(b.any_obj.small);
            b.any_obj.small.~T();
            b.any_obj.big = tmp;
        }
        std::swap(a.my_flags, b.my_flags);
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
                // any_obj.big->~buffer();
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

    union cur {
        buffer *big;
        T small;

        cur() : big(new buffer()) {}

        ~cur() {
        }
    } any_obj;


//fitst bit - empty, second bit - big
    uint8_t my_flags;
};


#endif //VECTOR_VECTOR_H
