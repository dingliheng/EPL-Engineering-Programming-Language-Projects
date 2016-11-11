#ifndef _Valarray_h
#define _Valarray_h

#include <iostream>
#include <cstdint>
#include "Vector.h"

using std::cout;
using std::endl;
using std::complex;
using epl::vector;

namespace epl {
	template <typename T> class wrapper;
	template <typename TL, typename TR, typename OP> class BinaryProxy;
	template <typename T, typename OP> class UnaryProxy;
	template <typename T> class Scalar;
    
    template <typename> struct SRank {static constexpr int value = 0;};
	template <> struct SRank<int> { static constexpr int value = 1; };
	template <> struct SRank<float> { static constexpr int value = 2; };
	template <> struct SRank<double> { static constexpr int value = 3; };
	template <typename T> struct SRank<complex<T>> {
		static constexpr int value = SRank<T>::value;
	};
    
	template <int> struct SType;
	template <> struct SType<1> { using type = int; };
	template <> struct SType<2> { using type = float; };
	template <> struct SType<3> { using type = double; };
    
	template <typename> struct isComplex;
	template <typename T> struct isComplex { static constexpr int bool_value = 0;static constexpr bool value = false;};
	template <typename T> struct isComplex<std::complex<T>> { static constexpr int bool_value = 1; static constexpr bool value = true;};

	template <bool p, typename T> struct complex_type;
	template <typename T> struct complex_type<true, T> { using type = std::complex<T>; };
	template <typename T> struct complex_type<false, T> { using type = T; };

	template <typename T1, typename T2>
	struct choose_type {

		static constexpr int t1_rank = SRank<T1>::value;
		static constexpr int t2_rank = SRank<T2>::value;
		static constexpr int max_rank = (t1_rank > t2_rank) ? t1_rank : t2_rank;

		using max_type = typename SType<max_rank>::type;

		static constexpr int t1_isComplex = isComplex<T1>::bool_value;
		static constexpr int t2_isComplex = isComplex<T2>::bool_value;
		static constexpr bool is_complex = (t1_isComplex + t2_isComplex) >= 1;

		using type = typename complex_type<is_complex, max_type>::type;
	};

	template <typename T1, typename T2> struct choose_Vtype;
	template <typename T1, typename T2>
	struct choose_Vtype<wrapper<T1>, wrapper<T2>> { using type = typename choose_type<typename T1::value_type, typename T2::value_type>::type; };
	template <typename V, typename T>
	struct choose_Vtype<wrapper<V>, T> { using type = typename choose_type<typename V::value_type, T>::type; };
	template <typename T, typename V>
	struct choose_Vtype<T, wrapper<V>> { using type = typename choose_type<typename V::value_type, T>::type; };
    
    template <bool b, typename T>
    using enableIf = typename std::enable_if<b, T>::type;

	template <typename T> struct R_V { using type = T; };
	template <typename T> struct R_V<vector<T>> { using type = const vector<T>&; };
	template <typename T> using RefOrVal = typename R_V<T>::type;
    
	template <typename T>
	class const_iterator {
	private:
		const T container;
		uint64_t index;
	public:
		const_iterator(const T c, const uint64_t& i) : container(c), index(i) {}
		typename T::value_type operator*() const { return container[index]; }
        bool operator==(const const_iterator& that) const {
            return this->index == that.index;
        }
        bool operator!=(const const_iterator& that) const {
            return !(*this == that);
        }
        bool operator>(const_iterator const& rhs)const {
            return(rhs.index  < this->index);
        }
        bool operator<(const_iterator const& rhs)const {
            return(rhs.index > this->index);
        }
        bool operator<=(const_iterator const& rhs)const {
            return !(rhs.index  < this->index);
        }
        bool operator>=(const_iterator const&rhs)const {
            return !(this->index  < rhs.index);
        }
        const_iterator& operator++() {
			index += 1;
			return *this;
		}
		const_iterator operator++(int) {
			const_iterator temp{ *this };
			this->operator++();
			return temp;
		}
		const_iterator& operator--() {
			index -= 1;
			return *this;
		}
		const_iterator operator--(int) {
			const_iterator temp{ *this };
			this->operator--();
			return temp;
		}
	};
    
	template <typename TL, typename TR, typename OP>
	class BinaryProxy {
	public:
		RefOrVal<TL> left;
		RefOrVal<TR> right;
		OP op;
        
		using value_type = typename choose_type<typename TL::value_type, typename TR::value_type>::type;
		BinaryProxy() = default;
		BinaryProxy(RefOrVal<TL> _left, RefOrVal<TR> _right, OP _op): left(_left), right(_right),op( _op) { }
		BinaryProxy(const BinaryProxy& that): left(that.left),right(that.right),op(that.op) { }
        ~BinaryProxy() {
//            cout<<"bproxy is gone"<<endl;
        };
		uint64_t size() const { return left.size() < right.size() ? left.size() : right.size(); }
		typename OP::result_type operator[](uint64_t index) const { return op(left[index], right[index]); };
		const_iterator<BinaryProxy> begin() const { return const_iterator<BinaryProxy>(*this, 0); };
		const_iterator<BinaryProxy> end() const { return const_iterator<BinaryProxy>(*this, size()); };
	};
    
