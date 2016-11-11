#pragma once
#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <stdexcept>
#include <utility>
//#include "Generic.h"

//Utility gives std::rel_ops which will fill in relational
//iterator operations so long as you provide the
//operators discussed in class.  In any case, ensure that
//all operations listed in this website are legal for your
//iterators:
//http://www.cplusplus.com/reference/iterator/RandomAccessIterator/
using namespace std::rel_ops;
using std::cout;
using std::endl;

namespace epl {

	class invalid_iterator {
	public:
		enum SeverityLevel { SEVERE, MODERATE, MILD, WARNING };
		SeverityLevel level;

		invalid_iterator(SeverityLevel level = SEVERE) { this->level = level; }
		virtual const char* what() const {
			switch (level) {
			case WARNING:   return "Warning"; // not used in Spring 2015
			case MILD:      return "Mild";
			case MODERATE:  return "Moderate";
			case SEVERE:    return "Severe";
			default:        return "ERROR"; // should not be used
			}
		}
	};

	template <typename T>
	class vector {
	private:
		uint64_t capacity, front;
		uint64_t length;
		uint64_t default_capacity = 8;
		T* vector_pointer;

		void copy(const vector<T>& that) {
			this->length = that.length;
			this->capacity = that.capacity;
			this->front = that.front;
			vector_pointer = (T*) ::operator new(capacity * sizeof(T));
			for (uint64_t i = 0; i < length; i++) {
				uint64_t index = getIndex(i);
				new (vector_pointer + index) T{ that[i] };
			}
		}

		void destroy(void) {
			if (length == 0) {
				::operator delete(vector_pointer);
			}
			else {
				for (uint64_t i = 0; i < length; i++) {
					uint64_t index = getIndex(i);
					vector_pointer[index].~T();
				}
				::operator delete(vector_pointer);
			}
		}

	public:
		uint64_t version_vector = 0;
		uint64_t version_reallocate = 0;
		vector<T>() {
			length = 0;
			front = 0;
			capacity = default_capacity;
			vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
			version_vector = 0;
			version_reallocate = 0;
		}

		explicit vector<T>(uint64_t n) {
			if (n == 0) {
				length = 0;
				front = 0;
				capacity = default_capacity;
				vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
				version_vector = 0;
				version_reallocate = 0;
				return;
			}
			capacity = n;
			length = n;
			front = 0;
			vector_pointer = (T*) ::operator new(n*sizeof(T));
			for (uint64_t i = 0; i < length; i++) {
				new(vector_pointer + i) T{};
			}
			version_vector = 0;
			version_reallocate = 0;
		}

		vector(std::initializer_list<T> list) {
			if (list.size() == 0) {
				length = 0;
				front = 0;
				capacity = default_capacity;
				vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
				version_vector = 0;
				version_reallocate = 0;
				return;
			}
			else {
				capacity = list.size();
				length = list.size();
				front = 0;
				vector_pointer = (T*) ::operator new(length*sizeof(T));
				for (uint64_t i = 0; i < length; i++) {
					new(vector_pointer + i) T{ *(list.begin() + i) };
				}
				version_vector = 0;
				version_reallocate = 0;
			}
		}

		template <typename IT>
		vector(IT begin, IT end) :vector(begin, end, typename std::iterator_traits<IT>::iterator_category{}) {}

		template<typename IT>
		vector(IT begin, IT end, std::random_access_iterator_tag) {
				uint64_t n = end - begin;
				if (n == 0) {
					length = 0;
					front = 0;
					capacity = default_capacity;
					vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
					version_vector = 0;
					version_reallocate = 0;
					return;
				}
				capacity = n;
				length = 0;
				front = 0;
				vector_pointer = (T*) ::operator new(n*sizeof(T));
				//for (int i = 0; i < length; i++) {
				//	new(vector_pointer + i) T{*begin};
				//	begin++;
				//}
				//for (int i = 0; i < length; i++) {
				//	this->operator[](i) = *begin;
				//	begin++;
				//}
				while (begin != end) {
					push_back(*begin);
					begin++;
				}
				version_reallocate = 0;
				version_vector = 0;
		}

		template<typename IT>
		vector(IT begin, IT end, std::forward_iterator_tag) {
			length = 0;
			front = 0;
			capacity = default_capacity;
			vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
			while (begin != end) {
				push_back(*begin);
				begin++;
			}
			version_vector = 0;
			version_reallocate = 0;
		}

		//template<typename It>
		//vector(It begin, It end) {
		//	typename iterator_traits<It>::iterator_category tag_var{};
		//	typename iterator_traits<It>::value_type t{};
		//	constructByIterator(begin, end, tag_var, t);
		//}

