#pragma once
#ifndef _Vector_h
#define _Vector_h

#include <iostream>
#include <cstdint>
#include <stdexcept>

using std::cout;
using std::endl;

namespace epl{
    
    template <typename T> class vector
    {
    private:
        uint64_t capacity, front;
        uint64_t length;
        uint64_t default_capacity = 8;
        T* vector_pointer;
        
        void copy(const vector& that) {
            this->length = that.length;
            this->capacity = that.capacity;
            this->front = that.front;
            vector_pointer = (T*) ::operator new(capacity * sizeof(T));
            for (uint64_t i = 0; i < length; i++) {
                uint64_t index = getIndex(i);
                new (vector_pointer+index) T{that[i]};
            }
        }
        
        void destroy(void){
            if(length==0){
                ::operator delete[](vector_pointer);
            }else{
                for(uint64_t i = 0; i<length; i++){
                    uint64_t index = getIndex(i);
                    vector_pointer[index].~T();
                }
                ::operator delete[](vector_pointer);
            }
        }
        
    public:
        vector() {
            length = 0;
            front = 0;
            capacity = default_capacity;
            vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
        }
        
        explicit vector(uint64_t n) {
            if ( n==0 ) {
                vector();
                return;
            }
            capacity = n;
            length = n;
            front = 0;
            vector_pointer = (T*) ::operator new(n*sizeof(T));
            for(uint64_t i = 0; i<length; i++){
                new(vector_pointer+i) T{};
            }
        }
        
        vector<T> (vector<T> const& that){
            copy(that);
        }
        
        vector<T> (vector<T>&& other){
            this->length = other.length;
            this->capacity = other.capacity;
            this->front = other.front;
            //destroy();
            this->vector_pointer = other.vector_pointer;
            other.vector_pointer = nullptr;
            other.length = 0;
        }
        
        vector& operator=(const vector& that){
            if(this != &that){
                destroy();
                copy(that);
            }
            return *this;
        }
        
        vector& operator=(vector&& other) {
            vector_pointer = std::move(other.vector_pointer);
//            other.vector_pointer = nullptr;
            this->length = other.length;
            this->capacity = other.capacity;
            this->front = other.front;
            return *this;
        }
        
        ~vector(void){
            destroy();
        }
        
        T& operator[](uint64_t k){
            if( k >= length ) { throw std::out_of_range{"index out of range"}; }
            else{
                uint64_t index = (front+capacity+k)%(capacity);
                return vector_pointer[index];
            }
        }
        
        const T& operator[](uint64_t k) const {
            if( k >= length ) { throw std::out_of_range{"index out of range"}; }
            else{
                uint64_t index = (front+capacity+k)%(capacity);
                return vector_pointer[index];
            }
        }

        uint64_t size(void) const{
            return this->length;
        }
        
        void push_back(const T &val){
            T temp{val};
            if(length == capacity){
                resize();
            }
            uint64_t index = getIndex(length);
            new (vector_pointer+index) T{std::move(temp)};
            length++;
        }
        
        void push_back(T&& val){
            if(length == capacity){
                resize();
            }
            uint64_t index = getIndex(length);
            new (vector_pointer+index) T{std::move(val)};
            length++;
        }
        
        void push_front(const T& val){
            T temp{val};
            if(length >= capacity){
                resize();
            }
            uint64_t index = getIndex(-1);
            new (vector_pointer+index) T{std::move(temp)};
            front = index;
            length++;
        }
        
        void push_front(T&& val){
            if(length >= capacity){
                resize();
            }
            uint64_t index = getIndex(-1);
            new (vector_pointer+index) T{std::move(val)};
            front = index;
            length++;
        }
        
        void pop_back(){
            if(length==0){
                throw std::out_of_range("empty vector");
            }else{
                vector_pointer[getIndex(length-1)].~T();
                length--;
            }
        }
        
        void pop_front(){
            if(length==0){
                throw std::out_of_range("empty vector");
            }else{
                vector_pointer[getIndex(0)].~T();
                front = getIndex(1);
                length--;
            }
        }
        
        uint64_t getIndex(uint64_t index) {
            return (front+capacity+index)%(capacity);
        }
        
        void resize(){
            T* temp = vector_pointer;
            vector_pointer = (T*) ::operator new((capacity*2) * sizeof(T));
            for(uint64_t i=0; i<length; i++){
                uint64_t index = getIndex(i);
                new (vector_pointer+i) T{std::move(temp[index])};
            }
            this->capacity = capacity*2;
            this->front = 0;
        }
    };
}

#endif /* Vector_h */
