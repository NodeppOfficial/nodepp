/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TYPE
#define NODEPP_TYPE

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type {

    struct false_type { static constexpr bool value = false; using type = false_type; };
    struct true_type  { static constexpr bool value = true;  using type = true_type;  };
    
    /*─······································································─*/

    template <bool B, typename T, typename F> struct conditional { using type = T; };
    template <typename T, typename F> struct conditional<false, T, F> { using type = F; };
    
    /*─······································································─*/

    template <bool B, typename T = void> struct enable_if {};
    template <typename T> struct enable_if<true, T> { using type = T; };
    
    /*─······································································─*/

    template <typename T> struct is_array : false_type {};
    template <typename T> struct is_array<T[]> : true_type {};
    template <typename T, ulong N> struct is_array<T[N]> : true_type {};
    
    /*─······································································─*/

    template <typename T, typename U> struct is_same : false_type {};
    template <typename T> struct is_same<T, T> : true_type {};
    
    /*─······································································─*/

    template <typename T, typename U> struct is_different : true_type {};
    template <typename T> struct is_different<T, T> : false_type {};
    
    /*─······································································─*/

    template <typename T> struct is_pointer : false_type {};
    template <typename T> struct is_pointer<T*> : true_type {};
        
    /*─······································································─*/

    template <typename T> struct is_void : false_type {};

    template <> struct is_void<const volatile void> : true_type {};
    template <> struct is_void<volatile void> : true_type {};
    template <> struct is_void<const void> : true_type {};
    template <> struct is_void<void> : true_type {};
    
    /*─······································································─*/

    template <typename T> struct is_null : false_type {};
    template <> struct is_null<null_t> : true_type {};
    
    /*─······································································─*/

    template <typename T, T v> struct integral_constant {
        static constexpr T value = v;

        using value_type = T;
        using type = integral_constant<T, v>;

        constexpr operator value_type()   const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };
    
    /*─······································································─*/

    template <typename T> struct is_character : false_type {};

    template <> struct is_character<char> : true_type {};
    template <> struct is_character<uchar> : true_type {};
    
    /*─······································································─*/

    template <typename T> struct is_integral : false_type {};

    template <> struct is_integral<int> : true_type {};
    template <> struct is_integral<bool> : true_type {};
    template <> struct is_integral<char> : true_type {};
    template <> struct is_integral<long> : true_type {};
    template <> struct is_integral<uint> : true_type {};
    template <> struct is_integral<short> : true_type {};
    template <> struct is_integral<llong> : true_type {};
    template <> struct is_integral<ulong> : true_type {};
    template <> struct is_integral<uchar> : true_type {};
    template <> struct is_integral<ushort> : true_type {};
    template <> struct is_integral<ullong> : true_type {};
    template <> struct is_integral<wchar_t> : true_type {};
    template <> struct is_integral<char16_t> : true_type {};
    template <> struct is_integral<char32_t> : true_type {};

    /*─······································································─*/

    template <typename T> struct is_floating_point : false_type {};
    template <> struct is_floating_point<float> : true_type {};
    template <> struct is_floating_point<double> : true_type {};
    template <> struct is_floating_point<ldouble> : true_type {};
    
    /*─······································································─*/

    template <typename T> struct is_cr : false_type {};
    template <typename T> struct is_cr<const T&> : true_type {};
    template <typename T> struct is_cr<const T&&> : true_type {};
    
    /*─······································································─*/

    template <typename T> struct is_const : false_type {};
    template <typename T> struct is_const<const T> : true_type {};
    
    /*────────────────────────────────────────────────────────────────────────────*/

    template <typename T> struct is_volatile : false_type {};
    template <typename T> struct is_volatile<volatile T> : true_type {};
    
    /*────────────────────────────────────────────────────────────────────────────*/

    template <typename T> struct is_reference : false_type {};
    template <typename T> struct is_reference<T&> : true_type {};
    template <typename T> struct is_reference<T&&> : true_type {};
    
    /*────────────────────────────────────────────────────────────────────────────*/

    template <typename T> struct is_lvalue_reference : false_type {};
    template <typename T> struct is_lvalue_reference<T&> : true_type {};
    
    /*─······································································─*/

    template <typename T> struct is_rvalue_reference : false_type {};
    template <typename T> struct is_rvalue_reference<T&&> : true_type {};
    
    /*─······································································─*/

    template <typename T, ulong N = 0> struct add_extent { typedef T type[N]; };
    template <typename T> struct add_extent<T[], 0> { typedef T type[]; };
    template <typename T, ulong N> struct add_extent<T[], N> { typedef T type[N]; };
    
    /*─······································································─*/

    template <typename T> struct remove_extent { typedef T type; };
    template <typename T, ulong N> struct remove_extent<T[N]> { typedef T type; };
    
    /*─······································································─*/

    template<typename T> struct add_const { using type = const T; };
    template<typename T> struct remove_const { using type = T; };
    template<typename T> struct remove_const<const T> { using type = T; };
    
    /*─······································································─*/

    template<typename T> struct add_volatile { using type = volatile T; };
    template<typename T> struct remove_volatile { using type = T; };
    template<typename T> struct remove_volatile<volatile T> { using type = T; };
    
    /*─······································································─*/

    template<typename T> struct remove_reference     { using type = T; };
    template<typename T> struct remove_reference<T&> { using type = T; };
    template<typename T> struct remove_reference<T&&>{ using type = T; };
    
    /*─······································································─*/

    template<typename T> struct add_pointer { using type = T*; };
    template<typename T> struct remove_pointer { using type = T; };
    template<typename T> struct remove_pointer<T*> { using type = T; };
    template<typename T> struct remove_pointer<T* const> { using type = T; };
    template<typename T> struct remove_pointer<T* volatile> { using type = T; };
    template<typename T> struct remove_pointer<T* const volatile> { using type = T; };
    
    /*─······································································─*/
    
    template<typename T> struct add_cv { using type = const volatile T; };
    template<typename T> struct remove_cv { using type = typename remove_volatile<typename remove_const<T>::type>::type; };

    /*─······································································─*/

    template<typename T> typename remove_reference<T>::type&& set_move(T&& arg){ return static_cast<typename remove_reference<T>::type&&>( arg ); }
    template<typename T> typename remove_reference<T>::type&  set_copy(T&  arg){ return static_cast<typename remove_reference<T>::type&> ( arg ); }
    
    /*─······································································─*/

    template<typename T> struct add_lvalue_reference { using type = T&; };
    template<typename T> struct add_rvalue_reference { using type = T&&; };
    template<typename T> typename add_rvalue_reference<T>::type add_rvalue_reference_fn(T&& t) { return static_cast<typename add_rvalue_reference<T>::type>(t); }
    template<typename T> struct add_reference : public conditional<is_lvalue_reference<T>::value, add_lvalue_reference<typename remove_reference<T>::type>, add_rvalue_reference<typename remove_reference<T>::type>>::type {};
    
    /*─······································································─*/

    template<typename T> void swap( T& a, T& b ) noexcept { T temp = a; a = b; b = temp; }

    /*─······································································─*/

    template <typename T> struct make_unsigned { using type = T; };

    template <> struct make_unsigned<int>                { using type = unsigned int;       };
    template <> struct make_unsigned<char>               { using type = unsigned char;      };
    template <> struct make_unsigned<long>               { using type = unsigned long;      };
    template <> struct make_unsigned<short>              { using type = unsigned short;     };
    template <> struct make_unsigned<long long>          { using type = unsigned long long; };

    /*─······································································─*/

    template <typename T> struct is_unsigned : false_type {};

    template <> struct is_unsigned<unsigned char>      : true_type {};
    template <> struct is_unsigned<unsigned short>     : true_type {};
    template <> struct is_unsigned<unsigned int>       : true_type {};
    template <> struct is_unsigned<unsigned long>      : true_type {};
    template <> struct is_unsigned<unsigned long long> : true_type {};

    /*─······································································─*/

    template<typename T1, typename T2, typename... Ts> struct max {
        using larger_type = typename conditional< (sizeof(T1) > sizeof(T2)), T1, T2>::type;
        using type = typename max<larger_type, Ts...>::type;
    };

    template<typename T1, typename T2> struct max< T1, T2 > {
        static constexpr bool t1_greater = sizeof(T1) > sizeof(T2);
        using type = typename conditional<t1_greater, T1, T2>::type;
    };

    /*─······································································─*/

    template<typename T1, typename T2, typename... Ts> struct min {
        using larger_type = typename conditional<sizeof(T1) < sizeof(T2), T1, T2>::type;
        using type = typename min<larger_type, Ts...>::type;
    };

    template<typename T1, typename T2> struct min< T1, T2 > {
        static constexpr bool t1_greater = sizeof(T1) < sizeof(T2);
        using type = typename conditional<t1_greater, T1, T2>::type;
    };

    /*─······································································─*/
    
    template< typename... Ts > struct get_size {
        static constexpr ulong value = sizeof...(Ts);
    };

    /*─······································································─*/

    template <typename T>
    struct is_member_pointer : false_type {};

    template <typename T, typename U>
    struct is_member_pointer<T U::*> : true_type {};

    template <typename T, typename U>
    struct is_member_pointer<T (U::*)()> : true_type {};

    /*─······································································─*/
    
    template< int V, typename... Ts > struct is_greater_than {
        static constexpr bool value = sizeof...(Ts) > V;
    };
    
    template< int V, typename... Ts > struct is_lower_than {
        static constexpr bool value = sizeof...(Ts) < V;
    };
    
    template< int V, typename... Ts > struct is_equal_to {
        static constexpr bool value = sizeof...(Ts) == V;
    };

    /*─······································································─*/

    /*
    template<typename T, typename V> struct is_trivially_assignable {
        static constexpr bool value = __is_trivially_assignable(T,V);
    };

    template<typename T> struct is_trivially_constructible {
        static constexpr bool value = __is_trivially_constructible(T);
    };

    template<typename T> struct is_trivially_destructible {
        static constexpr bool value = __is_trivially_destructible(T);
    };

    template<typename T> struct is_virtually_destructible {
        static constexpr bool value = __has_virtual_destructor(T);
    };

    template<typename T> struct is_trivially_copiable {
        static constexpr bool value = __is_trivially_copyable(T);
    };
    */

    /*─······································································─*/

    template< typename T, typename V > struct is_base_of {
        static constexpr bool value = __is_base_of( T, V );
    };

    template<typename T> struct is_polymorphic {
        static constexpr bool value = __is_polymorphic(T);
    };

    template<typename T> struct is_abstract {
        static constexpr bool value = __is_abstract(T);
    };

    template<typename T> struct is_class {
        static constexpr bool value = __is_class(T);
    };

    template<typename T> struct is_empty {
        static constexpr bool value = __is_empty(T);
    };

    template<typename T> struct is_union {
        static constexpr bool value = __is_union(T);
    };

    template<typename T> struct is_enum {
        static constexpr bool value = __is_enum(T);
    };

    template<typename T> struct is_pod {
        static constexpr bool value = __is_pod(T);
    };

    /*─······································································─*/

    template< typename T > 
    class optional { 
    protected: 

        bool has; T value; 

    public:

        optional( const T& val ) noexcept { memcpy( &value, &val, sizeof(T) ); has=true;  }
        
    template< typename V >
        optional( const V&     ) noexcept { /*------------------------------*/ has=false; }
        optional( /*--------*/ ) noexcept { /*------------------------------*/ has=false; }

        bool has_value() const noexcept { return has; }

        const T* get() const noexcept {
            if( has_value() ){ return &value;  } 
            else /*-------*/ { return nullptr; }
        }

    };

    /*─······································································─*/

    template<typename T, typename U> struct pair { T first; U second; };

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type {

    template<typename T> typename remove_reference<T>::type&& move(T&& arg){ 
      return static_cast<typename remove_reference<T>::type&&>( arg ); 
    }

    template < class A >
    inline int compare( A src_first, A src_last, A dst_first ) {
        while ( src_first != src_last ){
           if (*src_first <*dst_first ){ return -1; }
           if (*src_first >*dst_first ){ return  1; }
           ++src_first; ++dst_first;
        }  return 0;
    }

    template < class A, class B >
    inline void reverse( A src_first, A src_last, B dst_first ) {
        while ( src_first != src_last ) {
          --src_last;
           *dst_first=*src_last;
          ++dst_first;
        }
    }

    template < class A, class B >
    inline void copy( A src_first, A src_last, B dst_first ) {
        while ( src_first != src_last ) {
           *dst_first =*src_first;
          ++src_first;++dst_first;
        }
    }

    template < class A, class B >
    inline void fill( A src_first, A src_last, B value ) {
        while ( src_first != src_last ) {
           *src_first = value;
          ++src_first;
        }
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#include "ptr.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace type {

    template< class T, class V > T* cast( ptr_t<V>& object ){ if( object==nullptr ){ return nullptr; } return static_cast<T*>(object.get()); }
    template< class T, class V > T* cast(       V*  object ){ if( object==nullptr ){ return nullptr; } return static_cast<T*>(object); }
    template< class T, class V > T  cast(       V   object ){ return static_cast<T>( object ); }

    template<class T> ptr_t<T>      bind( ptr_t<T>& object ){ if( object==nullptr ){ return nullptr; } return    object.copy(); }
    template<class T> ptr_t<T>      bind(       T*  object ){ if( object==nullptr ){ return nullptr; } return new T( *object ); }
    template<class T> ptr_t<T>      bind(       T   object ){ return new T( object ); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