		//template<typename It, typename C>
		//void constructByIterator(It begin, It end, std::forward_iterator_tag, C) {
		//	length = 0;
		//	front = 0;
		//	capacity = default_capacity;
		//	vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
		//	while (begin != end) {
		//		push_back(*begin);
		//		begin++;
		//	}
		//	version_vector = 0;
		//	version_reallocate = 0;
		//}

		//template<typename It, typename C>
		//void constructByIterator(It begin, It end, std::random_access_iterator_tag, C) {
		//	uint64_t n = end - begin;
		//	if (n == 0) {
		//		length = 0;
		//		front = 0;
		//		capacity = default_capacity;
		//		vector_pointer = (T*) ::operator new(default_capacity*sizeof(T));
		//		version_vector = 0;
		//		version_reallocate = 0;
		//		return;
		//	}
		//	capacity = n;
		//	length = 0;
		//	front = 0;
		//	vector_pointer = (T*) ::operator new(n*sizeof(T));
		//	//for (int i = 0; i < length; i++) {
		//	//	new(vector_pointer + i) T{*begin};
		//	//	begin++;
		//	//}
		//	//for (int i = 0; i < length; i++) {
		//	//	this->operator[](i) = *begin;
		//	//	begin++;
		//	//}
		//	while (begin != end) {
		//		push_back(*begin);
		//		begin++;
		//	}
		//	version_reallocate = 0;
		//	version_vector = 0;
		//}


		vector<T>(vector<T> const& that) {
			copy(that);
			version_reallocate = 0;
			version_reallocate = 0;
		}

		vector<T>(vector<T>&& other) {
			this->length = other.length;
			this->capacity = other.capacity;
			this->front = other.front;
			//destroy();
			this->vector_pointer = other.vector_pointer;
			version_reallocate = 0;
			version_reallocate = 0;
			other.vector_pointer = nullptr;
			other.length = 0;
			other.version_vector++;
			other.version_reallocate++;
		}

		vector<T>& operator=(const vector<T>& that) {
			if (this != &that) {
				destroy();
				copy(that);
				this->version_vector++;
				this->version_reallocate++;
			}
			return *this;
		}

		vector<T>& operator=(vector<T>&& other) {
			if (this != &other) {
				destroy();
				this->length = other.length;
				this->capacity = other.capacity;
				this->front = other.front;
				this->vector_pointer = other.vector_pointer;
				other.vector_pointer = nullptr;
				other.length = 0;
				this->version_vector++;
				this->version_reallocate++;
				other.version_vector++;
				other.version_reallocate++;
			}

			return *this;
		}

		~vector(void) {
			destroy();
		}

		T& operator[](uint64_t k) {
			if (k >= length) { throw std::out_of_range{ "index out of range" }; }
			else {
				uint64_t index = (front + capacity + k) % (capacity);
				return vector_pointer[index];
			}
		}

		const T& operator[](uint64_t k) const {
			if (k >= length) { throw std::out_of_range{ "index out of range" }; }
			else {
				uint64_t index = (front + capacity + k) % (capacity);
				return vector_pointer[index];
			}
		}

		uint64_t size(void) const {
			return this->length;
		}

		void push_back(const T &val) {
			T temp{ val };
			if (length == capacity) {
				resize();
				this->version_reallocate++;
			}
			uint64_t index = getIndex(length);
			new (vector_pointer + index) T{ std::move(temp) };
			length++;
			this->version_vector++;
		}

		void push_back(T&& val) {
			if (length == capacity) {
				resize();
				this->version_reallocate++;
			}
			uint64_t index = getIndex(length);
			new (vector_pointer + index) T{ std::move(val) };
			length++;
			this->version_vector++;
		}

		void push_front(const T& val) {
			T temp{ val };
			if (length >= capacity) {
				resize();
				this->version_reallocate++;
			}
			uint64_t index = getIndex(-1);
			new (vector_pointer + index) T{ std::move(temp) };
			front = index;
			length++;
			this->version_vector++;
		}

		void push_front(T&& val) {
			if (length >= capacity) {
				resize();
				this->version_reallocate++;
			}
			uint64_t index = getIndex(-1);
			new (vector_pointer + index) T{ std::move(val) };
			front = index;
			length++;
			this->version_vector++;
		}

		void pop_back() {
			if (length == 0) {
				throw std::out_of_range("empty vector");
			}
			else {
				vector_pointer[getIndex(length - 1)].~T();
				length--;
			}
			this->version_vector++;
		}

