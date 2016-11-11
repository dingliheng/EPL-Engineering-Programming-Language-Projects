//
//  Vector.h
//  Project1
//
//  Created by 丁力恒 on 2/4/16.
//  Copyright © 2016 丁力恒. All rights reserved.
//

#ifndef Vector_h
#define Vector_h

#include <iostream>
#include <cstdint>
#include <stdexcept>

using std::cout;
using std::endl;

namespace epl {
    template<typename T> class vector
    {
    private:
        uint64_t length, capacity, front, back;
        uint64_t default_capacity = 8;
        T* vector_pointer;
        
        void copy(const vector& that) {
            this->length = that.length;
            this->capacity = that.capacity;
            this->front = that.front;
            this->back = that.back;
            vector_pointer = (T*) ::operator new(capacity * sizeof(T));
            for (uint64_t i = that.front; i <= back; i++) {
                new (vector_pointer+back) T{that[i]};
            }
        }
        
        void destroy(void) {
            ::operator delete[](vector_pointer);
//            delete[] vector_pointer;
        }
        
        
    public:
        vector() {
            vector_pointer = (T*) ::operator new(default_capacity * sizeof(T));
            length = 0;
            capacity = default_capacity;
            front = back = 0;
        }
        
        explicit vector (uint64_t n) {
            if (n == 0) {
                vector();
                return;
            }
            if (n <= default_capacity) {
                vector_pointer = (T*) ::operator new(default_capacity * sizeof(T));
                length = n;
                capacity = default_capacity;
                front = 0;
                back = n-1;
                for(int i=0; i<n; i++) {
                    new (vector_pointer+i) T{};
                }
            }
            else {
                vector_pointer = (T*) ::operator new(n * sizeof(T));
                length = n;
                capacity = n;
                front = 0;
                back = n-1;
                for(int i=0; i<n; i++) {
                    new (vector_pointer+i) T{};
                }
            }
        }
        
        vector<T> (vector const& that) {
            copy(that);
        }
        
        vector<T> (vector&& other)
        : vector_pointer(std::move(other.vector_pointer))
        {
            this->length = other.length;
            this->capacity = other.capacity;
            this->front = other.front;
            this->back = other.back;
            other.vector_pointer = nullptr;
        }
        
        vector& operator=(const vector& rhs) {
            if (this != &rhs) {
                destroy();
                copy(rhs);
            }
            return *this;
        }
        
        vector& operator=(vector&& other) {
            vector_pointer = std::move(other.vector_pointer);
            other.vector_pointer = nullptr;
            this->length = other.length;
            this->capacity = other.capacity;
            this->front = other.front;
            this->back = other.back;
            return *this;
        }
        
        ~vector ()
        {
            destroy();
        }
        
        uint64_t size(void) const {
            return this->length;
        }
        
        T& operator[](uint64_t k) {
            if( k >= length) { throw std::out_of_range{"index out of range"}; }
            return vector_pointer[front+k];
        }
        
        const T& operator[](uint64_t k) const {
            if( k >= length ) { throw std::out_of_range{"index out of range"}; }
            return vector_pointer[front+k];
        }
        
        void push_back(const T &val) {
            if (length == 0) {
                new (vector_pointer+back) T{val};
                length++;
                return;
            }
            if( back == capacity-1) {
                back_resize();
            }
            back++;
            new (vector_pointer+back) T{val};
            length++;
        }
        
        void push_back(T&& val) {
            if (length == 0) {
                new (vector_pointer+back) T{std::move(val)};
                length++;
                return;
            }
            if( back == capacity-1) {
                back_resize();
            }
            back++;
            new (vector_pointer+back) T{std::move(val)};
            length++;
        }
        
        
        void push_front(const T& val) {
            if (length == 0) {
                new (vector_pointer+front) T{val};
                length++;
                return;
            }
            if( front == 0) {
                front_resize();
            }
            front--;
            new (vector_pointer+front) T{val};
            length++;
        }
        
        void push_front(T&& val) {
            if (length == 0) {
                new (vector_pointer+front) T{std::move(val)};
                length++;
                return;
            }
            if( front == 0) {
                front_resize();
            }
            front--;
            new (vector_pointer+front) T{std::move(val)};
            length++;
        }
        
        void pop_back()  // Remove the last element of the vector..
        {
            if(length > 0) {
                vector_pointer[back].~T();
                if(back > front) {
                    back--;
                }
                length--;
            }
            else { throw std::out_of_range{"subscript out of range"}; }
        }  //  Doesn't actually deallocate the element block (for performance)..
        
        void pop_front()  // Remove the last element of the vector..
        {
            if(length > 0) {
                vector_pointer[front].~T();
                if(back > front) {
                    front++;
                }
                length--;
            }
            else { throw std::out_of_range{"subscript out of range"}; }
        }
        
        void back_resize()  // Change the capacity of the vector to be at least equal to "newcapacity"
        {
            uint64_t newcapacity = this->capacity * 2;  // Keep the capacity of the vector as a power of 2 to avoid space wastage..
            T* temp = (T*) ::operator new(newcapacity * sizeof(T));
            int k = 0;
            for (uint64_t i = front; i <= back; i++) {
                new (temp+k) T{vector_pointer[i]};
                k++;
            }
            ::operator delete[](vector_pointer);
            vector_pointer = std::move(temp);
//            temp = nullptr;
            this->front = 0;
            this->back = length-1;
            this->capacity = newcapacity;
        }
        
        void front_resize()  // Change the capacity of the vector to be at least equal to "newcapacity"
        {
            uint64_t newcapacity = this->capacity * 2;  // Keep the capacity of the vector as a power of 2 to avoid space wastage..
            T* temp = (T*)::operator new (newcapacity * sizeof(T));
            for (uint64_t i = front; i <= back; i++) {
                new (temp+i+capacity/2) T{vector_pointer[i]};
            }
            ::operator delete[](vector_pointer);
            vector_pointer = std::move(temp);
            temp = nullptr;
            this->front = front+capacity/2;
            this->back = back+capacity/2;
            this->capacity = newcapacity;
        }
        
    };
}

#endif /* Vector_h */