	template <typename T, typename OP>
	class UnaryProxy {
	public:
		RefOrVal<T> val;
		OP op;
        
		using value_type = typename T::value_type;
		UnaryProxy() = default;
		UnaryProxy(RefOrVal<T> _val, OP _op) : val(_val), op(_op) {}
		UnaryProxy(const UnaryProxy& that) : val(that.val), op(that.op) {}
		~UnaryProxy() = default;
		uint64_t size() const { return val.size(); }
		typename OP::result_type operator[](uint64_t index) const { return op(val[index]); };
		const_iterator<UnaryProxy> begin() const { return const_iterator<UnaryProxy>(*this, 0); };
		const_iterator<UnaryProxy> end() const { return const_iterator<UnaryProxy>(*this, size()); };
	};
    
	template <typename T>
	class Scalar {
	private:
		T num;
	public:
		using value_type = T;
		Scalar() { num = 0; }
		Scalar(const T& _num):num(_num) { }
		Scalar(const Scalar& that):num(that.num) { }
		~Scalar() = default;
		uint64_t size() const { return std::numeric_limits<uint64_t>::max(); }
		T operator[] (uint64_t n) const { return num; }
		const_iterator<Scalar> begin() const { return const_iterator<Scalar>(*this, 0); };
		const_iterator<Scalar> end() const { return const_iterator<Scalar>(*this, size()); };
	};
    
    template <typename T>
    struct sqrt_op {
        using result_type = typename complex_type<isComplex<T>::value, double>::type;
        result_type operator() (T x) const { return sqrt(x); }
    };
    
	template<typename T>
	using valarray = wrapper<vector<T>>;
    
	template <typename T>
	class wrapper : public T {
	public:
		wrapper() : T() {
//            cout<<"wrapper con"<<endl;
        }
        wrapper(const T& that) : T(that) {
//            cout<<"wrapper con"<<endl;
        }

		explicit wrapper(uint64_t n) : T(n) {
//            cout<<"wrapper con"<<endl;
        }

		template <typename type>
		wrapper(std::initializer_list<type> list) : T(list) {
//            cout<<"wrapper con"<<endl;
        }

		template <typename THAT>
		wrapper(const wrapper<THAT>& that) {
			for (int i = 0; i < that.size(); i++) {
				this->push_back(static_cast<typename T::value_type>(that[i]));
			}
//            cout<<"wrapper con"<<endl;
		}
        
        wrapper& operator=(const wrapper<T>& that) {
            if ((void*)this != (void*)&that) {
                uint64_t min_size = this->size() < that.size() ? this->size() : that.size();
                for (uint64_t i = 0; i < min_size; ++i) {
                    (*this)[i] = static_cast<typename T::value_type>(that[i]);
                }
            }
//            cout<<"wrapper con"<<endl;
            return *this;
        }
        
        template <typename T2>
        wrapper& operator=(const wrapper<T2>& that) {
            if ((void*)this != (void*)&that) {
                uint64_t min_size = this->size() < that.size() ? this->size() : that.size();
                for (uint64_t i = 0; i < min_size; ++i) {
                    (*this)[i] = static_cast<typename T::value_type>(that[i]);
                }
            }
//            cout<<"wrapper con"<<endl;
            return *this;
        }
        
        ~wrapper(){
//            cout<<"wrapper is gone"<<endl;
        };

        enableIf<true, wrapper<UnaryProxy<T, std::negate<typename T::value_type>>>> operator-(void) {
			using neg_op = typename std::negate<typename T::value_type>;
			return wrapper<UnaryProxy<T, neg_op>>(UnaryProxy<T, neg_op>(*this, neg_op{}));
		}
        
        template<typename OP>
        wrapper<UnaryProxy<T, OP>> apply(OP op) {
            UnaryProxy<T, OP> result(*this, op);
            return wrapper<UnaryProxy<T, OP>>(result);
        }
        
        wrapper<UnaryProxy<T, sqrt_op<typename T::value_type>>> sqrt(void) {
            return apply(sqrt_op<typename T::value_type>{});
        }
        
        using my_type = typename T::value_type;
        my_type sum() {
            my_type sum = 0;
            std::plus<my_type> plus{};
            for(int i=0; i<this->size(); i++) {
                sum=plus(this->operator[](i),sum);
            }
            return sum;
        }
        
        template <typename OP>
        typename OP::result_type accumulate(OP op) {
            if(this->size()==0) {typename OP::result_type accumulate=0;return accumulate;}
            else {
            typename OP::result_type accumulate=this->operator[](0);
            for(int i=1; i<this->size(); i++) {
                accumulate=op(accumulate,this->operator[](i));
            }
                return accumulate;
            }
            
        }
        
        
	};
    