		void pop_front() {
			if (length == 0) {
				throw std::out_of_range("empty vector");
			}
			else {
				vector_pointer[getIndex(0)].~T();
				front = getIndex(1);
				length--;
			}
			this->version_vector++;
		}

		uint64_t getIndex(uint64_t index) {
			return (front + capacity + index) % (capacity);
		}

		void resize() {
			T* temp = vector_pointer;
			vector_pointer = (T*) ::operator new((capacity * 2) * sizeof(T));
			for (uint64_t i = 0; i < length; i++) {
				uint64_t index = getIndex(i);
				new (vector_pointer + i) T{ std::move(temp[index]) };
			}
			this->capacity = capacity * 2;
			this->front = 0;
			::operator delete (temp); //WHY
		}
	private:
		class non_constIterator;
		class constIterator
			: public std::iterator<std::random_access_iterator_tag, T> {
		private:
			const T* ptr;
			const vector* my_vector;
			uint64_t version_iterator = 0;
			uint64_t version_reallocate = 0;
			uint64_t index = 0;
		public:
			constIterator() {
				ptr=nullptr;
				my_vector= nullptr;
				version_iterator = 0;
				version_reallocate = 0;
				index = 0;
			}

			constIterator(constIterator const& ci) {
				ci.use_iterator();
				ptr = ci.ptr;
				my_vector = ci.my_vector;
				version_iterator = ci.version_iterator;
				version_reallocate = ci.version_reallocate;
				index = ci.index;
			}

			constIterator(non_constIterator const& ni) {
				ni.use_iterator();
				ptr = ni.ptr;
				my_vector = ni.my_vector;
				version_iterator = ni.version_iterator;
				version_reallocate = ni.version_reallocate;
				index = ni.index;
			}

			const T& operator*(void) {
				use_iterator();
				return my_vector->operator[](index);
			}

			const T& operator[](uint64_t k) {
				index = index + k;
				use_iterator();
				index = index - k;
				return my_vector->operator[](index+k);
			}

			constIterator& operator++(void) {
				use_iterator();
				++index;
				return *this;
			}

			constIterator& operator--(void) {
				use_iterator();
				--index;
				return *this;
			}

			bool operator==(constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->ptr == rhs.ptr && this->index == rhs.index);
			}

			bool operator!=(constIterator const& rhs) const {
				return !(*this == rhs);
			}

			bool operator<(constIterator& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->index < rhs.index);
			}

			bool operator>=(constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				return !(*this < rhs);
			}

			bool operator>(constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->index > rhs.index);
			}

			bool operator<=(constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				return !(*this > rhs);
			}

			constIterator operator++(int) {
				use_iterator();
				constIterator t{ *this };
				operator++();
				return t;
			}

			constIterator operator--(int) {
				use_iterator();
				constIterator t{ *this };
				operator--();
				return t;
			}

			constIterator operator+(uint64_t k) {
				use_iterator();
				constIterator result(*this);
				result.index = this->index + k;
				return result;
			}

			friend constIterator operator+(const int &num, constIterator &ite) {
				constIterator result(ite + num);
				return result;
			}

			constIterator operator+=(uint64_t k) {
				use_iterator();
				index = index + k;
				return *this;
			}

			constIterator operator-(uint64_t k) {
				use_iterator();
				constIterator result(*this);
				result.index = this->index - k;
				return result;
			}

			constIterator operator-=(uint64_t k) {
				use_iterator();
				index = index - k;
				return *this;
			}

			uint64_t operator-(constIterator const& rhs) {
				use_iterator();
				rhs.use_iterator();
				return index - rhs.index;
			}

			//constIterator& operator=(constIterator& rhs) {
			//	rhs.use_iterator();
			//	this->index = rhs.index;
			//	this->my_vector = rhs.my_vector;
			//	this->ptr = rhs.ptr;
			//	this->version_iterator = rhs.version_iterator;
			//	this->version_reallocate = rhs.version_reallocate;
			//}

			const T* operator->() {
				return &(my_vector->operator[](index));
			}

			void use_iterator() const {
				invalid_iterator ii;
				if (version_iterator == my_vector->version_vector) return;
				if (index < 0 || index > my_vector->length) {
					ii.level = epl::invalid_iterator::SEVERE;
				}
				else {
					if (version_reallocate == my_vector->version_reallocate) {
						ii.level = epl::invalid_iterator::MILD;
					}
					else {
						ii.level = epl::invalid_iterator::MODERATE;
					}
				}
				throw ii;
			}

			friend vector;
		};

		class non_constIterator
			: public std::iterator<std::random_access_iterator_tag, T> {
		private:
			T* ptr;
			vector* my_vector;
			uint64_t version_iterator = 0;
			uint64_t version_reallocate = 0;
			uint64_t index = 0;
		public:
			non_constIterator() {
				ptr = nullptr;
				my_vector = nullptr;
				version_iterator = 0;
				version_reallocate = 0;
				index = 0;
			}

			non_constIterator(non_constIterator const& ni) {
				ni.use_iterator();
				ptr = ni.ptr;
				my_vector = ni.my_vector;
				version_iterator = ni.version_iterator;
				version_reallocate = ni.version_reallocate;
				index = ni.index;
			}

			//operator constIterator() {
			//	return constIterator(*this);
			//}

			T& operator[](uint64_t k) {
				index = index + k;
				use_iterator();
				index = index - k;
				return my_vector->operator[](index + k);
			}

			T& operator*(void) {
				use_iterator();
				return my_vector->operator[](index);
			}

			non_constIterator& operator++(void) {
				use_iterator();
				++index;
				return *this;
			}

			non_constIterator& operator--(void) {
				use_iterator();
				--index;
				return *this;
			}

			bool operator==(non_constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->ptr == rhs.ptr && this->index == rhs.index);
			}

			bool operator!=(non_constIterator const& rhs) const {
				return !(*this == rhs);
			}

			bool operator<(non_constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->index < rhs.index);
			}

			bool operator>=(non_constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				return !(*this < rhs);
			}

			bool operator>(non_constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				use_iterator();
				rhs.use_iterator();
				return (this->index > rhs.index);
			}

			bool operator<=(non_constIterator const& rhs) const {
				/*if (this->my_vector != rhs.my_vector) return false;*/
				return !(*this > rhs);
			}

			non_constIterator operator++(int) {
				use_iterator();
				non_constIterator t{ *this };
				operator++();
				return t;
			}

			non_constIterator operator--(int) {
				use_iterator();
				non_constIterator t{ *this };
				operator--();
				return t;
			}

			non_constIterator operator+(uint64_t k) {
				use_iterator();
				non_constIterator result(*this);
				result.index = this->index + k;
				return result;
			}

			friend non_constIterator operator+(const int &num, non_constIterator &ite) {
				non_constIterator result(ite + num);
				return result;
			}

			non_constIterator operator+=(uint64_t k) {
				use_iterator();
				index = index + k;
				return *this;
			}

			non_constIterator operator-(uint64_t k) {
				use_iterator();
				non_constIterator result(*this);
				result.index = this->index - k;
				return result;
			}

			non_constIterator operator-=(uint64_t k) {
				use_iterator();
				index = index - k;
				return *this;
			}

			uint64_t operator-(non_constIterator const& rhs) {
				use_iterator();
				rhs.use_iterator();
				return index - rhs.index;
			}

			non_constIterator& operator=(non_constIterator& rhs) {
				rhs.use_iterator();
				this->index = rhs.index;
				this->my_vector = rhs.my_vector;
				this->ptr = rhs.ptr;
				this->version_iterator = rhs.version_iterator;
				this->version_reallocate = rhs.version_reallocate;
			}

			T* operator->() {
				return &(my_vector->operator[](index));
			}

			friend void swap(non_constIterator& a, non_constIterator& b) {
				non_constIterator temp(a);
				a = b;
				b = temp;
			}

			void use_iterator() const {
				invalid_iterator ii;
				if (version_iterator == my_vector->version_vector) return;
				if (index < 0 || index > my_vector->length) {
					ii.level = epl::invalid_iterator::SEVERE;
				}
				else {
					if (version_reallocate == my_vector->version_reallocate) {
						ii.level = epl::invalid_iterator::MILD;
					}
					else {
						ii.level = epl::invalid_iterator::MODERATE;
					}
				}
				throw ii;
			}

			friend vector;
		};

	public:
		using iterator = non_constIterator;
		using const_iterator = constIterator;

		iterator begin(void) {
			iterator p;
			p.ptr = vector_pointer;
			p.my_vector = this;
			p.index = 0;
			p.version_iterator = version_vector;
			p.version_reallocate = version_reallocate;
			return p;
		}

		const_iterator begin(void) const {
			const_iterator p;
			p.ptr = vector_pointer;
			p.my_vector = this;
			p.index = 0;
			p.version_iterator = version_vector;
			p.version_reallocate = version_reallocate;
			return p;
		}

		iterator end(void) {
			iterator p;
			p.ptr = vector_pointer;
			p.my_vector = this;
			p.index = length;
			p.version_iterator = version_vector;
			p.version_reallocate = version_reallocate;
			return p;
		}

		const_iterator end(void) const {
			const_iterator p;
			p.ptr = vector_pointer;
			p.my_vector = this;
			p.index = length;
			p.version_iterator = version_vector;
			p.version_reallocate = version_reallocate;
			return p;
		}

		template<typename... Args>
		void emplace_back(Args&&... args) {
			T temp{ std::forward<Args>(args)... };
			if (length == capacity) {
				resize();
				this->version_reallocate++;
			}
			uint64_t index = getIndex(length);
			new (vector_pointer + index) T{ std::move(temp) };
			length++;
			this->version_vector++;
		}
	};

		//template <typename type> class iterator_helper
		//	: public std::iterator<std::random_access_iterator_tag, type> {
		//private:
		//	type* ptr;
		//	vector const* my_vector;
		//	uint64_t version_iterator;
		//	uint64_t version_reallocate;
		//	uint64_t index;
		//	using Same = iterator_helper<type>;
		//public:
		//	type& operator*(void) {
		//		use_iterator();
		//		return *ptr;
		//	}

		//	Same& operator++(void) {
		//		use_iterator();
		//		++ptr;
		//		++index;
		//		return *this;
		//	}

		//	Same& operator--(void) {
		//		use_iterator();
		//		--ptr;
		//		--index;
		//		return *this;
		//	}

		//	bool operator==(Same const& rhs) const {
		//		/*if (this->my_vector != rhs.my_vector) return false;*/
		//		use_iterator();
		//		rhs.use_iterator();
		//		return this->ptr == rhs.ptr;
		//	}

		//	bool operator!=(Same const& rhs) const {
		//		return !(*this == rhs);
		//	}

		//	bool operator<(Same const& rhs) const {
		//		/*if (this->my_vector != rhs.my_vector) return false;*/
		//		use_iterator();
		//		rhs.use_iterator();
		//		return (this->ptr < rhs.ptr);
		//	}

		//	bool operator>=(Same const& rhs) const {
		//		/*if (this->my_vector != rhs.my_vector) return false;*/
		//		return !(*this < rhs);
		//	}

		//	bool operator>(Same const& rhs) const {
		//		/*if (this->my_vector != rhs.my_vector) return false;*/
		//		use_iterator();
		//		rhs.use_iterator();
		//		return (this->ptr > rhs.ptr);
		//	}

		//	bool operator<=(Same const& rhs) const {
		//		if (this->my_vector != rhs.my_vector) return false;
		//		return !(*this > rhs);
		//	}

		//	Same operator++(int) {
		//		use_iterator();
		//		Same t{ *this };
		//		operator++();
		//		return t;
		//	}

		//	Same operator--(int) {
		//		use_iterator();
		//		Same t{ *this };
		//		operator--();
		//		return t;
		//	}

		//	Same operator+(uint64_t k) {
		//		use_iterator();
		//		Same result{};
		//		result.ptr = this->ptr + k;
		//		index = index + k;
		//		return result;
		//	}

		//	friend Same operator+(const int &num, Same &ite) {
		//		Same result(ite + num);
		//		return result;
		//	}

		//	Same operator+=(uint64_t k) {
		//		use_iterator();
		//		this->ptr = this->ptr + k;
		//		index = index + k;
		//		return *this;
		//	}

		//	Same operator-(uint64_t k) {
		//		use_iterator();
		//		Same result{};
		//		result.ptr = this->ptr - k;
		//		index = index - k;
		//		return result;
		//	}

		//	Same operator-=(uint64_t k) {
		//		use_iterator();
		//		this->ptr = this->ptr - k;
		//		index = index - k;
		//		return *this;
		//	}

		//	int operator-(Same const& rhs) {
		//		use_iterator();
		//		rhs.use_iterator();
		//		return rhs.prt - this->ptr;
		//	}

		//	Same* operator->() {
		//		return this;
		//	}

		//	friend void swap(Same& a, Same& b) {
		//		Same temp(a);
		//		a = b;
		//		b = temp;
		//	}

		//	void use_iterator() const {
		//		invalid_iterator ii;
		//		if (version_iterator == my_vector->version_vector) return;
		//		if (index < 0 || index > my_vector->length) {
		//			ii.level = epl::invalid_iterator::SEVERE;
		//		}
		//		else {
		//			if (version_reallocate == my_vector->version_reallocate) {
		//				ii.level = epl::invalid_iterator::MILD;
		//			}
		//			else {
		//				ii.level = epl::invalid_iterator::MODERATE;
		//			}
		//		}
		//		throw ii;
		//	}

		//	friend vector;
		//};


	} //namespace epl

#endif