	template <typename T1, typename T2, typename OP = std::plus<typename choose_Vtype<wrapper<T1>, wrapper<T2>>::type>>
    enableIf<SRank<typename T1::value_type>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<T1, T2, OP>>> operator+(const wrapper<T1>& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<T1, T2, OP> result{lhs, rhs, OP{}};
		return wrapper<BinaryProxy<T1, T2, OP>>(result);
	}
    
	template <typename T1, typename T2, typename OP = std::plus<typename choose_Vtype<wrapper<T1>, T2>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<T2>::value!=0,wrapper<BinaryProxy<T1, Scalar<T2>, OP>>> operator+(const wrapper<T1>& lhs, const T2& rhs) {
        BinaryProxy<T1, Scalar<T2>, OP> result{lhs, Scalar<T2>(rhs), OP{}};
		return wrapper<BinaryProxy<T1, Scalar<T2>, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::plus<typename choose_Vtype<T1, wrapper<T2>>::type>>
	enableIf<SRank<T1>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<Scalar<T1>, T2, OP>>> operator+(const T1& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<Scalar<T1>, T2, OP> result{Scalar<T1>(lhs), rhs, OP{}};
		return wrapper<BinaryProxy<Scalar<T1>, T2, OP>>(result);
	}


	template <typename T1, typename T2, typename OP = std::minus<typename choose_Vtype<wrapper<T1>, wrapper<T2>>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<T1, T2, OP>>> operator-(const wrapper<T1>& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<T1, T2, OP> result{lhs, rhs, OP{}};
        return wrapper<BinaryProxy<T1, T2, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::minus<typename choose_Vtype<wrapper<T1>, T2>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<T2>::value!=0,wrapper<BinaryProxy<T1, Scalar<T2>, OP>>> operator-(const wrapper<T1>& lhs, const T2& rhs) {
        BinaryProxy<T1, Scalar<T2>, OP> result{lhs, Scalar<T2>(rhs), OP{}};
        return wrapper<BinaryProxy<T1, Scalar<T2>, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::minus<typename choose_Vtype<T1, wrapper<T2>>::type>>
	enableIf<SRank<T1>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<Scalar<T1>, T2, OP>>> operator-(const T1& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<Scalar<T1>, T2, OP> result{Scalar<T1>(lhs), rhs, OP{}};
        return wrapper<BinaryProxy<Scalar<T1>, T2, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::multiplies<typename choose_Vtype<wrapper<T1>, wrapper<T2>>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<T1, T2, OP>>> operator*(const wrapper<T1>& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<T1, T2, OP> result{lhs, rhs, OP{}};
        return wrapper<BinaryProxy<T1, T2, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::multiplies<typename choose_Vtype<wrapper<T1>, T2>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<T2>::value!=0,wrapper<BinaryProxy<T1, Scalar<T2>, OP>>> operator*(const wrapper<T1>& lhs, const T2& rhs) {
        BinaryProxy<T1, Scalar<T2>, OP> result{lhs, Scalar<T2>(rhs), OP{}};
        return wrapper<BinaryProxy<T1, Scalar<T2>, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::multiplies<typename choose_Vtype<T1, wrapper<T2>>::type>>
	enableIf<SRank<T1>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<Scalar<T1>, T2, OP>>> operator*(const T1& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<Scalar<T1>, T2, OP> result{Scalar<T1>(lhs), rhs, OP{}};
        return wrapper<BinaryProxy<Scalar<T1>, T2, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::divides<typename choose_Vtype<wrapper<T1>, wrapper<T2>>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<T1, T2, OP>>> operator/(const wrapper<T1>& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<T1, T2, OP> result{lhs, rhs, OP{}};
        return wrapper<BinaryProxy<T1, T2, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::divides<typename choose_Vtype<wrapper<T1>, T2>::type>>
	enableIf<SRank<typename T1::value_type>::value!=0 && SRank<T2>::value!=0,wrapper<BinaryProxy<T1, Scalar<T2>, OP>>> operator/(const wrapper<T1>& lhs, const T2& rhs) {
        BinaryProxy<T1, Scalar<T2>, OP> result{lhs, Scalar<T2>(rhs), OP{}};
        return wrapper<BinaryProxy<T1, Scalar<T2>, OP>>(result);
	}

	template <typename T1, typename T2, typename OP = std::divides<typename choose_Vtype<T1, wrapper<T2>>::type>>
	enableIf<SRank<T1>::value!=0 && SRank<typename T2::value_type>::value!=0,wrapper<BinaryProxy<Scalar<T1>, T2, OP>>> operator/(const T1& lhs, const wrapper<T2>& rhs) {
        BinaryProxy<Scalar<T1>, T2, OP> result{Scalar<T1>(lhs), rhs, OP{}};
        return wrapper<BinaryProxy<Scalar<T1>, T2, OP>>(result);
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& out, const wrapper<T>& w) {
		for (uint64_t i = 0; i < w.size(); ++i) {
			out << w[i] << " ";
		}
		out << "\n";
		return out;
	}

}  //namespace epl end

#endif